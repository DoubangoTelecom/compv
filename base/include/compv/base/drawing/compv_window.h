/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_WINDOW_H_)
#define _COMPV_BASE_DRAWING_WINDOW_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/drawing/compv_surface.h"
#include "compv/base/drawing/compv_surfacelayer.h"
#include "compv/base/drawing/compv_surfacelayer_single.h"
#include "compv/base/drawing/compv_surfacelayer_matching.h"
#include "compv/base/drawing/compv_surfacelayer_multi.h"

#include <string>
#include <map>
#if COMPV_OS_ANDROID
#include <jni.h>
#endif /*COMPV_OS_ANDROID*/

COMPV_NAMESPACE_BEGIN()

typedef long compv_window_id_t;
typedef long compv_windowlistener_id_t;

enum COMPV_WINDOW_STATE {
	COMPV_WINDOW_STATE_NONE,
	COMPV_WINDOW_STATE_FOCUS_LOST,
	COMPV_WINDOW_STATE_FOCUS_GAINED,
	COMPV_WINDOW_STATE_CONTEXT_CREATED,
	COMPV_WINDOW_STATE_CONTEXT_DESTROYED,
	COMPV_WINDOW_STATE_CLOSED,
};


//
//	CompVWindow
//

COMPV_OBJECT_DECLARE_PTRS(WindowListener)

class COMPV_BASE_API CompVWindowListener : public CompVObj
{
protected:
    CompVWindowListener(): m_nId(compv_atomic_inc(&CompVWindowListener::s_nWindowListenerId)) { }
public:
    virtual ~CompVWindowListener() { }
	COMPV_OBJECT_GET_ID(CompVWindowListener);
    COMPV_INLINE compv_windowlistener_id_t id()const {
        return m_nId;
    }
    virtual COMPV_ERROR_CODE onSizeChanged(size_t newWidth, size_t newHeight) {
        return COMPV_ERROR_CODE_S_OK;
    };
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_WINDOW_STATE newState) {
		return COMPV_ERROR_CODE_S_OK;
	};

private:
    compv_windowlistener_id_t m_nId;
    static compv_windowlistener_id_t s_nWindowListenerId;
};

//
//	CompVWindow
//
COMPV_OBJECT_DECLARE_PTRS(Window)

class COMPV_BASE_API CompVWindow : public CompVObj
{
protected:
    CompVWindow(size_t width, size_t height, const char* title = "Unknown");
public:
    virtual ~CompVWindow();

    COMPV_INLINE size_t width()const {
        return m_nWidth;
    }
    COMPV_INLINE size_t height()const {
        return m_nHeight;
    }
    COMPV_INLINE const char* getTitle()const {
        return m_strTitle.c_str();
    }
    COMPV_INLINE compv_window_id_t getId()const {
        return m_nId;
    }
    COMPV_INLINE compv_thread_id_t getWindowCreationThreadId()const {
        return m_WindowCreationThreadId;
    }
    COMPV_INLINE const std::map<compv_windowlistener_id_t, CompVWindowListenerPtr>& listeners()const {
        return m_mapListeners;
    }

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

#if COMPV_OS_ANDROID
	virtual COMPV_ERROR_CODE attachToSurface(JNIEnv* jniEnv, jobject javaSurface) = 0;
#endif /*COMPV_OS_ANDROID*/

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
COMPV_OBJECT_DECLARE_PTRS(WindowPriv)

class COMPV_BASE_API CompVWindowPriv : public CompVWindow
{
protected:
    CompVWindowPriv(size_t width, size_t height, const char* title = "Unknown") : CompVWindow(width, height, title) { }
public:
    virtual ~CompVWindowPriv() {  }
    virtual COMPV_ERROR_CODE priv_updateSize(size_t newWidth, size_t newHeight) = 0;
	virtual COMPV_ERROR_CODE priv_updateState(COMPV_WINDOW_STATE newState) = 0;
};

//
//	CompVWindowFactory
//
struct COMPV_BASE_API CompVWindowFactory {
public:
    const char* name;
    COMPV_ERROR_CODE(*newObjFuncPtr)(CompVWindowPrivPtrPtr window, size_t width, size_t height, const char* title);
    static COMPV_ERROR_CODE set(const CompVWindowFactory* inst) {
        COMPV_CHECK_EXP_RETURN(!inst, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_DEBUG_INFO("Setting window factory: %s", inst->name);
        instance = inst;
        return COMPV_ERROR_CODE_S_OK;
    }
    static COMPV_ERROR_CODE newObj(CompVWindowPrivPtrPtr window, size_t width, size_t height, const char* title = "Unknown") {
        COMPV_CHECK_EXP_RETURN(!instance, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
        COMPV_CHECK_CODE_RETURN(instance->newObjFuncPtr(window, width, height, title));
        return COMPV_ERROR_CODE_S_OK;
    }
private:
    static const CompVWindowFactory* instance;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_WINDOW_H_ */
