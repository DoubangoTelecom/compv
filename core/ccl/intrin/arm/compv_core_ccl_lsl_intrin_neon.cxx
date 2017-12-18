/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/intrin/arm/compv_core_ccl_lsl_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/core/compv_core_simd_globals.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Function requires width > 16 (not ">=" but ">")
void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Intrin_NEON(
	COMPV_ALIGNED(NEON) const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Asm code faster");
	int16_t i, er, ner_max = 0;
	const int16_t width1 = static_cast<int16_t>(width);
	const int16_t width16 = (width1 - 1) & -16; // width > 16 (at least 17) which means never equal to zero
	int32_t ner_sum = 0;
	uint8x16_t vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8;
	int16x8_t vecER;
	const uint8x16_t vecOne = vdupq_n_u8(1);
	const uint8x16_t vecZero = vdupq_n_u8(0);

	for (compv_uscalar_t j = 0; j < height; ++j) {
		er = (Xi[0] & 1);
		ERi[0] = er;
		vecER = vdupq_n_s16(er);

		// In asm code, no need to test "width16 != 0" because "width1" > 16 (at least 17)

		for (i = 1; i < width16; i += 16) {
			vec0 = veorq_u8(
				vld1q_u8(&Xi[i - 1]), // aligned load
				vld1q_u8(&Xi[i]) // unaligned load
			);
			vec0 = vandq_u8(vec0, vecOne);

			if (COMPV_ARM_NEON_NEQ_ZEROQ(vec0)) {
				vec1 = vdupq_lane_u8(vget_low_u8(vec0), 0);
				vec2 = vdupq_lane_u8(vget_low_u8(vec0), 1);
				vec3 = vdupq_lane_u8(vget_low_u8(vec0), 2);
				vec4 = vdupq_lane_u8(vget_low_u8(vec0), 3);
				vec5 = vdupq_lane_u8(vget_low_u8(vec0), 4);
				vec6 = vdupq_lane_u8(vget_low_u8(vec0), 5);
				vec7 = vdupq_lane_u8(vget_low_u8(vec0), 6);
				vec8 = vdupq_lane_u8(vget_low_u8(vec0), 7);
				vec2 = vcombine_u8(vshl_n_u64(vget_low_u8(vec2), 8), vget_high_u8(vec2));
				vec3 = vcombine_u8(vshl_n_u64(vget_low_u8(vec3), 16), vget_high_u8(vec3));
				vec4 = vcombine_u8(vshl_n_u64(vget_low_u8(vec4), 24), vget_high_u8(vec4));
				vec5 = vcombine_u8(vshl_n_u64(vget_low_u8(vec5), 32), vget_high_u8(vec5));
				vec6 = vcombine_u8(vshl_n_u64(vget_low_u8(vec6), 40), vget_high_u8(vec6));
				vec7 = vcombine_u8(vshl_n_u64(vget_low_u8(vec7), 48), vget_high_u8(vec7));
				vec8 = vcombine_u8(vshl_n_u64(vget_low_u8(vec8), 56), vget_high_u8(vec8));
				vec2 = vaddq_u8(vec2, vec3);
				vec4 = vaddq_u8(vec4, vec5);
				vec6 = vaddq_u8(vec6, vec7);
				vec2 = vaddq_u8(vec2, vec8);
				vec4 = vaddq_u8(vec4, vec6);
				vec1 = vaddq_u8(vec1, vec2);
				vec1 = vaddq_u8(vec1, vec4);

				vec2 = vcombine_u8(vget_low_u8(vecZero), vdup_lane_u8(vget_high_u8(vec0), 0));
				vec3 = vcombine_u8(vget_low_u8(vecZero), vshl_n_u64(vdup_lane_u8(vget_high_u8(vec0), 1), 8));
				vec4 = vcombine_u8(vget_low_u8(vecZero), vshl_n_u64(vdup_lane_u8(vget_high_u8(vec0), 2), 16));
				vec5 = vcombine_u8(vget_low_u8(vecZero), vshl_n_u64(vdup_lane_u8(vget_high_u8(vec0), 3), 24));
				vec6 = vcombine_u8(vget_low_u8(vecZero), vshl_n_u64(vdup_lane_u8(vget_high_u8(vec0), 4), 32));
				vec7 = vcombine_u8(vget_low_u8(vecZero), vshl_n_u64(vdup_lane_u8(vget_high_u8(vec0), 5), 40));
				vec8 = vcombine_u8(vget_low_u8(vecZero), vshl_n_u64(vdup_lane_u8(vget_high_u8(vec0), 6), 48));
				vec0 = vcombine_u8(vget_low_u8(vecZero), vshl_n_u64(vdup_lane_u8(vget_high_u8(vec0), 7), 56));
				vec2 = vaddq_u8(vec2, vec3);
				vec4 = vaddq_u8(vec4, vec5);
				vec6 = vaddq_u8(vec6, vec7);
				vec8 = vaddq_u8(vec8, vec0);
				vec2 = vaddq_u8(vec2, vec4);
				vec6 = vaddq_u8(vec6, vec8);
				vec1 = vaddq_u8(vec1, vec2);
				vec1 = vaddq_u8(vec1, vec6);
				
				/* Convert erUint8 to erInt16 and sum then store */
				vec0 = vaddw_u8(vecER, vget_low_u8(vec1));
				vecER = vaddw_u8(vecER, vget_high_u8(vec1));
				vst1q_s16(&ERi[i], vec0); // unaligned store
				vst1q_s16(&ERi[i + 8], vecER); // unaligned store

				/* Duplicate latest element */
				vecER = vdupq_lane_s16(vget_high_s16(vecER), 7);
			}
			else {
				/* Store previous ER */
				vst1q_s16(&ERi[i], vecER); // unaligned store
				vst1q_s16(&ERi[i + 8], vecER); // unaligned store
			}
		}

		/* Get highest "er" before switching from SIMD to serial code */
		er = vgetq_lane_s16(vecER, 7);

		for (; i < width1; ++i) {
			er += ((Xi[i - 1] ^ Xi[i]) & 1);
			ERi[i] = er;
		}
		er += (Xi[width1 - 1] & 1);
		ner[j] = er;
		ner_sum += er;
		if (ner_max < er) { // TODO(dmi): asm use cmovl
			ner_max = er;
		}
		/* next */
		Xi += Xi_stride;
		ERi += ERi_stride;
	}

	*ner_max1 = ner_max;
	*ner_sum1 = ner_sum;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */