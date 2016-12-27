/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/camera/android/compv_camera_android.h"
#if COMPV_OS_ANDROID
#include "compv/base/compv_fileutils.h"
#include "compv/camera/android/compv_camera_android_proxy.h"
#include "compv/base/android/compv_android_dexclassloader.h"
#include "compv/base/android/compv_android_native_activity.h"
#include "compv/base/compv_jni.h"
#include "compv/base/image/compv_image.h"

#define COMPV_THIS_CLASSNAME "CompVCameraAndroid"

COMPV_NAMESPACE_BEGIN()

bool CompVCameraAndroid::s_bPluginCopied = false;
bool CompVCameraAndroid::s_bJniInitialized = false;

jclass CompVCameraAndroid::s_classCamera = NULL;
jmethodID CompVCameraAndroid::s_methodConstructor = NULL;
jmethodID CompVCameraAndroid::s_methodCameraStart = NULL;
jmethodID CompVCameraAndroid::s_methodCameraStop = NULL;
jmethodID CompVCameraAndroid::s_methodSetCallbackFunc = NULL;
jmethodID CompVCameraAndroid::s_methodGetNumberOfCameras = NULL;
jmethodID CompVCameraAndroid::s_methodGetCameraInfo = NULL;
jmethodID CompVCameraAndroid::s_methodSetCaps = NULL;

// These next pixel format values must be updated using java reflexion
int CompVCameraAndroid::PixelFormat_NV21 = COMPV_PixelFormat_YCbCr_420_SP;
int CompVCameraAndroid::PixelFormat_YUY2 = COMPV_PixelFormat_YCbCr_422_I;
int CompVCameraAndroid::PixelFormat_NV16 = COMPV_PixelFormat_YCbCr_422_SP;
int CompVCameraAndroid::PixelFormat_RGB565 = COMPV_PixelFormat_RGB_565;
int CompVCameraAndroid::PixelFormat_RGBA = COMPV_PixelFormat_RGBA_8888;
int CompVCameraAndroid::PixelFormat_RGB = COMPV_PixelFormat_RGBA_888;

CompVCameraAndroid::CompVCameraAndroid()
	: CompVCamera()
	, CompVLock()
	, m_bStarted(false)
	, m_eSubTypeNeg(COMPV_SUBTYPE_NONE)
	, m_jobjectCamera(NULL)
{
	m_CapsPref.format = CompVCameraAndroid::PixelFormat_NV21;
}

CompVCameraAndroid::~CompVCameraAndroid()
{
	COMPV_CHECK_CODE_NOP(stop());

	if (m_jobjectCamera) {
		JNIEnv* jEnv = NULL;
		JavaVM* jVM = NULL;
		if (COMPV_ERROR_CODE_IS_OK(CompVCameraAndroid::attachCurrentThread(&jVM, &jEnv))) {
			COMPV_jni_DeleteGlobalRef(jEnv, m_jobjectCamera);
			COMPV_CHECK_CODE_NOP(CompVCameraAndroid::detachCurrentThread(jVM, jEnv));
		}
	}
}

COMPV_ERROR_CODE CompVCameraAndroid::devices(CompVCameraDeviceInfoList& list) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVCameraAndroid>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	COMPV_CHECK_EXP_RETURN(!s_bJniInitialized, COMPV_ERROR_CODE_E_INVALID_STATE, "JNI not initialized");
	CompVCameraDeviceInfoList list_;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	JNIEnv* jEnv = NULL;
	JavaVM* jVM = NULL;	
	jint devicesCount;
	jstring jinfo = NULL;
	std::string strinfo;
	int id, orientation, facing; // facing: 1=front, 0=back
	COMPV_CHECK_CODE_BAIL(err = CompVCameraAndroid::attachCurrentThread(&jVM, &jEnv), "Failed to attach to the JVM");

	devicesCount = jEnv->CallStaticIntMethod(s_classCamera, s_methodGetNumberOfCameras);
	for (jint cameraId = 0; cameraId < devicesCount; ++cameraId) {
		jinfo = reinterpret_cast<jstring>(jEnv->CallStaticObjectMethod(s_classCamera, s_methodGetCameraInfo, cameraId));
		strinfo = CompVJNI::toString(jEnv, jinfo);
		COMPV_CHECK_EXP_BAIL(sscanf(strinfo.c_str(), "%d %d %d", &id, &orientation, &facing) == EOF, COMPV_ERROR_CODE_E_INVALID_CALL, "Failed to parse camera info");
		COMPV_jni_DeleteLocalRef(jEnv, jinfo);
		list_.push_back(CompVCameraDeviceInfo(
			CompVBase::to_string(id),
			facing ? "Front facing" : "Back facing",
			std::string(facing ? "Front facing camera from native plugin" : "Back facing camera from native plugin") + ", orientation:" + CompVBase::to_string(orientation)
		));
	}
	list = list_;

