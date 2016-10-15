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

typedef long compv_window_id_t;

class COMPV_UI_API CompVWindow : public CompVObj
{
protected:
	CompVWindow(int width, int height, const char* title = "Unknown");
public:
	virtual ~CompVWindow();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindow";
	};

	COMPV_INLINE compv_window_id_t getId() { return m_Id; }
	COMPV_INLINE compv_thread_id_t getWindowCreationThreadId() { return m_WindowCreationThreadId; }

	static COMPV_ERROR_CODE newObj(CompVPtr<CompVWindow*>* window, int width, int height, const char* title = "Unknown");
#if HAVE_GLFW
	COMPV_INLINE struct GLFWwindow * getGLFWwindow() { return m_pGLFWwindow; }
#endif

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_window_id_t m_sWindowId;
	compv_thread_id_t m_WindowCreationThreadId;
	int m_nWidth;
	int m_nHeight;
	std::string m_strTitle;
	compv_window_id_t m_Id;
#if HAVE_GLFW
	struct GLFWwindow *m_pGLFWwindow;
#endif
	COMPV_DISABLE_WARNINGS_END()
};

typedef CompVPtr<CompVWindow* > CompVWindowPtr;

COMPV_NAMESPACE_END()

#endif /* _COMPV_UI_WINDOW_H_ */
