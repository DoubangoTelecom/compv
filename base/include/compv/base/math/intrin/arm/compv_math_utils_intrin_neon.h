/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_UTILS_INTRIN_NEON_H_)
#define _COMPV_BASE_MATH_UTILS_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVMathUtilsMax_16u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint16_t *max);

void CompVMathUtilsSum_8u32u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint32_t *sum1);

void CompVMathUtilsSum2_32s32s_Intrin_NEON(COMPV_ALIGNED(NEON) const int32_t* a, COMPV_ALIGNED(NEON) const int32_t* b, COMPV_ALIGNED(NEON) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
void CompVMathUtilsSum2_32s32s_256x1_Intrin_NEON(COMPV_ALIGNED(NEON) const int32_t* a, COMPV_ALIGNED(NEON) const int32_t* b, COMPV_ALIGNED(NEON) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);

void CompVMathUtilsSumAbs_16s16u_Intrin_NEON(const COMPV_ALIGNED(NEON) int16_t* a, const COMPV_ALIGNED(NEON) int16_t* b, COMPV_ALIGNED(NEON) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);

void CompVMathUtilsScaleAndClipPixel8_16u32f_Intrin_NEON(COMPV_ALIGNED(NEON) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(NEON) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_UTILS_INTRIN_NEON_H_ */
