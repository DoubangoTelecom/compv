/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_UI_OPENGL_WINDOW_SDL_H_)
#define _COMPV_UI_OPENGL_WINDOW_SDL_H_

#include "compv/base/compv_config.h"
#if defined(HAVE_SDL_H)
#include "compv/base/compv_common.h"
#include "compv/ui/compv_window.h"
#include "compv/ui/opengl/compv_program.h"
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

	COMPV_INLINE struct SDL_Window * getSDLWwindow() { return m_pSDLWindow; }

	virtual bool isClosed();
	virtual COMPV_ERROR_CODE close();
	virtual COMPV_ERROR_CODE draw(CompVMatPtr mat);

	static COMPV_ERROR_CODE newObj(CompVWindowSDLPtrPtr sdlWindow, int width, int height, const char* title);
	static int FilterEvents(void *userdata, SDL_Event * event);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_pSDLContext;
	CompVMutexPtr m_SDLMutex;
	CompVProgramPtr m_Program;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_GLFW_GLFW3_H */

#endif /* _COMPV_UI_OPENGL_WINDOW_SDL_H_ */
