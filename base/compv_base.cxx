/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_base.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"
#include "compv/base/time/compv_time.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_exp.h"
#include "compv/base/math/compv_math_activation_functions.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/image/compv_image_decoder.h"
#include "compv/base/drawing/compv_window_registry.h"
#include "compv/base/android/compv_android_native_activity.h"
#include "compv/base/compv_jni.h"

#if COMPV_HAVE_INTEL_IPP
#	include <ipp.h>
#endif

#define COMPV_THIS_CLASSNAME	"CompVBase"

COMPV_NAMESPACE_BEGIN()

bool CompVBase::s_bInitialized = false;
bool CompVBase::s_bInitializing = false;
#if COMPV_OS_WINDOWS && !COMPV_OS_WINDOWS_RT
/* https://msdn.microsoft.com/en-us/library/windows/desktop/ms724834%28v=vs.85%29.aspx
Version Number    Description
6.2				  Windows 8 / Windows Server 2012
6.1               Windows 7     / Windows 2008 R2
6.0               Windows Vista / Windows 2008
5.2               Windows 2003
5.1               Windows XP
5.0               Windows 2000
*/
DWORD CompVBase::s_dwMajorVersion = -1;
DWORD CompVBase::s_dwMinorVersion = -1;
#endif
#if COMPV_OS_ANDROID
std::string CompVBase::s_strCPU_ABI = "";
std::string CompVBase::s_strMODEL = "";
std::string CompVBase::s_strHARDWARE = "";
std::string CompVBase::s_strANDROID_ID = "";
int CompVBase::s_intSDK_INT = 0;
#endif
bool CompVBase::s_bTesting = false;

CompVBase::CompVBase()
{

}

CompVBase::~CompVBase()
{

}

COMPV_ERROR_CODE CompVBase::init(int numThreads COMPV_DEFAULT(-1))
{
    if (s_bInitialized || s_bInitializing) {
        return COMPV_ERROR_CODE_S_OK;
    }

    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
#if COMPV_OS_ANDROID
    struct android_app* androidApp = AndroidApp_get();
    JavaVM* jVM = androidApp ? (androidApp->activity ? androidApp->activity->vm : NULL) : NULL;
#endif /* COMPV_OS_ANDROID */
    s_bInitializing = true;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing [base] modules (v %s, nt %d)...", COMPV_VERSION_STRING, numThreads);

    // Make sure sizeof(compv_scalar_t) is correct
#if COMPV_ASM || COMPV_INTRINSIC
    if (sizeof(compv_scalar_t) != sizeof(void*)) {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sizeof(compv_scalar_t)= #%zu not equal to sizeof(void*)= #%zu", sizeof(compv_scalar_t), sizeof(void*));
        return COMPV_ERROR_CODE_E_SYSTEM;
    }
    // https://en.wikipedia.org/wiki/Single-precision_floating-point_format
    if (sizeof(compv_float32_t) != 4) {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sizeof(compv_float32_t)= #%zu not equal to 4", sizeof(compv_float32_t));
        return COMPV_ERROR_CODE_E_SYSTEM;
    }
    // https://en.wikipedia.org/wiki/Double-precision_floating-point_format
    if (sizeof(compv_float64_t) != 8) {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sizeof(compv_float64_t)= #%zu not equal to 8", sizeof(compv_float64_t));
        return COMPV_ERROR_CODE_E_SYSTEM;
    }
#endif
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "sizeof(compv_scalar_t)= #%zu", sizeof(compv_scalar_t));
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "sizeof(float)= #%zu", sizeof(float));

    /* Windows version */
#if COMPV_OS_WINDOWS && !COMPV_OS_WINDOWS_RT
    // COM initialization
#	if !COMPV_OS_WINDOWS_CE && 0 // Call 'CoUninitialize' is you remove the '& 0'
    COMPV_CHECK_EXP_BAIL(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)), (err_ = COMPV_ERROR_CODE_E_SYSTEM), "CoInitializeEx failed");
#	endif

    // Timers accuracy
    COMPV_CHECK_EXP_BAIL(timeBeginPeriod(1) != 0, (err_ = COMPV_ERROR_CODE_E_SYSTEM));

    // Get OS version
#if defined(__INTEL_COMPILER)
	COMPV_VS_DISABLE_WARNINGS_BEGIN(1478) // GetVersionEx is deprecated
