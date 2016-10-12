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

CompVWindow::CompVWindow(int width, int height, const char* title /*= "Unknown"*/)
: m_nWidth(width)
, m_nHeight(height)
, m_strTitle(title)
{
#if HAVE_GLFW
	//m_pGLFWwindow = glfwCreateWindow(width, height, title, NULL, NULL);
	//if (!m_pGLFWwindow) {
	//	COMPV_DEBUG_ERROR("glfwCreateWindow(%d, %d, %s) failed", width, height, title);
	//	return;
	//}
	COMPV_CHECK_CODE_ASSERT(CompVThread::newObj(&m_GLFWwindowThread, CompVWindow::GLFWwindowThread, this));
#else
	COMPV_DEBUG_INFO("GLFW not enabled. No window will be created.")
#endif /* HAVE_GLFW */
}

CompVWindow::~CompVWindow()
{
#if HAVE_GLFW
	if (m_pGLFWwindow) {
		glfwDestroyWindow(m_pGLFWwindow);
	}
	m_GLFWwindowThread = NULL;
#endif /* HAVE_GLFW */
}

COMPV_ERROR_CODE CompVWindow::newObj(CompVPtr<CompVWindow*>* window, int width, int height, const char* title /*= "Unknown"*/)
{
	COMPV_CHECK_CODE_RETURN(CompVUI::init());
	COMPV_CHECK_EXP_RETURN(window == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVWindow*> window_ = new CompVWindow(width, height, title);
	COMPV_CHECK_EXP_RETURN(!window_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
#if HAVE_GLFW
	//COMPV_CHECK_EXP_RETURN(!window_->m_pGLFWwindow, COMPV_ERROR_CODE_E_GLFW);
	COMPV_CHECK_EXP_RETURN(!window_->m_GLFWwindowThread, COMPV_ERROR_CODE_E_SYSTEM);
#endif
	*window = window_;
	return COMPV_ERROR_CODE_S_OK;
}

#if HAVE_GLFW
void* COMPV_STDCALL CompVWindow::GLFWwindowThread(void *arg)
{
	CompVWindow* This = static_cast<CompVWindow*>(arg); //!\\ Must not take a reference to the object

	This->m_pGLFWwindow = glfwCreateWindow(This->m_nWidth, This->m_nHeight, This->m_strTitle.c_str(), NULL, NULL);
	if (!This->m_pGLFWwindow) {
		COMPV_DEBUG_ERROR("glfwCreateWindow(%d, %d, %s) failed", This->m_nWidth, This->m_nHeight, This->m_strTitle.c_str());
		return NULL;
	}
	glfwMakeContextCurrent(This->m_pGLFWwindow);
	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(This->m_pGLFWwindow)) {
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor((GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f);

		/* Swap buffers and render */
		glfwSwapBuffers(This->m_pGLFWwindow);
		glfwPollEvents();
	}
	COMPV_DEBUG_INFO("Window with title '%s' closed !!", This->m_strTitle.c_str());
	return NULL;
}
#endif /* HAVE_GLFW */

COMPV_NAMESPACE_END()

