/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/arm/compv_core_feature_houghkht_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

/*
TODO(dmi): No ASM version but it's possible to have one if we write the result ("vote_count >= nThreshold") to
a memory buffer then push to the vector in the c++ code
*/
// 4mpd -> minpack 4 for dwords (int32) - for rho_count
void CompVHoughKhtPeaks_Section3_4_VotesCount_4mpd_Intrin_NEON(const int32_t *pcount, const size_t pcount_stride, const size_t theta_index, const size_t rho_count, const int32_t nThreshold, CompVHoughKhtVotes& votes)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t rho_index;
	const int32_t *pcount_top, *pcount_bottom, *pcount_center;
	size_t rho_count_NEON = rho_count_NEON = (rho_count > 3) ? (rho_count - 3) : 0;

	// pcount.cols() have #2 more samples than rho_count which means no OutOfIndex issue for 'pcount_center[rho_index + 1]' (aka 'right') even for the last rho_index

	int32x4_t vec0, vec0Mask, vecPcount_center, vecPcount_top, vecPcount_bottom, vecVote_count;
	static const int32x4_t vecZero = vdupq_n_s32(0);
	const int32x4_t vecThreshold = vdupq_n_s32(nThreshold);

	for (rho_index = 1; rho_index < rho_count_NEON; rho_index += 4) {
		vec0 = vld1q_s32(&pcount[rho_index]); // same as vecPcount_center[0]
		vec0Mask = vcgtq_s32(vec0, vecZero);
		if (COMPV_ARM_NEON_NEQ_ZERO(vec0Mask)) {
			pcount_center = &pcount[rho_index];
			pcount_top = (pcount_center - pcount_stride);
			pcount_bottom = (pcount_center + pcount_stride);

			// To make the code cache friendly -> process left -> center -> right

			/* Left */
			vecPcount_center = vld1q_s32(&pcount_center[-1]);
			vecPcount_top = vld1q_s32(&pcount_top[-1]);
			vecPcount_bottom = vld1q_s32(&pcount_bottom[-1]);
			vecVote_count = vaddq_s32(
				vaddq_s32(vecPcount_top, vecPcount_bottom),
				vshlq_n_s32(vecPcount_center, 1)
			);

			/* Center */
			// pcount_center[0] already loaded in vec0
			vecPcount_top = vld1q_s32(&pcount_top[0]);
			vecPcount_bottom = vld1q_s32(&pcount_bottom[0]);
			vecVote_count = vaddq_s32(vecVote_count,
				vaddq_s32(
					vaddq_s32(vshlq_n_s32(vecPcount_top, 1), vshlq_n_s32(vecPcount_bottom, 1)),
					vshlq_n_s32(vec0, 2)
				)
			);

			/* Right */
			vecPcount_center = vld1q_s32(&pcount_center[1]);
			vecPcount_top = vld1q_s32(&pcount_top[1]);
			vecPcount_bottom = vld1q_s32(&pcount_bottom[1]);
			vecVote_count = vaddq_s32(vecVote_count,
				vaddq_s32(
					vaddq_s32(vecPcount_top, vecPcount_bottom),
					vshlq_n_s32(vecPcount_center, 1)
				)
			);

			vec0Mask = vandq_s32(
				vec0Mask,
				vcgeq_s32(vecVote_count, vecThreshold)
			);
			
			if (vgetq_lane_s32(vec0Mask, 0)) {
				votes.push_back(CompVHoughKhtVote(rho_index + 0, theta_index, vgetq_lane_s32(vecVote_count, 0)));
			}
			if (vgetq_lane_s32(vec0Mask, 1)) {
				votes.push_back(CompVHoughKhtVote(rho_index + 1, theta_index, vgetq_lane_s32(vecVote_count, 1)));
			}
			if (vgetq_lane_s32(vec0Mask, 2)) {
				votes.push_back(CompVHoughKhtVote(rho_index + 2, theta_index, vgetq_lane_s32(vecVote_count, 2)));
			}
			if (vgetq_lane_s32(vec0Mask, 3)) {
				votes.push_back(CompVHoughKhtVote(rho_index + 3, theta_index, vgetq_lane_s32(vecVote_count, 3)));
			}
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
