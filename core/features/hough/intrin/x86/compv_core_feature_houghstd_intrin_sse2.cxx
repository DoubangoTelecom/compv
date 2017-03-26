/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghstd_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// 4mpd -> minpack 4 for dwords (int32) - for maxCols
void CompVHoughStdNmsGatherRow_4mpd_Intrin_SSE2(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, compv_uscalar_t nThreshold, compv_uscalar_t colStart, compv_uscalar_t maxCols)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecThreshold = _mm_set1_epi32(static_cast<int32_t>(nThreshold));
	int stride = static_cast<int>(nAccStride);
	__m128i vecAcc, vec0, vec1;
	const int32_t *curr, *top, *bottom;

	maxCols &= -4; // backward align

	for (; colStart < maxCols; colStart += 4) {
		vecAcc = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pAcc[colStart]));
		vec0 = _mm_cmpgt_epi32(vecAcc, vecThreshold);
		if (_mm_movemask_epi8(vec0)) {
			curr = &pAcc[colStart];
			top = &pAcc[colStart - stride];
			bottom = &pAcc[colStart + stride];
			vec1 = _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&curr[-1])), vecAcc);
			vec1 = _mm_or_si128(vec1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&curr[+1])), vecAcc));
			vec1 = _mm_or_si128(vec1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[-1])), vecAcc));
			vec1 = _mm_or_si128(vec1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[0])), vecAcc));
			vec1 = _mm_or_si128(vec1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[+1])), vecAcc));
			vec1 = _mm_or_si128(vec1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[-1])), vecAcc));
			vec1 = _mm_or_si128(vec1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[0])), vecAcc));
			vec1 = _mm_or_si128(vec1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[+1])), vecAcc));
			vec0 = _mm_and_si128(vec0, vec1);
			*reinterpret_cast<uint32_t*>(&pNms[colStart]) = static_cast<uint32_t>(
				_mm_cvtsi128_si32(
					_mm_packs_epi16(
						_mm_packs_epi32(vec0, vec0), 
						vec0))); // ASM: second argument for packs could be anything, we only need the first 4 Bytes
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
