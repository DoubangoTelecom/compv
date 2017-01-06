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

#include <algorithm>

COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() uint8_t kCompVFast9Arcs[16][16];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() uint8_t kCompVFast12Arcs[16][16];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() uint16_t kCompVFast9Flags[16];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() uint16_t kCompVFast12Flags[16];

COMPV_NAMESPACE_BEGIN()

// No need to check for 'width'. The caller ('CompVFastProcessRange' function) already checked and prepared it for SSE.
void CompVFastDataRow16_Intrin_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_scalar_t width, COMPV_ALIGNED(DEFAULT) const compv_scalar_t *pixels16, compv_scalar_t N, compv_scalar_t threshold, uint8_t* strengths)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i, j, k, arcStart;
	int mask;
	compv_scalar_t NminusOne = N - 1;
	const compv_scalar_t minsum = (N == 12 ? 3 : 2);
	const __m128i vecThreshold = _mm_set1_epi8(static_cast<int8_t>(threshold));
	const __m128i vecNMinSumMinusOne = _mm_set1_epi8(static_cast<int8_t>(minsum - 1)); // no '_mm_cmpge_epu8'
	const __m128i vecNMinusOne = _mm_set1_epi8(static_cast<int8_t>(NminusOne)); // no '_mm_cmpge_epu8'
	const __m128i vecOne = _mm_load_si128(reinterpret_cast<const __m128i*>(k1_i8));
	const __m128i vecZero = _mm_setzero_si128();
	const __m128i vec0xFF = _mm_cmpeq_epi8(vecZero, vecZero); // 0xFF
	__m128i vec0, vec1, vecSum1, vecS1, vecStrengths, vecBrighter1, vecDarker1, vecMask16[16], vecDarkers16[16], vecBrighters16[16];
	const uint8_t* circle[16] = { // FIXME: use same circle with C++ code
		&IP[pixels16[0]], &IP[pixels16[1]], &IP[pixels16[2]], &IP[pixels16[3]],
		&IP[pixels16[4]], &IP[pixels16[5]], &IP[pixels16[6]], &IP[pixels16[7]],
		&IP[pixels16[8]], &IP[pixels16[9]], &IP[pixels16[10]], &IP[pixels16[11]],
		&IP[pixels16[12]], &IP[pixels16[13]], &IP[pixels16[14]], &IP[pixels16[15]]
	};

#define _mm_cmpgtz_epu8(vec, mask) _mm_andnot_si128(_mm_cmpeq_epi8(vec, vecZero), mask) // no '_mm_cmpgt_epu8', mask should be '0xff'

	for (i = 0; i < width; i += 16, strengths += 16) {
		vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(IP + i));
		vecBrighter1 = _mm_adds_epu8(vec0, vecThreshold);
		vecDarker1 = _mm_subs_epu8(vec0, vecThreshold);

		/* reset strength to zero */
		vecStrengths = _mm_setzero_si128();

		if (i == 1040) { // looking for 1050
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("FIXME");
		}

		// FIXME: loadu_circle done several times

#define _mm_fast_compute_Darkers(a, b) \
	vecDarkers16[a] = _mm_subs_epu8(vecDarker1, vec0); \
	vecDarkers16[b] = _mm_subs_epu8(vecDarker1, vec1)
#define _mm_fast_compute_Brighters(a, b) \
	vecBrighters16[a] = _mm_subs_epu8(vec0, vecBrighter1); \
	vecBrighters16[b] = _mm_subs_epu8(vec1, vecBrighter1)

