/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_window_egl.h"
#if defined(HAVE_EGL) && 0
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_headers_gl.h"

COMPV_NAMESPACE_BEGIN()

CompVWindowEGL::CompVWindowEGL(int width, int height, const char* title)
	: CompVWindow(width, height, title)
	, m_pEGLDisplay(EGL_NO_DISPLAY)
	, m_pEGLSurface(EGL_NO_SURFACE)
	, m_pEGLContex(EGL_NO_CONTEXT)
	, m_bDrawing(false)
{
	
}

CompVWindowEGL::~CompVWindowEGL()
{
	COMPV_CHECK_CODE_ASSERT(close());
}

COMPV_ERROR_CODE CompVWindowEGL::init()
{
	if (m_pEGLDisplay) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	static const EGLint CompVEGLAttribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};
	EGLint major, minor;
	EGLConfig config;
	EGLint numConfigs;
	m_pEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!m_pEGLDisplay) {
		COMPV_DEBUG_ERROR("eglGetDisplay failed");
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_EGL);
	}
	if (eglInitialize(m_pEGLDisplay, &major, &minor) != EGL_TRUE) {
		COMPV_DEBUG_ERROR("eglGetDisplay failed");
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_EGL);
	}
	COMPV_DEBUG_INFO("Initializing EGL display with major=%d and minor=%d", major, minor);
	if (eglChooseConfig(m_pEGLDisplay, CompVEGLAttribs, &config, 1, &numConfigs) != EGL_TRUE) {
		COMPV_DEBUG_ERROR("eglChooseConfig failed");
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_EGL);
	}

	// TODO(dmi): eglCreateWindowSurface requires a 'native_window'. This is why we stopped this implemenation

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
	}
	return err;
}

COMPV_ERROR_CODE CompVWindowEGL::deInit()
{

}

bool CompVWindowEGL::isClosed()const
{
	COMPV_ASSERT(false);
	return false;
}

COMPV_ERROR_CODE CompVWindowEGL::close()
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGL::beginDraw()
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGL::endDraw()
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

CompVSurfacePtr CompVWindowEGL::surface()
{

}

COMPV_ERROR_CODE CompVWindowEGL::makeGLContextCurrent()
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGL::unmakeGLContextCurrent()
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGL::newObj(CompVWindowEGLPtrPtr eglWindow, int width, int height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(eglWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowEGLPtr eglWindow_ = new CompVWindowEGL(width, height, title);
	COMPV_CHECK_EXP_RETURN(!eglWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(eglWindow_->init());
	*eglWindow = eglWindow_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_EGL */
