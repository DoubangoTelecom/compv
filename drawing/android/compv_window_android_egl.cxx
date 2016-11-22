/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/android/compv_window_android_egl.h"
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
#include "compv/drawing/compv_drawing.h"
#include "compv/base/android/compv_android_native_activity.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVContextGLAndroidEGL
//

CompVContextGLAndroidEGL::CompVContextGLAndroidEGL(EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex)
	: CompVContextGL()
	, m_pEGLDisplay(pEGLDisplay)
	, m_pEGLSurface(pEGLSurface)
	, m_pEGLContex(pEGLContex)
{

}

CompVContextGLAndroidEGL::~CompVContextGLAndroidEGL()
{

}

COMPV_ERROR_CODE CompVContextGLAndroidEGL::makeCurrent()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	//!\\ Order is important: call base class implementation to lock then set context then
	COMPV_CHECK_CODE_BAIL(err = CompVContextGL::makeCurrent()); // Base class implementation
	COMPV_CHECK_EXP_RETURN(eglMakeCurrent(m_pEGLDisplay, m_pEGLSurface, m_pEGLSurface, m_pEGLContex) != EGL_TRUE, COMPV_ERROR_CODE_E_GL);

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		// TODO(dmi): print error
		COMPV_CHECK_CODE_ASSERT(unmakeCurrent());
	}
	return err;
}

COMPV_ERROR_CODE CompVContextGLAndroidEGL::swabBuffers()
{
	COMPV_CHECK_CODE_RETURN(CompVContextGL::swabBuffers()); // Base class implementation
	COMPV_CHECK_EXP_RETURN(eglSwapBuffers(m_pEGLDisplay, m_pEGLSurface) != EGL_TRUE, COMPV_ERROR_CODE_E_EGL);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVContextGLAndroidEGL::unmakeCurrent()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	//!\\ Order is important: unset context then call base class implementation to unlock
	COMPV_CHECK_EXP_RETURN(eglMakeCurrent(m_pEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE, COMPV_ERROR_CODE_E_GL);
	COMPV_CHECK_CODE_BAIL(err = CompVContextGL::unmakeCurrent()); // Base class implementation

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		// TODO(dmi): print error
	}
	return err;
}

COMPV_ERROR_CODE CompVContextGLAndroidEGL::newObj(CompVContextGLAndroidEGLPtrPtr context, EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!context || pEGLDisplay == EGL_NO_DISPLAY || pEGLSurface == EGL_NO_SURFACE || pEGLContex == EGL_NO_CONTEXT, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVContextGLAndroidEGLPtr context_ = new CompVContextGLAndroidEGL(pEGLDisplay, pEGLSurface, pEGLContex);
	COMPV_CHECK_EXP_RETURN(!context_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*context = context_;
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVWindowAndroidEGL
//	

// FIXME(dmi): factore this class to have an egl implementation for rasberrypi

CompVWindowAndroidEGL::CompVWindowAndroidEGL(size_t width, size_t height, const char* title)
	: CompVWindowGL(width, height, title)
	, m_pEGLDisplay(EGL_NO_DISPLAY)
	, m_pEGLSurface(EGL_NO_SURFACE)
	, m_pEGLContex(EGL_NO_CONTEXT)
{
}

CompVWindowAndroidEGL::~CompVWindowAndroidEGL()
{
	COMPV_CHECK_CODE_ASSERT(close());
}

// Private function: do not autolock, up to the caller
COMPV_ERROR_CODE CompVWindowAndroidEGL::init()
{
	if (m_pEGLDisplay != EGL_NO_DISPLAY) {
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
	ANativeWindow* window;
	EGLint major, minor;
	EGLConfig config;
	EGLint numConfigs;
	EGLint width, height;
	int32_t result;

	// Retrieve the window associated to the native activity
	window = CompVDrawing::getAndroidNativeActivityWindow();
	if (!window) {
		COMPV_DEBUG_ERROR("No window is associated to the native activity");
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_EGL);
	}

	// Create and initialize the display
	m_pEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	COMPV_CHECK_EXP_BAIL(m_pEGLDisplay == EGL_NO_DISPLAY, (err = COMPV_ERROR_CODE_E_EGL));
	COMPV_CHECK_EXP_BAIL(eglInitialize(m_pEGLDisplay, &major, &minor) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));
	COMPV_DEBUG_INFO("Initializing EGL display with major=%d and minor=%d", major, minor);

	COMPV_CHECK_EXP_BAIL(eglChooseConfig(m_pEGLDisplay, CompVEGLAttribs, &config, 1, &numConfigs) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));	

	if ((result = ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888)) != 0) {
		EGLint format;
		COMPV_DEBUG_ERROR("ANativeWindow_setBuffersGeometry(WINDOW_FORMAT_RGBA_8888) failed with error code = %d", result);
		/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
		* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
		* As soon as we picked a EGLConfig, we can safely reconfigure the
		* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
		COMPV_CHECK_EXP_BAIL(eglGetConfigAttrib(m_pEGLDisplay, config, EGL_NATIVE_VISUAL_ID, &format) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));
		COMPV_DEBUG_INFO("EGL_NATIVE_VISUAL_ID = %d", format);
		COMPV_CHECK_EXP_BAIL((result = ANativeWindow_setBuffersGeometry(window, 0, 0, format)) != 0, (err = COMPV_ERROR_CODE_E_EGL));
	}

	// Create and initialize the surface
	COMPV_CHECK_EXP_BAIL((m_pEGLSurface = eglCreateWindowSurface(m_pEGLDisplay, config, window, NULL)) == EGL_NO_SURFACE, (err = COMPV_ERROR_CODE_E_EGL));

	// Create, initialize and make current the context
	static const EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2, // FIXME(dmi): important! must retrieve
		EGL_NONE
	};
	COMPV_CHECK_EXP_BAIL((m_pEGLContex = eglCreateContext(m_pEGLDisplay, config, NULL, contextAttribs)) == EGL_NO_CONTEXT, (err = COMPV_ERROR_CODE_E_EGL));
	COMPV_CHECK_EXP_BAIL(eglMakeCurrent(m_pEGLDisplay, m_pEGLSurface, m_pEGLSurface, m_pEGLContex) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));

	// Set swap interval
	COMPV_CHECK_EXP_BAIL(eglSwapInterval(m_pEGLDisplay, 0) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));

	// Create obj context
	COMPV_CHECK_CODE_RETURN(CompVContextGLAndroidEGL::newObj(&m_ptrContext, m_pEGLDisplay, m_pEGLSurface, m_pEGLContex));
	
	// Update width and height to set to fullscreen
	COMPV_CHECK_EXP_BAIL((eglQuerySurface(m_pEGLDisplay, m_pEGLSurface, EGL_WIDTH, &width) != EGL_TRUE), (err = COMPV_ERROR_CODE_E_EGL));
	COMPV_CHECK_EXP_BAIL((eglQuerySurface(m_pEGLDisplay, m_pEGLSurface, EGL_HEIGHT, &height) != EGL_TRUE), (err = COMPV_ERROR_CODE_E_EGL));
	if (width != CompVWindow::m_nWidth || height != CompVWindow::m_nHeight) {
		COMPV_DEBUG_INFO("Android, setting size to fullscreen: (%zd,%zd)->(%zd,%zd)", CompVWindow::m_nWidth, CompVWindow::m_nHeight, width, height);
		COMPV_CHECK_CODE_BAIL(err = priv_updateSize(static_cast<size_t>(width), static_cast<size_t>(height)));
	}

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
	}
	return err;
}

