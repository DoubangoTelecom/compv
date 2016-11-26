/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/gl/compv_window_glfw3.h"
#if defined(HAVE_GLFW_GLFW3_H)
#error "GLFW is deprecated and replaced with SDL"
#include "compv/drawing/compv_drawing.h"
#include "compv/gl/compv_gl_headers.h"

#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && _MSC_VER >= 1900
#	pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

COMPV_NAMESPACE_BEGIN()

CompVWindowGLFW3::CompVWindowGLFW3(int width, int height, const char* title)
: CompVWindow(width, height, title)
, m_pGLFWwindow(NULL)
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
	COMPV_glfwSetWindowUserPointer(m_pGLFWwindow, this);
	COMPV_glfwSetWindowCloseCallback(m_pGLFWwindow, CompVWindowGLFW3::GLFWwindowcloseCallback);
	COMPV_glfwMakeContextCurrent(m_pGLFWwindow);
	COMPV_glfwSwapInterval(1);
	COMPV_glfwMakeContextCurrent(NULL);
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
		COMPV_glfwSetWindowShouldClose(m_pGLFWwindow, GLFW_TRUE);
		COMPV_glfwDestroyWindow(m_pGLFWwindow);
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
		COMPV_glfwMakeContextCurrent(m_pGLFWwindow);
#if 0
		COMPV_glClear(GL_COLOR_BUFFER_BIT);
		COMPV_glClearColor((GLclampf)(rand() % 255) / 255.f,
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
			COMPV_glGenTextures(1, &tex);
			/* OpenGL-2 or later is assumed; OpenGL-2 supports NPOT textures. */
			COMPV_glBindTexture(GL_TEXTURE_2D, tex);
			COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			COMPV_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			if ((mat->stride() & 3)) { // multiple of 4?
				COMPV_glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)mat->stride());
				COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			}

			COMPV_glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB,
				static_cast<GLsizei>(mat->stride()),
				static_cast<GLsizei>(mat->rows()),
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				NULL);
			COMPV_glBindTexture(GL_TEXTURE_2D, 0);
		}

		COMPV_glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		COMPV_glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
		COMPV_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		COMPV_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		COMPV_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		COMPV_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		COMPV_glBindTexture(GL_TEXTURE_2D, tex);
		COMPV_glTexSubImage2D(
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

		COMPV_glClearColor(0.f, 0.f, 0.f, 1.f);
		int width, height;
		COMPV_glfwGetFramebufferSize(m_pGLFWwindow, &width, &height);
		COMPV_glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		COMPV_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		COMPV_glEnable(GL_DEPTH_TEST);
		COMPV_glMatrixMode(GL_PROJECTION);
		COMPV_glLoadIdentity();
		COMPV_gluOrtho2D((GLdouble)0, static_cast<GLdouble>(mat->stride()), static_cast<GLdouble>(mat->rows()), (GLdouble)0); // glOrtho((GLdouble)0, (GLdouble)m_nWidth, (GLdouble)m_nWidth, (GLdouble)0, (GLdouble)-1, (GLdouble)1);
		COMPV_glMatrixMode(GL_MODELVIEW);
		COMPV_glLoadIdentity();

		COMPV_glBegin(GL_QUADS);
		COMPV_glTexCoord2i(0, 0);
		COMPV_glVertex2i(0, 0);
		COMPV_glTexCoord2i(0, 1);
		COMPV_glVertex2i(0, static_cast<GLint>(mat->rows()));
		COMPV_glTexCoord2i(1, 1);
		COMPV_glVertex2i(static_cast<GLint>(mat->stride()), static_cast<GLint>(mat->rows()));
		COMPV_glTexCoord2i(1, 0);
		COMPV_glVertex2i(static_cast<GLint>(mat->stride()), 0);
		COMPV_glEnd();

		COMPV_CHECK_CODE_ASSERT(m_Program->useEnd());

		COMPV_glBindTexture(GL_TEXTURE_2D, 0);
#endif

		COMPV_glfwSwapBuffers(m_pGLFWwindow);
		COMPV_glfwMakeContextCurrent(NULL);
	}
	COMPV_CHECK_CODE_ASSERT(m_GLFWMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowGLFW3::newObj(CompVWindowGLFW3PtrPtr glfwWindow, int width, int height, const char* title)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
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
	COMPV_glfwSetWindowUserPointer(This->m_pGLFWwindow, NULL);
	COMPV_glfwDestroyWindow(This->m_pGLFWwindow);
	This->m_pGLFWwindow = NULL;
	COMPV_CHECK_CODE_ASSERT(This->m_GLFWMutex->unlock());
}

COMPV_NAMESPACE_END()

#endif /* HAVE_GLFW_GLFW3_H */
