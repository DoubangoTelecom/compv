/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/hough/compv_feature_houghstd_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/features/hough/compv_feature_houghstd.h"
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM
void HoughStdNmsGatherRow_Intrin_SSE2(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, int32_t nThreshold, compv_uscalar_t width)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t col, maxColsSSE = width - 3; // must not go beyond the stride as "acc" and "nms" have different ones
	const __m128i xmmThreshold = _mm_set1_epi32(nThreshold);
	int stride = static_cast<int>(nAccStride);
	__m128i xmmAcc, xmm0, xmm1;
	const int32_t *curr, *top, *bottom;
	int mask;

	for (col = 1; col < maxColsSSE; col += 4) {
		xmmAcc = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pAcc[col]));
		xmm0 = _mm_cmpgt_epi32(xmmAcc, xmmThreshold);
		if (_mm_movemask_epi8(xmm0)) {
			curr = &pAcc[col];
			top = &pAcc[col - stride];
			bottom = &pAcc[col + stride];
			xmm1 = _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&curr[-1])), xmmAcc);
			xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&curr[+1])), xmmAcc));
			xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[-1])), xmmAcc));
			xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[0])), xmmAcc));
			xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[+1])), xmmAcc));
			xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[-1])), xmmAcc));
			xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[0])), xmmAcc));
			xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[+1])), xmmAcc));
			xmm0 = _mm_and_si128(xmm0, xmm1);
			mask = _mm_movemask_epi8(xmm0);
			if (mask) {
				pNms[col + 0] = (mask & 0x000F);
				pNms[col + 1] = (mask & 0x00F0) >> 4;
				pNms[col + 2] = (mask & 0x0F00) >> 8;
				pNms[col + 3] = (mask & 0xF000) >> 12;
			}
		}
	}
	if (col < width) {
		HoughStdNmsGatherRow_C(pAcc, nAccStride, pNms, nThreshold, col, width);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