#endif
    if (s_dwMajorVersion == -1 || s_dwMinorVersion == -1) {
        OSVERSIONINFO osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        COMPV_CHECK_EXP_BAIL(GetVersionEx(&osvi) != TRUE, (err_ = COMPV_ERROR_CODE_E_SYSTEM));
        s_dwMajorVersion = osvi.dwMajorVersion;
        s_dwMinorVersion = osvi.dwMinorVersion;
#	if COMPV_OS_WINDOWS_CE
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Windows dwMajorVersion=%ld, dwMinorVersion=%ld\n", s_dwMajorVersion, s_dwMinorVersion);
#	else
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Windows dwMajorVersion=%ld, dwMinorVersion=%ld", s_dwMajorVersion, s_dwMinorVersion);
#	endif
    }
#endif
#if defined(__INTEL_COMPILER)
	COMPV_VS_DISABLE_WARNINGS_END()
#endif

    // rand()
    srand(static_cast<unsigned int>(CompVTime::nowMillis()));

    /* Make sure heap debugging is disabled (release mode only) */
#if COMPV_OS_WINDOWS && defined(_MSC_VER) && defined(NDEBUG)
    if (IsDebuggerPresent()) {
        // TODO(dmi): Looks like this feature is OFF (by default) on VS2015
        DWORD size = GetEnvironmentVariable(TEXT("_NO_DEBUG_HEAP"), NULL, 0);
        bool bHeapDebuggingDisabled = false;
        if (size) {
            TCHAR* _NO_DEBUG_HEAP = (TCHAR*)CompVMem::malloc(size * sizeof(TCHAR));
            if (_NO_DEBUG_HEAP) {
                size = GetEnvironmentVariable(TEXT("_NO_DEBUG_HEAP"), _NO_DEBUG_HEAP, size);
                if (size) {
                    bHeapDebuggingDisabled = (_NO_DEBUG_HEAP[0] == TEXT('1'));
                }
                CompVMem::free((void**)&_NO_DEBUG_HEAP);
            }
        }
        if (!bHeapDebuggingDisabled) {
            COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "/!\\ Heap debugging enabled on release mode while running your app from Visual Studio. You may experiment performance issues.\n"
                             "Consider disabling this feature: Configuration Properties->Debugging->Environment: _NO_DEBUG_HEAP=1\n"
                             "Must be set on the app (executable) itself.");
        }
    }
#endif

    /* Print Android system info */
#if COMPV_OS_ANDROID
    // Static API version used to buid the code
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "[Base] module: android static API version: %d", __ANDROID_API__);
    // Runtime API version used on the host device
    if (jVM) {
		COMPV_CHECK_CODE_RETURN(CompVBase::initAndroid(jVM));
    }
    if (androidApp && androidApp->activity) {
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "sdkVersion: %d", androidApp->activity->sdkVersion); // can also use 'AConfiguration_getSdkVersion'
    }
#endif /* COMPV_OS_ANDROID */

    /* Window registery */
    COMPV_CHECK_CODE_BAIL(err_ = CompVWindowRegistry::init());

    /* Image handlers initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::init());

    /* CPU features initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVCpu::init());
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU features: %s", CompVCpu::flagsAsString(CompVCpu::getFlags()).c_str());
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU cores: online=#%zu, conf=#%zu", CompVCpu::coresCount(), CompVCpu::coresCountConf());
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU cache1: line size: #%zuB, size :#%zuKB", CompVCpu::cache1LineSize(), CompVCpu::cache1Size() >> 10);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU Phys RAM size: #%zuGB", (CompVCpu::physMemSize() >> 20));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU endianness: %s", CompVCpu::isBigEndian() ? "BIG" : "LITTLE");
#if COMPV_ARCH_X86
    // even if we are on X64 CPU it's possible that we're running a 32-bit binary
#	if COMPV_ARCH_X64
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Binary type: X86_64");
#	else
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Binary type: X86_32");
    if (CompVCpu::isSupported(kCpuFlagX64)) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Using X86_32 binaries on X86_64 machine, you're missing many optimizations. Sad!!");
    }
#	endif
#elif COMPV_ARCH_ARM
#	if !COMPV_OS_ANDROID && !COMPV_OS_PI && !COMPV_OS_IPHONE && !COMPV_OS_JETSON
	// COMPV_OS_PI macro is set when cross-compiling using cmake
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Your ARM OS is neither Android nor Raspberry Pi nor iOS nor NVIDIA Jetson. You may miss some optimizations. Let us know what's your OS.");
#	endif
#	if COMPV_ARCH_ARM64
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Binary type: AArch64");
#	else
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Binary type: AArch32");
	if (CompVCpu::isSupported(kCpuFlagARM64)) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Using AArch32 binaries on AArch64 machine, you're missing many optimizations. Sad!!");
	}
#	endif /*!COMPV_ARCH_ARM64*/
#endif
#if COMPV_INTRINSIC
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intrinsic enabled");
#endif
#if COMPV_ASM
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Assembler enabled");
#else
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Assembler disabled, you're missing many optimizations. Sad!!");
#endif
#if defined(__INTEL_COMPILER)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Using Intel compiler");
#endif
    // https://msdn.microsoft.com/en-us/library/jj620901.aspx
