/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/drawing/compv_window.h"
#include "compv/base/drawing/compv_window_registry.h"

COMPV_NAMESPACE_BEGIN()

compv_window_id_t CompVWindow::s_nWindowId = 0;
compv_windowlistener_id_t CompVWindowListener::s_nWindowListenerId = 0;

CompVWindow::CompVWindow(size_t width, size_t height, const char* title /*= "Unknown"*/)
    : m_nWidth(width)
    , m_nHeight(height)
    , m_strTitle(title)
    , m_nId(compv_atomic_add(&CompVWindow::s_nWindowId, 1))
{
    m_WindowCreationThreadId = CompVThread::idCurrent();
    COMPV_DEBUG_INFO("Creating window (%s) on thread with id = %ld", title, (long)m_WindowCreationThreadId);
}

CompVWindow::~CompVWindow()
{
    m_mapListeners.clear();
    COMPV_CHECK_CODE_ASSERT(unregister());
}

COMPV_ERROR_CODE CompVWindow::unregister()
{
    COMPV_CHECK_CODE_RETURN(CompVWindowRegistry::remove(m_nId));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindow::addListener(CompVWindowListenerPtr listener)
{
    COMPV_CHECK_EXP_RETURN(!listener, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_mapListeners[listener->id()] = listener;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindow::removeListener(CompVWindowListenerPtr listener)
{
    COMPV_CHECK_EXP_RETURN(!listener, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_mapListeners.erase(listener->id());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindow::newObj(CompVWindowPtrPtr window, size_t width, size_t height, const char* title COMPV_DEFAULT("Unknown"))
{
	COMPV_CHECK_EXP_RETURN(!window, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVWindowPrivPtr window_;
    COMPV_CHECK_CODE_RETURN(CompVWindowFactory::newObj(&window_, width, height, title));
    COMPV_CHECK_CODE_RETURN(CompVWindowRegistry::add(window_));
	*window = *window_;
    return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVWindowFactory
//
const CompVWindowFactory* CompVWindowFactory::instance = NULL;

COMPV_NAMESPACE_END()

