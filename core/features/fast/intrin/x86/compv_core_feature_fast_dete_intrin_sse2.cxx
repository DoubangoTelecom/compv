/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/fast/intrin/x86/compv_core_feature_fast_dete_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

#define _mm_fast_check(a, b) \
	vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&circle[a][i])); \
	vec1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&circle[b][i])); \
	if (!_mm_movemask_epi8(_mm_or_si128( \
		_mm_or_si128(_mm_cmplt_epu8_SSE2(vec0, vecDarker1, vecZero, vec0xFF), _mm_cmplt_epu8_SSE2(vec1, vecDarker1, vecZero, vec0xFF)), \
		_mm_or_si128(_mm_cmpgt_epu8_SSE2(vec0, vecBrighter1, vecZero, vec0xFF), _mm_cmpgt_epu8_SSE2(vec1, vecBrighter1, vecZero, vec0xFF)) \
	))) goto Next; \
	vecCircle16[a] = vec0, vecCircle16[b] = vec1
	

#define _mm_fast_compute_Darkers(a, b, c, d) \
	vecDiff16[a] = _mm_subs_epu8(vecDarker1, vecCircle16[a]); \
	vecDiff16[b] = _mm_subs_epu8(vecDarker1, vecCircle16[b]); \
	vecDiff16[c] = _mm_subs_epu8(vecDarker1, vecCircle16[c]); \
	vecDiff16[d] = _mm_subs_epu8(vecDarker1, vecCircle16[d])
#define _mm_fast_compute_Brighters(a, b, c, d) \
	vecDiff16[a] = _mm_subs_epu8(vecCircle16[a], vecBrighter1); \
	vecDiff16[b] = _mm_subs_epu8(vecCircle16[b], vecBrighter1); \
	vecDiff16[c] = _mm_subs_epu8(vecCircle16[c], vecBrighter1); \
	vecDiff16[d] = _mm_subs_epu8(vecCircle16[d], vecBrighter1)

#define _mm_fast_load(a, b, c, d, type) \
	_mm_fast_compute_##type##s(a, b, c, d); \
	vecDiffBinary16[a] = _mm_cmpnot_epu8_SSE2(vecDiff16[a], vecZero, vecOne); \
	vecDiffBinary16[b] = _mm_cmpnot_epu8_SSE2(vecDiff16[b], vecZero, vecOne); \
	vecDiffBinary16[c] = _mm_cmpnot_epu8_SSE2(vecDiff16[c], vecZero, vecOne); \
	vecDiffBinary16[d] = _mm_cmpnot_epu8_SSE2(vecDiff16[d], vecZero, vecOne); \
	vecSum1 = _mm_add_epi8(vecSum1, _mm_add_epi8(vecDiffBinary16[a], vecDiffBinary16[b])); \
	vecSum1 = _mm_add_epi8(vecSum1, _mm_add_epi8(vecDiffBinary16[c], vecDiffBinary16[d]))

// Sum arc #0 without the tail (#NminusOne lines)
#define _mm_fast_init_diffbinarysum(vecSum1, vecBinary16) \
	vecSum1 = _mm_add_epi8(vecBinary16[0], vecBinary16[1]); \
	vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[2]); \
	vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[3]); \
	vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[4]); \
	vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[5]); \
	vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[6]); \
	vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[7]); \
	if (N == 12) { \
		vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[8]); \
		vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[9]); \
		vecSum1 = _mm_add_epi8(vecSum1, vecBinary16[10]); \
	}
#define _mm_fast_strength(ii, vecSum1, vecDiff16, vecStrengths, vecTemp) \
	vecSum1 = _mm_add_epi8(vecSum1, vecDiffBinary16[(NminusOne + ii) & 15]); /* add tail */ \
	if (_mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinusOne))) { \
		vecTemp = vecDiff16[(ii + 0) & 15]; \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 1) & 15], vecTemp); \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 2) & 15], vecTemp); \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 3) & 15], vecTemp); \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 4) & 15], vecTemp); \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 5) & 15], vecTemp); \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 6) & 15], vecTemp); \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 7) & 15], vecTemp); \
		vecTemp = _mm_min_epu8(vecDiff16[(ii + 8) & 15], vecTemp); \
		if (N == 12) { \
			vecTemp = _mm_min_epu8(vecDiff16[(ii + 9) & 15], vecTemp); \
			vecTemp = _mm_min_epu8(vecDiff16[(ii + 10) & 15], vecTemp); \
			vecTemp = _mm_min_epu8(vecDiff16[(ii + 11) & 15], vecTemp); \
		} \
		vecStrengths = _mm_max_epu8(vecStrengths, vecTemp); \
	} \
	vecSum1 = _mm_sub_epi8(vecSum1, vecDiffBinary16[ii & 15]) /* sub head */

