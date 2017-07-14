/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghkht_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

/*
TODO(dmi): No ASM version but it's possible to have one if we write the result ("vote_count >= nThreshold") to
	a memory buffer then push to the vector in the c++ code
*/
// 8mpd -> minpack 8 for dwords (int32) - for rho_count
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVHoughKhtPeaks_Section3_4_VotesCount_8mpd_Intrin_AVX2(const int32_t *pcount, const size_t pcount_stride, const size_t theta_index, const size_t rho_count, const int32_t nThreshold, CompVHoughKhtVotes& votes)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	compv_uscalar_t rho_index;
	const int32_t *pcount_top, *pcount_bottom, *pcount_center;
	COMPV_ALIGN_AVX() int32_t vote_count[8];
	size_t rho_count_SSE = rho_count_SSE = (rho_count > 7) ? (rho_count - 7) : 0;
	int mask;

	// pcount.cols() have #2 more samples than rho_count which means no OutOfIndex issue for 'pcount_center[rho_index + 1]' (aka 'right') even for the last rho_index
	
	__m256i vec0, vec0Mask, vecPcount_center, vecPcount_top, vecPcount_bottom, vecVote_count;
	static const __m256i vecZero = _mm256_setzero_si256();
	const __m256i vecThreshold = _mm256_set1_epi32(nThreshold);

	for (rho_index = 1; rho_index < rho_count_SSE; rho_index += 8) {
		vec0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount[rho_index])); // same as vecPcount_center[0]
		vec0Mask = _mm256_cmpgt_epi32(vec0, vecZero);
		if (_mm256_movemask_epi8(vec0Mask)) {
			pcount_center = &pcount[rho_index];
			pcount_top = (pcount_center - pcount_stride);
			pcount_bottom = (pcount_center + pcount_stride);

			// To make the code cache friendly -> process left -> center -> right

			/* Left */
			vecPcount_center = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_center[-1]));
			vecPcount_top = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_top[-1]));
			vecPcount_bottom = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_bottom[-1]));
			vecVote_count = _mm256_add_epi32(
				_mm256_add_epi32(vecPcount_top, vecPcount_bottom),
				_mm256_slli_epi32(vecPcount_center, 1)
			);

			/* Center */
			// pcount_center[0] already loaded in vec0
			vecPcount_top = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_top[0]));
			vecPcount_bottom = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_bottom[0]));
			vecVote_count = _mm256_add_epi32(vecVote_count,
				_mm256_add_epi32(
					_mm256_add_epi32(_mm256_slli_epi32(vecPcount_top, 1), _mm256_slli_epi32(vecPcount_bottom, 1)),
					_mm256_slli_epi32(vec0, 2)
				)
			);

			/* Right */
			vecPcount_center = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_center[1]));
			vecPcount_top = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_top[1]));
			vecPcount_bottom = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pcount_bottom[1]));
			vecVote_count = _mm256_add_epi32(vecVote_count,
				_mm256_add_epi32(
					_mm256_add_epi32(vecPcount_top, vecPcount_bottom),
					_mm256_slli_epi32(vecPcount_center, 1)
				)
			);

			vec0Mask = _mm256_and_si256(
				vec0Mask,
				_mm256_or_si256(_mm256_cmpeq_epi32(vecVote_count, vecThreshold), _mm256_cmpgt_epi32(vecVote_count, vecThreshold)) // TODO(dmi): for neon use '_mm256_cmpge_epi32' which doesn't exist
			);
			mask = _mm256_movemask_epi8(vec0Mask);
			if (mask) {
				_mm256_store_si256(reinterpret_cast<__m256i*>(vote_count), vecVote_count);
				if (mask & 0x0000000f) {
					votes.push_back(CompVHoughKhtVote(rho_index + 0, theta_index, vote_count[0]));
				}
				if (mask & 0x000000f0) {
					votes.push_back(CompVHoughKhtVote(rho_index + 1, theta_index, vote_count[1]));
				}
				if (mask & 0x00000f00) {
					votes.push_back(CompVHoughKhtVote(rho_index + 2, theta_index, vote_count[2]));
				}
				if (mask & 0x0000f000) {
					votes.push_back(CompVHoughKhtVote(rho_index + 3, theta_index, vote_count[3]));
				}
				if (mask & 0x000f0000) {
					votes.push_back(CompVHoughKhtVote(rho_index + 4, theta_index, vote_count[4]));
				}
				if (mask & 0x00f00000) {
					votes.push_back(CompVHoughKhtVote(rho_index + 5, theta_index, vote_count[5]));
				}
				if (mask & 0x0f000000) {
					votes.push_back(CompVHoughKhtVote(rho_index + 6, theta_index, vote_count[6]));
				}
				if (mask & 0xf0000000) {
					votes.push_back(CompVHoughKhtVote(rho_index + 7, theta_index, vote_count[7]));
				}
			}
		}
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
