/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ui/compv_window.h"
#include "compv/ui/compv_ui.h"

#if HAVE_GLFW
#include <GLFW/glfw3.h>
#endif /* HAVE_GLFW */

COMPV_NAMESPACE_BEGIN()

compv_window_id_t CompVWindow::m_sWindowId = 0;

CompVWindow::CompVWindow(int width, int height, const char* title /*= "Unknown"*/)
: m_nWidth(width)
, m_nHeight(height)
, m_strTitle(title)
, m_Id(compv_atomic_inc(&CompVWindow::m_sWindowId))
{
	m_WindowCreationThreadId = CompVThread::getIdCurrent();
#if HAVE_GLFW
    COMPV_DEBUG_INFO("Creating window on thread with id = %ld", (long)CompVThread::getIdCurrent());
#   if COMPV_OS_APPLE
    if (!pthread_main_np()) {
        COMPV_DEBUG_WARN("MacOS: Creating window outside main thread");
    }
#   endif /* COMPV_OS_APPLE */
	m_pGLFWwindow = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!m_pGLFWwindow) {
		COMPV_DEBUG_ERROR("glfwCreateWindow(%d, %d, %s) failed", width, height, title);
		return;
	}
    CompVThread::newObj(&m_GLFWThread, CompVWindow::GLFWThread, this);
#else
	COMPV_DEBUG_INFO("GLFW not enabled. No window will be created.")
#endif /* HAVE_GLFW */

	COMPV_CHECK_CODE_ASSERT(CompVUI::registerWindow(this));
}

CompVWindow::~CompVWindow()
{
#if HAVE_GLFW
	if (m_pGLFWwindow) {
		glfwDestroyWindow(m_pGLFWwindow);
	}
#endif /* HAVE_GLFW */

	COMPV_CHECK_CODE_ASSERT(CompVUI::unregisterWindow(m_Id)); // do not use "this" in the destructor (issue with reference counting which is equal to zero)
    m_GLFWThread = NULL;
}

COMPV_ERROR_CODE CompVWindow::close()
{
    if (m_pGLFWwindow) {
        glfwSetWindowShouldClose(m_pGLFWwindow, GLFW_TRUE);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindow::newObj(CompVPtr<CompVWindow*>* window, int width, int height, const char* title /*= "Unknown"*/)
{
	COMPV_CHECK_CODE_RETURN(CompVUI::init());
	COMPV_CHECK_EXP_RETURN(window == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVWindow*> window_ = new CompVWindow(width, height, title);
	COMPV_CHECK_EXP_RETURN(!window_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
#if HAVE_GLFW
	COMPV_CHECK_EXP_RETURN(!window_->m_pGLFWwindow, COMPV_ERROR_CODE_E_GLFW);
    COMPV_CHECK_EXP_RETURN(!window_->m_GLFWThread, COMPV_ERROR_CODE_E_SYSTEM);
#endif
	*window = window_;
	return COMPV_ERROR_CODE_S_OK;
}

#if HAVE_GLFW
void* CompVWindow::GLFWThread(void* arg)
{
    CompVWindow* This = static_cast<CompVWindow*>(arg);
    
    glfwMakeContextCurrent(This->m_pGLFWwindow);
    glfwSwapInterval(1);
    
    while (!glfwWindowShouldClose(This->m_pGLFWwindow)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor((GLclampf)(rand() % 255) / 255.f,
                     (GLclampf)(rand() % 255) / 255.f,
                     (GLclampf)(rand() % 255) / 255.f,
                     (GLclampf)(rand() % 255) / 255.f);
        glfwSwapBuffers(This->m_pGLFWwindow);
    }
    
    COMPV_CHECK_CODE_ASSERT(CompVUI::unregisterWindow(This));
    
    COMPV_DEBUG_INFO("GLFWThread, windowId = %ld exited", This->m_Id);
    
    return NULL;
}
#endif /* HAVE_GLFW */

COMPV_NAMESPACE_END()

