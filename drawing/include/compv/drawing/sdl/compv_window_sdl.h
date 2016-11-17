/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_SDL_H_)
#define _COMPV_DRAWING_WINDOW_SDL_H_

#include "compv/base/compv_config.h"
#if defined(HAVE_SDL_H)
#include "compv/drawing/opengl/compv_window_gl.h"
#include "compv/base/compv_common.h"

#include <SDL.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVContextGLSDL
//
class CompVContextGLSDL;
typedef CompVPtr<CompVContextGLSDL* > CompVContextGLSDLPtr;
typedef CompVContextGLSDLPtr* CompVContextGLSDLPtrPtr;

class CompVContextGLSDL : public CompVContextGL
{
protected:
	CompVContextGLSDL(SDL_Window *pSDLWindow, SDL_GLContext pSDLContext);
public:
	virtual ~CompVContextGLSDL();
	COMPV_GET_OBJECT_ID("CompVContextGLSDL");

	virtual COMPV_ERROR_CODE makeCurrent();
	virtual COMPV_ERROR_CODE swabBuffers();
	virtual COMPV_ERROR_CODE unmakeCurrent();

	static COMPV_ERROR_CODE newObj(CompVContextGLSDLPtrPtr context, SDL_Window *pSDLWindow, SDL_GLContext pSDLContext);

private:
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_pSDLContext;
};


//
//	CompVWindowSDL
//
class CompVWindowSDL;
typedef CompVPtr<CompVWindowSDL* > CompVWindowSDLPtr;
typedef CompVWindowSDLPtr* CompVWindowSDLPtrPtr;

class CompVWindowSDL : public CompVWindowGL
{
protected:
	CompVWindowSDL(size_t width, size_t height, const char* title);
public:
	virtual ~CompVWindowSDL();
	COMPV_GET_OBJECT_ID("CompVWindowSDL");
	
	/* CompVWindow overrides */
	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	// Override(CompVWindow::beginDraw) -> CompVWindowGL
	// Override(CompVWindow::endDraw) -> CompVWindowGL
	// Override(CompVWindow::surface) -> CompVWindowGL

	static COMPV_ERROR_CODE newObj(CompVWindowSDLPtrPtr sdlWindow, size_t width, size_t height, const char* title);

protected:
	/* CompVWindowGL overrides */
	virtual CompVContextGLPtr context();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_pSDLContext;
	CompVContextGLSDLPtr m_ptrContext;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_SDL_H */

#endif /* _COMPV_DRAWING_WINDOW_SDL_H_ */
