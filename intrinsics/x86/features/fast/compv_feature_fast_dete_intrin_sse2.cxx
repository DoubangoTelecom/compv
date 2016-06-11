/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_sse2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math_utils.h"
#include "compv/compv_bits.h"
#include "compv/compv_cpu.h"

#include <algorithm>

extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast9Arcs[16][16];
extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast12Arcs[16][16];
extern "C" const COMPV_ALIGN_DEFAULT() uint16_t Fast9Flags[16];
extern "C" const COMPV_ALIGN_DEFAULT() uint16_t Fast12Flags[16];

extern "C" void FastStrengths16(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x16, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);

COMPV_NAMESPACE_BEGIN()

void FastData16Row_Intrin_SSE2(
    const uint8_t* IP,
    const uint8_t* IPprev,
    compv_scalar_t width,
    const compv_scalar_t(&pixels16)[16],
    compv_scalar_t N,
    compv_scalar_t threshold,
    uint8_t* strengths,
    compv_scalar_t* me)
{
    compv_scalar_t i, sum, s;

    int colDarkersFlags, colBrightersFlags; // Flags defining which column has more than N non-zero bits
    bool loadB, loadD;
    __m128i xmm0, xmm1, xmm2, xmm3, xmmThreshold, xmmBrighter, xmmDarker, xmmZeros, xmmFF, xmmDarkersFlags[16], xmmBrightersFlags[16], xmmDataPtr[16], xmmOnes, xmmNMinusOne, xmm254;

    compv_scalar_t fdarkers16[16];
    compv_scalar_t fbrighters16[16];
    __m128i xmmDdarkers16x16[16];
    __m128i xmmDbrighters16x16[16];
    __m128i *xmmStrengths = (__m128i *)strengths;

    _mm_store_si128(&xmmZeros, _mm_setzero_si128());
    _mm_store_si128(&xmmThreshold, _mm_set1_epi8((uint8_t)threshold));
    _mm_store_si128(&xmmFF, _mm_cmpeq_epi8(xmmZeros, xmmZeros)); // 0xFF=255
    _mm_store_si128(&xmmOnes, _mm_load_si128((__m128i*)k1_i8));
    _mm_store_si128(&xmmNMinusOne, _mm_set1_epi8((uint8_t)N - 1));
    _mm_store_si128(&xmm254, _mm_load_si128((__m128i*)k254_u8)); // not(254) = 00000001 -> used to select the lowest bit in each u8

    for (i = 0; i < width; i += 16) {
        _mm_storeu_si128(xmmStrengths, xmmZeros); // cleanup strengths

        // build brighter and darker
        _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)IP)); // load samples
        _mm_store_si128(&xmmBrighter, _mm_adds_epu8(xmm0, xmmThreshold));
        _mm_store_si128(&xmmDarker, _mm_subs_epu8(xmm0, xmmThreshold));

        /* Motion estimation */
        if (IPprev) {
            (*me) = 0;
        }

        /*  Speed-Test-1 */

        // compare I1 and I9 aka 0 and 8
        _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&IP[pixels16[0]]));
        _mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&IP[pixels16[8]]));
        _mm_store_si128(&xmmDdarkers16x16[0], _mm_subs_epu8(xmmDarker, xmm0));
        _mm_store_si128(&xmmDdarkers16x16[8], _mm_subs_epu8(xmmDarker, xmm1));
        _mm_store_si128(&xmmDbrighters16x16[0], _mm_subs_epu8(xmm0, xmmBrighter));
        _mm_store_si128(&xmmDbrighters16x16[8], _mm_subs_epu8(xmm1, xmmBrighter));
        _mm_store_si128(&xmmDarkersFlags[0], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[0], xmmZeros), xmmFF));
        _mm_store_si128(&xmmDarkersFlags[8], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[8], xmmZeros), xmmFF));
        _mm_store_si128(&xmmBrightersFlags[0], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[0], xmmZeros), xmmFF));
        _mm_store_si128(&xmmBrightersFlags[8], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[8], xmmZeros), xmmFF));
        _mm_store_si128(&xmm0, _mm_or_si128(xmmDarkersFlags[0], xmmBrightersFlags[0]));
        _mm_store_si128(&xmm1, _mm_or_si128(xmmDarkersFlags[8], xmmBrightersFlags[8]));
        sum = (_mm_movemask_epi8(xmm0) ? 1 : 0) + (_mm_movemask_epi8(xmm1) ? 1 : 0);
        if (!sum) {
            goto next;
        }

        // compare I5 and I13 aka 4 and 12
        _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&IP[pixels16[4]]));
        _mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&IP[pixels16[12]]));
        _mm_store_si128(&xmmDdarkers16x16[4], _mm_subs_epu8(xmmDarker, xmm0));
        _mm_store_si128(&xmmDdarkers16x16[12], _mm_subs_epu8(xmmDarker, xmm1));
        _mm_store_si128(&xmmDbrighters16x16[4], _mm_subs_epu8(xmm0, xmmBrighter));
        _mm_store_si128(&xmmDbrighters16x16[12], _mm_subs_epu8(xmm1, xmmBrighter));
        _mm_store_si128(&xmmDarkersFlags[4], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[4], xmmZeros), xmmFF));
        _mm_store_si128(&xmmDarkersFlags[12], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[12], xmmZeros), xmmFF));
        _mm_store_si128(&xmmBrightersFlags[4], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[4], xmmZeros), xmmFF));
        _mm_store_si128(&xmmBrightersFlags[12], _mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[12], xmmZeros), xmmFF));
        _mm_store_si128(&xmm2, _mm_or_si128(xmmDarkersFlags[4], xmmBrightersFlags[4]));
        _mm_store_si128(&xmm3, _mm_or_si128(xmmDarkersFlags[12], xmmBrightersFlags[12]));
        s = (_mm_movemask_epi8(xmm2) ? 1 : 0) + (_mm_movemask_epi8(xmm3) ? 1 : 0);
        if (!s) {
            goto next;
        }
        sum += s;

        /*  Speed-Test-2 */
        if (N == 12 ? sum >= 3 : sum >= 2) {
            // Check whether to load Brighters
            _mm_store_si128(&xmm0, _mm_or_si128(xmmBrightersFlags[0], xmmBrightersFlags[8]));
            _mm_store_si128(&xmm1, _mm_or_si128(xmmBrightersFlags[4], xmmBrightersFlags[12]));
            sum = _mm_movemask_epi8(xmm0) ? 1 : 0;
            sum += _mm_movemask_epi8(xmm1) ? 1 : 0;
            loadB = (sum > 1); // sum cannot be > 2 -> dot not check it against 3 for N = 12

            // Check whether to load Darkers
            _mm_store_si128(&xmm0, _mm_or_si128(xmmDarkersFlags[0], xmmDarkersFlags[8]));
            _mm_store_si128(&xmm1, _mm_or_si128(xmmDarkersFlags[4], xmmDarkersFlags[12]));
            sum = _mm_movemask_epi8(xmm0) ? 1 : 0;
            sum += _mm_movemask_epi8(xmm1) ? 1 : 0;
            loadD = (sum > 1); // sum cannot be > 2 -> dot not check it against 3 for N = 12

            if (!(loadB || loadD)) {
                goto next;
            }

            colDarkersFlags = 0, colBrightersFlags = 0;

            _mm_store_si128(&xmmDataPtr[1], _mm_loadu_si128((__m128i*)&IP[pixels16[1]]));
            _mm_store_si128(&xmmDataPtr[2], _mm_loadu_si128((__m128i*)&IP[pixels16[2]]));
            _mm_store_si128(&xmmDataPtr[3], _mm_loadu_si128((__m128i*)&IP[pixels16[3]]));
            _mm_store_si128(&xmmDataPtr[5], _mm_loadu_si128((__m128i*)&IP[pixels16[5]]));
            _mm_store_si128(&xmmDataPtr[6], _mm_loadu_si128((__m128i*)&IP[pixels16[6]]));
            _mm_store_si128(&xmmDataPtr[7], _mm_loadu_si128((__m128i*)&IP[pixels16[7]]));
            _mm_store_si128(&xmmDataPtr[9], _mm_loadu_si128((__m128i*)&IP[pixels16[9]]));
            _mm_store_si128(&xmmDataPtr[10], _mm_loadu_si128((__m128i*)&IP[pixels16[10]]));
            _mm_store_si128(&xmmDataPtr[11], _mm_loadu_si128((__m128i*)&IP[pixels16[11]]));
            _mm_store_si128(&xmmDataPtr[13], _mm_loadu_si128((__m128i*)&IP[pixels16[13]]));
            _mm_store_si128(&xmmDataPtr[14], _mm_loadu_si128((__m128i*)&IP[pixels16[14]]));
            _mm_store_si128(&xmmDataPtr[15], _mm_loadu_si128((__m128i*)&IP[pixels16[15]]));

            // We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
            // Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
            // _mm_movemask_epi8 is cyclyvore

            if (loadD) {
                // Compute xmmDdarkers
                _mm_store_si128(&xmmDdarkers16x16[1], _mm_subs_epu8(xmmDarker, xmmDataPtr[1]));
                _mm_store_si128(&xmmDdarkers16x16[2], _mm_subs_epu8(xmmDarker, xmmDataPtr[2]));
                _mm_store_si128(&xmmDdarkers16x16[3], _mm_subs_epu8(xmmDarker, xmmDataPtr[3]));
                _mm_store_si128(&xmmDdarkers16x16[5], _mm_subs_epu8(xmmDarker, xmmDataPtr[5]));
                _mm_store_si128(&xmmDdarkers16x16[6], _mm_subs_epu8(xmmDarker, xmmDataPtr[6]));
                _mm_store_si128(&xmmDdarkers16x16[7], _mm_subs_epu8(xmmDarker, xmmDataPtr[7]));
                _mm_store_si128(&xmmDdarkers16x16[9], _mm_subs_epu8(xmmDarker, xmmDataPtr[9]));
                _mm_store_si128(&xmmDdarkers16x16[10], _mm_subs_epu8(xmmDarker, xmmDataPtr[10]));
                _mm_store_si128(&xmmDdarkers16x16[11], _mm_subs_epu8(xmmDarker, xmmDataPtr[11]));
                _mm_store_si128(&xmmDdarkers16x16[13], _mm_subs_epu8(xmmDarker, xmmDataPtr[13]));
                _mm_store_si128(&xmmDdarkers16x16[14], _mm_subs_epu8(xmmDarker, xmmDataPtr[14]));
                _mm_store_si128(&xmmDdarkers16x16[15], _mm_subs_epu8(xmmDarker, xmmDataPtr[15]));
                /* Compute flags (not really, we have the inverse: 0xFF when zero, the not will be applied later) */
                _mm_store_si128(&xmmDarkersFlags[1], _mm_cmpeq_epi8(xmmDdarkers16x16[1], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[2], _mm_cmpeq_epi8(xmmDdarkers16x16[2], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[3], _mm_cmpeq_epi8(xmmDdarkers16x16[3], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[5], _mm_cmpeq_epi8(xmmDdarkers16x16[5], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[6], _mm_cmpeq_epi8(xmmDdarkers16x16[6], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[7], _mm_cmpeq_epi8(xmmDdarkers16x16[7], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[9], _mm_cmpeq_epi8(xmmDdarkers16x16[9], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[10], _mm_cmpeq_epi8(xmmDdarkers16x16[10], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[11], _mm_cmpeq_epi8(xmmDdarkers16x16[11], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[13], _mm_cmpeq_epi8(xmmDdarkers16x16[13], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[14], _mm_cmpeq_epi8(xmmDdarkers16x16[14], xmmZeros));
                _mm_store_si128(&xmmDarkersFlags[15], _mm_cmpeq_epi8(xmmDdarkers16x16[15], xmmZeros));
                // Convert flags from 0xFF to 0x01
                // 0 4 8 12 already computed and contains the right values (not the inverse)
                _mm_store_si128(&xmmDarkersFlags[0], _mm_andnot_si128(xmm254, xmmDarkersFlags[0]));
                _mm_store_si128(&xmmDarkersFlags[4], _mm_andnot_si128(xmm254, xmmDarkersFlags[4]));
                _mm_store_si128(&xmmDarkersFlags[8], _mm_andnot_si128(xmm254, xmmDarkersFlags[8]));
                _mm_store_si128(&xmmDarkersFlags[12], _mm_andnot_si128(xmm254, xmmDarkersFlags[12]));
                // other values
                _mm_store_si128(&xmmDarkersFlags[1], _mm_andnot_si128(xmmDarkersFlags[1], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[2], _mm_andnot_si128(xmmDarkersFlags[2], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[3], _mm_andnot_si128(xmmDarkersFlags[3], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[5], _mm_andnot_si128(xmmDarkersFlags[5], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[6], _mm_andnot_si128(xmmDarkersFlags[6], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[7], _mm_andnot_si128(xmmDarkersFlags[7], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[9], _mm_andnot_si128(xmmDarkersFlags[9], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[10], _mm_andnot_si128(xmmDarkersFlags[10], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[11], _mm_andnot_si128(xmmDarkersFlags[11], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[13], _mm_andnot_si128(xmmDarkersFlags[13], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[14], _mm_andnot_si128(xmmDarkersFlags[14], xmmOnes));
                _mm_store_si128(&xmmDarkersFlags[15], _mm_andnot_si128(xmmDarkersFlags[15], xmmOnes));
                // add all flags
                _mm_store_si128(&xmmDarkersFlags[0], _mm_adds_epu8(xmmDarkersFlags[0], xmmDarkersFlags[1]));
                _mm_store_si128(&xmmDarkersFlags[2], _mm_adds_epu8(xmmDarkersFlags[2], xmmDarkersFlags[3]));
                _mm_store_si128(&xmmDarkersFlags[4], _mm_adds_epu8(xmmDarkersFlags[4], xmmDarkersFlags[5]));
                _mm_store_si128(&xmmDarkersFlags[6], _mm_adds_epu8(xmmDarkersFlags[6], xmmDarkersFlags[7]));
                _mm_store_si128(&xmmDarkersFlags[8], _mm_adds_epu8(xmmDarkersFlags[8], xmmDarkersFlags[9]));
                _mm_store_si128(&xmmDarkersFlags[10], _mm_adds_epu8(xmmDarkersFlags[10], xmmDarkersFlags[11]));
                _mm_store_si128(&xmmDarkersFlags[12], _mm_adds_epu8(xmmDarkersFlags[12], xmmDarkersFlags[13]));
                _mm_store_si128(&xmmDarkersFlags[14], _mm_adds_epu8(xmmDarkersFlags[14], xmmDarkersFlags[15]));
                _mm_store_si128(&xmmDarkersFlags[0], _mm_adds_epu8(xmmDarkersFlags[0], xmmDarkersFlags[2]));
                _mm_store_si128(&xmmDarkersFlags[4], _mm_adds_epu8(xmmDarkersFlags[4], xmmDarkersFlags[6]));
                _mm_store_si128(&xmmDarkersFlags[8], _mm_adds_epu8(xmmDarkersFlags[8], xmmDarkersFlags[10]));
                _mm_store_si128(&xmmDarkersFlags[12], _mm_adds_epu8(xmmDarkersFlags[12], xmmDarkersFlags[14]));
                _mm_store_si128(&xmmDarkersFlags[0], _mm_adds_epu8(xmmDarkersFlags[0], xmmDarkersFlags[4]));
                _mm_store_si128(&xmmDarkersFlags[8], _mm_adds_epu8(xmmDarkersFlags[8], xmmDarkersFlags[12]));
                _mm_store_si128(&xmmDarkersFlags[0], _mm_adds_epu8(xmmDarkersFlags[0], xmmDarkersFlags[8])); // sum is in xmmDarkersFlags[0]
                // Check the columns with at least N non-zero bits
                _mm_store_si128(&xmmDarkersFlags[0], _mm_cmpgt_epi8(xmmDarkersFlags[0], xmmNMinusOne));
                colDarkersFlags = _mm_movemask_epi8(xmmDarkersFlags[0]);
                loadD = (colDarkersFlags != 0);
            }

            if (loadB) {
                /* Compute Dbrighters */
                _mm_store_si128(&xmmDbrighters16x16[1], _mm_subs_epu8(xmmDataPtr[1], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[2], _mm_subs_epu8(xmmDataPtr[2], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[3], _mm_subs_epu8(xmmDataPtr[3], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[5], _mm_subs_epu8(xmmDataPtr[5], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[6], _mm_subs_epu8(xmmDataPtr[6], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[7], _mm_subs_epu8(xmmDataPtr[7], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[9], _mm_subs_epu8(xmmDataPtr[9], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[10], _mm_subs_epu8(xmmDataPtr[10], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[11], _mm_subs_epu8(xmmDataPtr[11], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[13], _mm_subs_epu8(xmmDataPtr[13], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[14], _mm_subs_epu8(xmmDataPtr[14], xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[15], _mm_subs_epu8(xmmDataPtr[15], xmmBrighter));
                /* Compute flags (not really, we have the inverse: 0xFF when zero, the not will be applied later) */
                _mm_store_si128(&xmmBrightersFlags[1], _mm_cmpeq_epi8(xmmDbrighters16x16[1], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[2], _mm_cmpeq_epi8(xmmDbrighters16x16[2], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[3], _mm_cmpeq_epi8(xmmDbrighters16x16[3], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[5], _mm_cmpeq_epi8(xmmDbrighters16x16[5], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[6], _mm_cmpeq_epi8(xmmDbrighters16x16[6], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[7], _mm_cmpeq_epi8(xmmDbrighters16x16[7], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[9], _mm_cmpeq_epi8(xmmDbrighters16x16[9], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[10], _mm_cmpeq_epi8(xmmDbrighters16x16[10], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[11], _mm_cmpeq_epi8(xmmDbrighters16x16[11], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[13], _mm_cmpeq_epi8(xmmDbrighters16x16[13], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[14], _mm_cmpeq_epi8(xmmDbrighters16x16[14], xmmZeros));
                _mm_store_si128(&xmmBrightersFlags[15], _mm_cmpeq_epi8(xmmDbrighters16x16[15], xmmZeros));
                // Convert flags from 0xFF to 0x01
                // 0 4 8 12 already computed and contains the right values (not the inverse)
                _mm_store_si128(&xmmBrightersFlags[0], _mm_andnot_si128(xmm254, xmmBrightersFlags[0]));
                _mm_store_si128(&xmmBrightersFlags[4], _mm_andnot_si128(xmm254, xmmBrightersFlags[4]));
                _mm_store_si128(&xmmBrightersFlags[8], _mm_andnot_si128(xmm254, xmmBrightersFlags[8]));
                _mm_store_si128(&xmmBrightersFlags[12], _mm_andnot_si128(xmm254, xmmBrightersFlags[12]));
                // other values
                _mm_store_si128(&xmmBrightersFlags[1], _mm_andnot_si128(xmmBrightersFlags[1], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[2], _mm_andnot_si128(xmmBrightersFlags[2], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[3], _mm_andnot_si128(xmmBrightersFlags[3], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[5], _mm_andnot_si128(xmmBrightersFlags[5], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[6], _mm_andnot_si128(xmmBrightersFlags[6], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[7], _mm_andnot_si128(xmmBrightersFlags[7], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[9], _mm_andnot_si128(xmmBrightersFlags[9], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[10], _mm_andnot_si128(xmmBrightersFlags[10], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[11], _mm_andnot_si128(xmmBrightersFlags[11], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[13], _mm_andnot_si128(xmmBrightersFlags[13], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[14], _mm_andnot_si128(xmmBrightersFlags[14], xmmOnes));
                _mm_store_si128(&xmmBrightersFlags[15], _mm_andnot_si128(xmmBrightersFlags[15], xmmOnes));
                // add all flags
                _mm_store_si128(&xmmBrightersFlags[0], _mm_adds_epu8(xmmBrightersFlags[0], xmmBrightersFlags[1]));
                _mm_store_si128(&xmmBrightersFlags[2], _mm_adds_epu8(xmmBrightersFlags[2], xmmBrightersFlags[3]));
                _mm_store_si128(&xmmBrightersFlags[4], _mm_adds_epu8(xmmBrightersFlags[4], xmmBrightersFlags[5]));
                _mm_store_si128(&xmmBrightersFlags[6], _mm_adds_epu8(xmmBrightersFlags[6], xmmBrightersFlags[7]));
                _mm_store_si128(&xmmBrightersFlags[8], _mm_adds_epu8(xmmBrightersFlags[8], xmmBrightersFlags[9]));
                _mm_store_si128(&xmmBrightersFlags[10], _mm_adds_epu8(xmmBrightersFlags[10], xmmBrightersFlags[11]));
                _mm_store_si128(&xmmBrightersFlags[12], _mm_adds_epu8(xmmBrightersFlags[12], xmmBrightersFlags[13]));
                _mm_store_si128(&xmmBrightersFlags[14], _mm_adds_epu8(xmmBrightersFlags[14], xmmBrightersFlags[15]));
                _mm_store_si128(&xmmBrightersFlags[0], _mm_adds_epu8(xmmBrightersFlags[0], xmmBrightersFlags[2]));
                _mm_store_si128(&xmmBrightersFlags[4], _mm_adds_epu8(xmmBrightersFlags[4], xmmBrightersFlags[6]));
                _mm_store_si128(&xmmBrightersFlags[8], _mm_adds_epu8(xmmBrightersFlags[8], xmmBrightersFlags[10]));
                _mm_store_si128(&xmmBrightersFlags[12], _mm_adds_epu8(xmmBrightersFlags[12], xmmBrightersFlags[14]));
                _mm_store_si128(&xmmBrightersFlags[0], _mm_adds_epu8(xmmBrightersFlags[0], xmmBrightersFlags[4]));
                _mm_store_si128(&xmmBrightersFlags[8], _mm_adds_epu8(xmmBrightersFlags[8], xmmBrightersFlags[12]));
                _mm_store_si128(&xmmBrightersFlags[0], _mm_adds_epu8(xmmBrightersFlags[0], xmmBrightersFlags[8])); // sum is in xmmDarkersFlags[0]
                // Check the columns with at least N non-zero bits
                _mm_store_si128(&xmm0, _mm_cmpgt_epi8(xmmBrightersFlags[0], xmmNMinusOne));
                colBrightersFlags = _mm_movemask_epi8(xmm0);
                loadB = (colBrightersFlags != 0);
            }

            if (loadD) {
                // Transpose
                COMPV_TRANSPOSE_I8_16X16_SSE2(
                    xmmDdarkers16x16[0], xmmDdarkers16x16[1], xmmDdarkers16x16[2], xmmDdarkers16x16[3],
                    xmmDdarkers16x16[4], xmmDdarkers16x16[5], xmmDdarkers16x16[6], xmmDdarkers16x16[7],
                    xmmDdarkers16x16[8], xmmDdarkers16x16[9], xmmDdarkers16x16[10], xmmDdarkers16x16[11],
                    xmmDdarkers16x16[12], xmmDdarkers16x16[13], xmmDdarkers16x16[14], xmmDdarkers16x16[15],
                    xmm0);
                // Flags
                fdarkers16[0] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[0], xmmZeros), xmmFF));
                fdarkers16[1] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[1], xmmZeros), xmmFF));
                fdarkers16[2] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[2], xmmZeros), xmmFF));
                fdarkers16[3] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[3], xmmZeros), xmmFF));
                fdarkers16[4] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[4], xmmZeros), xmmFF));
                fdarkers16[5] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[5], xmmZeros), xmmFF));
                fdarkers16[6] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[6], xmmZeros), xmmFF));
                fdarkers16[7] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[7], xmmZeros), xmmFF));
                fdarkers16[8] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[8], xmmZeros), xmmFF));
                fdarkers16[9] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[9], xmmZeros), xmmFF));
                fdarkers16[10] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[10], xmmZeros), xmmFF));
                fdarkers16[11] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[11], xmmZeros), xmmFF));
                fdarkers16[12] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[12], xmmZeros), xmmFF));
                fdarkers16[13] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[13], xmmZeros), xmmFF));
                fdarkers16[14] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[14], xmmZeros), xmmFF));
                fdarkers16[15] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDdarkers16x16[15], xmmZeros), xmmFF));
            }

            if (loadB) {
                // Transpose
                COMPV_TRANSPOSE_I8_16X16_SSE2(
                    xmmDbrighters16x16[0], xmmDbrighters16x16[1], xmmDbrighters16x16[2], xmmDbrighters16x16[3],
                    xmmDbrighters16x16[4], xmmDbrighters16x16[5], xmmDbrighters16x16[6], xmmDbrighters16x16[7],
                    xmmDbrighters16x16[8], xmmDbrighters16x16[9], xmmDbrighters16x16[10], xmmDbrighters16x16[11],
                    xmmDbrighters16x16[12], xmmDbrighters16x16[13], xmmDbrighters16x16[14], xmmDbrighters16x16[15],
                    xmm1);
                // Flags
                fbrighters16[0] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[0], xmmZeros), xmmFF));
                fbrighters16[1] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[1], xmmZeros), xmmFF));
                fbrighters16[2] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[2], xmmZeros), xmmFF));
                fbrighters16[3] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[3], xmmZeros), xmmFF));
                fbrighters16[4] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[4], xmmZeros), xmmFF));
                fbrighters16[5] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[5], xmmZeros), xmmFF));
                fbrighters16[6] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[6], xmmZeros), xmmFF));
                fbrighters16[7] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[7], xmmZeros), xmmFF));
                fbrighters16[8] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[8], xmmZeros), xmmFF));
                fbrighters16[9] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[9], xmmZeros), xmmFF));
                fbrighters16[10] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[10], xmmZeros), xmmFF));
                fbrighters16[11] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[11], xmmZeros), xmmFF));
                fbrighters16[12] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[12], xmmZeros), xmmFF));
                fbrighters16[13] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[13], xmmZeros), xmmFF));
                fbrighters16[14] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[14], xmmZeros), xmmFF));
                fbrighters16[15] = _mm_movemask_epi8(_mm_andnot_si128(_mm_cmpeq_epi8(xmmDbrighters16x16[15], xmmZeros), xmmFF));

            }
            if (colBrightersFlags || colDarkersFlags) {
                FastStrengths16(colBrightersFlags, colDarkersFlags, (const uint8_t*)xmmDbrighters16x16, (const uint8_t*)xmmDdarkers16x16, &fbrighters16, &fdarkers16, (uint8_t*)xmmStrengths, N);
            }
        }
