/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_UI_WINDOW_GLFW_H_)
#define _COMPV_UI_WINDOW_GLFW_H_

#include "compv/base/compv_config.h"
#if HAVE_GLFW
#include "compv/base/compv_common.h"
#include "compv/ui/compv_window.h"
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/parallel/compv_mutex.h"

#include <string>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

struct GLFWwindow;

COMPV_NAMESPACE_BEGIN()

class CompVWindowGLFW;
typedef CompVPtr<CompVWindowGLFW* > CompVWindowGLFWPtr;
typedef CompVWindowGLFWPtr* CompVWindowGLFWPtrPtr;

class CompVWindowGLFW : public CompVWindow
{
protected:
	CompVWindowGLFW(int width, int height, const char* title);
public:
	virtual ~CompVWindowGLFW();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindowGLFW";
	};
	
	COMPV_INLINE struct GLFWwindow * getGLFWwindow() { return m_pGLFWwindow; }

	virtual bool isClosed();
	virtual COMPV_ERROR_CODE close();
	virtual COMPV_ERROR_CODE draw();

	static COMPV_ERROR_CODE newObj(CompVWindowGLFWPtrPtr glfwWindow, int width, int height, const char* title);
	static void GLFWwindowcloseCallback(GLFWwindow* window);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	struct GLFWwindow *m_pGLFWwindow;
	CompVPtr<CompVMutex* > m_GLFWMutex;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* HAVE_GLFW */

#endif /* _COMPV_UI_WINDOW_GLFW_H_ */
