/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_window.h"
#include "compv/drawing/compv_drawing.h"

#include "compv/drawing/opengl/compv_window_glfw3.h"
#include "compv/drawing/opengl/compv_window_egl.h"
#include "compv/drawing/sdl/compv_window_sdl.h"
#include "compv/drawing/android/compv_window_android_egl.h"

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
	CompVWindowPtr window_;
	
	/* Create GLFW window (Deprecated) */
#if defined(HAVE_GLFW_GLFW3_H)
#error "GLFW is depreacted"
	if (!window_) {
		CompVWindowGLFW3Ptr glfw3Window;
		COMPV_CHECK_CODE_RETURN(CompVWindowGLFW3::newObj(&glfw3Window, width, height, title));
		window_ = dynamic_cast<CompVWindow*>(*glfw3Window);
	}
#endif /* HAVE_GLFW_GLFW3_H */

	/* Create SDL window */
#if defined(HAVE_SDL_H)
	if (!window_) {
		CompVWindowSDLPtr sdlWindow;
		COMPV_CHECK_CODE_RETURN(CompVWindowSDL::newObj(&sdlWindow, width, height, title));
		window_ = dynamic_cast<CompVWindow*>(*sdlWindow);
	}
#endif /* HAVE_SDL_H */

	/* Create Android EGL window */
#if COMPV_OS_ANDROID && defined(HAVE_EGL)
	if (!window_) {
		CompVWindowAndroidEGLPtr eglWindow;
		COMPV_CHECK_CODE_RETURN(CompVWindowAndroidEGL::newObj(&eglWindow, width, height, title));
		window_ = dynamic_cast<CompVWindow*>(*eglWindow);
	}
#endif /* HAVE_SDL_H */
	
	// Set output and check pointer validity
	COMPV_CHECK_EXP_RETURN(!(*window = window_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	// Register the newly created window
	COMPV_CHECK_CODE_RETURN(CompVDrawing::registerWindow(window_));

	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()