COMPV_NAMESPACE_BEGIN()

// No need to check for 'width'. The caller ('CompVFastProcessRange' function) already checked and prepared it for SSE.
void CompVFastDataRow_Intrin_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_uscalar_t width, const compv_scalar_t *pixels16, compv_uscalar_t N, compv_uscalar_t threshold, uint8_t* strengths)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t i;
	compv_uscalar_t NminusOne = N - 1;
	const compv_uscalar_t minsum = (N == 12 ? 3 : 2);
	const __m128i vecThreshold = _mm_set1_epi8(static_cast<int8_t>(threshold));
	const __m128i vecNMinSumMinusOne = _mm_set1_epi8(static_cast<int8_t>(minsum - 1)); // no '_mm_cmpge_epu8'
	const __m128i vecNMinusOne = _mm_set1_epi8(static_cast<int8_t>(NminusOne)); // no '_mm_cmpge_epu8'
	const __m128i vecOne = _mm_load_si128(reinterpret_cast<const __m128i*>(k1_i8));
	const __m128i vecZero = _mm_setzero_si128();
	const __m128i vec0xFF = _mm_cmpeq_epi8(vecZero, vecZero); // 0xFF
	__m128i vec0, vec1, vecSum1, vecStrengths, vecBrighter1, vecDarker1, vecDiffBinary16[16], vecDiff16[16], vecCircle16[16];
	const uint8_t* circle[16] = {
		&IP[pixels16[0]], &IP[pixels16[1]], &IP[pixels16[2]], &IP[pixels16[3]],
		&IP[pixels16[4]], &IP[pixels16[5]], &IP[pixels16[6]], &IP[pixels16[7]],
		&IP[pixels16[8]], &IP[pixels16[9]], &IP[pixels16[10]], &IP[pixels16[11]],
		&IP[pixels16[12]], &IP[pixels16[13]], &IP[pixels16[14]], &IP[pixels16[15]]
	};

	for (i = 0; i < width; i += 16) {
		vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&IP[i]));
		vecDarker1 = _mm_subs_epu8(vec0, vecThreshold); // IP < (Ix - t)
		vecBrighter1 = _mm_adds_epu8(vec0, vecThreshold); // Ip > (Ix + t)
		vecStrengths = _mm_setzero_si128();

		/* Check */ {
			// Order is important for performance: When (a, b) is a candidate pair then the points close to 
			// it are also probable candidates. This is what we check farrest neighborhoods. For example,
			// (4, 12) is the farrestneighborhood for (0, 8)
			_mm_fast_check(0, 8); _mm_fast_check(4, 12);
			_mm_fast_check(1, 9); _mm_fast_check(5, 13);
			_mm_fast_check(2, 10); _mm_fast_check(6, 14);
			_mm_fast_check(3, 11); _mm_fast_check(7, 15);
		}
		
		/* Darkers */ {
			vecSum1 = _mm_setzero_si128();
			_mm_fast_load(0, 8, 4, 12, Darker);
			if (!_mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne))) goto EndOfDarkers; // at least #3 for FAST12 and #2 for FAST9
			_mm_fast_load(1, 9, 5, 13, Darker);
			_mm_fast_load(2, 10, 6, 14, Darker);
			_mm_fast_load(3, 11, 7, 15, Darker);
			if (!_mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinusOne))) goto EndOfDarkers; // at least #12 for FAST12 and #9 for FAST9
			
			_mm_fast_init_diffbinarysum(vecSum1, vecDiffBinary16);
			_mm_fast_strength(0, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(1, vecSum1, vecDiff16, vecStrengths, vec0); 
			_mm_fast_strength(2, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(3, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(4, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(5, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(6, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(7, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(8, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(9, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(10, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(11, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(12, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(13, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(14, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(15, vecSum1, vecDiff16, vecStrengths, vec0);
		} EndOfDarkers:

		/* Brighters */ {
			vecSum1 = _mm_setzero_si128();
			_mm_fast_load(0, 8, 4, 12, Brighter);
			if (!_mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne))) goto EndOfBrighters; // at least #3 for FAST12 and #2 for FAST9
			_mm_fast_load(1, 9, 5, 13, Brighter);
			_mm_fast_load(2, 10, 6, 14, Brighter);
			_mm_fast_load(3, 11, 7, 15, Brighter);
			if (!_mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinusOne))) goto EndOfBrighters; // at least #12 for FAST12 and #9 for FAST9
			
			_mm_fast_init_diffbinarysum(vecSum1, vecDiffBinary16);
			_mm_fast_strength(0, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(1, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(2, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(3, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(4, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(5, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(6, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(7, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(8, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(9, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(10, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(11, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(12, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(13, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(14, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(15, vecSum1, vecDiff16, vecStrengths, vec0);
		} EndOfBrighters:
		
Next:
		_mm_storeu_si128(reinterpret_cast<__m128i*>(&strengths[i]), vecStrengths);
	}
}

void CompVFastNmsGather_Intrin_SSE2(const uint8_t* pcStrengthsMap, uint8_t* pNMS, const compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t i, j;
	__m128i vecStrength, vec0, vec1;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	const __m128i vecZero = _mm_setzero_si128();
	const __m128i vec0xFF = _mm_cmpeq_epi8(vecZero, vecZero); // 0xFF
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 3; i < width - 3; i += 16) {
			vecStrength = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i]));
			vec1 = _mm_cmpnot_epu8_SSE2(vecStrength, vecZero, vec0xFF);
			if (_mm_movemask_epi8(vec1)) {
				vec0 = _mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i - 1])), vecStrength);  // left
				vec0 = _mm_or_si128(vec0,
					_mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i + 1])), vecStrength)); // right
				vec0 = _mm_or_si128(vec0,
					_mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i - stride - 1])), vecStrength)); // left-top
				vec0 = _mm_or_si128(vec0,
					_mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i - stride])), vecStrength)); // top
				vec0 = _mm_or_si128(vec0,
					_mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i - stride + 1])), vecStrength)); // right-top
				vec0 = _mm_or_si128(vec0,
					_mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i + stride - 1])), vecStrength)); // left-bottom
				vec0 = _mm_or_si128(vec0,
					_mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i + stride])), vecStrength)); // bottom
				vec0 = _mm_or_si128(vec0,
					_mm_cmpge_epu8_SSE2(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i + stride + 1])), vecStrength)); // right-bottom

				// 'and' used to ignore nonzero coeffs. Zero-coeffs are always suppressed (0# >= strength) which means too much work for the nmsApply() function
				_mm_storeu_si128(reinterpret_cast<__m128i*>(&pNMS[i]), _mm_and_si128(vec0, vec1));
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

