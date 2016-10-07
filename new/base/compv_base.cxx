/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

bool CompVBase::s_bInitialized = false;

CompVBase::CompVBase()
{

}

CompVBase::~CompVBase()
{

}

COMPV_ERROR_CODE CompVBase::init()
{
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBase::deInit()
{
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
