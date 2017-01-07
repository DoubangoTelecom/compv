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

#define _mm_fast_compute_Darkers(a, b) \
	vecDiff16[a] = _mm_subs_epu8(vecDarker1, vecCircle16[a]); \
	vecDiff16[b] = _mm_subs_epu8(vecDarker1, vecCircle16[b])
#define _mm_fast_compute_Brighters(a, b) \
	vecDiff16[a] = _mm_subs_epu8(vecCircle16[a], vecBrighter1); \
	vecDiff16[b] = _mm_subs_epu8(vecCircle16[b], vecBrighter1)

#define _mm_fast_check(a, b, type) \
		_mm_fast_compute_##type##s(a, b); \
		vecDiffBinary16[a] = _mm_cmpgtz_epu8_SSE2(vecDiff16[a], vecOne); \
		vecDiffBinary16[b] = _mm_cmpgtz_epu8_SSE2(vecDiff16[b], vecOne); \
		vecS1 = _mm_add_epi8(vecDiffBinary16[a], vecDiffBinary16[b]); \
		mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecS1, vecZero)); \
		if (!mask) goto EndOf##type##s

// vecMinSum: 'vecNMinusOne' for last check, 'vecNMinSumMinusOne' otherwise
#define _mm_fast_check_x_x_x_x(a, b, c, d, type, vecMinSum) \
		_mm_fast_check(a, b, type); \
		vecSum1 = _mm_add_epi8(vecSum1, vecS1); \
		_mm_fast_check(c, d, type); \
		vecSum1 = _mm_add_epi8(vecSum1, vecS1); \
		mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecMinSum)); \
		if (!mask) goto EndOf##type##s

// Sum arc #0 without the tail (#NminusOne lines)
#define _mm_fast_int_diffbinarysum(vecSum1, vecBinary16) \
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
void CompVFastDataRow16_Intrin_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_scalar_t width, COMPV_ALIGNED(DEFAULT) const compv_scalar_t *pixels16, compv_scalar_t N, compv_scalar_t threshold, uint8_t* strengths)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i;
	int mask;
	compv_scalar_t NminusOne = N - 1;
	const compv_scalar_t minsum = (N == 12 ? 3 : 2);
	const __m128i vecThreshold = _mm_set1_epi8(static_cast<int8_t>(threshold));
	const __m128i vecNMinSumMinusOne = _mm_set1_epi8(static_cast<int8_t>(minsum - 1)); // no '_mm_cmpge_epu8'
	const __m128i vecNMinusOne = _mm_set1_epi8(static_cast<int8_t>(NminusOne)); // no '_mm_cmpge_epu8'
	const __m128i vecOne = _mm_load_si128(reinterpret_cast<const __m128i*>(k1_i8));
	const __m128i vecZero = _mm_setzero_si128();
	const __m128i vec0xFF = _mm_cmpeq_epi8(vecZero, vecZero); // 0xFF
	__m128i vec0, vecSum1, vecS1, vecStrengths, vecBrighter1, vecDarker1, vecDiffBinary16[16], vecDiff16[16], vecCircle16[16];
	const uint8_t* circle[16] = { // FIXME: use same circle with C++ code
		&IP[pixels16[0]], &IP[pixels16[1]], &IP[pixels16[2]], &IP[pixels16[3]],
		&IP[pixels16[4]], &IP[pixels16[5]], &IP[pixels16[6]], &IP[pixels16[7]],
		&IP[pixels16[8]], &IP[pixels16[9]], &IP[pixels16[10]], &IP[pixels16[11]],
		&IP[pixels16[12]], &IP[pixels16[13]], &IP[pixels16[14]], &IP[pixels16[15]]
	};

	for (i = 0; i < width; i += 16, strengths += 16) {
		vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(IP + i));
		vecBrighter1 = _mm_adds_epu8(vec0, vecThreshold);
		vecDarker1 = _mm_subs_epu8(vec0, vecThreshold);

		/* reset strength to zero */
		vecStrengths = _mm_setzero_si128();

		vecCircle16[0] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[0] + i)));
		vecCircle16[1] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[1] + i)));
		vecCircle16[2] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[2] + i)));
		vecCircle16[3] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[3] + i)));
		vecCircle16[4] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[4] + i)));
		vecCircle16[5] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[5] + i)));
		vecCircle16[6] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[6] + i)));
		vecCircle16[7] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[7] + i)));
		vecCircle16[8] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[8] + i)));
		vecCircle16[9] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[9] + i)));
		vecCircle16[10] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[10] + i)));
		vecCircle16[11] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[11] + i)));
		vecCircle16[12] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[12] + i)));
		vecCircle16[13] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[13] + i)));
		vecCircle16[14] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[14] + i)));
		vecCircle16[15] = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[15] + i)));
		
		/* Darkers */ {
			vecSum1 = _mm_setzero_si128();
			_mm_fast_check_x_x_x_x(0, 8, 4, 12, Darker, vecNMinSumMinusOne);
			_mm_fast_check_x_x_x_x(1, 9, 5, 13, Darker, vecNMinSumMinusOne);
			_mm_fast_check_x_x_x_x(2, 10, 6, 14, Darker, vecNMinSumMinusOne);
			_mm_fast_check_x_x_x_x(3, 11, 7, 15, Darker, vecNMinusOne);
			_mm_fast_int_diffbinarysum(vecSum1, vecDiffBinary16);
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
			_mm_fast_check_x_x_x_x(0, 8, 4, 12, Brighter, vecNMinSumMinusOne);
			_mm_fast_check_x_x_x_x(1, 9, 5, 13, Brighter, vecNMinSumMinusOne);
			_mm_fast_check_x_x_x_x(2, 10, 6, 14, Brighter, vecNMinSumMinusOne);
			_mm_fast_check_x_x_x_x(3, 11, 7, 15, Brighter, vecNMinusOne);
			_mm_fast_int_diffbinarysum(vecSum1, vecDiffBinary16);
			_mm_fast_strength(0, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(1, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(2, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(3, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(4, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(5, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(6, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(7, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(8, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(9, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(10, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(11, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(12, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(13, vecSum1, vecDiff16, vecStrengths, vec0);
			_mm_fast_strength(14, vecSum1, vecDiff16, vecStrengths, vec0); _mm_fast_strength(15, vecSum1, vecDiff16, vecStrengths, vec0);
		} EndOfBrighters:
		
		_mm_storeu_si128(reinterpret_cast<__m128i*>(strengths), vecStrengths);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
