/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_cast_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ARCH_ARM64

void CompVMathCastProcess_static_64f32f_Intrin_NEON64(
	COMPV_ALIGNED(NEON) const compv_float64_t* src,
	COMPV_ALIGNED(NEON) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(NEON) const compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const compv_uscalar_t width8 = width & -8;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width8; i += 8) {
			const float64x2_t vec0 = vld1q_f64(&src[i]);
			const float64x2_t vec1 = vld1q_f64(&src[i + 2]);
			const float64x2_t vec2 = vld1q_f64(&src[i + 4]);
			const float64x2_t vec3 = vld1q_f64(&src[i + 6]);
			vst1q_f32(&dst[i], vcombine_f32(vcvt_f32_f64(vec0), vcvt_f32_f64(vec1)));
			vst1q_f32(&dst[i + 4], vcombine_f32(vcvt_f32_f64(vec2), vcvt_f32_f64(vec3)));
		}
		for (; i < width; i += 4) {
			const float64x2_t vec0 = vld1q_f64(&src[i]);
			const float64x2_t vec1 = vld1q_f64(&src[i + 2]);
			vst1q_f32(&dst[i], vcombine_f32(vcvt_f32_f64(vec0), vcvt_f32_f64(vec1))); // NEON-aligned -> can write beyond width and up to stride
		}
		src += stride;
		dst += stride;
	}
}

#endif /* COMPV_ARCH_ARM64 */

void CompVMathCastProcess_static_32s32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const int32_t* src,
	COMPV_ALIGNED(NEON) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(NEON) const compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const compv_uscalar_t width16 = width & -16;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width16; i += 16) {
			vst1q_f32(&dst[i], vcvtq_f32_s32(vld1q_s32(&src[i])));
			vst1q_f32(&dst[i + 4], vcvtq_f32_s32(vld1q_s32(&src[i + 4])));
			vst1q_f32(&dst[i + 8], vcvtq_f32_s32(vld1q_s32(&src[i + 8])));
			vst1q_f32(&dst[i + 12], vcvtq_f32_s32(vld1q_s32(&src[i + 12])));
		}
		for (; i < width; i += 4) {
			vst1q_f32(&dst[i], vcvtq_f32_s32(vld1q_s32(&src[i])));
		}
		src += stride;
		dst += stride;
	}
}

void CompVMathCastProcess_static_16s32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const int16_t* src,
	COMPV_ALIGNED(NEON) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(NEON) const compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const compv_uscalar_t width16 = width & -16;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width16; i += 16) {
			const int16x8_t vec0 = vld1q_s16(&src[i]);
			const int16x8_t vec1 = vld1q_s16(&src[i + 8]);

			vst1q_f32(&dst[i], vcvtq_f32_s32(vmovl_s16(vget_low_s16(vec0))));
			vst1q_f32(&dst[i + 4], vcvtq_f32_s32(vmovl_s16(vget_high_s16(vec0))));
			vst1q_f32(&dst[i + 8], vcvtq_f32_s32(vmovl_s16(vget_low_s16(vec1))));
			vst1q_f32(&dst[i + 12], vcvtq_f32_s32(vmovl_s16(vget_high_s16(vec1))));
		}
		for (; i < width; i += 4) {
			const int16x4_t vec0 = vld1_s16(&src[i]);

			vst1q_f32(&dst[i], vcvtq_f32_s32(vmovl_s16(vec0)));
		}
		src += stride;
		dst += stride;
	}
}

void CompVMathCastProcess_static_8u32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const uint8_t* src,
	COMPV_ALIGNED(NEON) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(NEON) const compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			uint8x16_t vec1 = vld1q_u8(&src[i]);
			uint8x16_t vec3 = vmovl_u8(vget_high_u8(vec1));
			vec1 = vmovl_u8(vget_low_u8(vec1));
			uint8x16_t vec0 = vmovl_u16(vget_low_u16(vec1));
			vec1 = vmovl_u16(vget_high_u16(vec1));
			uint8x16_t vec2 = vmovl_u16(vget_low_u16(vec3));
			vec3 = vmovl_u16(vget_high_u16(vec3));
			vst1q_f32(&dst[i], vcvtq_f32_u32(vec0));
			vst1q_f32(&dst[i + 4], vcvtq_f32_u32(vec1));
			vst1q_f32(&dst[i + 8], vcvtq_f32_u32(vec2));
			vst1q_f32(&dst[i + 12], vcvtq_f32_u32(vec3));
		}
		src += stride;
		dst += stride;
	}
}

void CompVMathCastProcess_static_8u32s_Intrin_NEON(
	COMPV_ALIGNED(NEON) const uint8_t* src,
	COMPV_ALIGNED(NEON) int32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(NEON) const compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			uint8x16_t vec1 = vld1q_u8(&src[i]);
			uint8x16_t vec3 = vmovl_u8(vget_high_u8(vec1));
			vec1 = vmovl_u8(vget_low_u8(vec1));
			uint8x16_t vec0 = vmovl_u16(vget_low_u16(vec1));
			vec1 = vmovl_u16(vget_high_u16(vec1));
			uint8x16_t vec2 = vmovl_u16(vget_low_u16(vec3));
			vec3 = vmovl_u16(vget_high_u16(vec3));
			vst1q_s32(&dst[i], vec0);
			vst1q_s32(&dst[i + 4], vec1);
			vst1q_s32(&dst[i + 8], vec2);
			vst1q_s32(&dst[i + 12], vec3);
		}
		src += stride;
		dst += stride;
	}
}

void CompVMathCastProcess_static_pixel8_32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const compv_float32_t* src,
	COMPV_ALIGNED(NEON) uint8_t* dst,
	compv_uscalar_t width,
	compv_uscalar_t height,
	COMPV_ALIGNED(NEON) compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			int32x4_t vec0 = vcvtq_s32_f32(vld1q_f32(&src[i]));
			int32x4_t vec1 = vcvtq_s32_f32(vld1q_f32(&src[i + 4]));
			int32x4_t vec2 = vcvtq_s32_f32(vld1q_f32(&src[i + 8]));
			int32x4_t vec3 = vcvtq_s32_f32(vld1q_f32(&src[i + 12]));
			vec0 = vcombine_s16(vqmovn_s32(vec0), vqmovn_s32(vec1));
			vec2 = vcombine_s16(vqmovn_s32(vec2), vqmovn_s32(vec3));
			vec0 = vcombine_u8(vqmovun_s16(vec0), vqmovun_s16(vec2));
			vst1q_u8(&dst[i], vec0);
		}
		src += stride;
		dst += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
