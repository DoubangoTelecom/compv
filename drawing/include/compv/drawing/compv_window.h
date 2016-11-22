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
#include "compv/drawing/compv_surfacelayer.h"
#include "compv/drawing/compv_surfacelayer_single.h"
#include "compv/drawing/compv_surfacelayer_matching.h"
#include "compv/drawing/compv_surfacelayer_multi.h"

#include <string>
#include <map>

COMPV_NAMESPACE_BEGIN()

typedef long compv_window_id_t;
typedef long compv_windowlistener_id_t;


//
//	CompVWindow
//

class CompVWindowListener;
typedef CompVPtr<CompVWindowListener* > CompVWindowListenerPtr;
typedef CompVWindowListenerPtr* CompVWindowListenerPtrPtr;

class COMPV_DRAWING_API CompVWindowListener : public CompVObj
{
protected:
	CompVWindowListener(): m_nId(compv_atomic_inc(&CompVWindowListener::s_nWindowListenerId)) { }
public:
	virtual ~CompVWindowListener() { }
	COMPV_INLINE compv_windowlistener_id_t id()const { return m_nId; }
	virtual COMPV_ERROR_CODE onSizeChanged(size_t newWidth, size_t newHeight) { return COMPV_ERROR_CODE_S_OK; };

private:
	compv_windowlistener_id_t m_nId;
	static compv_windowlistener_id_t s_nWindowListenerId;
};

//
//	CompVWindow
//
class CompVWindow;
typedef CompVPtr<CompVWindow* > CompVWindowPtr;
typedef CompVWindowPtr* CompVWindowPtrPtr;

class COMPV_DRAWING_API CompVWindow : public CompVObj
{
protected:
	CompVWindow(size_t width, size_t height, const char* title = "Unknown");
public:
	virtual ~CompVWindow();

	COMPV_INLINE size_t width()const { return m_nWidth; }
	COMPV_INLINE size_t height()const { return m_nHeight; }
	COMPV_INLINE const char* getTitle()const { return m_strTitle.c_str(); }
	COMPV_INLINE compv_window_id_t getId()const { return m_nId; }
	COMPV_INLINE compv_thread_id_t getWindowCreationThreadId()const { return m_WindowCreationThreadId; }
	COMPV_INLINE const std::map<compv_windowlistener_id_t, CompVWindowListenerPtr>& listeners()const { return m_mapListeners; }
    
	virtual bool isGLEnabled()const = 0;
	virtual bool isClosed()const = 0;
    virtual COMPV_ERROR_CODE close() = 0;
	virtual COMPV_ERROR_CODE beginDraw() = 0;
	virtual COMPV_ERROR_CODE endDraw() = 0;
	virtual COMPV_ERROR_CODE addSingleLayerSurface(CompVSingleSurfaceLayerPtrPtr layer) = 0;
	virtual COMPV_ERROR_CODE removeSingleLayerSurface(const CompVSingleSurfaceLayerPtr& layer) = 0;
	virtual COMPV_ERROR_CODE addMatchingLayerSurface(CompVMatchingSurfaceLayerPtrPtr layer) = 0;
	virtual COMPV_ERROR_CODE removeMatchingLayerSurface(const CompVMatchingSurfaceLayerPtr& layer) = 0;
	virtual COMPV_ERROR_CODE addMultiLayerSurface(CompVMultiSurfaceLayerPtrPtr layer) = 0;
	virtual COMPV_ERROR_CODE removeMultiLayerSurface(const CompVMultiSurfaceLayerPtr& layer) = 0;

	virtual COMPV_ERROR_CODE addListener(CompVWindowListenerPtr listener);
	virtual COMPV_ERROR_CODE removeListener(CompVWindowListenerPtr listener);
	static COMPV_ERROR_CODE newObj(CompVWindowPtrPtr window, size_t width, size_t height, const char* title = "Unknown");

protected:
	COMPV_ERROR_CODE unregister();

protected:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	size_t m_nWidth;
	size_t m_nHeight;
	std::map<compv_windowlistener_id_t, CompVWindowListenerPtr> m_mapListeners;
	COMPV_VS_DISABLE_WARNINGS_END()

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static compv_window_id_t s_nWindowId;
	compv_thread_id_t m_WindowCreationThreadId;
	std::string m_strTitle;
	compv_window_id_t m_nId;
	COMPV_VS_DISABLE_WARNINGS_END()
};


//
//	CompVWindowPriv
//

class CompVWindowPriv;
typedef CompVPtr<CompVWindowPriv* > CompVWindowPrivPtr;
typedef CompVWindowPrivPtr* CompVWindowPrivPtrPtr;

class CompVWindowPriv : public CompVWindow
{
protected:
	CompVWindowPriv(size_t width, size_t height, const char* title = "Unknown") : CompVWindow(width, height, title) { }
public:
	virtual ~CompVWindowPriv() {  }
	virtual COMPV_ERROR_CODE priv_updateSize(size_t newWidth, size_t newHeight) = 0;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_WINDOW_H_ */
