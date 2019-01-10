/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_DOT_H_)
#define _COMPV_BASE_MATH_DOT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVMathDot
{
public:
	static COMPV_ERROR_CODE dot(const CompVMatPtr &A, const CompVMatPtr &B, double* ret);
	static COMPV_ERROR_CODE dotSub(const CompVMatPtr &A, const CompVMatPtr &B, double* ret);
	static COMPV_ERROR_CODE hookDotSub_64f(
		void(**CompVMathDotDotSub_64f64f)(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
	);
	static COMPV_ERROR_CODE hookDot_64f(
		void(**CompVMathDotDot_64f64f)(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
	);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_DOT_H_ */
