/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_GLFW_H_)
#define _COMPV_DRAWING_WINDOW_GLFW_H_

#include "compv/drawing/compv_drawing_config.h"
#if defined(HAVE_GLFW_GLFW3_H)
#include "compv/drawing/compv_drawing_common.h"
#include "compv/ui/compv_window.h"
#include "compv/ui/opengl/compv_program.h"
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_mutex.h"

#include <string>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

struct GLFWwindow;

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GLWindowFW3)

class CompVGLWindowFW3 : public CompVWindow
{
protected:
	CompVGLWindowFW3(int width, int height, const char* title);
public:
	virtual ~CompVGLWindowFW3();
	COMPV_OBJECT_GET_ID(CompVGLWindowFW3);
	
	COMPV_INLINE struct GLFWwindow * getGLFWwindow() { return m_pGLFWwindow; }

	virtual bool isClosed()const;
	virtual COMPV_ERROR_CODE close();
	virtual COMPV_ERROR_CODE draw(CompVMatPtr mat);

	static COMPV_ERROR_CODE newObj(CompVGLWindowFW3PtrPtr glfwWindow, int width, int height, const char* title);
	static void GLFWwindowcloseCallback(GLFWwindow* window);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	struct GLFWwindow *m_pGLFWwindow;
	CompVMutexPtr m_GLFWMutex;
	CompVProgramPtr m_Program;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_GLFW_GLFW3_H */

#endif /* _COMPV_DRAWING_WINDOW_GLFW_H_ */
