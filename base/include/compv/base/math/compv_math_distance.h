/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_DISTANCE_H_)
#define _COMPV_BASE_MATH_DISTANCE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/math/compv_math_utils.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathDistance
{
public:
	static COMPV_ERROR_CODE hamming(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr);
	static COMPV_ERROR_CODE line(const CompVMatPtr& points, const double(&lineEq)[3], CompVMatPtrPtr distances);
	static COMPV_ERROR_CODE parabola(const CompVMatPtr& points, const double(&parabolaEq)[3], CompVMatPtrPtr distances, const COMPV_MATH_PARABOLA_TYPE type = COMPV_MATH_PARABOLA_TYPE_REGULAR);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_DISTANCE_H_ */

