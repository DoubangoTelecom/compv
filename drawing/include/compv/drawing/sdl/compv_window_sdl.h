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
#include "compv/base/compv_common.h"
#include "compv/drawing/compv_window.h"
#include "compv/drawing/compv_program.h"
#include "compv/drawing/opengl/compv_headers_gl.h" // FIXME(dmi): remove
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_mutex.h"

#include <string>

#include <SDL.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVWindowSDL;
typedef CompVPtr<CompVWindowSDL* > CompVWindowSDLPtr;
typedef CompVWindowSDLPtr* CompVWindowSDLPtrPtr;

class CompVWindowSDL : public CompVWindow
{
protected:
	CompVWindowSDL(int width, int height, const char* title);
public:
	virtual ~CompVWindowSDL();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindowSDL";
	};

	COMPV_INLINE struct SDL_Window * getSDLWwindow()const { return m_pSDLWindow; }
	
	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	virtual COMPV_ERROR_CODE test(CompVMatPtr mat);
	virtual COMPV_ERROR_CODE beginDraw();
	virtual COMPV_ERROR_CODE endDraw();
	virtual COMPV_ERROR_CODE drawImage(CompVMatPtr mat);
	virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes);

	static COMPV_ERROR_CODE newObj(CompVWindowSDLPtrPtr sdlWindow, int width, int height, const char* title);
	static int FilterEvents(void *userdata, SDL_Event * event);

protected:
	virtual CompVGLContext getGLContext()const { return static_cast<CompVGLContext>(m_pSDLContext); }

private:
	COMPV_ERROR_CODE makeGLContextCurrent();
	COMPV_ERROR_CODE unmakeGLContextCurrent();

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_pSDLContext;
	CompVMutexPtr m_ptrSDLMutex;
	CompVProgramPtr m_ptrProgram;
	CompVSurfacePtr m_ptrSurface;
	bool m_bDrawing;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_SDL_H */

#endif /* _COMPV_DRAWING_WINDOW_SDL_H_ */
