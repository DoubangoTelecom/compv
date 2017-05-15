/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/arm/compv_core_feature_houghstd_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define AccInc_Intrin_NEON(vecIndices) \
		pACC[vgetq_lane_s32(vecIndices, 0)]++; \
		pACC[vgetq_lane_s32(vecIndices, 1)]++; \
		pACC[vgetq_lane_s32(vecIndices, 2)]++; \
		pACC[vgetq_lane_s32(vecIndices, 3)]++


// 4mpd -> minpack 4 for dwords (int32) - for maxTheta
void CompVHoughStdAccGatherRow_4mpd_Intrin_NEON(COMPV_ALIGNED(NEON) const int32_t* pCosRho, COMPV_ALIGNED(NEON) const int32_t* pRowTimesSinRho, compv_uscalar_t col, int32_t* pACC, compv_uscalar_t accStride, compv_uscalar_t maxTheta)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const int32x4_t vecColInt32 = vdupq_n_s32(static_cast<int32_t>(col));
	const int32x4_t vecStride = vdupq_n_s32(static_cast<int32_t>(accStride));
	compv_uscalar_t theta = 0;
	int32x4_t vec0, vec1, vec2, vec3;
	const int32x4_t vec4 = vdupq_n_s32(4);
	int32x4_t vecTheta = (int32x4_t) { 0, 1, 2, 3 };

	for (theta = 0; theta < maxTheta - 15; theta += 16) { // maxTheta always > 16
		vec0 = vmlaq_s32(vld1q_s32(&pRowTimesSinRho[theta + 0]), vecColInt32, vld1q_s32(&pCosRho[theta + 0]));
		vec1 = vmlaq_s32(vld1q_s32(&pRowTimesSinRho[theta + 4]), vecColInt32, vld1q_s32(&pCosRho[theta + 4]));
		vec2 = vmlaq_s32(vld1q_s32(&pRowTimesSinRho[theta + 8]), vecColInt32, vld1q_s32(&pCosRho[theta + 8]));
		vec3 = vmlaq_s32(vld1q_s32(&pRowTimesSinRho[theta + 12]), vecColInt32, vld1q_s32(&pCosRho[theta + 12]));

		vec0 = vshrq_n_s32(vec0, 16);
		vec1 = vshrq_n_s32(vec1, 16);
		vec2 = vshrq_n_s32(vec2, 16);
		vec3 = vshrq_n_s32(vec3, 16);

		vec0 = vmlsq_s32(vecTheta, vec0, vecStride);
		vecTheta = vaddq_s32(vecTheta, vec4);

		vec1 = vmlsq_s32(vecTheta, vec1, vecStride);
		vecTheta = vaddq_s32(vecTheta, vec4);

		vec2 = vmlsq_s32(vecTheta, vec2, vecStride);
		vecTheta = vaddq_s32(vecTheta, vec4);

		vec3 = vmlsq_s32(vecTheta, vec3, vecStride);
		vecTheta = vaddq_s32(vecTheta, vec4);

		AccInc_Intrin_NEON(vec0);
		AccInc_Intrin_NEON(vec1);
		AccInc_Intrin_NEON(vec2);
		AccInc_Intrin_NEON(vec3);
	}

	for (; theta < maxTheta; theta += 4) { // maxTheta is already 4d-aligned
		vec0 = vmlaq_s32(vld1q_s32(&pRowTimesSinRho[theta + 0]), vecColInt32, vld1q_s32(&pCosRho[theta + 0]));

		vec0 = vshrq_n_s32(vec0, 16);

		vec0 = vmlsq_s32(vecTheta, vec0, vecStride);
		vecTheta = vaddq_s32(vecTheta, vec4);

		AccInc_Intrin_NEON(vec0);
	}
}

// 4mpd -> minpack 8 for dwords (int32) - for maxCols
void CompVHoughStdNmsGatherRow_8mpd_Intrin_NEON(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, compv_uscalar_t nThreshold, compv_uscalar_t colStart, compv_uscalar_t maxCols)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const int32x4_t vecThreshold = vdupq_n_s32(static_cast<int32_t>(nThreshold));
	int stride = static_cast<int>(nAccStride);
	int32x4_t vecAcc, vec0low, vec0high, vec1;
	const int32_t *curr, *top, *bottom;

	maxCols &= -8; // backward align

