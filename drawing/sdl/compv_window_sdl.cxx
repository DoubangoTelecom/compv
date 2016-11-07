/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/sdl/compv_window_sdl.h"
#if defined(HAVE_SDL_H)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_canvas.h"

#include "compv/drawing/opengl/compv_surface_gl.h"

#if !defined(COMPV_SDL_DISABLE_GL)
#	define COMPV_SDL_DISABLE_GL 0 // To test CPU drawing (no GL)
#endif

COMPV_NAMESPACE_BEGIN()

CompVWindowSDL::CompVWindowSDL(int width, int height, const char* title)
	: CompVWindow(width, height, title)
	, m_pSDLWindow(NULL)
	, m_pSDLContext(NULL)
	, m_bDrawing(false)
{
#   if COMPV_OS_APPLE
	if (!pthread_main_np()) {
		COMPV_DEBUG_WARN("MacOS: Creating window outside main thread");
	}
#   endif /* COMPV_OS_APPLE */
	static const SDL_WindowFlags sWindowFlags = static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE
#if !COMPV_SDL_DISABLE_GL
		| SDL_WINDOW_OPENGL
#endif
#if COMPV_OS_ANDROID || COMPV_OS_IPHONE || COMPV_OS_IPHONE_SIMULATOR
		 | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI
#endif
		);
	m_pSDLWindow = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		sWindowFlags
	);
	if (!m_pSDLWindow) {
		COMPV_DEBUG_ERROR("SDL_CreateWindow(%d, %d, %s) failed: %s", width, height, title, SDL_GetError());
		return;
	}
	m_pSDLContext = SDL_GL_CreateContext(m_pSDLWindow);
	if (!m_pSDLContext) {
		COMPV_DEBUG_ERROR("SDL_GL_CreateContext() failed: %s", SDL_GetError());
	}

	COMPV_CHECK_CODE_ASSERT(CompVMutex::newObj(&m_ptrSDLMutex));
	SDL_SetWindowData(m_pSDLWindow, "This", this);
	if (m_pSDLContext) {
		COMPV_ASSERT(SDL_GL_MakeCurrent(m_pSDLWindow, m_pSDLContext) == 0);
		SDL_GL_SetSwapInterval(1);
		SDL_SetEventFilter(CompVWindowSDL::FilterEvents, this);
		SDL_GL_MakeCurrent(m_pSDLWindow, NULL);
	}
}

CompVWindowSDL::~CompVWindowSDL()
{
	COMPV_CHECK_CODE_ASSERT(close());
}

bool CompVWindowSDL::isClosed()const
{
	return !m_pSDLWindow;
}

COMPV_ERROR_CODE CompVWindowSDL::close()
{
	COMPV_CHECK_CODE_ASSERT(unregister());
	COMPV_CHECK_CODE_ASSERT(m_ptrSDLMutex->lock());
	if (m_pSDLContext) {
		// GL Surface needs GLContext to deInit state
		SDL_GL_MakeCurrent(m_pSDLWindow, m_pSDLContext);
		m_ptrSurface = NULL;
		SDL_GL_MakeCurrent(m_pSDLWindow, NULL);
		// Delete GL context
		SDL_GL_DeleteContext(m_pSDLContext);
		m_pSDLContext = NULL;
	}
	if (m_pSDLWindow) {
		SDL_SetWindowData(m_pSDLWindow, "This", NULL);
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = NULL;
	}
	COMPV_CHECK_CODE_ASSERT(m_ptrSDLMutex->unlock());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowSDL::beginDraw()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVSurfaceBlit* surfaceBlit = NULL;
	COMPV_CHECK_CODE_BAIL(err = m_ptrSDLMutex->lock());
	COMPV_CHECK_EXP_BAIL(m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_CODE_BAIL(err = makeGLContextCurrent());

	// FIXME: factor (same code in all window implementations)
	glEnable(GL_TEXTURE_2D);
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

	if (m_ptrSurface) {
		surfaceBlit = dynamic_cast<CompVSurfaceBlit*>(*m_ptrSurface);
		if (surfaceBlit) {
			COMPV_CHECK_CODE_BAIL(err = surfaceBlit->clear());
		}
	}

	m_bDrawing = true;
bail:
	COMPV_CHECK_CODE_ASSERT(m_ptrSDLMutex->unlock());
	return err;
}

COMPV_ERROR_CODE CompVWindowSDL::endDraw()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = m_ptrSDLMutex->lock());
	COMPV_CHECK_EXP_BAIL(!m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));

	CompVSurfaceBlit* surfaceBlit = dynamic_cast<CompVSurfaceBlit*>(*m_ptrSurface);

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

	COMPV_CHECK_CODE_BAIL(err = surfaceBlit->blit());

	//glBindTexture(GL_TEXTURE_2D, 0);

	if (m_pSDLWindow) {
		SDL_GL_SwapWindow(m_pSDLWindow);
	}

bail:
	m_bDrawing = false;
	COMPV_CHECK_CODE_ASSERT(unmakeGLContextCurrent());
	COMPV_CHECK_CODE_ASSERT(m_ptrSDLMutex->unlock());
	return err;
}

CompVSurfacePtr CompVWindowSDL::surface()
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
	return COMPV_ERROR_CODE_IS_OK(err) ? m_ptrSurface : NULL;
}

// Private function: not thread-safe
COMPV_ERROR_CODE CompVWindowSDL::makeGLContextCurrent()
{
	if (m_pSDLWindow) { // FIXME: if null return error
		if (SDL_GL_MakeCurrent(m_pSDLWindow, m_pSDLContext) != 0) {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SDL);
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Private function: not thread-safe
COMPV_ERROR_CODE CompVWindowSDL::unmakeGLContextCurrent()
{
	if (m_pSDLWindow) { // FIXME: if null return error
		if (SDL_GL_MakeCurrent(m_pSDLWindow, NULL) != 0) {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SDL);
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowSDL::newObj(CompVWindowSDLPtrPtr sdlWindow, int width, int height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(sdlWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowSDLPtr sdlWindow_ = new CompVWindowSDL(width, height, title);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->m_pSDLWindow, COMPV_ERROR_CODE_E_SDL);
#if 0 // OpenGL could be unavailable -> CPU drawing fallback
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->m_pSDLContext, COMPV_ERROR_CODE_E_SDL);
#endif
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->m_ptrSDLMutex, COMPV_ERROR_CODE_E_SYSTEM);
	*sdlWindow = sdlWindow_;
	return COMPV_ERROR_CODE_S_OK;
}

// TODO(dmi): remove if not used
int CompVWindowSDL::FilterEvents(void *userdata, SDL_Event* event)
{
	/*if (event->type == SDL_WINDOWEVENT) {
		CompVWindowSDLPtr This = static_cast<CompVWindowSDL*>(userdata);
		switch (event->window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			COMPV_CHECK_CODE_ASSERT(This->m_ptrSDLMutex->lock());
			COMPV_CHECK_CODE_ASSERT(This->unregister());
			COMPV_CHECK_CODE_ASSERT(This->close());
			COMPV_CHECK_CODE_ASSERT(This->m_ptrSDLMutex->unlock());
			return 0;
		default:
			break;
		}
	}*/
	return (1);
}

COMPV_NAMESPACE_END()

#endif /* HAVE_SDL_H */
