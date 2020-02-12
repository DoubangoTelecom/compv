/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_drawing_window_egl.h"
#if defined(HAVE_EGL)
#include "compv/drawing/compv_drawing.h"
#include "compv/gl/compv_gl_info.h"

#define COMPV_THIS_CLASSNAME	"CompVGLContextEGL"

COMPV_NAMESPACE_BEGIN()

//
//	CompVGLContextEGL
//

CompVGLContextEGL::CompVGLContextEGL(EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex)
    : CompVGLContext()
    , m_pEGLDisplay(pEGLDisplay)
    , m_pEGLSurface(pEGLSurface)
    , m_pEGLContex(pEGLContex)
{

}

CompVGLContextEGL::~CompVGLContextEGL()
{

}

COMPV_ERROR_CODE CompVGLContextEGL::makeCurrent() /* Overrides(CompVGLContext) */
{
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    //!\\ Order is important: call base class implementation to lock then set context then
    COMPV_CHECK_CODE_BAIL(err = CompVGLContext::makeCurrent()); // Base class implementation
    COMPV_CHECK_EXP_RETURN(eglMakeCurrent(m_pEGLDisplay, m_pEGLSurface, m_pEGLSurface, m_pEGLContex) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_GL), "Failed to make the context current");

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        // TODO(dmi): print error
        COMPV_CHECK_CODE_NOP(unmakeCurrent());
    }
    return err;
}

COMPV_ERROR_CODE CompVGLContextEGL::swapBuffers() /* Overrides(CompVGLContext) */
{
    COMPV_CHECK_CODE_RETURN(CompVGLContext::swapBuffers()); // Base class implementation
    COMPV_CHECK_EXP_RETURN(eglSwapBuffers(m_pEGLDisplay, m_pEGLSurface) != EGL_TRUE, COMPV_ERROR_CODE_E_EGL);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLContextEGL::unmakeCurrent() /* Overrides(CompVGLContext) */
{
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    //!\\ Order is important: unset context then call base class implementation to unlock
    COMPV_CHECK_EXP_RETURN(eglMakeCurrent(m_pEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_GL), "Failed to unmake the GL-ES context current.");
    COMPV_CHECK_CODE_BAIL(err = CompVGLContext::unmakeCurrent()); // Base class implementation

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        // TODO(dmi): print error
    }
    return err;
}

