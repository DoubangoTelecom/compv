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
	const compv_scalar_t minsum = (N == 12 ? 3 : 2);
	const uint16_t *FastXFlags = N == 9 ? kCompVFast9Flags : kCompVFast12Flags;
	compv_scalar_t i, j, k, arcStart, sumb, sumd, sb, sd;
	int mask0B, mask1B, mask1D, mask0D;
	bool load0B, load1B, load0D, load1D;
	const __m128i vecThreshold = _mm_set1_epi8(static_cast<int8_t>(threshold));
	const __m128i vecN = _mm_set1_epi8(static_cast<int8_t>(N)); // FIXME: remove
	const __m128i vecNMinusOne = _mm_set1_epi8(static_cast<int8_t>(N - 1)); // no '_mm_cmpge_epu8'
	const __m128i vecOne = _mm_load_si128(reinterpret_cast<const __m128i*>(k1_i8));
	const __m128i vecZero = _mm_setzero_si128();
	const __m128i vec0xFF = _mm_cmpeq_epi8(vecZero, vecZero); // 0xFF
	__m128i vec0, vec1, vecStrengths, vecBrighter1, vecDarker1, vecDarkers16[16], vecBrighters16[16];
	const uint8_t* circle[16] = { // FIXME: use same circle with C++ code
		&IP[pixels16[0]], &IP[pixels16[1]], &IP[pixels16[2]], &IP[pixels16[3]],
		&IP[pixels16[4]], &IP[pixels16[5]], &IP[pixels16[6]], &IP[pixels16[7]],
		&IP[pixels16[8]], &IP[pixels16[9]], &IP[pixels16[10]], &IP[pixels16[11]],
		&IP[pixels16[12]], &IP[pixels16[13]], &IP[pixels16[14]], &IP[pixels16[15]]
	};