#define CompVHoughStdNmsGatherRowVec0_8mpd_Intrin_NEON(vec0_, vecAcc_, curr_, top_, bottom_) \
	/* ASM: curr, top and bottom loads are unaligned */  \
	vec1 = vcgtq_s32(vld1q_s32(&curr_[-1]), vecAcc_); \
	vec1 = vorrq_s32(vec1, vcgtq_s32(vld1q_s32(&curr_[+1]), vecAcc_)); \
	vec1 = vorrq_s32(vec1, vcgtq_s32(vld1q_s32(&top_[-1]), vecAcc_)); \
	vec1 = vorrq_s32(vec1, vcgtq_s32(vld1q_s32(&top_[0]), vecAcc_)); \
	vec1 = vorrq_s32(vec1, vcgtq_s32(vld1q_s32(&top_[+1]), vecAcc_)); \
	vec1 = vorrq_s32(vec1, vcgtq_s32(vld1q_s32(&bottom_[-1]), vecAcc_)); \
	vec1 = vorrq_s32(vec1, vcgtq_s32(vld1q_s32(&bottom_[0]), vecAcc_)); \
	vec1 = vorrq_s32(vec1, vcgtq_s32(vld1q_s32(&bottom_[+1]), vecAcc_)); \
	vec0_ = vandq_s32(vec0_, vec1)

	for (; colStart < maxCols; colStart += 8) {
		/* == Low part == */
		vecAcc = vld1q_s32(&pAcc[colStart]); // ASM: load unaligned
		vec0low = vcgtq_s32(vecAcc, vecThreshold);
		if (COMPV_ARM_NEON_NEQ_ZERO(vec0low)) {
			// TODO(dmi): ARM neon no need for curr, top and bottom -> use post-increment
			curr = &pAcc[colStart];
			top = &pAcc[colStart - stride];
			bottom = &pAcc[colStart + stride];
			CompVHoughStdNmsGatherRowVec0_8mpd_Intrin_NEON(vec0low, vecAcc, curr, top, bottom);
		}

		/* == High part == */
		vecAcc = vld1q_s32(&pAcc[colStart + 4]); // ASM: load unaligned
		vec0high = vcgtq_s32(vecAcc, vecThreshold);
		if (COMPV_ARM_NEON_NEQ_ZERO(vec0high)) {
			// TODO(dmi): ARM neon no need for curr, top and bottom -> use post-increment
			curr = &pAcc[colStart + 4];
			top = &pAcc[colStart + 4 - stride];
			bottom = &pAcc[colStart + 4 + stride];
			CompVHoughStdNmsGatherRowVec0_8mpd_Intrin_NEON(vec0high, vecAcc, curr, top, bottom);
		}
		
		vst1_u8(&pNms[colStart], vmovn_s16(vcombine_s16(vmovn_s32(vec0low), vmovn_s32(vec0high))));
	}

#undef CompVHoughStdNmsGatherRowVec0_8mpd_Intrin_NEON
}

void CompVHoughStdNmsApplyRow_Intrin_NEON(COMPV_ALIGNED(NEON) int32_t* pACC, COMPV_ALIGNED(NEON) uint8_t* pNMS, size_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t colStart, size_t maxCols, CompVHoughLineVector& lines)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const int32x4_t vecThreshold = vdupq_n_s32(static_cast<int32_t>(threshold));
	const int32_t thresholdInt32 = static_cast<int32_t>(threshold);
	static const uint8x16_t vecZero = vdupq_n_s32(0);
	int32x4_t vec0, vec1, vec2, vec3, vec4;
	uint8x8_t vec0n;
	int mask;
#if COMPV_ARCH_ARM32
	static const uint8x16_t vecMask = (uint32x4_t) { 0x8040201, 0x80402010, 0x8040201, 0x80402010 };
