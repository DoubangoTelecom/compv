/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_H_)
#define _COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathActivationFunctions
{
public:
	static COMPV_ERROR_CODE tanh(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr);
	static COMPV_ERROR_CODE tanhMul(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr);
	static COMPV_ERROR_CODE logistic(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr);
	static COMPV_ERROR_CODE logisticMul(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr);	

private:
	static bool s_bInitialized;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_H_ */
