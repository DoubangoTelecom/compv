/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_CONVLT_INTRIN_AVX2_H_)
#define _COMPV_BASE_MATH_CONVLT_INTRIN_AVX2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Intrin_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize);

void CompVMathConvlt1VtHz_8u32f8u_Intrin_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);

void CompVMathConvlt1VtHz_8u16s16s_Intrin_AVX2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);

void CompVMathConvlt1VtHz_16s16s16s_Intrin_AVX2(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);


COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_CONVLT_INTRIN_AVX2_H_ */
