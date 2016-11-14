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

class CompVWindowSDL;
typedef CompVPtr<CompVWindowSDL* > CompVWindowSDLPtr;
typedef CompVWindowSDLPtr* CompVWindowSDLPtrPtr;

class CompVWindowSDL : public CompVWindowGL
{
protected:
	CompVWindowSDL(int width, int height, const char* title);
public:
	virtual ~CompVWindowSDL();
	COMPV_GET_OBJECT_ID("CompVWindowSDL");
	
	/* CompVWindow overrides */
	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	// Override(CompVWindow::beginDraw) -> CompVWindowGL
	// Override(CompVWindow::endDraw) -> CompVWindowGL
	// Override(CompVWindow::surface) -> CompVWindowGL

	static COMPV_ERROR_CODE newObj(CompVWindowSDLPtrPtr sdlWindow, int width, int height, const char* title);
	static int FilterEvents(void *userdata, SDL_Event * event);

protected:
	/* CompVWindowGL overrides */
	virtual CompVGLContext getGLContext()const { return static_cast<CompVGLContext>(m_pSDLContext); } // FIXME: CompVWindow override
	virtual COMPV_ERROR_CODE makeGLContextCurrent();
	virtual COMPV_ERROR_CODE unmakeGLContextCurrent();
	virtual COMPV_ERROR_CODE swapGLBuffers();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_pSDLContext;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_SDL_H */

#endif /* _COMPV_DRAWING_WINDOW_SDL_H_ */
