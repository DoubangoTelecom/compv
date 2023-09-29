/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_uselocal.h"

#include <clocale> /* LC_ALL */

COMPV_NAMESPACE_BEGIN()

CompVUselocal::CompVUselocal(const int& category, const std::string& newLocal)
	: m_dCategory(category)
{
	// Set new local
#if COMPV_OS_WINDOWS
	// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/configthreadlocale?view=msvc-170
	_configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
	const char *old_locale = setlocale(category, nullptr);
	m_strOldLocal = std::string(old_locale ? old_locale : "");
	const char *new_locale = setlocale(category, newLocal.c_str());
#else
	// https://man7.org/linux/man-pages/man3/uselocale.3.html
	new_local = newlocale(category, newLocal.c_str(), nullptr);
	m_OldLocal = uselocale(new_local);
	freelocale(new_local);
#endif
}

CompVUselocal::~CompVUselocal()
{
	// Restore the old local when the object is destroyed (e.g. goes auto of scope)
#if COMPV_OS_WINDOWS
	_configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
	setlocale(m_dCategory, m_strOldLocal.c_str());
#else
	uselocale(m_OldLocal);
#endif
}

COMPV_NAMESPACE_END()