#if defined(__AVX2__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:AVX2");
#endif
#if defined(__AVX__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:AVX");
#endif
#if defined(__FMA3__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:FMA3");
#endif
#if defined(__SSE__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:SSE");
#endif
#if defined(__SSE2__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:SSE2");
#endif
#if defined(__SSE3__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:SSE3");
#endif
#if defined(__SSSE3__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:SSSE3");
#endif
#if defined(__SSE4_1__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:SSE41");
#endif
#if defined(__SSE4_2__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:SSE42");
#endif
#if defined(__ARM_NEON__)
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with option /arch:NEON");
#endif

#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with debug enabled");
#endif

	/* OS name */
	{
		const char* OS_name =
#if COMPV_OS_ANDROID 
			"Android"
#elif COMPV_OS_PI
			"Raspberry Pi"
#elif COMPV_OS_IPHONE
			"iOS"
#elif COMPV_OS_JETSON
#	if COMPV_HAVE_TFTRT
			"Jetson_TFTRT"
#	else
			"Jetson"
#	endif
#elif COMPV_OS_LINUX
			"Generic Linux"
#elif COMPV_OS_WINDOWS
			"Windows"
#else
			"Unknown"
#endif
			;
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "OS name: %s", OS_name);
	}

    /* Math functions: Must be after CPU initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMathUtils::init());
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Math Fast Trig.: %s", CompVCpu::isMathTrigFastEnabled() ? "true" : "fast");
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Math Fixed Point: %s", CompVCpu::isMathFixedPointEnabled() ? "true" : "fast");

	/* Math exp lut tables */
	COMPV_CHECK_CODE_BAIL(err_ = CompVMathExp::init());

    /* Memory alignment */
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Default alignment: #%d", COMPV_ALIGNV_SIMD_DEFAULT);
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Best alignment: #%d", CompVMem::bestAlignment());

    /* Memory management */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMem::init());
	if (CompVMem::isTbbMallocEnabled()) {
		const size_t physMemSizeInBytes20th = CompVCpu::physMemSize() / 20;
		const size_t heapLimitInBytes = COMPV_MATH_MAX(COMPV_HEAP_LIMIT, physMemSizeInBytes20th);
		COMPV_CHECK_CODE_BAIL(err_ = CompVMem::poolSetHeapLimit(heapLimitInBytes), "Failed to set heap limit");
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Heap limit: #%zuKB (#%zuMB)", (heapLimitInBytes >> 10), (heapLimitInBytes >> 20));
	}

	/* Intel IPP */
#if COMPV_HAVE_INTEL_IPP
	{
		IppStatus status = ippInit();
		const IppLibraryVersion *lib;
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intel IPP enabled");
		if (status != ippStsNoErr) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "ippInit failed with error code: %d", status);
			COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_INTEL_IPP);
		}
		lib = ippGetLibVersion();
		if (lib) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "IPP lib version: %s %s", lib->Name, lib->Version);
		}
	}
#endif /* COMPV_HAVE_INTEL_IPP */

#if 0
    /* Features */
    COMPV_CHECK_CODE_BAIL(err_ = CompVFeature::init());
#endif

#if 0
    /* Matchers */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMatcher::init());
#endif

bail:
    s_bInitialized = COMPV_ERROR_CODE_IS_OK(err_);
    s_bInitializing = false;
    if (s_bInitialized) {
        // The next functions are called here because they recursively call "CompVBase::init()"
        // We call them now because "s_bInitialized" is already set to "true" and this is the way to avoid endless loops

        /* Parallel */
        COMPV_CHECK_CODE_BAIL(err_ = CompVParallel::init(numThreads)); // If error will go back to bail then "s_bInitialized" wil be set to false which means deInit() will be called
    }
    else {
        // cleanup if initialization failed
        CompVParallel::deInit();
    }

    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "[Base] modules initialized");
    return err_;
}

