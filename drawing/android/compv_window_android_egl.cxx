/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/android/compv_window_android_egl.h"
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_headers_gl.h"

// FIXME:
#include "compv/drawing/opengl/compv_surface_gl.h"

COMPV_NAMESPACE_BEGIN()

// FIXME:factore this class to have an egl implementation for rasberrypi

CompVWindowAndroidEGL::CompVWindowAndroidEGL(int width, int height, const char* title)
	: CompVWindow(width, height, title)
	, m_pEGLDisplay(EGL_NO_DISPLAY)
	, m_pEGLSurface(EGL_NO_SURFACE)
	, m_pEGLContex(EGL_NO_CONTEXT)
	, m_bDrawing(false)
{
	COMPV_CHECK_CODE_ASSERT(CompVMutex::newObj(&m_ptrSDLMutex));
}

CompVWindowAndroidEGL::~CompVWindowAndroidEGL()
{
	COMPV_CHECK_CODE_ASSERT(close());
}

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

	// Create and initialize the context
	static const EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2, // FIXME(dmi): important! must retrieve
		EGL_NONE
	};
	COMPV_CHECK_EXP_BAIL((m_pEGLContex = eglCreateContext(m_pEGLDisplay, config, NULL, contextAttribs)) == EGL_NO_CONTEXT, (err = COMPV_ERROR_CODE_E_EGL));
	
	// Update width and height
	COMPV_CHECK_EXP_BAIL((eglQuerySurface(m_pEGLDisplay, m_pEGLSurface, EGL_WIDTH, &width) != EGL_TRUE), COMPV_ERROR_CODE_E_EGL);
	COMPV_CHECK_EXP_BAIL((eglQuerySurface(m_pEGLDisplay, m_pEGLSurface, EGL_HEIGHT, &height) != EGL_TRUE), COMPV_ERROR_CODE_E_EGL);
	m_nWidth = static_cast<int>(width);
	m_nHeight = static_cast<int>(height);

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(deInit());
	}
	return err;
}

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
	COMPV_CHECK_CODE_ASSERT(deInit());
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::beginDraw()
{
	COMPV_CHECK_CODE_RETURN(init());
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	//CompVSurfaceBlit* surfaceBlit = NULL;
	COMPV_CHECK_CODE_BAIL(err = m_ptrSDLMutex->lock());
	COMPV_CHECK_EXP_BAIL(m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_CODE_BAIL(err = makeGLContextCurrent());

	// FIXME: factor (same code in all window implementations)
	//glEnable(GL_TEXTURE_2D); // <process_gl_state_enables:519>: GL_INVALID_ENUM
	glDisable(GL_DEPTH_TEST); // Required by Skia, otherwise we'll need to use 'glPushAttrib(GL_ALL_ATTRIB_BITS); glPopAttrib();' before/after canvas drawing
	glDisable(GL_BLEND);
	glViewport(0, 0, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluOrtho2D((GLdouble)0, static_cast<GLdouble>(width), (GLdouble)0, static_cast<GLdouble>(height));
	//glOrtho((GLdouble)0, static_cast<GLdouble>(getWidth()), (GLdouble)0, static_cast<GLdouble>(getHeight()), (GLdouble)-1, (GLdouble)1);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	//if (m_ptrSurface) {
	//	surfaceBlit = dynamic_cast<CompVSurfaceBlit*>(*m_ptrSurface);
	//	if (surfaceBlit) {
	//		COMPV_CHECK_CODE_BAIL(err = surfaceBlit->clear());
	//	}
	//}

	m_bDrawing = true;
bail:
	COMPV_CHECK_CODE_ASSERT(m_ptrSDLMutex->unlock());
	return err;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::endDraw()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVSurfaceBlit* surfaceBlit;
	COMPV_CHECK_CODE_BAIL(err = m_ptrSDLMutex->lock());
	COMPV_CHECK_EXP_BAIL(!m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));

	surfaceBlit = (CompVSurfaceBlit*)(*m_ptrSurface);

	// FIXME:
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system

	//glBindTexture(GL_TEXTURE_2D, m_uNameTexture);

	//COMPV_CHECK_CODE_BAIL(err = surfaceBlit->blit());

	/*glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);

	glTexCoord2i(0, 1);
	glVertex2i(0, static_cast<GLint>(getHeight()));

	glTexCoord2i(1, 1);
	glVertex2i(static_cast<GLint>(getWidth()), static_cast<GLint>(getHeight()));

	glTexCoord2i(1, 0);
	glVertex2i(static_cast<GLint>(getWidth()), 0);
	glEnd();*/

#if 0 // FIXME(dmi): this is the correct code
	COMPV_CHECK_CODE_BAIL(err = surfaceBlit->blit());
#else
	((CompVSurfaceGL*)*m_ptrSurface)->blit();
#endif

	//glBindTexture(GL_TEXTURE_2D, 0);

	COMPV_CHECK_EXP_BAIL(eglSwapBuffers(m_pEGLDisplay, m_pEGLSurface) != EGL_TRUE, (err = COMPV_ERROR_CODE_E_EGL));

bail:
	m_bDrawing = false;
	COMPV_CHECK_CODE_ASSERT(unmakeGLContextCurrent());
	COMPV_CHECK_CODE_ASSERT(m_ptrSDLMutex->unlock());
	return err;
}

CompVSurfacePtr CompVWindowAndroidEGL::surface()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = m_ptrSDLMutex->lock());
	COMPV_CHECK_EXP_BAIL(!m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	if (!m_ptrSurface) {
		// Create the surface
		CompVSurfacePtr surface_;
		COMPV_CHECK_CODE_BAIL(err = CompVSurface::newObj(&surface_, this));
		// If GL activation (enabled/disabled) must be the same on both the window and surace
		COMPV_CHECK_EXP_BAIL(isGLEnabled() != surface_->isGLEnabled(), err = COMPV_ERROR_CODE_E_INVALID_STATE);
		m_ptrSurface = surface_;
	}
bail:
	COMPV_CHECK_CODE_ASSERT(m_ptrSDLMutex->unlock());
	if (COMPV_ERROR_CODE_IS_OK(err)) {
		return m_ptrSurface;
	}
	return NULL;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::makeGLContextCurrent()
{
	COMPV_CHECK_EXP_RETURN(!m_pEGLDisplay || !m_pEGLSurface || !m_pEGLContex, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(eglMakeCurrent(m_pEGLDisplay, m_pEGLSurface, m_pEGLSurface, m_pEGLContex) != EGL_TRUE, COMPV_ERROR_CODE_E_GL);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::unmakeGLContextCurrent()
{
	COMPV_CHECK_EXP_RETURN(!m_pEGLDisplay, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(eglMakeCurrent(m_pEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE, COMPV_ERROR_CODE_E_GL);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowAndroidEGL::newObj(CompVWindowAndroidEGLPtrPtr eglWindow, int width, int height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(eglWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowAndroidEGLPtr eglWindow_ = new CompVWindowAndroidEGL(width, height, title);
	COMPV_CHECK_EXP_RETURN(!eglWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(!eglWindow_->m_ptrSDLMutex, COMPV_ERROR_CODE_E_SYSTEM);
	*eglWindow = eglWindow_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_EGL */
