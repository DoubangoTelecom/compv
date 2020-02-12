/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/intrin/arm/compv_core_ccl_lsl_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/core/compv_core_simd_globals.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Function requires width > 16 (not ">=" but ">")
void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Intrin_NEON(
	COMPV_ALIGNED(NEON) const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, 
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
	uint8x16_t vec0, vec1;
	uint8x8_t vec0n, vec2n, vec3n, vec4n, vec5n, vec6n, vec7n, vec8n;
	int16x8_t vecER;
	const uint8x16_t vecOne = vdupq_n_u8(1);
	const uint8x8_t vecZeron = vdup_n_u8(0);

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
				vec1 = vdupq_n_u8(0);

				/* == Low == */
				vec0n = vget_low_u8(vec0);
				if (COMPV_ARM_NEON_NEQ_ZEROD(vec0n)) {
					vec2n = vdup_lane_u8(vec0n, 0);
					vec3n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 1), 8);
					vec4n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 2), 16);
					vec5n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 3), 24);
					vec6n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 4), 32);
					vec7n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 5), 40);
					vec8n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 6), 48);
					vec0n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 7), 56);
					vec2n = vadd_u8(vec2n, vec3n);
					vec4n = vadd_u8(vec4n, vec5n);
					vec6n = vadd_u8(vec6n, vec7n);
					vec8n = vadd_u8(vec8n, vec0n);
					vec2n = vadd_u8(vec2n, vec4n);
					vec6n = vadd_u8(vec6n, vec8n);
					vec2n = vadd_u8(vec2n, vec6n);
					vec1 = vcombine_u8(vec2n, vdup_lane_u8(vec2n, 7));
				}

				/* == High == */
				vec0n = vget_high_u8(vec0);
				if (COMPV_ARM_NEON_NEQ_ZEROD(vec0n)) {
					vec2n = (uint8x8_t)vdup_lane_u8(vec0n, 0);
					vec3n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 1), 8);
					vec4n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 2), 16);
					vec5n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 3), 24);
					vec6n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 4), 32);
					vec7n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 5), 40);
					vec8n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 6), 48);
					vec0n = (uint8x8_t)vshl_n_u64((uint64x1_t)vdup_lane_u8(vec0n, 7), 56);
					vec2n = vadd_u8(vec2n, vec3n);
					vec4n = vadd_u8(vec4n, vec5n);
					vec6n = vadd_u8(vec6n, vec7n);
					vec8n = vadd_u8(vec8n, vec0n);
					vec2n = vadd_u8(vec2n, vec4n);
					vec6n = vadd_u8(vec6n, vec8n);
					vec2n = vadd_u8(vec2n, vec6n);
					vec1 = vaddq_u8(vec1, vcombine_u8(vecZeron, vec2n));
				}
				
				/* Convert erUint8 to erInt16 and sum then store */
				vec0 = vaddw_u8(vecER, vget_low_u8(vec1));
				vecER = vaddw_u8(vecER, vget_high_u8(vec1));
				vst1q_s16(&ERi[i], vec0); // unaligned store
				vst1q_s16(&ERi[i + 8], vecER); // unaligned store

				/* Duplicate latest element */
				vecER = vdupq_lane_s16(vget_high_s16(vecER), 3); // TODO(dmi): check why 7 instead of 3 was working. 7 is out of range the compiler should have complained
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
		if (ner_max < er) { // TODO(dmi): asm use movlt
			ner_max = er;
		}
		/* next */
		Xi += Xi_stride;
		ERi += width;
	}

	*ner_max1 = ner_max;
	*ner_sum1 = ner_sum;
}

void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Intrin_NEON(
	const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, 
	int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	const compv_uscalar_t width, const compv_uscalar_t height
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM version is #2 times faster (thanks to 'rbit' and 'clz' instructions)");

	const int16_t width1 = static_cast<int16_t>(width);
	const int16_t width16 = (width1 - 1) & -16; // width > 16 (at least 17) which means never equal to zero
	int16_t er, i;
	int16x8_t vec0, vec1;
	int8x8_t vec0n;
	unsigned mask, m; // unsigned shift (shr) instead of signed shift (sar)
#if COMPV_ARCH_ARM32
	static const uint8x16_t vecMask = (uint32x4_t) { 0x8040201, 0x80402010, 0x8040201, 0x80402010 };
#else
	static const uint8x16_t vecMask = (uint64x2_t) { 9241421688590303745ULL, 9241421688590303745ULL };
#endif

	for (compv_uscalar_t j = 0; j < height; ++j) {
		er = (Xi[0] & 1);
		RLCi[0] = 0;

		// In asm code, no need to test "width16 != 0" because "width1" > 16 (at least 17)
		for (i = 1; i < width16; i += 16) {
			vec0 = vceqq_s16(
				vld1q_s16(&ERi[i - 1]), // unaligned load (q0)
				vld1q_s16(&ERi[i]) // unaligned load (q2)
			);
			vec1 = vceqq_s16(
				vld1q_s16(&ERi[i + 7]), // unaligned load (q1)
				vld1q_s16(&ERi[i + 8]) // unaligned load (q3)
			);
			vec0 = vcombine_u8(vqmovn_u16(vec0), vqmovn_u16(vec1));
			vec0 = vmvnq_u8(vec0);			
			if (COMPV_ARM_NEON_NEQ_ZEROQ(vec0)) {
				// _mm_movemask_epi8 doesn't exist on ARM NEON
				vec0 = vandq_u8(vec0, vecMask);
				vec0n = vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0));
				vec0n = vpadd_u8(vec0n, vec0n);
				vec0n = vpadd_u8(vec0n, vec0n);
				mask = vget_lane_u16(vec0n, 0);
				m = i;
				do {
					if (mask & 1) {
						RLCi[er++] = m;
					}
					++m;
				} while (mask >>= 1);
			}
		}

		for (; i < width1; ++i) {
			if (ERi[i - 1] != ERi[i]) {
				RLCi[er++] = i;
			}
		}

		RLCi[er] = width1 - ((Xi[width1 - 1] & 1) ^ 1);

		/* next */
		Xi += Xi_stride;
		RLCi += RLCi_stride;
		ERi += width;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */