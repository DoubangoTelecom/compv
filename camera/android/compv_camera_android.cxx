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

COMPV_NAMESPACE_BEGIN()

bool CompVAndroidCamera::s_bPluginCopied = false;

CompVAndroidCamera::CompVAndroidCamera()
	: CompVCamera()
	, CompVLock()
	, m_bStarted(false)
	, m_jobjectCamera(NULL)
	, m_methodCameraStart(NULL)
	, m_methodSetCallbackFunc(NULL)
{

}

CompVAndroidCamera::~CompVAndroidCamera()
{
	COMPV_CHECK_CODE_NOP(stop());

	if (m_jobjectCamera) {
		JNIEnv* jEnv = NULL;
		JavaVM* jVM = NULL;
		if (COMPV_ERROR_CODE_IS_OK(CompVAndroidCamera::attachCurrentThread(&jVM, &jEnv))) {
			COMPV_jni_DeleteGlobalRef(jEnv, m_jobjectCamera);
			COMPV_CHECK_CODE_NOP(CompVAndroidCamera::detachCurrentThread(jVM, jEnv));
		}
	}
}

COMPV_ERROR_CODE CompVAndroidCamera::devices(CompVCameraDeviceInfoList& list) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVAndroidCamera>(this);
	list.clear();
	// FIXME(dmi): not correct, get the list from 'm_jobjectCamera'
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	list.push_back(CompVCameraDeviceInfo("0", "back", "back camera"));
	list.push_back(CompVCameraDeviceInfo("1", "front", "front camera"));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAndroidCamera::start(const std::string& deviceId COMPV_DEFAULT("")) /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVAndroidCamera>(this);
	if (m_bStarted) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!m_jobjectCamera, COMPV_ERROR_CODE_E_INVALID_STATE);
	// FIXME(dmi): not correct, deviceId ignored
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();

	JNIEnv* jEnv = NULL;
	JavaVM* jVM = NULL;
	jboolean bSucceed;
	bool bExcOccured;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	COMPV_CHECK_CODE_BAIL(err = CompVAndroidCamera::attachCurrentThread(&jVM, &jEnv));
	bSucceed = jEnv->CallBooleanMethod(m_jobjectCamera, m_methodCameraStart);
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
	COMPV_CHECK_EXP_BAIL(!bSucceed, (err = COMPV_ERROR_CODE_E_SYSTEM));
	m_bStarted = true;

bail:
	COMPV_CHECK_CODE_NOP(CompVAndroidCamera::detachCurrentThread(jVM, jEnv));
	return err;
}