next:
        IP += 16;
        if (IPprev) {
            IPprev += 16;
        }
        xmmStrengths += 1;
    } // for i
}

// TODO(dmi): ASM version
// TODO(dmi): add AVX
void FastStrengths16_Intrin_SSE2(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x16, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv_scalar_t N)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // FastStrengths_(ASM/INTRIN)_SSE41 is the best choice

    __m128i xmm0, xmm1, xmmFbrighters, xmmFdarkers, xmmZeros, xmmFastXFlagsLow, xmmFastXFlagsHigh;
    int r0 = 0, r1 = 0, maxn;
    unsigned i, j, k;
    const uint16_t(&FastXFlags)[16] = N == 9 ? Fast9Flags : Fast12Flags;
    uint16_t rb = (uint16_t)rbrighters, rd = (uint16_t)rdarkers;

    _mm_store_si128(&xmmZeros, _mm_setzero_si128());

    // FAST hard-coded flags
    _mm_store_si128(&xmmFastXFlagsLow, _mm_load_si128((__m128i*)(FastXFlags + 0)));
    _mm_store_si128(&xmmFastXFlagsHigh, _mm_load_si128((__m128i*)(FastXFlags + 8)));

    for (unsigned p = 0; p < 16; ++p) {
        maxn = 0;
        // Brighters
        if (rb & (1 << p)) {
            // brighters flags
            _mm_store_si128(&xmmFbrighters, _mm_set1_epi16((short)(*fbrighters16)[p]));

            _mm_store_si128(&xmm0, _mm_and_si128(xmmFbrighters, xmmFastXFlagsLow));
            _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
            _mm_store_si128(&xmm1, _mm_and_si128(xmmFbrighters, xmmFastXFlagsHigh));
            _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
            r0 = _mm_movemask_epi8(_mm_packs_epi16(xmm0, xmm1)); // r0's popcnt is equal to N as FastXFlags contains values with popcnt==N
            if (r0) {
                uint8_t nbrighter;
                for (i = 0; i < 16; ++i) {
                    if (r0 & (1 << i)) {
                        // Compute Horizontal minimum (TODO: Find SSE2 method)
                        nbrighter = 255;
                        k = unsigned(i + N);
                        for (j = i; j < k; ++j) {
                            if (dbrighters16x16[j & 15] < nbrighter) {
                                nbrighter = dbrighters16x16[j & 15];
                            }
                        }
                        maxn = (uint8_t)std::max((int)nbrighter, maxn);
                    }
                }
            }
        }

        // Darkers
        if (rd & (1 << p)) {
            // darkers flags
            _mm_store_si128(&xmmFdarkers, _mm_set1_epi16((short)(*fdarkers16)[p]));

            _mm_store_si128(&xmm0, _mm_and_si128(xmmFdarkers, xmmFastXFlagsLow));
            _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
            _mm_store_si128(&xmm1, _mm_and_si128(xmmFdarkers, xmmFastXFlagsHigh));
            _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
            r1 = _mm_movemask_epi8(_mm_packs_epi16(xmm0, xmm1)); // r1's popcnt is equal to N as FastXFlags contains values with popcnt==N
            if (r1) {
                uint8_t ndarker;
                for (i = 0; i < 16; ++i) {
                    if (r1 & (1 << i)) {
                        // Compute Horizontal minimum
                        ndarker = 255;
                        k = unsigned(i + N);
                        for (j = i; j < k; ++j) {
                            if (ddarkers16x16[j & 15] < ndarker) {
                                ndarker = ddarkers16x16[j & 15];
                            }
                        }
                        maxn = (uint8_t)std::max((int)ndarker, maxn);
                    }
                }
            }
        }

        strengths16[p] = (uint8_t)maxn;

        ddarkers16x16 += 16;
        dbrighters16x16 += 16;
    }
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