COMPV_ERROR_CODE CompVGLContextEGL::newObj(CompVGLContextEGLPtrPtr context, EGLDisplay pEGLDisplay, EGLSurface pEGLSurface, EGLContext pEGLContex)
{
	COMPV_CHECK_EXP_RETURN(!CompVDrawing::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    COMPV_CHECK_EXP_RETURN(!context || pEGLDisplay == EGL_NO_DISPLAY || pEGLSurface == EGL_NO_SURFACE || pEGLContex == EGL_NO_CONTEXT, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLContextEGLPtr context_ = new CompVGLContextEGL(pEGLDisplay, pEGLSurface, pEGLContex);
    COMPV_CHECK_EXP_RETURN(!context_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    *context = context_;
    return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVWindowEGL
//

CompVWindowEGL::CompVWindowEGL(size_t width, size_t height, const char* title)
    : CompVGLWindow(width, height, title)
    , m_pEGLDisplay(EGL_NO_DISPLAY)
    , m_pEGLSurface(EGL_NO_SURFACE)
    , m_pEGLContex(EGL_NO_CONTEXT)
{
}

CompVWindowEGL::~CompVWindowEGL()
{
    COMPV_CHECK_CODE_NOP(close());
}

// Private function: do not autolock, up to the caller
COMPV_ERROR_CODE CompVWindowEGL::init()
{
    if (m_pEGLDisplay != EGL_NO_DISPLAY) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    static const EGLint CompVEGLAttribs[] = {
#if COMPV_GL_EGL_CONTEXT_CLIENT_VERSION == 3
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
#else
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#endif
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    EGLNativeWindowType window;
    EGLint major, minor;
    EGLConfig config;
    EGLint numConfigs;
    EGLint width, height;

    // Retrieve the window associated to the native activity
    window = nativeWindow();
    COMPV_CHECK_EXP_BAIL(!window, (err = COMPV_ERROR_CODE_E_EGL), "No window is associated to the native activity");

    // Create and initialize the display
    m_pEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    COMPV_CHECK_EXP_BAIL(m_pEGLDisplay == EGL_NO_DISPLAY, (err = COMPV_ERROR_CODE_E_EGL));
    COMPV_CHECK_EXP_BAIL(eglInitialize(m_pEGLDisplay, &major, &minor) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing EGL display with major=%d and minor=%d", major, minor);

    COMPV_CHECK_EXP_BAIL(eglChooseConfig(m_pEGLDisplay, CompVEGLAttribs, &config, 1, &numConfigs) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));

    // Create and initialize the surface
    COMPV_CHECK_EXP_BAIL((m_pEGLSurface = eglCreateWindowSurface(m_pEGLDisplay, config, window, NULL)) == EGL_NO_SURFACE, (err = COMPV_ERROR_CODE_E_EGL));

    // Create, initialize and make current the context
    static const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, COMPV_GL_EGL_CONTEXT_CLIENT_VERSION,
        EGL_NONE
    };
    COMPV_CHECK_EXP_BAIL((m_pEGLContex = eglCreateContext(m_pEGLDisplay, config, NULL, contextAttribs)) == EGL_NO_CONTEXT, (err = COMPV_ERROR_CODE_E_EGL));
    COMPV_CHECK_EXP_BAIL(eglMakeCurrent(m_pEGLDisplay, m_pEGLSurface, m_pEGLSurface, m_pEGLContex) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));

    // Gather info (init supported extensions)
    COMPV_CHECK_CODE_BAIL(err = CompVGLInfo::gather());

    // Set swap interval
    COMPV_CHECK_EXP_BAIL(eglSwapInterval(m_pEGLDisplay, COMPV_GL_SWAP_INTERVAL) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));

    // Create obj context
    COMPV_CHECK_CODE_RETURN(CompVGLContextEGL::newObj(&m_ptrContext, m_pEGLDisplay, m_pEGLSurface, m_pEGLContex));

    // Update width and height to set to fullscreen
    COMPV_CHECK_EXP_BAIL((eglQuerySurface(m_pEGLDisplay, m_pEGLSurface, EGL_WIDTH, &width) != EGL_TRUE), (err = COMPV_ERROR_CODE_E_EGL));
    COMPV_CHECK_EXP_BAIL((eglQuerySurface(m_pEGLDisplay, m_pEGLSurface, EGL_HEIGHT, &height) != EGL_TRUE), (err = COMPV_ERROR_CODE_E_EGL));
    if (width != CompVWindow::m_nWidth || height != CompVWindow::m_nHeight) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Android, setting size to fullscreen: (%zd,%zd)->(%zd,%zd)", CompVWindow::m_nWidth, CompVWindow::m_nHeight, width, height);
        COMPV_CHECK_CODE_BAIL(err = priv_updateSize(static_cast<size_t>(width), static_cast<size_t>(height)));
    }

bail:
    if (m_pEGLDisplay) {
        eglMakeCurrent(m_pEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
    }
    return err;
}

// Private function: do not autolock, up to the caller
COMPV_ERROR_CODE CompVWindowEGL::deInit()
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

    m_ptrContext = NULL;

    return COMPV_ERROR_CODE_S_OK;
}

bool CompVWindowEGL::isClosed() const /* Overrides(CompVGLWindow) */
{
    return m_pEGLDisplay == EGL_NO_DISPLAY
           || m_pEGLContex == EGL_NO_CONTEXT
           || m_pEGLSurface == EGL_NO_SURFACE
           || !m_ptrContext;
}

COMPV_ERROR_CODE CompVWindowEGL::close() /* Overrides(CompVGLWindow) */
{
	COMPV_AUTOLOCK_THIS(CompVWindowEGL);
    COMPV_CHECK_CODE_NOP(CompVGLWindow::close()); // base class implementation
    COMPV_CHECK_CODE_RETURN(deInit());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGL::beginDraw() /* Overrides(CompVGLWindow) */
{
	COMPV_AUTOLOCK_THIS(CompVWindowEGL);
    COMPV_CHECK_CODE_RETURN(init());
    COMPV_CHECK_CODE_RETURN(CompVGLWindow::beginDraw()); // Base class implementation
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowEGL::priv_updateState(COMPV_WINDOW_STATE newState) /*Overrides(CompVWindowPriv)*/
{
	COMPV_AUTOLOCK_THIS(CompVWindowEGL);
	COMPV_CHECK_CODE_NOP(CompVGLWindow::priv_updateState(newState)); // call base class implementation (*must* be call first)
	switch (newState){
	case COMPV_WINDOW_STATE_CONTEXT_DESTROYED:
	case COMPV_WINDOW_STATE_CLOSED:
		COMPV_CHECK_CODE_NOP(close());
		break;
	default:
		break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

CompVGLContextPtr CompVWindowEGL::context() /* Overrides(CompVGLWindow) */
{
    return *m_ptrContext;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_EGL */
