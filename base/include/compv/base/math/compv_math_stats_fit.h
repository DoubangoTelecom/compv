/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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

enum COMPV_MATH_PARABOLA_TYPE {
	COMPV_MATH_PARABOLA_TYPE_REGULAR, // y = ax^2 + bx + c
	COMPV_MATH_PARABOLA_TYPE_SIDEWAYS // x = ay^2 + by + c -> http://www.coolmath.com/algebra/21-advanced-graphing/05-sideways-parabolas-01
};

class COMPV_BASE_API CompVMathStatsFit
{
public:
	static COMPV_ERROR_CODE line(const CompVMatPtr& points, const double threshold, CompVMatPtrPtr params);
	static COMPV_ERROR_CODE parabola(const CompVMatPtr& points, const double threshold, CompVMatPtrPtr params, const COMPV_MATH_PARABOLA_TYPE type = COMPV_MATH_PARABOLA_TYPE_REGULAR);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_STATS_FIT_H_ */

