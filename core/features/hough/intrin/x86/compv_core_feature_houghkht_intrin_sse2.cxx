/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
