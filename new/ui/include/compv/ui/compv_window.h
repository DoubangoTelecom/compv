/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_UI_WINDOW_H_)
#define _COMPV_UI_WINDOW_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/parallel/compv_thread.h"

#include <string>

struct GLFWwindow;

COMPV_NAMESPACE_BEGIN()

class COMPV_UI_API CompVWindow : public CompVObj
{
protected:
	CompVWindow(int width, int height, const char* title = "Unknown");
public:
	virtual ~CompVWindow();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindow";
	};

	static COMPV_ERROR_CODE newObj(CompVPtr<CompVWindow*>* window, int width, int height, const char* title = "Unknown");
#if HAVE_GLFW
	static void* COMPV_STDCALL GLFWwindowThread(void *arg);
#endif

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	int m_nWidth;
	int m_nHeight;
	std::string m_strTitle;
#if HAVE_GLFW
	struct GLFWwindow *m_pGLFWwindow;
	CompVPtr<CompVThread* > m_GLFWwindowThread;
#endif
	COMPV_DISABLE_WARNINGS_END()
};

typedef CompVPtr<CompVWindow* > CompVWindowPtr;

COMPV_NAMESPACE_END()

#endif /* _COMPV_UI_WINDOW_H_ */
