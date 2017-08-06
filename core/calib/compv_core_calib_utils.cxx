/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/calib/compv_core_calib_utils.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_stats.h"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVCalibUtils::projError(const CompVMatPtr& aPoints, const CompVMatPtr& bPoints, compv_float64_t& error)
{
	CompVMathStats<compv_float64_t>::mse2D(aPoints, bPoints, error); // Mean Square Error
	error = std::sqrt(error / static_cast<compv_float64_t>(aPoints->cols())); // mean(sqrt)
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