bail:
	COMPV_jni_checkException1(jEnv);
	COMPV_CHECK_CODE_NOP(CompVCameraAndroid::detachCurrentThread(jVM, jEnv), "Failed to detach from the JVM");
	return err;
}

COMPV_ERROR_CODE CompVCameraAndroid::start(const std::string& deviceId COMPV_DEFAULT("")) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVCameraAndroid>(this);
	if (m_bStarted) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%s)", __FUNCTION__, deviceId.c_str());
	COMPV_CHECK_EXP_RETURN(!m_jobjectCamera, COMPV_ERROR_CODE_E_INVALID_STATE, "Null JNI wrapped object");
	COMPV_CHECK_EXP_RETURN(!s_bJniInitialized, COMPV_ERROR_CODE_E_INVALID_STATE, "JNI not initialized");

	// Make sure cameraId is correct or empty
	long cameraId = 0;
	if (!deviceId.empty()) {
		char* placeholder;
		cameraId = strtol(deviceId.c_str(), &placeholder, 10);
		COMPV_CHECK_EXP_RETURN(*placeholder, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Integer number expected as camera id");
	}

	JNIEnv* jEnv = NULL;
	JavaVM* jVM = NULL;
	jboolean bSucceed;
	bool bExcOccured;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// Attach to the JVM
	COMPV_CHECK_CODE_BAIL(err = CompVCameraAndroid::attachCurrentThread(&jVM, &jEnv), "Failed to attach to the JVM");

	// Set Caps
	jEnv->CallVoidMethod(m_jobjectCamera, s_methodSetCaps, 
		static_cast<jint>(m_CapsPref.width),
		static_cast<jint>(m_CapsPref.height),
		static_cast<jint>(m_CapsPref.fps),
		static_cast<jint>(m_CapsPref.format));
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI), "JNI: exception occured on camera 'setCaps' function");
	m_eSubTypeNeg = COMPV_SUBTYPE_NONE; // means you need to query neg caps
	
	// Start the camera
	bSucceed = jEnv->CallBooleanMethod(m_jobjectCamera, s_methodCameraStart, static_cast<jint>(cameraId));
	COMPV_jni_checkException(jEnv, &bExcOccured); // Camera permission issues: 'java.lang.RuntimeException: Fail to connect to camera service'
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI), "JNI: exception occured on camera 'start' function");
	COMPV_CHECK_EXP_BAIL(!bSucceed, (err = COMPV_ERROR_CODE_E_SYSTEM), "JNI: failed to start the camera. Java function returned false.");
	m_bStarted = true;

bail:
	COMPV_jni_checkException1(jEnv);
	COMPV_CHECK_CODE_NOP(CompVCameraAndroid::detachCurrentThread(jVM, jEnv), "Failed to detach from the JVM");
	return err;
}

