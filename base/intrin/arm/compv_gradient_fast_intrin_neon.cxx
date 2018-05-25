/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/arm/compv_gradient_fast_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): On iPhone5 ARM32, 1282x720, 1 thread -> Intrin: 895ms, ASM: 632ms
void CompVGradientFastGradX_8u16s_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			const uint8x16_t vec0 = vld1q_u8(&input[i - 1]); // unaligned load
			const uint8x16_t vec1 = vld1q_u8(&input[i + 1]); // unaligned load
			const int16x8_t vec2 = vsubl_u8(vget_low_u8(vec1), vget_low_u8(vec0));
			const int16x8_t vec3 = vsubl_u8(vget_high_u8(vec1), vget_high_u8(vec0));
			vst1q_s16(&dx[i], vec2);
			vst1q_s16(&dx[i + 8], vec3);
		}
		input += stride;
		dx += stride;
	}
}

// TODO(dmi): On iPhone5 ARM32, 1282x720, 1 thread -> Intrin: 1718ms, ASM: 1063ms
void CompVGradientFastGradX_8u32f_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
    COMPV_DEBUG_INFO_CHECK_NEON();
    for (compv_uscalar_t j = 0; j < height; ++j) {
        for (compv_uscalar_t i = 0; i < width; i += 16) {
            const uint8x16_t vec0 = vld1q_u8(&input[i - 1]); // unaligned load
            const uint8x16_t vec1 = vld1q_u8(&input[i + 1]); // unaligned load
            const int16x8_t vec2 = vsubl_u8(vget_low_u8(vec1), vget_low_u8(vec0));
            const int16x8_t vec3 = vsubl_u8(vget_high_u8(vec1), vget_high_u8(vec0));
            vst1q_f32(&dx[i], vcvtq_f32_s32(vmovl_s16(vget_low_s16(vec2))));
            vst1q_f32(&dx[i + 4], vcvtq_f32_s32(vmovl_s16(vget_high_s16(vec2))));
            vst1q_f32(&dx[i + 8], vcvtq_f32_s32(vmovl_s16(vget_low_s16(vec3))));
            vst1q_f32(&dx[i + 12], vcvtq_f32_s32(vmovl_s16(vget_high_s16(vec3))));
        }
        input += stride;
        dx += stride;
    }
}

// TODO(dmi): On iPhone5 ARM32, 1282x720, 1 thread -> Intrin: 1306ms, ASM: 638ms
void CompVGradientFastGradY_8u16s_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
    const uint8_t* inputMinus1 = input - stride;
    const uint8_t* inputPlus1 = input + stride;
    for (compv_uscalar_t j = 0; j < height; ++j) {
        for (compv_uscalar_t i = 0; i < width; i+= 16) {
            const uint8x16_t vec0 = vld1q_u8(&inputMinus1[i]); // aligned load
            const uint8x16_t vec1 = vld1q_u8(&inputPlus1[i]); // aligned load
            const int16x8_t vec2 = vsubl_u8(vget_low_u8(vec1), vget_low_u8(vec0));
            const int16x8_t vec3 = vsubl_u8(vget_high_u8(vec1), vget_high_u8(vec0));
            vst1q_s16(&dy[i], vec2);
            vst1q_s16(&dy[i + 8], vec3);
        }
        inputPlus1 += stride;
        inputMinus1 += stride;
        dy += stride;
    }
}

// TODO(dmi): On iPhone5 ARM32, 1282x720, 1 thread -> Intrin: 1806ms, ASM: 1028ms
void CompVGradientFastGradY_8u32f_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
    COMPV_DEBUG_INFO_CHECK_NEON();
    const uint8_t* inputMinus1 = input - stride;
    const uint8_t* inputPlus1 = input + stride;
    for (compv_uscalar_t j = 0; j < height; ++j) {
        for (compv_uscalar_t i = 0; i < width; i+= 16) {
            const uint8x16_t vec0 = vld1q_u8(&inputMinus1[i]); // aligned load
            const uint8x16_t vec1 = vld1q_u8(&inputPlus1[i]); // aligned load
            const int16x8_t vec2 = vsubl_u8(vget_low_u8(vec1), vget_low_u8(vec0));
            const int16x8_t vec3 = vsubl_u8(vget_high_u8(vec1), vget_high_u8(vec0));
            vst1q_f32(&dy[i], vcvtq_f32_s32(vmovl_s16(vget_low_s16(vec2))));
            vst1q_f32(&dy[i + 4], vcvtq_f32_s32(vmovl_s16(vget_high_s16(vec2))));
            vst1q_f32(&dy[i + 8], vcvtq_f32_s32(vmovl_s16(vget_low_s16(vec3))));
            vst1q_f32(&dy[i + 12], vcvtq_f32_s32(vmovl_s16(vget_high_s16(vec3))));
        }
        inputPlus1 += stride;
        inputMinus1 += stride;
        dy += stride;
    }
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
