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
#include "compv/base/compv_mat.h"
#include "compv/base/parallel/compv_thread.h"

#include <string>

COMPV_NAMESPACE_BEGIN()

typedef long compv_window_id_t;

class CompVWindow;
typedef CompVPtr<CompVWindow* > CompVWindowPtr;
typedef CompVWindowPtr* CompVWindowPtrPtr;

class COMPV_DRAWING_API CompVWindow : public CompVObj
{
protected:
	CompVWindow(int width, int height, const char* title = "Unknown");
public:
	virtual ~CompVWindow();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVWindow";
	};

	COMPV_INLINE int getWidth() { return m_nWidth; }
	COMPV_INLINE int getHeight() { return m_nHeight; }
	COMPV_INLINE const char* getTitle() { return m_strTitle.c_str(); }
	COMPV_INLINE compv_window_id_t getId() { return m_Id; }
	COMPV_INLINE compv_thread_id_t getWindowCreationThreadId() { return m_WindowCreationThreadId; }
    
	virtual bool isClosed() = 0;

    virtual COMPV_ERROR_CODE close() = 0;
	virtual COMPV_ERROR_CODE draw(CompVMatPtr mat) = 0;

	static COMPV_ERROR_CODE newObj(CompVWindowPtrPtr window, int width, int height, const char* title = "Unknown");

protected:
	COMPV_ERROR_CODE unregister();

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_window_id_t s_WindowId;
	compv_thread_id_t m_WindowCreationThreadId;
	int m_nWidth;
	int m_nHeight;
	std::string m_strTitle;
	compv_window_id_t m_Id;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_UI_WINDOW_H_ */
