/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_TRIG_H_)
#define _COMPV_BASE_MATH_TRIG_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

#include <cmath> /* std::atan2 */

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathTrig
{
public:
	template<typename FloatType>
	static COMPV_ERROR_CODE rodriguesVectorToMatrix(const FloatType(&vector)[3], CompVMatPtrPtr matrix);

	template<typename FloatType>
	static COMPV_ERROR_CODE rodriguesMatrixToVector(const CompVMatPtr& matrix, FloatType(&vector)[3]);

	static COMPV_ERROR_CODE fastAtan2(const CompVMatPtr& y, const CompVMatPtr& x, CompVMatPtrPtr r, const bool angleInDeg);
	template<typename FloatType>
	static FloatType stdAtan2(const FloatType& y, const FloatType& x) {
		// atan2 is within [-pi, pi] and undefined from (y==0 && x==0) - https://en.wikipedia.org/wiki/Atan2#Definition_and_computation
		const static FloatType eps = std::numeric_limits<FloatType>::epsilon();
		return (std::abs(y) <= eps && std::abs(x) <= eps) ? 0 : std::atan2(y, x);
	}
	static COMPV_ERROR_CODE hypot(const CompVMatPtr& x, const CompVMatPtr& y, CompVMatPtrPtr r);
	static COMPV_ERROR_CODE hypot_naive(const CompVMatPtr& x, const CompVMatPtr& y, CompVMatPtrPtr r);
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesVectorToMatrix(const compv_float64x3_t &vector, CompVMatPtrPtr matrix);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesVectorToMatrix(const compv_float32x3_t &vector, CompVMatPtrPtr matrix);

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesMatrixToVector(const CompVMatPtr& matrix, compv_float64x3_t &vector);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesMatrixToVector(const CompVMatPtr& matrix, compv_float32x3_t &vector);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_TRIG_H_ */