#else
	static const uint8x16_t vecMask = (uint64x2_t) { 9241421688590303745ULL, 9241421688590303745ULL };
#endif
#define CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, index) if (mask & (1<<index)) lines.push_back(CompVHoughLine(static_cast<compv_float32_t>(barrier - row), (colStart + index) * theta, static_cast<size_t>(pACC[(colStart + index)])))

	for (; colStart < maxCols - 15; colStart += 16) {
		vec0 = vceqq_u8(vld1q_u8(&pNMS[colStart]), vecZero);
		if (COMPV_ARM_NEON_NEQ_ZERO(vec0)) {
			vec1 = vcgtq_s32(vld1q_s32(&pACC[colStart]), vecThreshold);
			vec2 = vcgtq_s32(vld1q_s32(&pACC[colStart + 4]), vecThreshold);
			vec3 = vcgtq_s32(vld1q_s32(&pACC[colStart + 8]), vecThreshold);
			vec4 = vcgtq_s32(vld1q_s32(&pACC[colStart + 12]), vecThreshold);
			vec1 = vcombine_u8(
				vmovn_s16(vcombine_s16(vmovn_s32(vec1), vmovn_s32(vec2))),
				vmovn_s16(vcombine_s16(vmovn_s32(vec3), vmovn_s32(vec4))));
			vec0 = vandq_u8(vec0, vec1);

			// mask = _mm_movemask_epi8(vec0);
			vec0 = vandq_u8(vec0, vecMask);
			vec0n = vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0));
			vec0n = vpadd_u8(vec0n, vec0n);
			vec0n = vpadd_u8(vec0n, vec0n);
			mask = vget_lane_u16(vec0n, 0);
			if (mask) {
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 0); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 1);
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 2); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 3);
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 4); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 5);
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 6); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 7);
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 8); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 9);
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 10); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 11);
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 12); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 13);
				CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 14); CompVHoughStdNmsApplyRowPush_Intrin_NEON(mask, 15);
			}
		}
		vst1q_u8(&pNMS[colStart], vecZero);
	}

	for (; colStart < maxCols; ++colStart) {
		if (pNMS[colStart]) {
			pNMS[colStart] = 0; // reset for  next time
			// do not push the line
		}
		else if (pACC[colStart] > thresholdInt32) {
			lines.push_back(CompVHoughLine(
				static_cast<compv_float32_t>(barrier - row),
				colStart * theta,
				static_cast<size_t>(pACC[colStart])
			));
		}
	}
}

// pSinRho and rowTimesSinRhoPtr must be strided and NEON-aligned -> reading beyond count
// count must be >= 16
void CompVHoughStdRowTimesSinRho_Intrin_NEON(COMPV_ALIGNED(NEON) const int32_t* pSinRho, COMPV_ALIGNED(NEON) compv_uscalar_t row, COMPV_ALIGNED(NEON) int32_t* rowTimesSinRhoPtr, compv_uscalar_t count)
{
	const int32x4_t vecRowInt32 = vdupq_n_s32(static_cast<int32_t>(row));
	compv_uscalar_t i;
	for (i = 0; i < count - 15; i += 16) {
		vst1q_s32(&rowTimesSinRhoPtr[i + 0], vmulq_s32(vecRowInt32, vld1q_s32(&pSinRho[i])));
		vst1q_s32(&rowTimesSinRhoPtr[i + 4], vmulq_s32(vecRowInt32, vld1q_s32(&pSinRho[i + 4])));
		vst1q_s32(&rowTimesSinRhoPtr[i + 8], vmulq_s32(vecRowInt32, vld1q_s32(&pSinRho[i + 8])));
		vst1q_s32(&rowTimesSinRhoPtr[i + 12], vmulq_s32(vecRowInt32, vld1q_s32(&pSinRho[i + 12])));
	}
	for (; i < count; i += 4) {
		vst1q_s32(&rowTimesSinRhoPtr[i], vmulq_s32(vecRowInt32, vld1q_s32(&pSinRho[i])));
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
