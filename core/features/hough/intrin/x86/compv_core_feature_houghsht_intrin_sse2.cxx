/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/intrin/x86/compv_core_feature_houghsht_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// 4mpd -> minpack 4 for dwords (int32) - for maxCols
void CompVHoughShtNmsGatherRow_4mpd_Intrin_SSE2(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, compv_uscalar_t nThreshold, compv_uscalar_t colStart, compv_uscalar_t maxCols)
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
			// TODO(dmi): ARM neon no need for curr, top and bottom -> use post-increment
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

void CompVHoughShtNmsApplyRow_Intrin_SSE2(COMPV_ALIGNED(SSE) int32_t* pACC, COMPV_ALIGNED(SSE) uint8_t* pNMS, size_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t colStart, size_t maxCols, CompVHoughLineVector& lines)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecThreshold = _mm_set1_epi32(static_cast<int32_t>(threshold));
	const int32_t thresholdInt32 = static_cast<int32_t>(threshold);
	static const __m128i vecZero = _mm_setzero_si128();
	__m128i vec0, vec1, vec2, vec3, vec4;
	int mask;
#define CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, index) if (mask & (1<<index)) lines.push_back(CompVHoughLine(static_cast<compv_float32_t>(barrier - row), (colStart + index) * theta, static_cast<size_t>(pACC[(colStart + index)])))

	for (; colStart < maxCols - 15; colStart += 16) {
		vec0 = _mm_cmpeq_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(&pNMS[colStart])), vecZero);
		if (_mm_movemask_epi8(vec0)) {
			vec1 = _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[colStart])), vecThreshold);
			vec2 = _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[colStart + 4])), vecThreshold);
			vec3 = _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[colStart + 8])), vecThreshold);
			vec4 = _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[colStart + 12])), vecThreshold);
			vec1 = _mm_packs_epi32(vec1, vec2);
			vec3 = _mm_packs_epi32(vec3, vec4);
			vec1 = _mm_packs_epi16(vec1, vec3);
			vec0 = _mm_and_si128(vec0, vec1);
			mask = _mm_movemask_epi8(vec0);
			if (mask) {
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 0); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 1);
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 2); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 3);
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 4); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 5);
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 6); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 7);
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 8); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 9);
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 10); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 11);
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 12); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 13);
				CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 14); CompVHoughShtNmsApplyRowPush_Intrin_SSE2(mask, 15);
			}
		}
		_mm_store_si128(reinterpret_cast<__m128i*>(&pNMS[colStart]), vecZero);
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

// pSinRho and rowTimesSinRhoPtr must be strided and SSE-aligned -> reading beyond count
// count must be >= 16
// Not optimized, see SSE41 version
void CompVHoughShtRowTimesSinRho_Intrin_SSE2(COMPV_ALIGNED(SSE) const int32_t* pSinRho, COMPV_ALIGNED(SSE) compv_uscalar_t row, COMPV_ALIGNED(SSE) int32_t* rowTimesSinRhoPtr, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecRowInt32 = _mm_set1_epi32(static_cast<int32_t>(row));
	compv_uscalar_t i;
	for (i = 0; i < count - 15; i += 16) {
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i]),
			_mm_mullo_epi32_SSE2(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i]))));
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i + 4]),
			_mm_mullo_epi32_SSE2(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i + 4]))));
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i + 8]),
			_mm_mullo_epi32_SSE2(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i + 8]))));
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i + 12]),
			_mm_mullo_epi32_SSE2(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i + 12]))));
	}
	for (; i < count; i += 4) {
		_mm_store_si128(reinterpret_cast<__m128i*>(&rowTimesSinRhoPtr[i]),
			_mm_mullo_epi32_SSE2(vecRowInt32, _mm_load_si128(reinterpret_cast<const __m128i*>(&pSinRho[i]))));
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
