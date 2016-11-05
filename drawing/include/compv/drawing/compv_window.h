/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_WINDOW_H_)
#define _COMPV_DRAWING_WINDOW_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"
#include "compv/base/parallel/compv_thread.h"
#include "compv/drawing/compv_surface.h"

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

	COMPV_INLINE int getWidth()const { return m_nWidth; }
	COMPV_INLINE int getHeight()const { return m_nHeight; }
	COMPV_INLINE const char* getTitle()const { return m_strTitle.c_str(); }
	COMPV_INLINE compv_window_id_t getId()const { return m_nId; }
	COMPV_INLINE compv_thread_id_t getWindowCreationThreadId()const { return m_WindowCreationThreadId; }
	COMPV_INLINE bool isGLEnabled()const { return getGLContext() != NULL; }
    
	virtual bool isClosed()const = 0;
	
    virtual COMPV_ERROR_CODE close() = 0;
	virtual COMPV_ERROR_CODE beginDraw() = 0;
	virtual COMPV_ERROR_CODE endDraw() = 0;
	virtual CompVSurfacePtr surface() = 0;
	static COMPV_ERROR_CODE newObj(CompVWindowPtrPtr window, int width, int height, const char* title = "Unknown");

protected:
	virtual CompVGLContext getGLContext()const = 0;
	COMPV_ERROR_CODE unregister();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_window_id_t s_nWindowId;
	compv_thread_id_t m_WindowCreationThreadId;
	int m_nWidth;
	int m_nHeight;
	std::string m_strTitle;
	compv_window_id_t m_nId;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_WINDOW_H_ */
