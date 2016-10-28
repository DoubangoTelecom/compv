/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_window.h"
#include "compv/drawing/compv_drawing.h"

#include "compv/drawing/opengl/compv_window_glfw3.h"
#include "compv/drawing/opengl/compv_window_sdl.h"

COMPV_NAMESPACE_BEGIN()

compv_window_id_t CompVWindow::s_nWindowId = 0;

CompVWindow::CompVWindow(int width, int height, const char* title /*= "Unknown"*/)
: m_nWidth(width)
, m_nHeight(height)
, m_strTitle(title)
, m_nId(compv_atomic_inc(&CompVWindow::s_nWindowId))
{
	m_WindowCreationThreadId = CompVThread::getIdCurrent();
	COMPV_DEBUG_INFO("Creating window (%s) on thread with id = %ld", title, (long)m_WindowCreationThreadId);
}

CompVWindow::~CompVWindow()
{
	COMPV_CHECK_CODE_ASSERT(unregister());
}

COMPV_ERROR_CODE CompVWindow::unregister()
{
	COMPV_CHECK_CODE_ASSERT(CompVDrawing::unregisterWindow(m_nId)); // do not use "this" in the destructor (issue with reference counting which is equal to zero)
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindow::newObj(CompVWindowPtrPtr window, int width, int height, const char* title /*= "Unknown"*/)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!window || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*window = NULL;
#if defined(HAVE_GLFW_GLFW3_H)
	if (!*window) {
		CompVWindowGLFW3Ptr glfw3Window;
		COMPV_CHECK_CODE_RETURN(CompVWindowGLFW3::newObj(&glfw3Window, width, height, title));
		*window = dynamic_cast<CompVWindow*>(*glfw3Window);
	}
#endif /* HAVE_GLFW_GLFW3_H */

#if defined(HAVE_SDL_H)
	if (!*window) {
		CompVWindowSDLPtr sdlWindow;
		COMPV_CHECK_CODE_RETURN(CompVWindowSDL::newObj(&sdlWindow, width, height, title));
		*window = dynamic_cast<CompVWindow*>(*sdlWindow);
	}
#endif /* HAVE_GLFW_GLFW3_H */

	COMPV_CHECK_EXP_RETURN(!*window, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);	
	COMPV_CHECK_CODE_RETURN(CompVDrawing::registerWindow(*window));

	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()

