/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghkht_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

/*
TODO(dmi): No ASM version but it's possible to have one if we write the result ("vote_count >= nThreshold") to
	a memory buffer then push to the vector in the c++ code
*/
// 4mpd -> minpack 4 for dwords (int32) - for rho_count
void CompVHoughKhtPeaks_Section3_4_VotesCount_4mpd_Intrin_SSE2(const int32_t *pcount, const size_t pcount_stride, const size_t theta_index, const size_t rho_count, const int32_t nThreshold, CompVHoughKhtVotes& votes)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t rho_index;
	const int32_t *pcount_top, *pcount_bottom, *pcount_center;
	COMPV_ALIGN_SSE() int32_t vote_count[4];
	size_t rho_count_SSE = rho_count_SSE = (rho_count > 3) ? (rho_count - 3) : 0;
	int mask;

	// pcount.cols() have #2 more samples than rho_count which means no OutOfIndex issue for 'pcount_center[rho_index + 1]' (aka 'right') even for the last rho_index
	
	__m128i vec0, vec0Mask, vecPcount_center, vecPcount_top, vecPcount_bottom, vecVote_count;
	static const __m128i vecZero = _mm_setzero_si128();
	const __m128i vecThreshold = _mm_set1_epi32(nThreshold);

	for (rho_index = 1; rho_index < rho_count_SSE; rho_index += 4) {
		vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount[rho_index])); // same as vecPcount_center[0]
		vec0Mask = _mm_cmpgt_epi32(vec0, vecZero);
		if (_mm_movemask_epi8(vec0Mask)) {
			pcount_center = &pcount[rho_index];
			pcount_top = (pcount_center - pcount_stride);
			pcount_bottom = (pcount_center + pcount_stride);

			// To make the code cache friendly -> process left -> center -> right

			/* Left */
			vecPcount_center = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_center[-1]));
			vecPcount_top = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_top[-1]));
			vecPcount_bottom = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_bottom[-1]));
			vecVote_count = _mm_add_epi32(
				_mm_add_epi32(vecPcount_top, vecPcount_bottom),
				_mm_slli_epi32(vecPcount_center, 1)
			);

			/* Center */
			// pcount_center[0] already loaded in vec0
			vecPcount_top = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_top[0]));
			vecPcount_bottom = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_bottom[0]));
			vecVote_count = _mm_add_epi32(vecVote_count,
				_mm_add_epi32(
					_mm_add_epi32(_mm_slli_epi32(vecPcount_top, 1), _mm_slli_epi32(vecPcount_bottom, 1)),
					_mm_slli_epi32(vec0, 2)
				)
			);

			/* Right */
			vecPcount_center = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_center[1]));
			vecPcount_top = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_top[1]));
			vecPcount_bottom = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcount_bottom[1]));
			vecVote_count = _mm_add_epi32(vecVote_count,
				_mm_add_epi32(
					_mm_add_epi32(vecPcount_top, vecPcount_bottom),
					_mm_slli_epi32(vecPcount_center, 1)
				)
			);

			vec0Mask = _mm_and_si128(
				vec0Mask,
				_mm_or_si128(_mm_cmpeq_epi32(vecVote_count, vecThreshold), _mm_cmpgt_epi32(vecVote_count, vecThreshold)) // TODO(dmi): for neon use '_mm_cmpge_epi32' which doesn't exist
			);
			mask = _mm_movemask_epi8(vec0Mask);
			if (mask) {
				_mm_store_si128(reinterpret_cast<__m128i*>(vote_count), vecVote_count);
				if (mask & 0x000f) {
					votes.push_back(CompVHoughKhtVote(rho_index + 0, theta_index, vote_count[0]));
				}
				if (mask & 0x00f0) {
					votes.push_back(CompVHoughKhtVote(rho_index + 1, theta_index, vote_count[1]));
				}
				if (mask & 0x0f00) {
					votes.push_back(CompVHoughKhtVote(rho_index + 2, theta_index, vote_count[2]));
				}
				if (mask & 0xf000) {
					votes.push_back(CompVHoughKhtVote(rho_index + 3, theta_index, vote_count[3]));
				}
			}
		}
	}
}