void CompVFastNmsApply_Intrin_SSE2(COMPV_ALIGNED(SSE) uint8_t* pcStrengthsMap, COMPV_ALIGNED(SSE) uint8_t* pNMS, compv_uscalar_t width, compv_uscalar_t heigth, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t i, j;
	__m128i vec0;
	pcStrengthsMap += (stride * 3);
	pNMS += (stride * 3);
	const __m128i vecZero = _mm_setzero_si128();
	const __m128i vec0xFF = _mm_cmpeq_epi8(vecZero, vecZero); // 0xFF
	for (j = 3; j < heigth - 3; ++j) {
		for (i = 0; i < width; i += 16) { // SIMD: start at #zero index to have aligned memory
			vec0 = _mm_cmpnot_epu8_SSE2(_mm_load_si128(reinterpret_cast<const __m128i*>(&pNMS[i])), vecZero, vec0xFF);
			if (_mm_movemask_epi8(vec0)) {
				_mm_store_si128(reinterpret_cast<__m128i*>(&pNMS[i]), vecZero); // must, for next frame
				_mm_store_si128(reinterpret_cast<__m128i*>(&pcStrengthsMap[i]), _mm_andnot_si128(vec0, _mm_load_si128(reinterpret_cast<const __m128i*>(&pcStrengthsMap[i])))); // suppress
			}
		}
		pcStrengthsMap += stride;
		pNMS += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