COMPV_ERROR_CODE CompVAndroidCamera::stop() /* Overrides(CompVCamera) */
{
	CompVAutoLock<CompVAndroidCamera>(this);
	// FIXME(dmi): not implemented
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAndroidCamera::newObj(CompVAndroidCameraPtrPtr camera)
{
	COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	static CompVCameraAndroidProxyOnPreviewFrameFunc sSetCallbackFuncPtr = CompVAndroidCamera::onPreviewFrame;
	JNIEnv* jEnv = NULL;
	JavaVM* jVM = NULL;
	jclass jclassCompVCamera = NULL;
	jboolean bExcOccured;
	struct android_app* nativeApp = AndroidApp_get();
	COMPV_DEBUG_INFO("CompVAndroidCamera::newObj called with  nativeAndroidApp = %p", nativeApp); // Debug message used to check if we are running a native or managed application.
	COMPV_CHECK_EXP_RETURN(!CompVAndroidDexClassLoader::isInitialized(), COMPV_ERROR_CODE_E_INVALID_STATE);
	jobject activity = nativeApp ? nativeApp->activity->clazz : NULL; // TODO(dmi): Allow using activity from Java code (retrieved using JNI_OnLoad)
	COMPV_CHECK_EXP_RETURN(!activity, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	// FIXME(dmi): change the file name (app-debug.apk -> plugin_androidcamera.apk) and sign the package
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	static const std::string kPluginFileName = "app-debug.apk";
	static const std::string kNativeLibFileName = "libandroidcamera.so";
	static const std::string kClassName = "org/doubango/java/androidcamera/CompVCamera"; // org.doubango.java.androidcamera.CompVCamera
	static std::string dexPath;
	static std::string optimizedDirectory;
	static std::string librarySearchPath;
	CompVAndroidCameraPtr camera_;

	// Attach the thread to the JVM
	COMPV_CHECK_CODE_BAIL(err = CompVAndroidCamera::attachCurrentThread(&jVM, &jEnv));

	// Copy the plugin from the assets to a readable folder and unzip the ".dex" and ".so"
	if (!s_bPluginCopied) {
		if (!CompVFileUtils::exists(kPluginFileName.c_str())) {
			COMPV_DEBUG_ERROR("Android plugin with file name '%s' not found", kPluginFileName.c_str());
			COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_FILE_NOT_FOUND);
		}
		COMPV_CHECK_EXP_BAIL(jVM->AttachCurrentThread(&jEnv, NULL) < 0 || !jEnv, (err = COMPV_ERROR_CODE_E_JNI));
		COMPV_CHECK_CODE_BAIL(err = CompVAndroidDexClassLoader::moveDexFileFromAssetsToData(jEnv, activity, kPluginFileName, kNativeLibFileName, dexPath, optimizedDirectory, librarySearchPath));
		s_bPluginCopied = true;
	}

	// Create CompVAndroidCamera class wrapping the JNI object
	camera_ = new CompVAndroidCamera();
	COMPV_CHECK_EXP_RETURN(!camera_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	// Create a camera instance using the Dex Class loader and init methods
	COMPV_CHECK_CODE_BAIL(err = CompVAndroidDexClassLoader::newInstance(jEnv, kClassName, dexPath, optimizedDirectory, librarySearchPath, &camera_->m_jobjectCamera));
	jclassCompVCamera = jEnv->GetObjectClass(camera_->m_jobjectCamera);
	COMPV_CHECK_EXP_BAIL(!jclassCompVCamera, (err = COMPV_ERROR_CODE_E_JNI));
	camera_->m_methodCameraStart = jEnv->GetMethodID(jclassCompVCamera, "start", "()Z"); // boolean start()
	COMPV_CHECK_EXP_BAIL(!camera_->m_methodCameraStart, (err = COMPV_ERROR_CODE_E_JNI));
	camera_->m_methodSetCallbackFunc = jEnv->GetMethodID(jclassCompVCamera, "setCallbackFunc", "(JJ)V"); // void setCallbackFunc(long funcptr, long userData)
	COMPV_CHECK_EXP_BAIL(!camera_->m_methodSetCallbackFunc, (err = COMPV_ERROR_CODE_E_JNI));

	// Set callback function
	jEnv->CallVoidMethod(camera_->m_jobjectCamera, camera_->m_methodSetCallbackFunc, reinterpret_cast<jlong>(sSetCallbackFuncPtr), reinterpret_cast<jlong>(*camera_));
	COMPV_jni_checkException(jEnv, &bExcOccured);
	COMPV_CHECK_EXP_BAIL(bExcOccured, (err = COMPV_ERROR_CODE_E_JNI));
	
	*camera = *camera_;

bail:
	COMPV_jni_checkException1(jEnv);
	COMPV_jni_DeleteLocalRef(jEnv, jclassCompVCamera);
	COMPV_CHECK_CODE_NOP(CompVAndroidCamera::detachCurrentThread(jVM, jEnv));
	return err;
}

COMPV_ERROR_CODE CompVAndroidCamera::attachCurrentThread(JavaVM** jVM, JNIEnv** jEnv)
{
	*jEnv = NULL;
	*jVM = NULL;
	struct android_app* nativeApp = AndroidApp_get();
	*jVM = nativeApp ? (nativeApp->activity ? nativeApp->activity->vm : NULL) : NULL; // TODO(dmi): Allow using vm from java code (retrieved using JNI_OnLoad)
	COMPV_CHECK_EXP_RETURN(!*jVM, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN((*jVM)->AttachCurrentThread(jEnv, NULL) < 0 || !jEnv, COMPV_ERROR_CODE_E_JNI);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAndroidCamera::detachCurrentThread(JavaVM* jVM, JNIEnv* jEnv)
{
	COMPV_CHECK_EXP_RETURN(!jEnv || !jVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	jEnv->ExceptionClear();
	jVM->DetachCurrentThread();
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVAndroidCamera::onPreviewFrame(const void* dataPtr, size_t dataSize, const void* userData)
{
	CompVAndroidCameraPtr camera = const_cast<CompVAndroidCamera*>(static_cast<const CompVAndroidCamera*>(userData));
	CompVAutoLock<CompVAndroidCamera> autoLock(*camera);
	CompVCameraListenerPtr listener = camera->listener();
	if (!listener) {
		return true;
	}
	if (!camera->m_bStarted) {
		COMPV_DEBUG_WARN("Android camera not started");
		return false;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// FIXME: framesize and chroma hard-coded
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	COMPV_CHECK_EXP_BAIL(((640 * 480 * 3) >> 1) != dataSize, (err = COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT));
	COMPV_CHECK_CODE_BAIL(err = CompVImage::wrap(COMPV_SUBTYPE_PIXELS_NV21, dataPtr, static_cast<size_t>(640), static_cast<size_t>(480), static_cast<size_t>(640), &camera->m_ptrImageCB));
	COMPV_CHECK_CODE_BAIL(err = listener->onNewFrame(camera->m_ptrImageCB));

bail:
	return COMPV_ERROR_CODE_IS_OK(err);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID */
