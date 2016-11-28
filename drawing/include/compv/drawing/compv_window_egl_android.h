/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_WINDOW_EGL_ANDROID_H_)
#define _COMPV_WINDOW_EGL_ANDROID_H_

#include "compv/base/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
#include "compv/drawing/compv_window_egl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

extern const CompVWindowFactory CompVWindowFactoryEGLAndroid;

//
//	CompVWindowEGLAndroid
//
class CompVWindowEGLAndroid;
typedef CompVPtr<CompVWindowEGLAndroid* > CompVWindowEGLAndroidPtr;
typedef CompVWindowEGLAndroidPtr* CompVWindowEGLAndroidPtrPtr;

class CompVWindowEGLAndroid : public CompVWindowEGL
{
protected:
	CompVWindowEGLAndroid(size_t width, size_t height, const char* title);
public:
	virtual ~CompVWindowEGLAndroid();
	COMPV_GET_OBJECT_ID(CompVWindowEGLAndroid);
	
	virtual EGLNativeWindowType nativeWindow() override /* Overrides(CompVWindowEGL) */;

	static COMPV_ERROR_CODE newObj(CompVWindowEGLAndroidPtrPtr eglWindow, size_t width, size_t height, const char* title);
	
private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	EGLNativeWindowType m_pNativeWindow;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID && defined(HAVE_EGL) */

#endif /* _COMPV_WINDOW_EGL_ANDROID_H_ */
