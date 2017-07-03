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

// FIXME: remove
// https://en.wikipedia.org/wiki/Taylor_series#Exponential_function
COMPV_ALWAYS_INLINE double __compv_math_exp_fast_small(double x) {
#if 1
	static const double scale = 1.0 / 1024.0;
	x = 1.0 + (x * scale);
	x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x; x *= x;
	return x;
#else
	return std::exp(x);
#endif
}

#if COMPV_ARCH_ARM64
void CompVHoughKhtGauss_Eq15_Intrin_NEON(const double rho, const double theta, const double(*M)[4], double* result1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Not faster");
	static const float64x1_t one = vdup_n_f64(1.0);
	static const float64x1_t two = vdup_n_f64(2.0);
	static const float64x1_t twopi = vdup_n_f64(2.0 * COMPV_MATH_PI);
	const float64x1_t rho_ = vld1_f64(&rho);
	const float64x1_t theta_ = vld1_f64(&theta);
	const float64x1_t sigma_theta_square = vld1_f64(&(*M)[3]);
	const float64x1_t sigma_rho_square = vld1_f64(&(*M)[0]);
	const float64x1_t sigma_rho_times_theta = vld1_f64(&(*M)[1]);
	const float64x1_t sigma_rho_times_sigma_theta = vmul_f64(vsqrt_f64(sigma_rho_square), vsqrt_f64(sigma_theta_square));
	const float64x1_t sigma_rho_times_sigma_theta_scale = vrecpe_f64(sigma_rho_times_sigma_theta);
	const float64x1_t r = vmul_f64(sigma_rho_times_theta, sigma_rho_times_sigma_theta_scale);
	const float64x1_t one_minus_r_square = vsub_f64(one, vmul_f64(r, r));
	const float64x1_t x = vrecpe_f64(vmul_f64(vmul_f64(twopi, sigma_rho_times_sigma_theta), vsqrt_f64(one_minus_r_square)));
	const float64x1_t y = vrecpe_f64(vmul_f64(two, one_minus_r_square));
	const float64x1_t a = vmul_f64(vmul_f64(rho_, rho_), vrecpe_f64(sigma_rho_square));
	const float64x1_t b = vmul_f64(vmul_f64(vmul_f64(r, two), vmul_f64(rho_, theta_)), sigma_rho_times_sigma_theta_scale);
	const float64x1_t c = vmul_f64(vmul_f64(theta_, theta_), vrecpe_f64(sigma_theta_square));
	const float64x1_t z = vneg_f64(vadd_f64(vsub_f64(a, b), c));
	const float64x1_t zy = vmul_f64(z, y);
	*result1 = vget_lane_f64(x, 0) * __compv_math_exp_fast_small(vget_lane_f64(zy, 0));
}
#endif /* COMPV_ARCH_ARM64 */

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