#if COMPV_OS_ANDROID

// Won't work if not called from MainThread
static jobject androidGetContext(JNIEnv* jEnv)
{
	jclass activityThread = jEnv->FindClass("android/app/ActivityThread");
	if (activityThread) {
		jmethodID currentActivityThread = jEnv->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
		if (currentActivityThread) {
			jobject thread = jEnv->CallStaticObjectMethod(activityThread, currentActivityThread);
			if (thread) {
				jmethodID getApplication = jEnv->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
				if (getApplication) {
					return jEnv->CallObjectMethod(thread, getApplication);
				}
			}
		}
	}
	return nullptr;
}

// Init android values
COMPV_ERROR_CODE CompVBase::initAndroid(JavaVM* jVM)
{
	if (!jVM) { // not an issue
		return COMPV_ERROR_CODE_S_OK;
	}

	if (!Build_MODEL().empty()) {
		return COMPV_ERROR_CODE_S_OK; // already initialized
	}

	JNIEnv* jEnv = NULL;
	if (jVM->AttachCurrentThread(&jEnv, NULL) == JNI_OK) {
		COMPV_CHECK_CODE_RETURN(initAndroid(jEnv));
		jVM->DetachCurrentThread();
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBase::initAndroid(JNIEnv* jEnv)
{
	if (!jEnv) { // not an issue
		return COMPV_ERROR_CODE_S_OK;
	}

	if (!Build_MODEL().empty()) {
		return COMPV_ERROR_CODE_S_OK; // already initialized
	}

	jclass clazz_VERSION = jEnv->FindClass("android/os/Build$VERSION");
	if (clazz_VERSION) {
		jfieldID fieldID_SDK_INT = jEnv->GetStaticFieldID(clazz_VERSION, "SDK_INT", "I");
		if (fieldID_SDK_INT) {
			s_intSDK_INT = static_cast<int>(jEnv->GetStaticIntField(clazz_VERSION, fieldID_SDK_INT));
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "android/os/Build$VERSION.SDK_INT: %d", s_intSDK_INT);
		}
	}
	jclass clazz_Build = jEnv->FindClass("android/os/Build");
	if (clazz_Build) {
		jfieldID fieldID_CPU_ABI = jEnv->GetStaticFieldID(clazz_Build, "CPU_ABI", "Ljava/lang/String;");
		jfieldID fieldID_MODEL = jEnv->GetStaticFieldID(clazz_Build, "MODEL", "Ljava/lang/String;");
		jfieldID fieldID_HARDWARE = jEnv->GetStaticFieldID(clazz_Build, "HARDWARE", "Ljava/lang/String;");
		if (fieldID_CPU_ABI) {
			jstring object_CPU_ABI = reinterpret_cast<jstring>(jEnv->GetStaticObjectField(clazz_Build, fieldID_CPU_ABI));
			if (object_CPU_ABI) {
				s_strCPU_ABI = CompVJNI::toString(jEnv, object_CPU_ABI);
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "android/os/Build.CPU_ABI: %s", s_strCPU_ABI.c_str());
			}
		}
		if (fieldID_MODEL) {
			jstring object_MODEL = reinterpret_cast<jstring>(jEnv->GetStaticObjectField(clazz_Build, fieldID_MODEL));
			if (object_MODEL) {
				s_strMODEL = CompVJNI::toString(jEnv, object_MODEL);
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "android/os/Build.MODEL: %s", s_strMODEL.c_str());
			}
		}
		if (fieldID_HARDWARE) {
			jstring object_HARDWARE = reinterpret_cast<jstring>(jEnv->GetStaticObjectField(clazz_Build, fieldID_HARDWARE));
			if (object_HARDWARE) {
				s_strHARDWARE = CompVJNI::toString(jEnv, object_HARDWARE);
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "android/os/Build.HARDWARE: %s", s_strHARDWARE.c_str());
			}
		}
	}

	// Get Context and retrieve 'ANDROID_ID' value
	jobject context = androidGetContext(jEnv);
	COMPV_CHECK_EXP_NOP(!context, COMPV_ERROR_CODE_E_INVALID_STATE, "Null context. You should call the init function from main thread or activity thread");
	if (context) {
		jclass contextKlass = jEnv->GetObjectClass(context);
		if (contextKlass) {
			jmethodID getContentResolverMethodId = jEnv->GetMethodID(contextKlass, "getContentResolver", "()Landroid/content/ContentResolver;");
			if (getContentResolverMethodId) {
				jobject contentResolver = jEnv->CallObjectMethod(context, getContentResolverMethodId);
				if (contentResolver) {
					jclass settingsSecureKlass = jEnv->FindClass("android/provider/Settings$Secure");
					if (settingsSecureKlass) {
						jmethodID getStringMethodId = jEnv->GetStaticMethodID(settingsSecureKlass, "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
						if (getStringMethodId) { // https://developer.android.com/reference/android/provider/Settings.Secure#getString(android.content.ContentResolver,%20java.lang.String)
							jfieldID fieldID_ANDROID_ID = jEnv->GetStaticFieldID(settingsSecureKlass, "ANDROID_ID", "Ljava/lang/String;");
							if (fieldID_ANDROID_ID) {
								jstring object_ANDROID_ID = reinterpret_cast<jstring>(jEnv->GetStaticObjectField(settingsSecureKlass, fieldID_ANDROID_ID));
								if (object_ANDROID_ID) {
									jstring ANDROID_ID = reinterpret_cast<jstring>(jEnv->CallStaticObjectMethod(settingsSecureKlass, getStringMethodId, contentResolver, object_ANDROID_ID));
									if (ANDROID_ID) { // https://developer.android.com/reference/android/provider/Settings.Secure.html#ANDROID_ID
										s_strANDROID_ID = CompVJNI::toString(jEnv, ANDROID_ID);
										COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "AID: %s", "****");
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

#endif /* COMPV_OS_ANDROID */

COMPV_ERROR_CODE CompVBase::deInit()
{
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "DeInitializing base modules (v %s)...", COMPV_VERSION_STRING);

    s_bInitialized = false;
    s_bInitializing = false;

    COMPV_CHECK_CODE_NOP(CompVParallel::deInit());

	COMPV_CHECK_CODE_NOP(CompVWindowRegistry::deInit());

    // TODO(dmi): deInit other modules (not an issue because there is no memory allocation)
	COMPV_CHECK_CODE_NOP(CompVMem::deInit());

	COMPV_CHECK_CODE_NOP(CompVImageDecoder::deInit());

    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Base modules deinitialized");

    return COMPV_ERROR_CODE_S_OK;
}


COMPV_ERROR_CODE CompVBase::setTestingModeEnabled(bool bTesting)
{
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Engine testing mode = %s", bTesting ? "true" : "false");
    s_bTesting = bTesting;
    return COMPV_ERROR_CODE_S_OK;
}

bool CompVBase::isInitialized()
{
    return s_bInitialized;
}

bool CompVBase::isInitializing()
{
    return s_bInitializing;
}

bool CompVBase::isTestingMode()
{
    return s_bTesting;
}

#if COMPV_OS_WINDOWS

bool CompVBase::isWin8OrLater()
{
    COMPV_CHECK_EXP_NOP(s_dwMajorVersion == -1 || s_dwMinorVersion == -1, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    return ((s_dwMajorVersion > 6) || ((s_dwMajorVersion == 6) && (s_dwMinorVersion >= 2)));
}

bool CompVBase::isWin7OrLater()
{
    COMPV_CHECK_EXP_NOP(s_dwMajorVersion == -1 || s_dwMinorVersion == -1, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    return ((s_dwMajorVersion > 6) || ((s_dwMajorVersion == 6) && (s_dwMinorVersion >= 1)));
}

bool CompVBase::isWinVistaOrLater()
{
    COMPV_CHECK_EXP_NOP(s_dwMajorVersion == -1 || s_dwMinorVersion == -1, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    return (s_dwMajorVersion >= 6);
}

bool CompVBase::isWinXPOrLater()
{
    COMPV_CHECK_EXP_NOP(s_dwMajorVersion == -1 || s_dwMinorVersion == -1, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    return ((s_dwMajorVersion > 5) || ((s_dwMajorVersion == 5) && (s_dwMinorVersion >= 1)));
}

#endif /* COMPV_OS_WINDOWS */

COMPV_NAMESPACE_END()