// Private function: do not autolock, up to the caller
COMPV_ERROR_CODE CompVWindowAndroidEGL::deInit()
{
	if (m_pEGLDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(m_pEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (m_pEGLContex != EGL_NO_CONTEXT) {
			eglDestroyContext(m_pEGLDisplay, m_pEGLContex);
		}
		if (m_pEGLSurface != EGL_NO_SURFACE) {
			eglDestroySurface(m_pEGLDisplay, m_pEGLSurface);
		}
		eglTerminate(m_pEGLDisplay);
	}
	m_pEGLDisplay = EGL_NO_DISPLAY;
	m_pEGLContex = EGL_NO_CONTEXT;
	m_pEGLSurface = EGL_NO_SURFACE;

	return COMPV_ERROR_CODE_S_OK;
}

bool CompVWindowAndroidEGL::isClosed()const
{
	COMPV_ASSERT(false);
	return false;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::close()
{
	CompVAutoLock<CompVWindowAndroidEGL>(this);
	COMPV_CHECK_CODE_RETURN(deInit());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::beginDraw()
{
	CompVAutoLock<CompVWindowAndroidEGL>(this);
	COMPV_CHECK_CODE_RETURN(init());
	COMPV_CHECK_CODE_RETURN(CompVWindowGL::beginDraw()); // Base class implementation
	return COMPV_ERROR_CODE_S_OK;
}

CompVContextGLPtr CompVWindowAndroidEGL::context()
{
	return *m_ptrContext;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::newObj(CompVWindowAndroidEGLPtrPtr eglWindow, size_t width, size_t height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(eglWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	struct android_app* app = AndroidApp_get();
	if (app && app->window) {
		size_t newWidth = static_cast<size_t>(ANativeWindow_getWidth(app->window));
		size_t newheight = static_cast<size_t>(ANativeWindow_getHeight(app->window));
		if (width && height) {
			COMPV_DEBUG_INFO("Android, setting size to fullscreen: (%zd,%zd)->(%zd,%zd)", width, height, newWidth, newheight);
			width = newWidth;
			height = newheight;
		}
	}
	CompVWindowAndroidEGLPtr eglWindow_ = new CompVWindowAndroidEGL(width, height, title);
	COMPV_CHECK_EXP_RETURN(!eglWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(!eglWindow_->isInitialized(), COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	//COMPV_CHECK_CODE_RETURN(CompVContextGLAndroidEGL::newObj(&eglWindow_->m_ptrContext, eglWindow_->m_pEGLDisplay, eglWindow_->m_pEGLSurface, eglWindow_->m_pEGLContex));
	*eglWindow = eglWindow_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_EGL */
