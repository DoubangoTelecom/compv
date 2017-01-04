/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/compv_core.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_base.h"
#include "compv/base/parallel/compv_mutex.h"

COMPV_NAMESPACE_BEGIN()

bool CompVCore::s_bInitialized = false;

COMPV_ERROR_CODE CompVCore::init()
{
	if (s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	COMPV_DEBUG_INFO("Initializing [core] module (v %s)...", COMPV_VERSION_STRING);

	COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

	s_bInitialized = true;

bail:
	return err;
}

COMPV_ERROR_CODE CompVCore::deInit()
{
	COMPV_CHECK_CODE_ASSERT(CompVBase::deInit());
	s_bInitialized = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
