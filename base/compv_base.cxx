/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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
#endif
bool CompVBase::s_bTesting = false;

CompVBase::CompVBase()
{

}

CompVBase::~CompVBase()
{

}

COMPV_ERROR_CODE CompVBase::init(int32_t numThreads COMPV_DEFAULT(-1))
{
    if (s_bInitialized || s_bInitializing) {
        return COMPV_ERROR_CODE_S_OK;
    }

    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
#if COMPV_OS_ANDROID
    struct android_app* androidApp = AndroidApp_get();
    JavaVM* jVM = androidApp ? (androidApp->activity ? androidApp->activity->vm : NULL) : NULL;
#endif
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
    srand((unsigned int)CompVTime::getNowMills());

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
        JNIEnv* jEnv = NULL;
        if (jVM->AttachCurrentThread(&jEnv, NULL) == JNI_OK) {
            jclass clazz_VERSION = jEnv->FindClass("android/os/Build$VERSION");
            if (clazz_VERSION) {
                jfieldID fieldID_SDK_INT = jEnv->GetStaticFieldID(clazz_VERSION, "SDK_INT", "I");
                if (fieldID_SDK_INT) {
                    jint SDK_INT = jEnv->GetStaticIntField(clazz_VERSION, fieldID_SDK_INT);
                    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "android/os/Build$VERSION.SDK_INT: %d", static_cast<int>(SDK_INT));
                }
            }
            jclass clazz_Build = jEnv->FindClass("android/os/Build");
            if (clazz_Build) {
                jfieldID fieldID_CPU_ABI = jEnv->GetStaticFieldID(clazz_Build, "CPU_ABI", "Ljava/lang/String;");
                if (fieldID_CPU_ABI) {
                    jstring object_CPU_ABI = reinterpret_cast<jstring>(jEnv->GetStaticObjectField(clazz_Build, fieldID_CPU_ABI));
                    if (object_CPU_ABI) {
						s_strCPU_ABI = CompVJNI::toString(jEnv, object_CPU_ABI);
                        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "android/os/Build.CPU_ABI: %s", s_strCPU_ABI.c_str());
                    }
                }
            }
            jVM->DetachCurrentThread();
        }
    }
    else {
        COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "sdkVersion: %d", androidApp->activity->sdkVersion); // can also use 'AConfiguration_getSdkVersion'
    }
#endif

    /* Window registery */
    COMPV_CHECK_CODE_BAIL(err_ = CompVWindowRegistry::init());

    /* Image handlers initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::init());

    /* CPU features initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVCpu::init());
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU features: %s", CompVCpu::flagsAsString(CompVCpu::getFlags()));
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU cores: #%d", CompVCpu::coresCount());
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU cache1: line size: #%dB, size :#%dKB", CompVCpu::cache1LineSize(), CompVCpu::cache1Size() >> 10);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CPU endianness: %s", CompVCpu::isBigEndian() ? "BIG" : "LITTLE");
#if COMPV_ARCH_X86
    // even if we are on X64 CPU it's possible that we're running a 32-bit binary
#	if COMPV_ARCH_X64
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Binary type: X86_64");
#	else
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Binary type: X86_32");
    if (CompVCpu::isSupported(kCpuFlagX64)) {
		// TODO(dmi): show same message for ARM code running on ARM64 device
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Using X86_32 binaries on X86_64 machine, you're missing many optimizations. Sad!!");
    }
#	endif
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

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Code built with debug enabled");
#endif

    /* Math functions: Must be after CPU initialization */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMathUtils::init());
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Math Fast Trig.: %s", CompVCpu::isMathTrigFast() ? "true" : "fast");
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Math Fixed Point: %s", CompVCpu::isMathFixedPoint() ? "true" : "fast");

    /* Memory alignment */
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Default alignment: #%d", COMPV_SIMD_ALIGNV_DEFAULT);
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Best alignment: #%d", CompVMem::bestAlignment());

    /* Memory management */
    COMPV_CHECK_CODE_BAIL(err_ = CompVMem::init());

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
#endif

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

    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Base modules initialized");
    return err_;
}

COMPV_ERROR_CODE CompVBase::deInit()
{
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "DeInitializing base modules (v %s)...", COMPV_VERSION_STRING);

    s_bInitialized = false;
    s_bInitializing = false;

    CompVParallel::deInit();

    CompVWindowRegistry::deInit();

    // TODO(dmi): deInit other modules (not an issue because there is no memory allocation)
    CompVMem::deInit();

    CompVImageDecoder::deInit();

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
