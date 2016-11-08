/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_ANDROID_EGL_H_)
#define _COMPV_DRAWING_WINDOW_ANDROID_EGL_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
#include "compv/drawing/opengl/compv_window_gl.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVWindowAndroidEGL;
typedef CompVPtr<CompVWindowAndroidEGL* > CompVWindowAndroidEGLPtr;
typedef CompVWindowAndroidEGLPtr* CompVWindowAndroidEGLPtrPtr;

class CompVWindowAndroidEGL : public CompVWindowGL
{
protected:
	CompVWindowAndroidEGL(int width, int height, const char* title);
public:
	virtual ~CompVWindowAndroidEGL();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindowAndroidEGL";
	};

	/* CompVWindow overrides */
	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	// Override(CompVWindow::beginDraw) -> CompVWindowGL
	// Override(CompVWindow::endDraw) -> CompVWindowGL
	// Override(CompVWindow::surface) -> CompVWindowGL

	static COMPV_ERROR_CODE newObj(CompVWindowAndroidEGLPtrPtr eglWindow, int width, int height, const char* title);

protected:
	/* CompVWindowGL overrides */
	virtual CompVGLContext getGLContext()const { return static_cast<CompVGLContext>(m_pEGLContex); } // FIXME: CompVWindow override
	virtual COMPV_ERROR_CODE makeGLContextCurrent();
	virtual COMPV_ERROR_CODE unmakeGLContextCurrent();
	virtual COMPV_ERROR_CODE swapGLBuffers();

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();
	
private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	EGLDisplay m_pEGLDisplay;
	EGLSurface m_pEGLSurface;
	EGLContext m_pEGLContex;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* COMPV_OS_ANDROID && defined(HAVE_EGL) */

#endif /* _COMPV_DRAWING_WINDOW_ANDROID_EGL_H_ */