COMPV_ERROR_CODE CompVCameraAndroid::stop() /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVCameraAndroid>(this);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	JNIEnv* jEnv = NULL;
	JavaVM* jVM = NULL;
	if (m_jobjectCamera && s_bJniInitialized) {
		jboolean bSucceed, bExcOccured ;
		COMPV_CHECK_CODE_BAIL(err = CompVCameraAndroid::attachCurrentThread(&jVM, &jEnv), "Failed to attach to the JVM");
		bSucceed = jEnv->CallBooleanMethod(m_jobjectCamera, s_methodCameraStop);
		COMPV_jni_checkException(jEnv, &bExcOccured);
		COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI), "JNI: exception occured on camera 'stop' function");
		COMPV_CHECK_EXP_BAIL(!bSucceed, (err = COMPV_ERROR_CODE_E_SYSTEM), "JNI: failed to stop the camera. Java function returned false.");
	}
	m_bStarted = false;
bail:
	if (jEnv && jVM) {
		COMPV_jni_checkException1(jEnv);
		COMPV_CHECK_CODE_NOP(CompVCameraAndroid::detachCurrentThread(jVM, jEnv), "Failed to detach from the JVM");
	}
	return err;
}

COMPV_ERROR_CODE CompVCameraAndroid::set(int id, const void* valuePtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVCameraAndroid>(this);
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%d, %p, %zu)", __FUNCTION__, id, valuePtr, valueSize);
	switch (id) {
	case COMPV_CAMERA_CAP_INT_WIDTH: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_CapsPref.width = *reinterpret_cast<const int*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_CAMERA_CAP_INT_HEIGHT: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_CapsPref.height = *reinterpret_cast<const int*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_CAMERA_CAP_INT_FPS: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_CapsPref.fps = *reinterpret_cast<const int*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_CAMERA_CAP_INT_SUBTYPE: {
		COMPV_CHECK_CODE_RETURN(CompVCameraAndroid::convertFormat(static_cast<COMPV_SUBTYPE>(*reinterpret_cast<const int*>(valuePtr)), m_CapsPref.format), "Invalid pixel format");
		return COMPV_ERROR_CODE_S_OK;
	}
	default: {
		COMPV_DEBUG_ERROR("DirectShow camera implementation doesn't support capability id %d", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	}
}

COMPV_ERROR_CODE CompVCameraAndroid::get(int id, const void*& valuePtr, size_t valueSize) /* Overrides(CompVCaps) */
{
	CompVAutoLock<CompVCameraAndroid>(this);
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "CompVCaps::get not implemented for Android camera implementation.");
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCameraAndroid::newObj(CompVCameraAndroidPtrPtr camera)
{
	COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	static CompVCameraAndroidProxyOnPreviewFrameFunc sSetCallbackFuncPtr = CompVCameraAndroid::onPreviewFrame; // keep this useless code as guard: error if the funcptr's signature change
	JNIEnv* jEnv = NULL;
	JavaVM* jVM = NULL;
	jobject jobjectCamera = NULL;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	jboolean bExcOccured;
	
	CompVCameraAndroidPtr camera_;

	// Attach the thread to the JVM
	COMPV_CHECK_CODE_BAIL(err = CompVCameraAndroid::attachCurrentThread(&jVM, &jEnv));

	// Initialize JNI if not already done
	COMPV_CHECK_CODE_BAIL(err = CompVCameraAndroid::initJNI(jEnv));

	// Create CompVCameraAndroid class wrapping the JNI object
	camera_ = new CompVCameraAndroid();
	COMPV_CHECK_EXP_RETURN(!camera_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Out of memory, failed to allocate Android camera object");	

	// Create the JNI wrapped object
	jobjectCamera = jEnv->NewObject(s_classCamera, s_methodConstructor);
	COMPV_CHECK_EXP_BAIL(!jobjectCamera, (err = COMPV_ERROR_CODE_E_JNI));
	camera_->m_jobjectCamera = jEnv->NewGlobalRef(jobjectCamera);
	COMPV_CHECK_EXP_BAIL(!camera_->m_jobjectCamera, (err = COMPV_ERROR_CODE_E_JNI));

	// Set callback function
	jEnv->CallVoidMethod(camera_->m_jobjectCamera, s_methodSetCallbackFunc, reinterpret_cast<jlong>(sSetCallbackFuncPtr), reinterpret_cast<jlong>(*camera_));
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI), "JNI Exception occured on setCallbackFuncPtr(camera, funcPtr)");
	
	*camera = *camera_;

bail:
	COMPV_jni_checkException1(jEnv);
	COMPV_jni_DeleteLocalRef(jEnv, jobjectCamera);
	COMPV_CHECK_CODE_NOP(CompVCameraAndroid::detachCurrentThread(jVM, jEnv));
	return err;
}

