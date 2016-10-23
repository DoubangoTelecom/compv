/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ui/opengl/compv_window_glfw3.h"
#if defined(HAVE_GLFW_GLFW3_H)
#include "compv/ui/compv_ui.h"

#if defined(HAVE_GL_GLEW_H)
#include <GL/glew.h>
#endif /* HAVE_GL_GLEW_H */

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
	m_Program = NULL;
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

COMPV_ERROR_CODE CompVWindowGLFW3::draw(CompVMatPtr mat)
{
	COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ASSERT(mat->subType() == COMPV_MAT_SUBTYPE_PIXELS_R8G8B8);
	COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->lock());
	if (!m_pGLFWwindow) {
		COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->unlock());
		// COMPV_DEBUG_INFO("Window closed. Ignoring draw() instruction");
		return COMPV_ERROR_CODE_W_WINDOW_CLOSED;
	}
	if (!glfwWindowShouldClose(m_pGLFWwindow)) {
		glfwMakeContextCurrent(m_pGLFWwindow);
#if 0
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor((GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f,
			(GLclampf)(rand() % 255) / 255.f);
#else
		if (!m_Program) {
			COMPV_CHECK_CODE_ASSERT(CompVProgram::newObj(&m_Program));
			COMPV_CHECK_CODE_ASSERT(m_Program->shadAttachVertexFile("C:/Projects/GitHub/compv/ui/glsl/test.vert.glsl"));
			COMPV_CHECK_CODE_ASSERT(m_Program->shadAttachFragmentFile("C:/Projects/GitHub/compv/ui/glsl/test.frag.glsl"));
			COMPV_CHECK_CODE_ASSERT(m_Program->link());
		}

		static GLuint tex = 0;
		if (!tex) {
			glGenTextures(1, &tex);
			/* OpenGL-2 or later is assumed; OpenGL-2 supports NPOT textures. */
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			if ((mat->stride() & 3)) { // multiple of 4?
				glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)mat->stride());
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			}

			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB,
				static_cast<GLsizei>(mat->stride()),
				static_cast<GLsizei>(mat->rows()),
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			static_cast<GLsizei>(mat->stride()),
			static_cast<GLsizei>(mat->rows()),
			GL_RGB,
			GL_UNSIGNED_BYTE,
			mat->ptr());

		COMPV_CHECK_CODE_ASSERT(m_Program->useBegin());

		glClearColor(0.f, 0.f, 0.f, 1.f);
		int width, height;
		glfwGetFramebufferSize(m_pGLFWwindow, &width, &height);
		glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D((GLdouble)0, static_cast<GLdouble>(mat->stride()), static_cast<GLdouble>(mat->rows()), (GLdouble)0); // glOrtho((GLdouble)0, (GLdouble)m_nWidth, (GLdouble)m_nWidth, (GLdouble)0, (GLdouble)-1, (GLdouble)1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex2i(0, 0);
		glTexCoord2i(0, 1);
		glVertex2i(0, static_cast<GLint>(mat->rows()));
		glTexCoord2i(1, 1);
		glVertex2i(static_cast<GLint>(mat->stride()), static_cast<GLint>(mat->rows()));
		glTexCoord2i(1, 0);
		glVertex2i(static_cast<GLint>(mat->stride()), 0);
		glEnd();

		COMPV_CHECK_CODE_ASSERT(m_Program->useEnd());

		glBindTexture(GL_TEXTURE_2D, 0);
#endif

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
