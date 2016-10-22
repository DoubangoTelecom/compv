/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ui/opengl/compv_window_glfw3.h"
#if defined(HAVE_GLFW_GLFW3_H)
#include "compv/ui/compv_ui.h"

#if defined(HAVE_GLFW_GLFW3_H)
#	include <GLFW/glfw3.h>
#endif /* HAVE_GLFW_GLFW3_H */

COMPV_NAMESPACE_BEGIN()

CompVWindowGLFW3::CompVWindowGLFW3(int width, int height, const char* title)
: CompVWindow(width, height, title)
{
#   if COMPV_OS_APPLE
	if (!pthread_main_np()) {
		COMPV_DEBUG_WARN("MacOS: Creating window outside main thread");
	}
#   endif /* COMPV_OS_APPLE */
	m_pGLFWwindow = glfwCreateWindow(width, height, title, NULL, CompVUI::getGLFWWindow());
	if (!m_pGLFWwindow) {
		COMPV_DEBUG_ERROR("glfwCreateWindow(%d, %d, %s) failed", width, height, title);
		return;
	}
	COMPV_CHECK_CODE_ASSERT(CompVMutex::newObj(&m_GLFWMutex));
	glfwSetWindowUserPointer(m_pGLFWwindow, this);
	glfwSetWindowCloseCallback(m_pGLFWwindow, CompVWindowGLFW3::GLFWwindowcloseCallback);
	glfwMakeContextCurrent(m_pGLFWwindow);
	glfwSwapInterval(1);
	glfwMakeContextCurrent(NULL);
}

CompVWindowGLFW3::~CompVWindowGLFW3()
{
	COMPV_CHECK_CODE_ASSERT(close());
	m_GLFWMutex = NULL;
}

bool CompVWindowGLFW3::isClosed()
{
	return !m_pGLFWwindow;
}

COMPV_ERROR_CODE CompVWindowGLFW3::close()
{
	COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->lock());
	if (m_pGLFWwindow) {
		glfwSetWindowShouldClose(m_pGLFWwindow, GLFW_TRUE);
		glfwDestroyWindow(m_pGLFWwindow);
		m_pGLFWwindow = NULL;
	}
	COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->unlock());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowGLFW3::draw()
{
	COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->lock());
	if (!m_pGLFWwindow) {
		COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->unlock());
		// COMPV_DEBUG_INFO("Window closed. Ignoring draw() instruction");
		return COMPV_ERROR_CODE_W_WINDOW_CLOSED;
	}
	if (!glfwWindowShouldClose(m_pGLFWwindow)) {
		glfwMakeContextCurrent(m_pGLFWwindow);

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor((GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f);

		glfwSwapBuffers(m_pGLFWwindow);
		glfwMakeContextCurrent(NULL);
	}
	COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowGLFW3::newObj(CompVWindowGLFW3PtrPtr glfwWindow, int width, int height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVUI::init());
	COMPV_CHECK_EXP_RETURN(glfwWindow == NULL || width <= 0 || height <= 0 || !title || !::strlen(title), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowGLFW3Ptr glfwWindow_ = new CompVWindowGLFW3(width, height, title);
	COMPV_CHECK_EXP_RETURN(!glfwWindow_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(!glfwWindow_->m_pGLFWwindow, COMPV_ERROR_CODE_E_GLFW);
	COMPV_CHECK_EXP_RETURN(!glfwWindow_->m_GLFWMutex, COMPV_ERROR_CODE_E_SYSTEM);
	*glfwWindow = glfwWindow_;
	return COMPV_ERROR_CODE_S_OK;
}

void CompVWindowGLFW3::GLFWwindowcloseCallback(GLFWwindow* window)
{
	CompVWindowGLFW3Ptr This = static_cast<CompVWindowGLFW3*>(glfwGetWindowUserPointer(window));
	COMPV_DEBUG_INFO("GLFWwindowcloseCallback(Id=%ld, Title=%s)", This->getId(), This->getTitle());
	COMPV_ASSERT(window == This->m_pGLFWwindow);
	COMPV_CHECK_CODE_ASSERT(This->m_GLFWMutex->lock());
	COMPV_CHECK_CODE_ASSERT(This->unregister());
	glfwSetWindowUserPointer(This->m_pGLFWwindow, NULL);
	glfwDestroyWindow(This->m_pGLFWwindow);
	This->m_pGLFWwindow = NULL;
	COMPV_CHECK_CODE_ASSERT(This->m_GLFWMutex->unlock());
}

COMPV_NAMESPACE_END()

#endif /* HAVE_GLFW_GLFW3_H */
