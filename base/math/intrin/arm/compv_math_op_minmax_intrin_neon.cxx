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
	minn = vget_lane_u8(vec0n, 0);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