#define _mm_fast_check(a, b, tt, next) \
		vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[a] + i))); \
		vec1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[b] + i))); \
		_mm_fast_compute_##tt##s(a, b); \
		vecMask16[a] = _mm_cmpgtz_epu8(vec##tt##s16[a], vecOne); \
		vecMask16[b] = _mm_cmpgtz_epu8(vec##tt##s16[b], vecOne); \
		vecS1 = _mm_add_epi8(vecMask16[a], vecMask16[b]); \
		mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecS1, vecZero)); \
		if (!mask) goto next;

		/****************************** Darkers ******************************/
		{
			/* 1, 9, 5, 13 */ {
				// compare I1 and I9 aka 0 and 8
				_mm_fast_check(0, 8, Darker, EndOfDarkers);
				vecSum1 = vecS1; // initial -> equal

				// compare I5 and I13 aka 4 and 12
				_mm_fast_check(4, 12, Darker, EndOfDarkers);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1); // not initial -> add
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne));
				if (!mask) goto EndOfDarkers;
			}
			/* 2, 10, 6, 14 */ {
				// I2 and I10 aka 1 and 9
				_mm_fast_check(1, 9, Darker, EndOfDarkers);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);

				// I6 and I14 aka 5 and 13
				_mm_fast_check(5, 13, Darker, EndOfDarkers);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne));
				if (!mask) goto EndOfDarkers;
			}
			/* 3, 11, 7, 15 */ {
				// I3 and I11 aka 2 and 10
				_mm_fast_check(2, 10, Darker, EndOfDarkers);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);

				// I7 and I15 aka 6 and 14
				_mm_fast_check(6, 14, Darker, EndOfDarkers);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne));
				if (!mask) goto EndOfDarkers;
			}
			/* 4, 12, 8, 16 */ {
				// I4 and I12 aka 3 and 11
				_mm_fast_check(3, 11, Darker, EndOfDarkers);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);

				// I8 and I16 aka 7 and 15
				_mm_fast_check(7, 15, Darker, EndOfDarkers);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinusOne)); // last pair -> not 'vecNMinSumMinusOne' but 'vecNMinusOne'
				if (!mask) goto EndOfDarkers;
			}

			////// Strengths /////

			// Sum arc #0 without the tail (#NminusOne lines)
			vec0 = _mm_add_epi8(vecMask16[0], vecMask16[1]);
			vec0 = _mm_add_epi8(vec0, vecMask16[2]);
			vec0 = _mm_add_epi8(vec0, vecMask16[3]);
			vec0 = _mm_add_epi8(vec0, vecMask16[4]);
			vec0 = _mm_add_epi8(vec0, vecMask16[5]);
			vec0 = _mm_add_epi8(vec0, vecMask16[6]);
			vec0 = _mm_add_epi8(vec0, vecMask16[7]);
			if (N == 12) {
				vec0 = _mm_add_epi8(vec0, vecMask16[8]);
				vec0 = _mm_add_epi8(vec0, vecMask16[9]);
				vec0 = _mm_add_epi8(vec0, vecMask16[10]);
			}
			for (j = 0; j < 16; ++j) {
				vec0 = _mm_add_epi8(vec0, vecMask16[(NminusOne + j) & 15]); // add tail
				if (_mm_movemask_epi8(_mm_cmpgt_epi8(vec0, vecNMinusOne))) {
					vec1 = _mm_cmpeq_epi8(vec1, vec1); // 0xff
					for (arcStart = j, k = 0; k < N; ++arcStart, ++k) { // FIXME: unroll
						vec1 = _mm_min_epu8(vecDarkers16[arcStart & 15], vec1);
					}
					vecStrengths = _mm_max_epu8(vecStrengths, vec1);
				}
				vec0 = _mm_sub_epi8(vec0, vecMask16[j & 15]); // sub head
			}
		}
	EndOfDarkers:

		/****************************** Brighters ******************************/
		{
			/* 1, 9, 5, 13 */ {
				// compare I1 and I9 aka 0 and 8
				_mm_fast_check(0, 8, Brighter, EndOfBrighters);
				vecSum1 = vecS1; // initial -> equal

				// compare I5 and I13 aka 4 and 12
				_mm_fast_check(4, 12, Brighter, EndOfBrighters);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1); // not initial -> add
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne));
				if (!mask) goto EndOfBrighters;
			}
			/* 2, 10, 6, 14 */ {
				// I2 and I10 aka 1 and 9
				_mm_fast_check(1, 9, Brighter, EndOfBrighters);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);

				// I6 and I14 aka 5 and 13
				_mm_fast_check(5, 13, Brighter, EndOfBrighters);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne));
				if (!mask) goto EndOfBrighters;
			}
			/* 3, 11, 7, 15 */ {
				// I3 and I11 aka 2 and 10
				_mm_fast_check(2, 10, Brighter, EndOfBrighters);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);

				// I7 and I15 aka 6 and 14
				_mm_fast_check(6, 14, Brighter, EndOfBrighters);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinSumMinusOne));
				if (!mask) goto EndOfBrighters;
			}
			/* 4, 12, 8, 16 */ {
				// I4 and I12 aka 3 and 11
				_mm_fast_check(3, 11, Brighter, EndOfBrighters);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);

				// I8 and I16 aka 7 and 15
				_mm_fast_check(7, 15, Brighter, EndOfBrighters);
				vecSum1 = _mm_add_epi8(vecSum1, vecS1);
				mask = _mm_movemask_epi8(_mm_cmpgt_epi8(vecSum1, vecNMinusOne)); // last pair -> not 'vecNMinSumMinusOne' but 'vecNMinusOne'
				if (!mask) goto EndOfBrighters;
			}

			////// Strengths /////

			// Sum arc #0 without the tail (#NminusOne lines)
			vec0 = _mm_add_epi8(vecMask16[0], vecMask16[1]);
			vec0 = _mm_add_epi8(vec0, vecMask16[2]);
			vec0 = _mm_add_epi8(vec0, vecMask16[3]);
			vec0 = _mm_add_epi8(vec0, vecMask16[4]);
			vec0 = _mm_add_epi8(vec0, vecMask16[5]);
			vec0 = _mm_add_epi8(vec0, vecMask16[6]);
			vec0 = _mm_add_epi8(vec0, vecMask16[7]);
			if (N == 12) {
				vec0 = _mm_add_epi8(vec0, vecMask16[8]);
				vec0 = _mm_add_epi8(vec0, vecMask16[9]);
				vec0 = _mm_add_epi8(vec0, vecMask16[10]);
			}
			for (j = 0; j < 16; ++j) {
				vec0 = _mm_add_epi8(vec0, vecMask16[(NminusOne + j) & 15]); // add tail
				if (_mm_movemask_epi8(_mm_cmpgt_epi8(vec0, vecNMinusOne))) {
					vec1 = _mm_cmpeq_epi8(vec1, vec1); // 0xff
					for (arcStart = j, k = 0; k < N; ++arcStart, ++k) { // FIXME: unroll
						vec1 = _mm_min_epu8(vecBrighters16[arcStart & 15], vec1);
					}
					vecStrengths = _mm_max_epu8(vecStrengths, vec1);
				}
				vec0 = _mm_sub_epi8(vec0, vecMask16[j & 15]); // sub head
			}
		}
	EndOfBrighters:
		
		_mm_storeu_si128(reinterpret_cast<__m128i*>(strengths), vecStrengths);

	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
