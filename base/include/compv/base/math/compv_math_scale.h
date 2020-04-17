/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_SCALE_H_)
#define _COMPV_BASE_MATH_SCALE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVMathScale
{
public:
	static COMPV_ERROR_CODE scale(const CompVMatPtr &in, const double& s, CompVMatPtrPtr out, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE hookScale_64f(
		void(**CompVMathScaleScale_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const compv_float64_t* s1)
	);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_SCALE_H_ */
