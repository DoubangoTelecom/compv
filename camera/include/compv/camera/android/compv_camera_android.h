/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CAMERA_ANDROID_H_)
#define _COMPV_CAMERA_ANDROID_H_

#include "compv/base/compv_config.h"
#if COMPV_OS_ANDROID
#include "compv/camera/compv_camera.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_lock.h"
#include "compv/base/compv_mat.h"

#include <jni.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(AndroidCamera)

class CompVAndroidCamera : public CompVCamera, public CompVLock
{
protected:
	CompVAndroidCamera();
public:
	virtual ~CompVAndroidCamera();
	COMPV_OBJECT_GET_ID(CompVAndroidCamera);

	virtual COMPV_ERROR_CODE devices(CompVCameraDeviceInfoList& list) override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE start(const std::string& deviceId = "") override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE stop() override /* Overrides(CompVCamera) */;

	static COMPV_ERROR_CODE newObj(CompVAndroidCameraPtrPtr camera);

private:
	static COMPV_ERROR_CODE attachCurrentThread(JavaVM** jVM, JNIEnv** jEnv);
	static COMPV_ERROR_CODE detachCurrentThread(JavaVM* jVM, JNIEnv* jEnv);
	static bool onPreviewFrame(const void* dataPtr, size_t dataSize, const void* userData);

private:
	static bool s_bPluginCopied;
	bool m_bStarted;
	jobject m_jobjectCamera;
	jmethodID m_methodCameraStart;
	jmethodID m_methodSetCallbackFunc;
	CompVMatPtr m_ptrImageCB;
};

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID */
#endif /* _COMPV_CAMERA_ANDROID_H_ */
