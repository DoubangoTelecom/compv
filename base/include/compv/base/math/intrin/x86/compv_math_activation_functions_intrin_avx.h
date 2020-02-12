/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_INTRIN_AVX_H_)
#define _COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_INTRIN_AVX_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathActivationFunctionsTanh_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
);

void CompVMathActivationFunctionsTanhMul_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
);

void CompVMathActivationFunctionsLogistic_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
);

void CompVMathActivationFunctionsLogisticMul_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_INTRIN_AVX_H_ */
