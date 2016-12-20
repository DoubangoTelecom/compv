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
#include "compv/base/compv_base.h"

#include <jni.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#define COMPV_PixelFormat_YCbCr_420_SP	17	// https://developer.android.com/reference/android/graphics/PixelFormat.html#YCbCr_420_SP
#define COMPV_PixelFormat_YCbCr_422_I	20	// https://developer.android.com/reference/android/graphics/PixelFormat.html#YCbCr_422_I
#define COMPV_PixelFormat_YCbCr_422_SP	16	// https://developer.android.com/reference/android/graphics/PixelFormat.html#YCbCr_422_SP
#define COMPV_PixelFormat_RGB_565		4	// https://developer.android.com/reference/android/graphics/PixelFormat.html#RGB_565
#define COMPV_PixelFormat_RGBA_8888		1	// https://developer.android.com/reference/android/graphics/PixelFormat.html#RGBA_8888
#define COMPV_PixelFormat_RGBA_888		3	// https://developer.android.com/reference/android/graphics/PixelFormat.html#RGB_888

COMPV_NAMESPACE_BEGIN()

class CompVAndroidCamera;

static const std::string COMPV_CompVAndroidCamera_formatName(int format);

struct CompVAndroidCameraCaps {
	int width;
	int height;
	int fps;
	int format;
	// Important: update 'isEquals' and 'toString' functions if you add new field

	CompVAndroidCameraCaps(int width_ = 640, int height_ = 480, int fps_ = 25, int format_ = COMPV_PixelFormat_YCbCr_420_SP) {
		width = width_;
		height = height_;
		fps = fps_;
		format = format_;
	}

	COMPV_INLINE bool isEquals(const CompVAndroidCameraCaps& caps)const {
		return width == caps.width && height == caps.height && fps == caps.fps && format == caps.format;
	}

	COMPV_INLINE const std::string toString()const {
		return
			std::string("width=") + CompVBase::to_string(width) + std::string(", ")
			+ std::string("height=") + CompVBase::to_string(height) + std::string(", ")
			+ std::string("fps=") + CompVBase::to_string(fps) + std::string(", ")
			+ std::string("format=") + COMPV_CompVAndroidCamera_formatName(format);
	}
};

COMPV_OBJECT_DECLARE_PTRS(AndroidCamera)

class CompVAndroidCamera : public CompVCamera, public CompVLock
{
	friend struct CompVAndroidCameraCaps;
protected:
	CompVAndroidCamera();
public:
	virtual ~CompVAndroidCamera();
	COMPV_OBJECT_GET_ID(CompVAndroidCamera);

	virtual COMPV_ERROR_CODE devices(CompVCameraDeviceInfoList& list) override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE start(const std::string& deviceId = "") override /* Overrides(CompVCamera) */;
	virtual COMPV_ERROR_CODE stop() override /* Overrides(CompVCamera) */;

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /* Overrides(CompVCaps) */;
	virtual COMPV_ERROR_CODE get(int id, const void*& valuePtr, size_t valueSize) override /* Overrides(CompVCaps) */;

	static COMPV_ERROR_CODE newObj(CompVAndroidCameraPtrPtr camera);
	
	static const std::string formatName(int format);

private:
	static COMPV_ERROR_CODE initJNI(JNIEnv* jEnv);
	static COMPV_ERROR_CODE deInitJNI(JNIEnv* jEnv);
	static COMPV_ERROR_CODE convertFormat(const COMPV_SUBTYPE& subType, int& format);
	static COMPV_ERROR_CODE convertFormat(const int& format, COMPV_SUBTYPE& subtype);
	static COMPV_ERROR_CODE attachCurrentThread(JavaVM** jVM, JNIEnv** jEnv);
	static COMPV_ERROR_CODE detachCurrentThread(JavaVM* jVM, JNIEnv* jEnv);
	static bool onPreviewFrame(const void* frameDataPtr, size_t frameDataSize, size_t frameWidth, size_t frameHeight, int frameFps, int framePixelFormat, const void* userData);

private:	
	bool m_bStarted;
	CompVAndroidCameraCaps m_CapsPref;
	COMPV_SUBTYPE m_eSubTypeNeg;
	jobject m_jobjectCamera;
	CompVMatPtr m_ptrImageCB;

	static bool s_bPluginCopied;
	static bool s_bJniInitialized;
	static jclass s_classCamera;
	static jmethodID s_methodConstructor;
	static jmethodID s_methodCameraStart;
	static jmethodID s_methodCameraStop;
	static jmethodID s_methodSetCallbackFunc;
	static jmethodID s_methodGetNumberOfCameras;
	static jmethodID s_methodGetCameraInfo;
	static jmethodID s_methodSetCaps;
	static int PixelFormat_NV21;
	static int PixelFormat_YUY2;
	static int PixelFormat_NV16;
	static int PixelFormat_RGB565;
	static int PixelFormat_RGBA;
	static int PixelFormat_RGB;
};

static const std::string COMPV_CompVAndroidCamera_formatName(int format) {
	return CompVAndroidCamera::formatName(format);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID */
#endif /* _COMPV_CAMERA_ANDROID_H_ */
