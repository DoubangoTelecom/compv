/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_caps.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVCaps::CompVCaps()
{

}

CompVCaps::~CompVCaps()
{

}

COMPV_ERROR_CODE CompVCaps::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVCaps::get(int id, const void*& valuePtr, size_t valueSize)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
