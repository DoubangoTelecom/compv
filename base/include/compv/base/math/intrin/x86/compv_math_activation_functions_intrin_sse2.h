/* Copyright (C) 2011-2025 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_INTRIN_SSE2_H_)
#define _COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathActivationFunctionsSoftmaxInPlace_32f32f_Intrin_SSE2(
	const compv_uscalar_t& in_out_length,
	compv_float32_t* in_out_ptr
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_ACTIVATION_FUNCTIONS_INTRIN_SSE2_H_ */