COMPV_ERROR_CODE CompVCameraAndroid::initJNI(JNIEnv* jEnv)
{
	if (s_bJniInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%p)", __FUNCTION__, jEnv);
	COMPV_CHECK_EXP_RETURN(!jEnv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	static const std::string kPluginFileName = "plugin_androidcamera.apk"; // the plugin apk should be part of the source code at "compv/thirdparties/apks/plugin_androidcamera.apk"
	static const std::string kNativeLibFileName = "libandroidcamera.so"; // shared library packed in the apk, supported archs: mips64, mips, x86_64, x86, arm64-v8a, armeabi-v7a, armeabi
	static const std::string kClassNameCamera = "org/doubango/java/androidcamera/CompVCamera"; // org.doubango.java.androidcamera.CompVCamera
	jclass classCompVCamera = NULL; // global ref
	jmethodID methodConstructor;
	jmethodID methodCameraStart;
	jmethodID methodCameraStop;
	jmethodID methodSetCallbackFunc;
	jmethodID methodGetNumberOfCameras;
	jmethodID methodGetCameraInfo;
	jmethodID methodSetCaps;
	jfieldID fieldPixelFormat;
	struct android_app* nativeApp = AndroidApp_get();
	COMPV_DEBUG_INFO("%s called with  nativeAndroidApp = %p", __FUNCTION__, nativeApp); // Debug message used to check if we are running a native or managed application.
	COMPV_CHECK_EXP_RETURN(!CompVAndroidDexClassLoader::isInitialized(), COMPV_ERROR_CODE_E_INVALID_STATE);
	jobject activity = nativeApp ? nativeApp->activity->clazz : NULL; // TODO(dmi): Allow using activity from Java code (retrieved using JNI_OnLoad)
	COMPV_CHECK_EXP_RETURN(!activity, COMPV_ERROR_CODE_E_INVALID_STATE, "No activity associated to the native app");

	static std::string dexPath;
	static std::string optimizedDirectory;
	static std::string librarySearchPath;
	
	// TODO(dmi): Add version number to Android plugins (apk) to avoid unpacking every startup
	// https://github.com/DoubangoTelecom/compv/issues/112

	// Copy the plugin from the assets to a readable folder and unzip the ".dex" and ".so"
	if (!CompVCameraAndroid::s_bPluginCopied) {
		if (!CompVFileUtils::exists(kPluginFileName.c_str())) {
			COMPV_DEBUG_ERROR("Android plugin with file name '%s' not found", kPluginFileName.c_str());
			COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_FILE_NOT_FOUND, "Failed to find plugin file for Android camera");
		}
		COMPV_CHECK_CODE_BAIL(err = CompVAndroidDexClassLoader::moveDexFileFromAssetsToData(jEnv, activity, kPluginFileName, kNativeLibFileName, dexPath, optimizedDirectory, librarySearchPath));
		CompVCameraAndroid::s_bPluginCopied = true;
	}

	// load the class
	COMPV_CHECK_CODE_BAIL(err = CompVAndroidDexClassLoader::loadClass(jEnv, kClassNameCamera, dexPath, optimizedDirectory, librarySearchPath, &classCompVCamera));
	COMPV_CHECK_EXP_BAIL(!classCompVCamera, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to load CompVCamera java class");
	
	/* PixelFormats */
	fieldPixelFormat = jEnv->GetStaticFieldID(classCompVCamera, "PIXEL_FORMAT_NV21", "I");
	COMPV_CHECK_EXP_BAIL(!fieldPixelFormat, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get PIXEL_FORMAT_NV21 field");
	CompVCameraAndroid::PixelFormat_NV21 = static_cast<int>(jEnv->GetStaticIntField(classCompVCamera, fieldPixelFormat));
	fieldPixelFormat = jEnv->GetStaticFieldID(classCompVCamera, "PIXEL_FORMAT_YUY2", "I");
	COMPV_CHECK_EXP_BAIL(!fieldPixelFormat, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get PIXEL_FORMAT_YUY2 field");
	CompVCameraAndroid::PixelFormat_YUY2 = static_cast<int>(jEnv->GetStaticIntField(classCompVCamera, fieldPixelFormat));
	fieldPixelFormat = jEnv->GetStaticFieldID(classCompVCamera, "PIXEL_FORMAT_NV16", "I");
	COMPV_CHECK_EXP_BAIL(!fieldPixelFormat, (err = COMPV_ERROR_CODE_E_JNI), "JNI: Failed to get PIXEL_FORMAT_NV16 field");
	CompVCameraAndroid::PixelFormat_NV16 = static_cast<int>(jEnv->GetStaticIntField(classCompVCamera, fieldPixelFormat));
	fieldPixelFormat = jEnv->GetStaticFieldID(classCompVCamera, "PIXEL_FORMAT_RGB565", "I");
	COMPV_CHECK_EXP_BAIL(!fieldPixelFormat, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get PIXEL_FORMAT_RGB565 field");
	CompVCameraAndroid::PixelFormat_RGB565 = static_cast<int>(jEnv->GetStaticIntField(classCompVCamera, fieldPixelFormat));
	fieldPixelFormat = jEnv->GetStaticFieldID(classCompVCamera, "PIXEL_FORMAT_RGBA", "I");
	COMPV_CHECK_EXP_BAIL(!fieldPixelFormat, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get PIXEL_FORMAT_RGBA field");
	CompVCameraAndroid::PixelFormat_RGBA = static_cast<int>(jEnv->GetStaticIntField(classCompVCamera, fieldPixelFormat));
	fieldPixelFormat = jEnv->GetStaticFieldID(classCompVCamera, "PIXEL_FORMAT_RGB", "I");
	COMPV_CHECK_EXP_BAIL(!fieldPixelFormat, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get PIXEL_FORMAT_RGB field");
	CompVCameraAndroid::PixelFormat_RGB = static_cast<int>(jEnv->GetStaticIntField(classCompVCamera, fieldPixelFormat));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Static PixelFormat values: %s->%d, %s->%d, %s->%d, %s->%d, %s->%d, %s->%d",
		CompVCameraAndroid::formatName(CompVCameraAndroid::PixelFormat_NV21).c_str(), CompVCameraAndroid::PixelFormat_NV21,
		CompVCameraAndroid::formatName(CompVCameraAndroid::PixelFormat_YUY2).c_str(), CompVCameraAndroid::PixelFormat_YUY2,
		CompVCameraAndroid::formatName(CompVCameraAndroid::PixelFormat_NV16).c_str(), CompVCameraAndroid::PixelFormat_NV16,
		CompVCameraAndroid::formatName(CompVCameraAndroid::PixelFormat_RGB565).c_str(), CompVCameraAndroid::PixelFormat_RGB565,
		CompVCameraAndroid::formatName(CompVCameraAndroid::PixelFormat_RGBA).c_str(), CompVCameraAndroid::PixelFormat_RGBA,
		CompVCameraAndroid::formatName(CompVCameraAndroid::PixelFormat_RGB).c_str(), CompVCameraAndroid::PixelFormat_RGB);

	/* Methods */
	methodConstructor = jEnv->GetMethodID(classCompVCamera, "<init>", "()V");
	COMPV_CHECK_EXP_BAIL(!methodConstructor, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get '<init>' method from CompVCamera java class");
	methodCameraStart = jEnv->GetMethodID(classCompVCamera, "start", "(I)Z"); // boolean start(int)
	COMPV_CHECK_EXP_BAIL(!methodCameraStart, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get 'start' method from CompVCamera java class");
	methodCameraStop = jEnv->GetMethodID(classCompVCamera, "stop", "()Z"); // boolean start(int)
	COMPV_CHECK_EXP_BAIL(!methodCameraStop, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get 'stop' method from CompVCamera java class");
	methodSetCallbackFunc = jEnv->GetMethodID(classCompVCamera, "setCallbackFunc", "(JJ)V"); // void setCallbackFunc(long funcptr, long userData)
	COMPV_CHECK_EXP_BAIL(!methodSetCallbackFunc, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get 'setCallbackFunc' method from CompVCamera java class");
	methodGetNumberOfCameras = jEnv->GetStaticMethodID(classCompVCamera, "getNumberOfCameras", "()I"); //  static int getNumberOfCameras();
	COMPV_CHECK_EXP_BAIL(!methodGetNumberOfCameras, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get 'getNumberOfCameras' method from CompVCamera java class");
	methodGetCameraInfo = jEnv->GetStaticMethodID(classCompVCamera, "getCameraInfo", "(I)Ljava/lang/String;"); //  static String getCameraInfo(int);
	COMPV_CHECK_EXP_BAIL(!methodGetCameraInfo, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get 'getCameraInfo' method from CompVCamera java class");
	methodSetCaps = jEnv->GetMethodID(classCompVCamera, "setCaps", "(IIII)V"); // void setCaps(int width, int height, int fps, int format);
	COMPV_CHECK_EXP_BAIL(!methodSetCaps, (err = COMPV_ERROR_CODE_E_JNI), "JNI: failed to get 'setCaps' method from CompVCamera java class");

	s_classCamera = classCompVCamera, classCompVCamera = NULL; // 'classCompVCamera' already global ref
	s_methodConstructor = methodConstructor;
	s_methodCameraStart = methodCameraStart;
	s_methodCameraStop = methodCameraStop;
	s_methodSetCallbackFunc = methodSetCallbackFunc;
	s_methodGetNumberOfCameras = methodGetNumberOfCameras;
	s_methodGetCameraInfo = methodGetCameraInfo;
	s_methodSetCaps = methodSetCaps;

	s_bJniInitialized = true;

bail:
	COMPV_jni_checkException1(jEnv);
	COMPV_jni_DeleteGlobalRef(jEnv, classCompVCamera);
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(deInitJNI(jEnv));
	}
	return err;
}

COMPV_ERROR_CODE CompVCameraAndroid::deInitJNI(JNIEnv* jEnv)
{
	COMPV_CHECK_EXP_RETURN(!jEnv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_jni_DeleteGlobalRef(jEnv, s_classCamera);
	s_methodConstructor = NULL;
	s_methodCameraStart = NULL;
	s_methodCameraStop = NULL;
	s_methodSetCallbackFunc = NULL;
	s_methodGetNumberOfCameras = NULL;
	s_methodGetCameraInfo = NULL;
	s_methodSetCaps = NULL;

	s_bJniInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

const std::string CompVCameraAndroid::formatName(int format)
{
	if (format == CompVCameraAndroid::PixelFormat_NV21) return "NV21";
	else if (format == CompVCameraAndroid::PixelFormat_YUY2) return "YUY2";
	else if (format == CompVCameraAndroid::PixelFormat_RGB565) return "RGB565";
	else if (format == CompVCameraAndroid::PixelFormat_RGBA) return "RGBA";
	else if (format == CompVCameraAndroid::PixelFormat_RGB) return "RGB";
	else if (format == CompVCameraAndroid::PixelFormat_NV16) return "NV16";
	else return CompVBase::to_string(format);
}

COMPV_ERROR_CODE CompVCameraAndroid::convertFormat(const COMPV_SUBTYPE& subType, int& format)
{
	switch (subType) {
	case COMPV_SUBTYPE_PIXELS_NV21: format = CompVCameraAndroid::PixelFormat_NV21; return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_YUY2: format = CompVCameraAndroid::PixelFormat_YUY2; return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_RGB565: format = CompVCameraAndroid::PixelFormat_RGB565; return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_RGBA32: format = CompVCameraAndroid::PixelFormat_RGBA; return COMPV_ERROR_CODE_S_OK;
	case COMPV_SUBTYPE_PIXELS_RGB24: format = CompVCameraAndroid::PixelFormat_RGB; return COMPV_ERROR_CODE_S_OK;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%d isn't a valid format for Android camera implementation", subType);
		return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
	}
}

COMPV_ERROR_CODE CompVCameraAndroid::convertFormat(const int& format, COMPV_SUBTYPE& subtype)
{
	if (format == CompVCameraAndroid::PixelFormat_NV21) subtype = COMPV_SUBTYPE_PIXELS_NV21;
	else if (format == CompVCameraAndroid::PixelFormat_YUY2) subtype = COMPV_SUBTYPE_PIXELS_YUY2;
	else if (format == CompVCameraAndroid::PixelFormat_RGB565) subtype = COMPV_SUBTYPE_PIXELS_RGB565;
	else if (format == CompVCameraAndroid::PixelFormat_RGBA) subtype = COMPV_SUBTYPE_PIXELS_RGBA32;
	else if (format == CompVCameraAndroid::PixelFormat_RGB) subtype = COMPV_SUBTYPE_PIXELS_RGB24;
	else {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%d isn't a valid format for Android camera implementation", format);
		return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCameraAndroid::attachCurrentThread(JavaVM** jVM, JNIEnv** jEnv)
{
	*jEnv = NULL;
	*jVM = NULL;
	struct android_app* nativeApp = AndroidApp_get();
	*jVM = nativeApp ? (nativeApp->activity ? nativeApp->activity->vm : NULL) : NULL; // TODO(dmi): Allow using vm from java code (retrieved using JNI_OnLoad)
	COMPV_CHECK_EXP_RETURN(!*jVM, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN((*jVM)->AttachCurrentThread(jEnv, NULL) < 0 || !jEnv, COMPV_ERROR_CODE_E_JNI);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCameraAndroid::detachCurrentThread(JavaVM* jVM, JNIEnv* jEnv)
{
	COMPV_CHECK_EXP_RETURN(!jEnv || !jVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	jEnv->ExceptionClear();
	jVM->DetachCurrentThread();
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVCameraAndroid::onPreviewFrame(const void* frameDataPtr, size_t frameDataSize, size_t frameWidth, size_t frameHeight, int frameFps, int framePixelFormat, const void* userData)
{
	CompVCameraAndroidPtr camera = const_cast<CompVCameraAndroid*>(static_cast<const CompVCameraAndroid*>(userData));
	CompVAutoLock<CompVCameraAndroid> autoLock(*camera);
	CompVCameraListenerPtr listener = camera->listener();
	if (!listener) {
		return true;
	}
	if (!camera->m_bStarted) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Android camera not started");
		return false;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// Update 'm_eSubTypeNeg' when pixel format is undefined (means first time)
	// 'm_eSubTypeNeg' must be set to none before starting the camera
	if (camera->m_eSubTypeNeg == COMPV_SUBTYPE_NONE) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%p, %zu, %zu, %zu, %d, %s, %p): First time... initializing", __FUNCTION__, frameDataPtr, frameDataSize, frameWidth, frameHeight, frameFps, CompVCameraAndroid::formatName(framePixelFormat).c_str(), userData);
		COMPV_CHECK_CODE_BAIL(err = CompVCameraAndroid::convertFormat(framePixelFormat, camera->m_eSubTypeNeg), "Query neg caps failed");
	}

	COMPV_CHECK_CODE_BAIL(err = CompVImage::wrap(camera->m_eSubTypeNeg, frameDataPtr, frameWidth, frameHeight, frameWidth, &camera->m_ptrImageCB));
	COMPV_CHECK_CODE_BAIL(err = listener->onNewFrame(camera->m_ptrImageCB));

bail:
	return COMPV_ERROR_CODE_IS_OK(err);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID */
