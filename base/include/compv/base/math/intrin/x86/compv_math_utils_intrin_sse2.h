/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_UTILS_INTRIN_SSE2_H_)
#define _COMPV_BASE_MATH_UTILS_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVMathUtilsMax_16u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint16_t *max);

void CompVMathUtilsSum_8u32u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, uint32_t *sum1);

void CompVMathUtilsSum2_32s32s_Intrin_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

void CompVMathUtilsSum2_32s32s_256x1_Intrin_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

void CompVMathUtilsScaleAndClipPixel8_16u32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(SSE) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_UTILS_INTRIN_SSE2_H_ */
