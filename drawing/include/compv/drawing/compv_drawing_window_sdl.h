/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_SDL_H_)
#define _COMPV_DRAWING_WINDOW_SDL_H_

#include "compv/drawing/compv_drawing_config.h"
#if defined(HAVE_SDL_H)
#include "compv/drawing/compv_drawing_common.h"
#include "compv/gl/compv_gl_window.h"

#include <SDL.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

extern const CompVWindowFactory CompVWindowFactorySDL;

//
//	CompVGLContextSDL
//
COMPV_OBJECT_DECLARE_PTRS(GLContextSDL)

class CompVGLContextSDL : public CompVGLContext
{
protected:
	CompVGLContextSDL(SDL_Window *pSDLWindow, SDL_GLContext pSDLContext);
public:
	virtual ~CompVGLContextSDL();
	COMPV_OBJECT_GET_ID(CompVGLContextSDL);

	virtual COMPV_ERROR_CODE makeCurrent();
	virtual COMPV_ERROR_CODE swapBuffers();
	virtual COMPV_ERROR_CODE unmakeCurrent();

	static COMPV_ERROR_CODE newObj(CompVGLContextSDLPtrPtr context, SDL_Window *pSDLWindow, SDL_GLContext pSDLContext);

private:
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_pSDLContext;
};


//
//	CompVWindowSDL
//
COMPV_OBJECT_DECLARE_PTRS(WindowSDL)

class CompVWindowSDL : public CompVGLWindow
{
protected:
	CompVWindowSDL(size_t width, size_t height, const char* title);
public:
	virtual ~CompVWindowSDL();
	COMPV_OBJECT_GET_ID(CompVWindowSDL);
	
	/* CompVWindow overrides */
	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	// Override(CompVWindow::beginDraw) -> CompVGLWindow
	// Override(CompVWindow::endDraw) -> CompVGLWindow
	// Override(CompVWindow::surface) -> CompVGLWindow

	static COMPV_ERROR_CODE newObj(CompVWindowSDLPtrPtr sdlWindow, size_t width, size_t height, const char* title);

protected:
	/* CompVGLWindow overrides */
	virtual CompVGLContextPtr context();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_pSDLContext;
	CompVGLContextSDLPtr m_ptrContext;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_SDL_H */

#endif /* _COMPV_DRAWING_WINDOW_SDL_H_ */