// up to the caller to set padding bytes to zeros (otherwise max will be invalid)
// 2mpq -> minpack 2 for qwords (float64) - for count
void CompVHoughKhtKernelHeight_2mpq_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(SSE) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(SSE) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(SSE) const compv_float64_t* n_scale,
	COMPV_ALIGNED(SSE) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(SSE) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(SSE) compv_float64_t* m2, COMPV_ALIGNED(SSE) compv_float64_t* sigma_theta_square,
	COMPV_ALIGNED(SSE) compv_float64_t* height, COMPV_ALIGNED(SSE) compv_float64_t* heightMax1, COMPV_ALIGNED(SSE) compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	static const __m128d vecTwoPi = _mm_set1_pd(6.2831853071795862); // 0x401921fb54442d18
	static const __m128d vecOne = _mm_set1_pd(1.0); // 0x3ff0000000000000
	static const __m128d vecFour = _mm_set1_pd(4.0); // 0x4010000000000000
	static const __m128d vecZeroDotOne = _mm_set1_pd(0.1); // 0x3fb999999999999a
	static const __m128d vecZero = _mm_set1_pd(0.0);
	__m128d vecheightMax1, vecM_Eq14_0, vecM_Eq14_2;
	__m128d vecSigma_rho_square, vecSigma_rho_times_sigma_theta, vecSigma_rho_times_theta, vecSigma_theta_square;
	__m128d vecOne_minus_r_square, vecHeight;
	__m128d vecMaskEqZero;

	vecheightMax1 = _mm_load_sd(heightMax1);

	for (compv_uscalar_t i = 0; i < count; i += 2) {
		vecSigma_theta_square = _mm_div_pd(vecOne, _mm_load_pd(&M_Eq14_r0[i]));
		vecM_Eq14_0 = _mm_load_pd(&M_Eq14_0[i]);
		vecM_Eq14_2 = _mm_load_pd(&M_Eq14_2[i]);
		vecSigma_rho_times_theta = _mm_mul_pd(vecM_Eq14_0, vecSigma_theta_square);
		vecSigma_theta_square = _mm_mul_pd(vecM_Eq14_2, vecSigma_theta_square);
		vecSigma_rho_square = _mm_add_pd(_mm_mul_pd(vecSigma_rho_times_theta, vecM_Eq14_0), _mm_load_pd(&n_scale[i]));
		vecSigma_rho_times_theta = _mm_mul_pd(vecSigma_rho_times_theta, vecM_Eq14_2);
		vecM_Eq14_0 = _mm_mul_pd(vecM_Eq14_0, vecSigma_theta_square);
		vecSigma_theta_square = _mm_mul_pd(vecSigma_theta_square, vecM_Eq14_2);
		vecMaskEqZero = _mm_cmpneq_pd(vecZero, vecSigma_theta_square);
		vecSigma_theta_square = _mm_or_pd(_mm_andnot_pd(vecMaskEqZero, vecZeroDotOne), _mm_and_pd(vecSigma_theta_square, vecMaskEqZero));
		vecSigma_rho_square = _mm_mul_pd(vecSigma_rho_square, vecFour);
		vecSigma_theta_square = _mm_mul_pd(vecSigma_theta_square, vecFour);
		vecSigma_rho_times_sigma_theta = _mm_mul_pd(_mm_sqrt_pd(vecSigma_rho_square), _mm_sqrt_pd(vecSigma_theta_square));
		vecOne_minus_r_square = _mm_div_pd(vecSigma_rho_times_theta, vecSigma_rho_times_sigma_theta);
		vecOne_minus_r_square = _mm_sub_pd(vecOne, _mm_mul_pd(vecOne_minus_r_square, vecOne_minus_r_square));
		vecOne_minus_r_square = _mm_sqrt_pd(vecOne_minus_r_square);
		vecOne_minus_r_square = _mm_mul_pd(vecOne_minus_r_square, vecSigma_rho_times_sigma_theta);
		vecOne_minus_r_square = _mm_mul_pd(vecOne_minus_r_square, vecTwoPi);
		vecHeight = _mm_div_pd(vecOne, vecOne_minus_r_square);
		
		_mm_store_pd(&sigma_rho_square[i], vecSigma_rho_square);
		_mm_store_pd(&sigma_rho_times_theta[i], vecSigma_rho_times_theta);
		_mm_store_pd(&m2[i], vecM_Eq14_0);
		_mm_store_pd(&sigma_theta_square[i], vecSigma_theta_square);
		_mm_store_pd(&height[i], vecHeight);
		vecheightMax1 = _mm_max_pd(vecheightMax1, vecHeight);
	}
	vecheightMax1 = _mm_max_sd(vecheightMax1, _mm_shuffle_pd(vecheightMax1, vecheightMax1, 0x01));
	_mm_store_sd(heightMax1, vecheightMax1);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
