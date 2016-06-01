/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/calib/compv_calib_homography.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
COMPV_ERROR_CODE CompVHomography<T>::find(const CompVPtrBoxPoint(T) &src, const CompVPtrBoxPoint(T) &dst, CompVPtrArray(T) &H)
{
	COMPV_CHECK_EXP_RETURN(!src || !dst || src->size() < 4 || src->size() != dst->size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;


	return err_;
}

COMPV_NAMESPACE_END()