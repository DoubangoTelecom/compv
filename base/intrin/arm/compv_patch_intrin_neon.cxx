/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/arm/compv_patch_intrin_neon.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// No restriction on radius like what we've with SSE and AVX code
// TODO(dmi): iPhone 5 (arm32), ASM code is by far faster than intrin code
// TODO(dmi): Galaxy Tab A6 (arm32), ASM code is by far faster than intrin code
void CompVPatchMoments0110_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* top, COMPV_ALIGNED(NEON) const uint8_t* bottom, COMPV_ALIGNED(NEON) const int16_t* x, COMPV_ALIGNED(NEON) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	int32x4_t vecS10[4], vecS01[4];
	uint8x16_t vec0, vec1, vec2, vec3, vec4, vec5;
	vecS10[0] = vecS10[1] = vecS10[2] = vecS10[3] = vdupq_n_s32(0);
	vecS01[0] = vecS01[1] = vecS01[2] = vecS01[3] = vdupq_n_s32(0);
	// s10 = sum[(x * top) + (x * bottom)] = sum[x * (top + bottom)]
	// s01 = sum[(y * top) - (y * bottom)] = sum[y * (top - bottom)]
	for (compv_uscalar_t i = 0; i < count; i += 16) {
		vec0 = vld1q_u8(&top[i]);
		vec1 = vld1q_u8(&bottom[i]);

		// Low Part //
		vec4 = vaddl_u8(vget_low_u8(vec0), vget_low_u8(vec1)); // top + bottom
		vec5 = vsubl_u8(vget_low_u8(vec0), vget_low_u8(vec1)); // top - bottom
		vec2 = vld1q_s16(&x[i]); // x
		vec3 = vld1q_s16(&y[i]); // y
		vecS10[0] = vmlal_s16(vecS10[0], vget_low_s16(vec4), vget_low_s16(vec2));
		vecS10[1] = vmlal_s16(vecS10[1], vget_high_s16(vec4), vget_high_s16(vec2));
		vecS01[0] = vmlal_s16(vecS01[0], vget_low_s16(vec5), vget_low_s16(vec3));
		vecS01[1] = vmlal_s16(vecS01[1], vget_high_s16(vec5), vget_high_s16(vec3));

		// High Part //
		vec4 = vaddl_u8(vget_high_u8(vec0), vget_high_u8(vec1)); // top + bottom
		vec5 = vsubl_u8(vget_high_u8(vec0), vget_high_u8(vec1)); // top - bottom
		vec2 = vld1q_s16(&x[i + 8]); // x
		vec3 = vld1q_s16(&y[i + 8]); // y
		vecS10[2] = vmlal_s16(vecS10[2], vget_low_s16(vec4), vget_low_s16(vec2));
		vecS10[3] = vmlal_s16(vecS10[3], vget_high_s16(vec4), vget_high_s16(vec2));
		vecS01[2] = vmlal_s16(vecS01[2], vget_low_s16(vec5), vget_low_s16(vec3));
		vecS01[3] = vmlal_s16(vecS01[3], vget_high_s16(vec5), vget_high_s16(vec3));
	}

	vec0 = vaddq_s32(vecS10[0], vecS10[1]);
	vec1 = vaddq_s32(vecS10[2], vecS10[3]);
	vec2 = vaddq_s32(vecS01[0], vecS01[1]);
	vec3 = vaddq_s32(vecS01[2], vecS01[3]);

	vec0 = vaddq_s32(vec0, vec1);
	vec2 = vaddq_s32(vec2, vec3);

	vec0 = vcombine_s32(
		vpadd_s32(vget_low_s32(vec0), vget_high_s32(vec0)),
		vpadd_s32(vget_low_s32(vec2), vget_high_s32(vec2))
	);

	const int32x2_t vec6 = vpadd_s32(vget_low_s32(vec0), vget_high_s32(vec0));

	*s10 += static_cast<compv_scalar_t>(vget_lane_s32(vec6, 0));
	*s01 += static_cast<compv_scalar_t>(vget_lane_s32(vec6, 1));
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

