/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/sdl/compv_window_sdl.h"
#if defined(HAVE_SDL_H)
#include "compv/drawing/compv_drawing.h"
#include "compv/base/parallel/compv_thread.h"

#if !defined(COMPV_SDL_DISABLE_GL)
#	define COMPV_SDL_DISABLE_GL 0 // To test CPU drawing (no GL)
#endif

COMPV_NAMESPACE_BEGIN()

CompVWindowSDL::CompVWindowSDL(int width, int height, const char* title)
	: CompVWindowGL(width, height, title)
	, m_pSDLWindow(NULL)
	, m_pSDLContext(NULL)
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

// Overrides 'CompVWindow::isClosed'
bool CompVWindowSDL::isClosed()const
{
	return !m_pSDLWindow;
}

// Overrides 'CompVWindow::close'
COMPV_ERROR_CODE CompVWindowSDL::close()
{
	CompVAutoLock<CompVWindowSDL>(this);
	COMPV_CHECK_CODE_ASSERT(unregister());
	if (m_pSDLContext) {
		// Delete GL context
		SDL_GL_DeleteContext(m_pSDLContext);
		m_pSDLContext = NULL;
	}
	if (m_pSDLWindow) {
		SDL_SetWindowData(m_pSDLWindow, "This", NULL);
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = NULL;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides 'CompVWindowGL::makeGLContextCurrent'
COMPV_ERROR_CODE CompVWindowSDL::makeGLContextCurrent()
{
	CompVAutoLock<CompVWindowSDL>(this);
	COMPV_CHECK_EXP_RETURN(!m_pSDLWindow, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(SDL_GL_MakeCurrent(m_pSDLWindow, m_pSDLContext) != 0, COMPV_ERROR_CODE_E_SDL);
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides 'CompVWindowGL::unmakeGLContextCurrent'
COMPV_ERROR_CODE CompVWindowSDL::unmakeGLContextCurrent()
{
	CompVAutoLock<CompVWindowSDL>(this);
	COMPV_CHECK_EXP_RETURN(!m_pSDLWindow, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(SDL_GL_MakeCurrent(m_pSDLWindow, NULL) != 0, COMPV_ERROR_CODE_E_SDL);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowSDL::swapGLBuffers()
{
	CompVAutoLock<CompVWindowSDL>(this);
	COMPV_CHECK_EXP_RETURN(!m_pSDLWindow, COMPV_ERROR_CODE_E_INVALID_STATE);
	SDL_GL_SwapWindow(m_pSDLWindow);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowSDL::newObj(CompVWindowSDLPtrPtr sdlWindow, int width, int height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(sdlWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowSDLPtr sdlWindow_ = new CompVWindowSDL(width, height, title);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->isInitialized(), COMPV_ERROR_CODE_E_SYSTEM);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->m_pSDLWindow, COMPV_ERROR_CODE_E_SDL);
#if 1 // OpenGL could be unavailable -> CPU drawing fallback. TODO(dmi): For now OpenGL is required.
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->m_pSDLContext, COMPV_ERROR_CODE_E_SDL);
#endif
	*sdlWindow = sdlWindow_;
	return COMPV_ERROR_CODE_S_OK;
}

// FIXME(dmi): remove if not used
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
