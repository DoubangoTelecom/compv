/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_STATS_FIT_H_)
#define _COMPV_BASE_MATH_STATS_FIT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathStatsFit
{
public:
	static COMPV_ERROR_CODE line(const CompVMatPtr& points, const double threshold, CompVMatPtrPtr params);
	static COMPV_ERROR_CODE parabola(const CompVMatPtr& points, const double threshold, CompVMatPtrPtr params, const COMPV_MATH_PARABOLA_TYPE type = COMPV_MATH_PARABOLA_TYPE_REGULAR);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_STATS_FIT_H_ */

