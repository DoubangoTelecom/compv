/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_op_minmax_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathOpMinMax_32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, compv_float32_t* min1, compv_float32_t* max1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width4 = width & -4;
	compv_float32_t& minn = *min1;
	compv_float32_t& maxx = *max1;
	float32x4_t vv0_minn = vdupq_n_f32(minn), vv1_minn = vv0_minn, vv2_minn = vv0_minn, vv3_minn = vv0_minn;
	float32x4_t vv0_maxx = vdupq_n_f32(maxx), vv1_maxx = vv0_maxx, vv2_maxx = vv0_maxx, vv3_maxx = vv0_maxx;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width16; i += 16) {
			const float32x4_t vv0 = vld1q_f32(&APtr[i]);
			const float32x4_t vv1 = vld1q_f32(&APtr[i + 4]);
			const float32x4_t vv2 = vld1q_f32(&APtr[i + 8]);
			const float32x4_t vv3 = vld1q_f32(&APtr[i + 12]);
			vv0_minn = vminq_f32(vv0_minn, vv0);
			vv1_minn = vminq_f32(vv1_minn, vv1);
			vv2_minn = vminq_f32(vv2_minn, vv2);
			vv3_minn = vminq_f32(vv3_minn, vv3);
			vv0_maxx = vmaxq_f32(vv0_maxx, vv0);
			vv1_maxx = vmaxq_f32(vv1_maxx, vv1);
			vv2_maxx = vmaxq_f32(vv2_maxx, vv2);
			vv3_maxx = vmaxq_f32(vv3_maxx, vv3);
		}
		for (; i < width4; i += 4) {
			const float32x4_t vv0 = vld1q_f32(&APtr[i]);
			vv0_minn = vminq_f32(vv0_minn, vv0);
			vv0_maxx = vmaxq_f32(vv0_maxx, vv0);
		}
		
		for (; i < width; ++i) {
			const compv_float32_t& vv = APtr[i];
			minn = COMPV_MATH_MIN(minn, vv);
			maxx = COMPV_MATH_MAX(maxx, vv);
		}

		APtr += stride;
	}

	vv0_minn = vminq_f32(vv0_minn, vv1_minn);
	vv2_minn = vminq_f32(vv2_minn, vv3_minn);
	vv0_minn = vminq_f32(vv0_minn, vv2_minn);
	float32x2_t vv0_minn__ = vmin_f32(vget_low_f32(vv0_minn), vget_high_f32(vv0_minn));
	vv0_minn__ = vpmin_f32(vv0_minn__, vv0_minn__);
	minn = std::min(minn, vget_lane_f32(vv0_minn__, 0));

	vv0_maxx = vmaxq_f32(vv0_maxx, vv1_maxx);
	vv2_maxx = vmaxq_f32(vv2_maxx, vv3_maxx);
	vv0_maxx = vmaxq_f32(vv0_maxx, vv2_maxx);
	float32x2_t vv0_maxx__ = vmax_f32(vget_low_f32(vv0_maxx), vget_high_f32(vv0_maxx));
	vv0_maxx__ = vpmax_f32(vv0_maxx__, vv0_maxx__);
	maxx = std::max(maxx, vget_lane_f32(vv0_maxx__, 0));
}

void CompVMathOpMin_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint8_t* min1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	uint8_t& minn = *min1;
	uint8x16_t vec0 = vdupq_n_u8(minn);
	uint8x16_t vec1 = vec0;
	uint8x16_t vec2 = vec0;
	uint8x16_t vec3 = vec0;
	const compv_uscalar_t width64 = width & -64;
	const compv_uscalar_t width16 = width & -16;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width64; i += 64) {
			vec0 = vminq_u8(vec0, vld1q_u8(&APtr[i]));
			vec1 = vminq_u8(vec1, vld1q_u8(&APtr[i + 16]));
			vec2 = vminq_u8(vec2, vld1q_u8(&APtr[i + 32]));
			vec3 = vminq_u8(vec3, vld1q_u8(&APtr[i + 48]));
		}
		for (; i < width16; i += 16) {
			vec0 = vminq_u8(vec0, vld1q_u8(&APtr[i]));
		}
		for (; i < width; i += 1) {
			const uint8_t& vv = APtr[i];
			minn = COMPV_MATH_MIN(minn, vv);
		}
		APtr += stride;
	}
	vec0 = vminq_u8(vec0, vec1);
	vec2 = vminq_u8(vec2, vec3);
	vec0 = vminq_u8(vec0, vec2);
	uint8x8_t vec0n = vmin_u8(vget_low_u8(vec0), vget_high_u8(vec0));
	vec0n = vpmin_u8(vec0n, vec0n);
	vec0n = vpmin_u8(vec0n, vec0n);
	vec0n = vpmin_u8(vec0n, vec0n);
	minn = std::min(minn, vget_lane_u8(vec0n, 0));
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