#define _mm_cmpgtz_epu8(vec, mask) _mm_andnot_si128(_mm_cmpeq_epi8(vec, vecZero), mask) // no '_mm_cmpgt_epu8', mask should be '0xff'
#define _mm_fast_masks(a, b) \
	vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[a] + i))); \
	vec1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>((circle[b] + i))); \
	vecDarkers16[a] = _mm_subs_epu8(vecDarker1, vec0); \
	vecDarkers16[b] = _mm_subs_epu8(vecDarker1, vec1); \
	vecBrighters16[a] = _mm_subs_epu8(vec0, vecBrighter1); \
	vecBrighters16[b] = _mm_subs_epu8(vec1, vecBrighter1); \
	mask0D = _mm_movemask_epi8(_mm_cmpgtz_epu8(vecDarkers16[a], vec0xFF)); \
	mask1D = _mm_movemask_epi8(_mm_cmpgtz_epu8(vecDarkers16[b], vec0xFF)); \
	mask0B = _mm_movemask_epi8(_mm_cmpgtz_epu8(vecBrighters16[a], vec0xFF)); \
	mask1B = _mm_movemask_epi8(_mm_cmpgtz_epu8(vecBrighters16[b], vec0xFF))

	for (i = 0; i < width; i += 16, strengths += 16) {
		vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(IP + i));
		vecBrighter1 = _mm_adds_epu8(vec0, vecThreshold);
		vecDarker1 = _mm_subs_epu8(vec0, vecThreshold);
		load0B = load0D = false;

		/* reset strength to zero */
		_mm_storeu_si128(reinterpret_cast<__m128i*>(strengths), vecZero); // neon: must not increment strengths now, use later

		if (i == 1040) { // looking for 1050
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("FIXME");
		}

		/***** Cross: 1, 9, 5, 13 *****/
		{
			// compare I1 and I9 aka 0 and 8
			_mm_fast_masks(0, 8);
			sumd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sumb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}

			// compare I5 and I13 aka 4 and 12
			_mm_fast_masks(4, 12);
			sd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!(sb || sd)) {
				continue;
			}
			sumb += sb, sumd += sd;
			load1B = (sumb >= minsum), load1D = (sumd >= minsum);
			if (!(load1B || load1D)) {
				continue;
			}
			load0B |= load1B, load0D |= load1D;
		}

		/***** Cross: 2, 10, 6, 14 *****/
		{
			// I2 and I10 aka 1 and 9
			_mm_fast_masks(1, 9);
			sumd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sumb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}

			// I6 and I14 aka 5 and 13
			_mm_fast_masks(5, 13);
			sd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!(sb || sd)) {
				continue;
			}
			sumb += sb, sumd += sd;
			load1B = (sumb >= minsum), load1D = (sumd >= minsum);
			if (!(load1B || load1D)) {
				continue;
			}
			load0B |= load1B, load0D |= load1D;
		}

		/***** Cross: 3, 11, 7, 15 *****/
		{
			// I3 and I11 aka 2 and 10
			_mm_fast_masks(2, 10);
			sumd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sumb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}
			// I7 and I15 aka 6 and 14
			_mm_fast_masks(6, 14);
			sd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!(sb || sd)) {
				continue;
			}
			sumb += sb, sumd += sd;
			load1B = (sumb >= minsum), load1D = (sumd >= minsum);
			if (!(load1B || load1D)) {
				continue;
			}
			load0B |= load1B, load0D |= load1D;
		}

		/***** Cross: 4, 12, 8, 16 *****/
		{
			// I4 and I12 aka 3 and 11
			_mm_fast_masks(3, 11);
			sumd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sumb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!sumb && !sumd) {
				continue;
			}
			// I8 and I16 aka 7 and 15
			_mm_fast_masks(7, 15);
			sd = (mask0D ? 1 : 0) + (mask1D ? 1 : 0);
			sb = (mask0B ? 1 : 0) + (mask1B ? 1 : 0);
			if (!(sb || sd)) {
				continue;
			}
			sumb += sb, sumd += sd;
			load1B = (sumb >= minsum), load1D = (sumd >= minsum);
			if (!(load1B || load1D)) {
				continue;
			}
			load0B |= load1B, load0D |= load1D;
		}

		// FIXME: remove
		//vec0 = _mm_cmpgt_epi8(vecDarkers16[0], vecZero);
		//vec1 = _mm_and_si128(vec0, vecOne);

		// FIXME: add first N then remove top, add bottom
		// instead of for (arcStart....
		// FIXME: same for computing min
		// FIXME: rotate(vecOnesD16) and rotate(vecOnesB16) instead of using "& 15"

		vecStrengths = _mm_setzero_si128();

		if (i == 1040) { // looking for 1050
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("FIXME");
		}

		if (load0B) {
			vec0 = _mm_cmpgtz_epu8(vecBrighters16[0], vecOne);
			for (j = 1; j < 16; ++j) { // FIXME: unroll
				vec0 = _mm_add_epi8(_mm_cmpgtz_epu8(vecBrighters16[j], vecOne), vec0);
			}
			vec0 = _mm_cmpgt_epi8(vec0, vecNMinusOne); // 0xFF when N-nonzero diffs found, initial value for minb
			mask0B = _mm_movemask_epi8(vec0);
			if (mask0B) {
				for (arcStart = 0; arcStart < 16; ++arcStart) {
					vec1 = vec0;
					for (j = arcStart, k = 0; k < N; ++j, ++k) {
						vec1 = _mm_min_epu8(vecBrighters16[j & 15], vec1);
						// FIXME: if 'vecMinB1' contains zeros then break the loop -> use mask
					}
					vecStrengths = _mm_max_epu8(vecStrengths, vec1);
				}
			}
		}

		if (load0D) {
			vec0 = _mm_cmpgtz_epu8(vecDarkers16[0], vecOne);
			for (j = 1; j < 16; ++j) { // FIXME: unroll
				vec0 = _mm_add_epi8(_mm_cmpgtz_epu8(vecDarkers16[j], vecOne), vec0);
			}
			vec0 = _mm_cmpgt_epi8(vec0, vecNMinusOne); // 0xFF when N-nonzero diffs found, initial value for minb
			mask0D = _mm_movemask_epi8(vec0);
			if (mask0D) {
				for (arcStart = 0; arcStart < 16; ++arcStart) {
					vec1 = vec0;
					for (j = arcStart, k = 0; k < N; ++j, ++k) {
						vec1 = _mm_min_epu8(vecDarkers16[j & 15], vec1);
						// FIXME: if 'vecMinB1' contains zeros then break the loop -> use mask
					}
					vecStrengths = _mm_max_epu8(vecStrengths, vec1);
				}
			}
		}
		
		_mm_storeu_si128(reinterpret_cast<__m128i*>(strengths), vecStrengths);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
