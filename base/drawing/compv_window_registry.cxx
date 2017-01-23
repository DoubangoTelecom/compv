/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/drawing/compv_window_registry.h"
#include "compv/base/android/compv_android_native_activity.h"

COMPV_NAMESPACE_BEGIN()

std::map<compv_window_id_t, CompVWindowPrivPtr > CompVWindowRegistry::m_sWindows;
CompVMutexPtr CompVWindowRegistry::s_WindowsMutex = NULL;
bool CompVWindowRegistry::s_bInitialized = false;


COMPV_ERROR_CODE CompVWindowRegistry::init()
{
    if (s_bInitialized) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_DEBUG_INFO("Initializing window registery");
    COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&s_WindowsMutex));
    s_bInitialized = true;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowRegistry::deInit()
{
    if (!s_bInitialized) {
        return COMPV_ERROR_CODE_S_OK;
    }
	COMPV_CHECK_CODE_NOP(closeAll());
	m_sWindows.clear();
    s_WindowsMutex = NULL;
    s_bInitialized = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowRegistry::add(CompVWindowPrivPtr window)
{
    COMPV_CHECK_EXP_RETURN(!s_bInitialized, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    COMPV_CHECK_EXP_RETURN(!window, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_NOP(s_WindowsMutex->lock());
    m_sWindows.insert(std::pair<compv_window_id_t, CompVWindowPrivPtr >(window->getId(), window));
    COMPV_CHECK_CODE_NOP(s_WindowsMutex->unlock());

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowRegistry::remove(CompVWindowPrivPtr window)
{
	if (s_bInitialized) {
		COMPV_CHECK_CODE_RETURN(remove(window->getId()));
	}
	COMPV_CHECK_EXP_RETURN(!s_bInitialized && !m_sWindows.empty(), COMPV_ERROR_CODE_E_NOT_INITIALIZED, "Window registry not initialized and remove called while it holds entries");
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowRegistry::remove(compv_window_id_t windowId)
{
	if (s_bInitialized) {
		COMPV_CHECK_CODE_NOP(s_WindowsMutex->lock());
		m_sWindows.erase(windowId);
		COMPV_CHECK_CODE_NOP(s_WindowsMutex->unlock());
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!s_bInitialized && !m_sWindows.empty(), COMPV_ERROR_CODE_E_NOT_INITIALIZED, "Window registry not initialized and remove called while it holds entries");
    return COMPV_ERROR_CODE_S_OK;
}

size_t CompVWindowRegistry::count()
{
    COMPV_CHECK_CODE_NOP(s_WindowsMutex->lock());
    size_t count = m_sWindows.size();
    COMPV_CHECK_CODE_NOP(s_WindowsMutex->unlock());
    return count;
}

COMPV_ERROR_CODE CompVWindowRegistry::closeAll()
{
	if (s_bInitialized) {
		COMPV_CHECK_CODE_NOP(s_WindowsMutex->lock());
		std::map<compv_window_id_t, CompVWindowPrivPtr >::iterator it;
		for (it = m_sWindows.begin(); it != m_sWindows.end(); ++it) {
			CompVWindowPrivPtr window = it->second;
			COMPV_CHECK_CODE_NOP(window->close());
		}
		m_sWindows.clear();
		COMPV_CHECK_CODE_NOP(s_WindowsMutex->unlock());
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!m_sWindows.empty(), COMPV_ERROR_CODE_E_NOT_INITIALIZED, "Window registry not initialized and closeAll called while it holds entries");
	return COMPV_ERROR_CODE_S_OK;
}

#if COMPV_OS_ANDROID
COMPV_ERROR_CODE CompVWindowRegistry::android_handle_cmd(int cmd)
{
	if (s_bInitialized) {
		COMPV_WINDOW_STATE newState = COMPV_WINDOW_STATE_NONE;
		switch (cmd) {
			case APP_CMD_INIT_WINDOW: newState = COMPV_WINDOW_STATE_CONTEXT_CREATED; break;
			case APP_CMD_TERM_WINDOW: newState = COMPV_WINDOW_STATE_CONTEXT_DESTROYED; break;
			case APP_CMD_GAINED_FOCUS: newState = COMPV_WINDOW_STATE_FOCUS_GAINED; break;
			case APP_CMD_LOST_FOCUS: newState = COMPV_WINDOW_STATE_FOCUS_LOST; break;
			default: return COMPV_ERROR_CODE_S_OK;
		}
		COMPV_CHECK_CODE_NOP(s_WindowsMutex->lock());
		std::map<compv_window_id_t, CompVWindowPrivPtr >::iterator it;
		for (it = m_sWindows.begin(); it != m_sWindows.end(); ++it) {
			CompVWindowPrivPtr window = it->second;
			COMPV_CHECK_CODE_NOP(window->priv_updateState(newState));
		}
		COMPV_CHECK_CODE_NOP(s_WindowsMutex->unlock());
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!m_sWindows.empty(), COMPV_ERROR_CODE_E_NOT_INITIALIZED, "Window registry not initialized and android_handle_cmd called while it holds entries");
	return COMPV_ERROR_CODE_S_OK;
}
#endif

COMPV_NAMESPACE_END()

