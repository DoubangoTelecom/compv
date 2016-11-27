/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/drawing/compv_window_registry.h"

COMPV_NAMESPACE_BEGIN()

std::map<compv_window_id_t, CompVWindowPtr > CompVWindowRegistry::m_sWindows;
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
	s_WindowsMutex = NULL;
	s_bInitialized = false;
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVWindowRegistry::add(CompVWindowPtr window)
{
	COMPV_CHECK_EXP_RETURN(!s_bInitialized, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(!window, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->lock());
	m_sWindows.insert(std::pair<compv_window_id_t, CompVPtr<CompVWindow* > >(window->getId(), window));
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowRegistry::remove(CompVWindowPtr window)
{
	COMPV_CHECK_EXP_RETURN(!s_bInitialized, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_CODE_RETURN(remove(window->getId()));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowRegistry::remove(compv_window_id_t windowId)
{
	COMPV_CHECK_EXP_RETURN(!s_bInitialized, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->lock());
	m_sWindows.erase(windowId);
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->unlock());

	return COMPV_ERROR_CODE_S_OK;
}

size_t CompVWindowRegistry::count()
{
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->lock());
	size_t count = m_sWindows.size();
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->unlock());
	return count;
}

COMPV_ERROR_CODE CompVWindowRegistry::closeAll()
{
	COMPV_CHECK_EXP_RETURN(!s_bInitialized, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->lock());
	std::map<compv_window_id_t, CompVPtr<CompVWindow* > >::iterator it;
	for (it = m_sWindows.begin(); it != m_sWindows.end(); ++it) {
		CompVPtr<CompVWindow* > window = it->second;
		COMPV_CHECK_CODE_ASSERT(window->close());
	}
	m_sWindows.clear();
	COMPV_CHECK_CODE_ASSERT(s_WindowsMutex->unlock());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

