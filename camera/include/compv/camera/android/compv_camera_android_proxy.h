/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CAMERA_ANDROID_PROXY_H_)
#define _COMPV_CAMERA_ANDROID_PROXY_H_
#if defined(__ANDROID__) || defined(ANDROID)

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

// This file is used by SWIG processor and must not contain fancy includes or add dependencies to the libcompv
// This file is included in 'plugin_androidcamera' project built using Android Studio and embedded in compv project as a plugin (apk).

#include <stdint.h>
#include <android/api-level.h>
#include <android/log.h>

#if !defined(SWIG)
#define COMPV_CAMERA_ANDROID_LOG_TAG "org.doubango.compv.androidcamera.proxy"
typedef bool(*CompVCameraAndroidProxyOnPreviewFrameFunc)(const void* frameDataPtr, size_t frameDataSize, size_t frameWidth, size_t frameHeight, int frameFps, int framePixelFormat, const void* userData);
#endif

class CompVCameraAndroidProxy
{
public:
	CompVCameraAndroidProxy() 
		: m_funcOnPreviewFrame(NULL)
		, m_ptrUserData(0)
	{
		__android_log_print(ANDROID_LOG_INFO, COMPV_CAMERA_ANDROID_LOG_TAG, "CompVCameraAndroidProxy.ctor()");
	}
	virtual ~CompVCameraAndroidProxy() {
		__android_log_print(ANDROID_LOG_INFO, COMPV_CAMERA_ANDROID_LOG_TAG, "~CompVCameraAndroidProxy.dtor()");
	}

	void setCallbackFunc(uintptr_t funcptr, uintptr_t userData) {
		m_funcOnPreviewFrame = reinterpret_cast<CompVCameraAndroidProxyOnPreviewFrameFunc>(funcptr);
		m_ptrUserData = reinterpret_cast<const void*>(userData);
		__android_log_print(ANDROID_LOG_INFO, COMPV_CAMERA_ANDROID_LOG_TAG, "setCallbackFunc(%p, %p)", m_funcOnPreviewFrame, m_ptrUserData);
	}

	virtual bool pushFrame(const void* frameDataPtr, size_t frameDataSize, size_t frameWidth, size_t frameHeight, int frameFps, int framePixelFormat) {
		if (m_funcOnPreviewFrame) {
			return m_funcOnPreviewFrame(frameDataPtr, frameDataSize, frameWidth, frameHeight, frameFps, framePixelFormat, m_ptrUserData);
		}
		__android_log_print(ANDROID_LOG_ERROR, COMPV_CAMERA_ANDROID_LOG_TAG, "m_funcOnPreviewFrame not defined");
		return false;
	}

private:
#if !defined(SWIG)
	CompVCameraAndroidProxyOnPreviewFrameFunc m_funcOnPreviewFrame;
	const void* m_ptrUserData;
#endif
};

#endif /* defined(__ANDROID__) || defined(ANDROID) */
#endif /* _COMPV_CAMERA_ANDROID_PROXY_H_ */
