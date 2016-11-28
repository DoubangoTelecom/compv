/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_window_sdl.h"
#if defined(HAVE_SDL_H)
#include "compv/drawing/compv_drawing.h"
#include "compv/base/parallel/compv_thread.h"

#if !defined(COMPV_SDL_DISABLE_GL)
#	define COMPV_SDL_DISABLE_GL 0 // To test CPU drawing (no GL)
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVWindowFactorySDL
//
static COMPV_ERROR_CODE CompVWindowFactorySDL_newObj(CompVWindowPtrPtr window, size_t width, size_t height, const char* title)
{
	COMPV_CHECK_EXP_RETURN(!window || !width || !height || !title, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowSDLPtr sdlWindow;
	COMPV_CHECK_CODE_RETURN(CompVWindowSDL::newObj(&sdlWindow, width, height, title));
	*window = *sdlWindow;
	return COMPV_ERROR_CODE_S_OK;
}

const CompVWindowFactory CompVWindowFactorySDL = {
	"SDL",
	CompVWindowFactorySDL_newObj
};

//
//	CompVGLContextSDL
//

CompVGLContextSDL::CompVGLContextSDL(SDL_Window *pSDLWindow, SDL_GLContext pSDLContext)
	: CompVGLContext()
	, m_pSDLWindow(pSDLWindow)
	, m_pSDLContext(pSDLContext)
{
}

CompVGLContextSDL::~CompVGLContextSDL()
{
}

	
COMPV_ERROR_CODE CompVGLContextSDL::makeCurrent()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	//!\\ Order is important: call base class implementation to lock then set context then
	COMPV_CHECK_CODE_BAIL(err = CompVGLContext::makeCurrent()); // Base class implementation
	COMPV_CHECK_EXP_BAIL(SDL_GL_MakeCurrent(m_pSDLWindow, m_pSDLContext) != 0, (err = COMPV_ERROR_CODE_E_SDL));

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_DEBUG_ERROR("SDL_GL_MakeCurrent failed: %s", SDL_GetError());
		COMPV_CHECK_CODE_ASSERT(unmakeCurrent());
	}
	return err;
}

COMPV_ERROR_CODE CompVGLContextSDL::swapBuffers()
{
	COMPV_CHECK_CODE_RETURN(CompVGLContext::swapBuffers()); // Base class implementation
	SDL_GL_SwapWindow(m_pSDLWindow);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVGLContextSDL::unmakeCurrent()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	//!\\ Order is important: unset context then call base class implementation to unlock
	COMPV_CHECK_EXP_BAIL(SDL_GL_MakeCurrent(m_pSDLWindow, NULL) != 0, (err = COMPV_ERROR_CODE_E_SDL));
	COMPV_CHECK_CODE_BAIL(err = CompVGLContext::unmakeCurrent()); // Base class implementation

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_DEBUG_ERROR("SDL_GL_MakeCurrent failed: %s", SDL_GetError());
	}
	return err;
}

COMPV_ERROR_CODE CompVGLContextSDL::newObj(CompVGLContextSDLPtrPtr context, SDL_Window *pSDLWindow, SDL_GLContext pSDLContext)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!context || !pSDLWindow || !pSDLContext, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLContextSDLPtr context_ = new CompVGLContextSDL(pSDLWindow, pSDLContext);
	COMPV_CHECK_EXP_RETURN(!context_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*context = context_;
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVWindowSDL
//

CompVWindowSDL::CompVWindowSDL(size_t width, size_t height, const char* title)
	: CompVGLWindow(width, height, title)
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
		static_cast<int>(width),
		static_cast<int>(height),
		sWindowFlags
	);
	if (!m_pSDLWindow) {
		COMPV_DEBUG_ERROR("SDL_CreateWindow(%zd, %zd, %s) failed: %s", width, height, title, SDL_GetError());
		return;
	}
	m_pSDLContext = SDL_GL_CreateContext(m_pSDLWindow);
	if (!m_pSDLContext) {
		COMPV_DEBUG_ERROR("SDL_GL_CreateContext() failed: %s", SDL_GetError());
	}

	SDL_SetWindowData(m_pSDLWindow, "This", this);
	if (m_pSDLContext) {
		COMPV_ASSERT(SDL_GL_MakeCurrent(m_pSDLWindow, m_pSDLContext) == 0);
		SDL_GL_SetSwapInterval(COMPV_GL_SWAP_INTERVAL);
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
	if (!isClosed()) {
		COMPV_CHECK_CODE_ASSERT(unregister());
		COMPV_CHECK_CODE_ASSERT(CompVGLWindow::close()); // base class implementation
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
		m_ptrContext = NULL;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVGLWindow)
CompVGLContextPtr CompVWindowSDL::context()
{
	return *m_ptrContext;
}

COMPV_ERROR_CODE CompVWindowSDL::newObj(CompVWindowSDLPtrPtr sdlWindow, size_t width, size_t height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(sdlWindow == NULL || !width || !height || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowSDLPtr sdlWindow_ = new CompVWindowSDL(width, height, title);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->isInitialized(), COMPV_ERROR_CODE_E_SYSTEM);
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->m_pSDLWindow, COMPV_ERROR_CODE_E_SDL);
#if 1 // OpenGL could be unavailable -> CPU drawing fallback. TODO(dmi): For now OpenGL is required.
	COMPV_CHECK_EXP_RETURN(!sdlWindow_->m_pSDLContext, COMPV_ERROR_CODE_E_SDL);
#endif
	COMPV_CHECK_CODE_RETURN(CompVGLContextSDL::newObj(&sdlWindow_->m_ptrContext, sdlWindow_->m_pSDLWindow, sdlWindow_->m_pSDLContext));

	*sdlWindow = sdlWindow_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_SDL_H */
