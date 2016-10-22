/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ui/compv_window.h"
#include "compv/ui/compv_ui.h"

#include "compv/ui/opengl/compv_window_glfw3.h"

COMPV_NAMESPACE_BEGIN()

compv_window_id_t CompVWindow::s_WindowId = 0;

CompVWindow::CompVWindow(int width, int height, const char* title /*= "Unknown"*/)
: m_nWidth(width)
, m_nHeight(height)
, m_strTitle(title)
, m_Id(compv_atomic_inc(&CompVWindow::s_WindowId))
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
	COMPV_CHECK_CODE_ASSERT(CompVUI::unregisterWindow(m_Id)); // do not use "this" in the destructor (issue with reference counting which is equal to zero)
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindow::newObj(CompVWindowPtrPtr window, int width, int height, const char* title /*= "Unknown"*/)
{
	COMPV_CHECK_CODE_RETURN(CompVUI::init());
	COMPV_CHECK_EXP_RETURN(window == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
#if defined(HAVE_GLFW_GLFW3_H)
	CompVWindowGLFW3Ptr glfw3Window;
	COMPV_CHECK_CODE_RETURN(CompVWindowGLFW3::newObj(&glfw3Window, width, height, title));
	*window = dynamic_cast<CompVWindow*>(*glfw3Window);
#endif /* HAVE_GLFW_GLFW3_H */

	COMPV_CHECK_EXP_RETURN(!*window, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);	
	COMPV_CHECK_CODE_RETURN(CompVUI::registerWindow(*window));

	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()

