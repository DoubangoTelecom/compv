/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_USELOCAL_H_)
#define _COMPV_BASE_USELOCAL_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"

#include <locale.h>

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVUselocal
{
public:
	CompVUselocal() = delete;
	CompVUselocal(const int& category, const std::string& newLocal);
	virtual ~CompVUselocal();
private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
#if COMPV_OS_WINDOWS
	int m_dCategory;
	std::string m_strOldLocal;
#else
	locale_t m_OldLocal;
	locale_t m_NewLocal;
#endif
	COMPV_VS_DISABLE_WARNINGS_END()
};

#define COMPV_AUTO_USELOCAL(category, newLocal)		CompVUselocal __COMPV_local__((category), (newLocal))

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_USELOCAL_H_ */
