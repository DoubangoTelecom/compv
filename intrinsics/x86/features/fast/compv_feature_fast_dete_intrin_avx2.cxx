/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_avx2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/image/conv/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

#include <algorithm>

extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast9Arcs[16][16];
extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast12Arcs[16][16];
extern "C" const COMPV_ALIGN_DEFAULT() uint16_t Fast9Flags[16];
extern "C" const COMPV_ALIGN_DEFAULT() uint16_t Fast12Flags[16];

extern "C" void FastStrengths32(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(AVX) const uint8_t* dbrighters16x32, COMPV_ALIGNED(AVX) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);

COMPV_NAMESPACE_BEGIN()

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void FastData32Row_Intrin_AVX2(
	const uint8_t* IP,
	const uint8_t* IPprev,
	compv_scalar_t width,
	const compv_scalar_t(&pixels16)[16],
	compv_scalar_t N,
	compv_scalar_t threshold,
	uint8_t* strengths,
	compv_scalar_t* me)
{
    _mm256_zeroupper();
    compv_scalar_t i, sum, s;

    int colDarkersFlags, colBrightersFlags; // Flags defining which column has more than N non-zero bits
    bool loadB, loadD;
    __m256i ymm0, ymm1, ymm2, ymm3, ymmThreshold, ymmBrighter, ymmDarker, ymmZeros, ymmFF, ymmDarkersFlags[16], ymmBrightersFlags[16], ymmDataPtr[16], ymmOnes, ymmNMinusOne, ymm254;

	compv_scalar_t fdarkers16[16];
	compv_scalar_t fbrighters16[16];
	__m256i ymmDdarkers16x32[16];
	__m256i ymmDbrighters16x32[16];
	__m256i *ymmStrengths = (__m256i *)strengths;

    _mm256_store_si256(&ymmZeros, _mm256_setzero_si256());
    _mm256_store_si256(&ymmThreshold, _mm256_set1_epi8((uint8_t)threshold));
    _mm256_store_si256(&ymmFF, _mm256_cmpeq_epi8(ymmZeros, ymmZeros)); // 0xFF=255
    _mm256_store_si256(&ymmOnes, _mm256_load_si256((__m256i*)k1_i8));
    _mm256_store_si256(&ymmNMinusOne, _mm256_set1_epi8((uint8_t)N - 1));
    _mm256_store_si256(&ymm254, _mm256_load_si256((__m256i*)k254_u8)); // not(254) = 00000001 -> used to select the lowest bit in each u8

    for (i = 0; i < width; i += 32) {
		_mm256_storeu_si256(ymmStrengths, ymmZeros); // cleanup strengths
        _mm256_store_si256(&ymm0, _mm256_loadu_si256((__m256i*)IP));
        _mm256_store_si256(&ymmBrighter, _mm256_adds_epu8(ymm0, ymmThreshold));
        _mm256_store_si256(&ymmDarker, _mm256_subs_epu8(ymm0, ymmThreshold));

        /* Motion estimation */
        if (IPprev) {
            (*me) = 0;
        }

        /*  Speed-Test-1 */

        // compare I1 and I9 aka 0 and 8
        _mm256_store_si256(&ymm0, _mm256_loadu_si256((__m256i*)&IP[pixels16[0]]));
        _mm256_store_si256(&ymm1, _mm256_loadu_si256((__m256i*)&IP[pixels16[8]]));
        _mm256_store_si256(&ymmDdarkers16x32[0], _mm256_subs_epu8(ymmDarker, ymm0));
        _mm256_store_si256(&ymmDdarkers16x32[8], _mm256_subs_epu8(ymmDarker, ymm1));
        _mm256_store_si256(&ymmDbrighters16x32[0], _mm256_subs_epu8(ymm0, ymmBrighter));
        _mm256_store_si256(&ymmDbrighters16x32[8], _mm256_subs_epu8(ymm1, ymmBrighter));
        _mm256_store_si256(&ymmDarkersFlags[0], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[0], ymmZeros), ymmFF));
        _mm256_store_si256(&ymmDarkersFlags[8], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[8], ymmZeros), ymmFF));
        _mm256_store_si256(&ymmBrightersFlags[0], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[0], ymmZeros), ymmFF));
        _mm256_store_si256(&ymmBrightersFlags[8], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[8], ymmZeros), ymmFF));
        _mm256_store_si256(&ymm0, _mm256_or_si256(ymmDarkersFlags[0], ymmBrightersFlags[0]));
        _mm256_store_si256(&ymm1, _mm256_or_si256(ymmDarkersFlags[8], ymmBrightersFlags[8]));
        sum = (_mm256_movemask_epi8(ymm0) ? 1 : 0) + (_mm256_movemask_epi8(ymm1) ? 1 : 0);
        if (!sum) {
            goto next;
        }

        // compare I5 and I13 aka 4 and 12
        _mm256_store_si256(&ymm0, _mm256_loadu_si256((__m256i*)&IP[pixels16[4]]));
        _mm256_store_si256(&ymm1, _mm256_loadu_si256((__m256i*)&IP[pixels16[12]]));
        _mm256_store_si256(&ymmDdarkers16x32[4], _mm256_subs_epu8(ymmDarker, ymm0));
        _mm256_store_si256(&ymmDdarkers16x32[12], _mm256_subs_epu8(ymmDarker, ymm1));
        _mm256_store_si256(&ymmDbrighters16x32[4], _mm256_subs_epu8(ymm0, ymmBrighter));
        _mm256_store_si256(&ymmDbrighters16x32[12], _mm256_subs_epu8(ymm1, ymmBrighter));
        _mm256_store_si256(&ymmDarkersFlags[4], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[4], ymmZeros), ymmFF));
        _mm256_store_si256(&ymmDarkersFlags[12], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[12], ymmZeros), ymmFF));
        _mm256_store_si256(&ymmBrightersFlags[4], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[4], ymmZeros), ymmFF));
        _mm256_store_si256(&ymmBrightersFlags[12], _mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[12], ymmZeros), ymmFF));
        _mm256_store_si256(&ymm2, _mm256_or_si256(ymmDarkersFlags[4], ymmBrightersFlags[4]));
        _mm256_store_si256(&ymm3, _mm256_or_si256(ymmDarkersFlags[12], ymmBrightersFlags[12]));
        s = (_mm256_movemask_epi8(ymm2) ? 1 : 0) + (_mm256_movemask_epi8(ymm3) ? 1 : 0);
        if (!s) {
            goto next;
        }
        sum += s;

        /*  Speed-Test-2 */
        if (N == 12 ? sum >= 3 : sum >= 2) {
            // Check wheter to load Brighters
            _mm256_store_si256(&ymm0, _mm256_or_si256(ymmBrightersFlags[0], ymmBrightersFlags[8]));
            _mm256_store_si256(&ymm1, _mm256_or_si256(ymmBrightersFlags[4], ymmBrightersFlags[12]));
            sum = _mm256_movemask_epi8(ymm0) ? 1 : 0;
            sum += _mm256_movemask_epi8(ymm1) ? 1 : 0;
            loadB = (sum > 1); // sum cannot be > 2 -> dot not check it against 3 for N = 12

            // Check wheter to load Darkers
            _mm256_store_si256(&ymm0, _mm256_or_si256(ymmDarkersFlags[0], ymmDarkersFlags[8]));
            _mm256_store_si256(&ymm1, _mm256_or_si256(ymmDarkersFlags[4], ymmDarkersFlags[12]));
            sum = _mm256_movemask_epi8(ymm0) ? 1 : 0;
            sum += _mm256_movemask_epi8(ymm1) ? 1 : 0;
            loadD = (sum > 1); // sum cannot be > 2 -> dot not check it against 3 for N = 12

            if (!(loadB || loadD)) {
                goto next;
            }

			colDarkersFlags = 0, colBrightersFlags = 0;

            _mm256_store_si256(&ymmDataPtr[1], _mm256_loadu_si256((__m256i*)&IP[pixels16[1]]));
            _mm256_store_si256(&ymmDataPtr[2], _mm256_loadu_si256((__m256i*)&IP[pixels16[2]]));
            _mm256_store_si256(&ymmDataPtr[3], _mm256_loadu_si256((__m256i*)&IP[pixels16[3]]));
            _mm256_store_si256(&ymmDataPtr[5], _mm256_loadu_si256((__m256i*)&IP[pixels16[5]]));
            _mm256_store_si256(&ymmDataPtr[6], _mm256_loadu_si256((__m256i*)&IP[pixels16[6]]));
            _mm256_store_si256(&ymmDataPtr[7], _mm256_loadu_si256((__m256i*)&IP[pixels16[7]]));
            _mm256_store_si256(&ymmDataPtr[9], _mm256_loadu_si256((__m256i*)&IP[pixels16[9]]));
            _mm256_store_si256(&ymmDataPtr[10], _mm256_loadu_si256((__m256i*)&IP[pixels16[10]]));
            _mm256_store_si256(&ymmDataPtr[11], _mm256_loadu_si256((__m256i*)&IP[pixels16[11]]));
            _mm256_store_si256(&ymmDataPtr[13], _mm256_loadu_si256((__m256i*)&IP[pixels16[13]]));
            _mm256_store_si256(&ymmDataPtr[14], _mm256_loadu_si256((__m256i*)&IP[pixels16[14]]));
            _mm256_store_si256(&ymmDataPtr[15], _mm256_loadu_si256((__m256i*)&IP[pixels16[15]]));

            // We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
            // Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
            // _mm256_movemask_epi8 is cyclyvore

            if (loadD) {
                // Compute ymmDdarkers
                _mm256_store_si256(&ymmDdarkers16x32[1], _mm256_subs_epu8(ymmDarker, ymmDataPtr[1]));
                _mm256_store_si256(&ymmDdarkers16x32[2], _mm256_subs_epu8(ymmDarker, ymmDataPtr[2]));
                _mm256_store_si256(&ymmDdarkers16x32[3], _mm256_subs_epu8(ymmDarker, ymmDataPtr[3]));
                _mm256_store_si256(&ymmDdarkers16x32[5], _mm256_subs_epu8(ymmDarker, ymmDataPtr[5]));
                _mm256_store_si256(&ymmDdarkers16x32[6], _mm256_subs_epu8(ymmDarker, ymmDataPtr[6]));
                _mm256_store_si256(&ymmDdarkers16x32[7], _mm256_subs_epu8(ymmDarker, ymmDataPtr[7]));
                _mm256_store_si256(&ymmDdarkers16x32[9], _mm256_subs_epu8(ymmDarker, ymmDataPtr[9]));
                _mm256_store_si256(&ymmDdarkers16x32[10], _mm256_subs_epu8(ymmDarker, ymmDataPtr[10]));
                _mm256_store_si256(&ymmDdarkers16x32[11], _mm256_subs_epu8(ymmDarker, ymmDataPtr[11]));
                _mm256_store_si256(&ymmDdarkers16x32[13], _mm256_subs_epu8(ymmDarker, ymmDataPtr[13]));
                _mm256_store_si256(&ymmDdarkers16x32[14], _mm256_subs_epu8(ymmDarker, ymmDataPtr[14]));
                _mm256_store_si256(&ymmDdarkers16x32[15], _mm256_subs_epu8(ymmDarker, ymmDataPtr[15]));
                /* Compute flags (not really, we have the inverse: 0xFF when zero, the not will be applied later) */
                _mm256_store_si256(&ymmDarkersFlags[1], _mm256_cmpeq_epi8(ymmDdarkers16x32[1], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[2], _mm256_cmpeq_epi8(ymmDdarkers16x32[2], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[3], _mm256_cmpeq_epi8(ymmDdarkers16x32[3], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[5], _mm256_cmpeq_epi8(ymmDdarkers16x32[5], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[6], _mm256_cmpeq_epi8(ymmDdarkers16x32[6], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[7], _mm256_cmpeq_epi8(ymmDdarkers16x32[7], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[9], _mm256_cmpeq_epi8(ymmDdarkers16x32[9], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[10], _mm256_cmpeq_epi8(ymmDdarkers16x32[10], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[11], _mm256_cmpeq_epi8(ymmDdarkers16x32[11], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[13], _mm256_cmpeq_epi8(ymmDdarkers16x32[13], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[14], _mm256_cmpeq_epi8(ymmDdarkers16x32[14], ymmZeros));
                _mm256_store_si256(&ymmDarkersFlags[15], _mm256_cmpeq_epi8(ymmDdarkers16x32[15], ymmZeros));
                // Convert flags from 0xFF to 0x01
                // 0 4 8 12 already computed and contains the right values (not the inverse)
                _mm256_store_si256(&ymmDarkersFlags[0], _mm256_andnot_si256(ymm254, ymmDarkersFlags[0]));
                _mm256_store_si256(&ymmDarkersFlags[4], _mm256_andnot_si256(ymm254, ymmDarkersFlags[4]));
                _mm256_store_si256(&ymmDarkersFlags[8], _mm256_andnot_si256(ymm254, ymmDarkersFlags[8]));
                _mm256_store_si256(&ymmDarkersFlags[12], _mm256_andnot_si256(ymm254, ymmDarkersFlags[12]));
                // other values
                _mm256_store_si256(&ymmDarkersFlags[1], _mm256_andnot_si256(ymmDarkersFlags[1], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[2], _mm256_andnot_si256(ymmDarkersFlags[2], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[3], _mm256_andnot_si256(ymmDarkersFlags[3], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[5], _mm256_andnot_si256(ymmDarkersFlags[5], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[6], _mm256_andnot_si256(ymmDarkersFlags[6], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[7], _mm256_andnot_si256(ymmDarkersFlags[7], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[9], _mm256_andnot_si256(ymmDarkersFlags[9], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[10], _mm256_andnot_si256(ymmDarkersFlags[10], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[11], _mm256_andnot_si256(ymmDarkersFlags[11], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[13], _mm256_andnot_si256(ymmDarkersFlags[13], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[14], _mm256_andnot_si256(ymmDarkersFlags[14], ymmOnes));
                _mm256_store_si256(&ymmDarkersFlags[15], _mm256_andnot_si256(ymmDarkersFlags[15], ymmOnes));
                // add all flags
                _mm256_store_si256(&ymmDarkersFlags[0], _mm256_adds_epu8(ymmDarkersFlags[0], ymmDarkersFlags[1]));
                _mm256_store_si256(&ymmDarkersFlags[2], _mm256_adds_epu8(ymmDarkersFlags[2], ymmDarkersFlags[3]));
                _mm256_store_si256(&ymmDarkersFlags[4], _mm256_adds_epu8(ymmDarkersFlags[4], ymmDarkersFlags[5]));
                _mm256_store_si256(&ymmDarkersFlags[6], _mm256_adds_epu8(ymmDarkersFlags[6], ymmDarkersFlags[7]));
                _mm256_store_si256(&ymmDarkersFlags[8], _mm256_adds_epu8(ymmDarkersFlags[8], ymmDarkersFlags[9]));
                _mm256_store_si256(&ymmDarkersFlags[10], _mm256_adds_epu8(ymmDarkersFlags[10], ymmDarkersFlags[11]));
                _mm256_store_si256(&ymmDarkersFlags[12], _mm256_adds_epu8(ymmDarkersFlags[12], ymmDarkersFlags[13]));
                _mm256_store_si256(&ymmDarkersFlags[14], _mm256_adds_epu8(ymmDarkersFlags[14], ymmDarkersFlags[15]));
                _mm256_store_si256(&ymmDarkersFlags[0], _mm256_adds_epu8(ymmDarkersFlags[0], ymmDarkersFlags[2]));
                _mm256_store_si256(&ymmDarkersFlags[4], _mm256_adds_epu8(ymmDarkersFlags[4], ymmDarkersFlags[6]));
                _mm256_store_si256(&ymmDarkersFlags[8], _mm256_adds_epu8(ymmDarkersFlags[8], ymmDarkersFlags[10]));
                _mm256_store_si256(&ymmDarkersFlags[12], _mm256_adds_epu8(ymmDarkersFlags[12], ymmDarkersFlags[14]));
                _mm256_store_si256(&ymmDarkersFlags[0], _mm256_adds_epu8(ymmDarkersFlags[0], ymmDarkersFlags[4]));
                _mm256_store_si256(&ymmDarkersFlags[8], _mm256_adds_epu8(ymmDarkersFlags[8], ymmDarkersFlags[12]));
                _mm256_store_si256(&ymmDarkersFlags[0], _mm256_adds_epu8(ymmDarkersFlags[0], ymmDarkersFlags[8])); // sum is in ymmDarkersFlags[0]
                // Check the columns with at least N non-zero bits
                _mm256_store_si256(&ymmDarkersFlags[0], _mm256_cmpgt_epi8(ymmDarkersFlags[0], ymmNMinusOne));
                colDarkersFlags = _mm256_movemask_epi8(ymmDarkersFlags[0]);
				loadD = (colDarkersFlags != 0);
            }

            if (loadB) {
                /* Compute Dbrighters */
                _mm256_store_si256(&ymmDbrighters16x32[1], _mm256_subs_epu8(ymmDataPtr[1], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[2], _mm256_subs_epu8(ymmDataPtr[2], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[3], _mm256_subs_epu8(ymmDataPtr[3], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[5], _mm256_subs_epu8(ymmDataPtr[5], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[6], _mm256_subs_epu8(ymmDataPtr[6], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[7], _mm256_subs_epu8(ymmDataPtr[7], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[9], _mm256_subs_epu8(ymmDataPtr[9], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[10], _mm256_subs_epu8(ymmDataPtr[10], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[11], _mm256_subs_epu8(ymmDataPtr[11], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[13], _mm256_subs_epu8(ymmDataPtr[13], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[14], _mm256_subs_epu8(ymmDataPtr[14], ymmBrighter));
                _mm256_store_si256(&ymmDbrighters16x32[15], _mm256_subs_epu8(ymmDataPtr[15], ymmBrighter));
                /* Compute flags (not really, we have the inverse: 0xFF when zero, the not will be applied later) */
                _mm256_store_si256(&ymmBrightersFlags[1], _mm256_cmpeq_epi8(ymmDbrighters16x32[1], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[2], _mm256_cmpeq_epi8(ymmDbrighters16x32[2], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[3], _mm256_cmpeq_epi8(ymmDbrighters16x32[3], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[5], _mm256_cmpeq_epi8(ymmDbrighters16x32[5], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[6], _mm256_cmpeq_epi8(ymmDbrighters16x32[6], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[7], _mm256_cmpeq_epi8(ymmDbrighters16x32[7], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[9], _mm256_cmpeq_epi8(ymmDbrighters16x32[9], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[10], _mm256_cmpeq_epi8(ymmDbrighters16x32[10], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[11], _mm256_cmpeq_epi8(ymmDbrighters16x32[11], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[13], _mm256_cmpeq_epi8(ymmDbrighters16x32[13], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[14], _mm256_cmpeq_epi8(ymmDbrighters16x32[14], ymmZeros));
                _mm256_store_si256(&ymmBrightersFlags[15], _mm256_cmpeq_epi8(ymmDbrighters16x32[15], ymmZeros));
                // Convert flags from 0xFF to 0x01
                // 0 4 8 12 already computed and contains the right values (not the inverse)
                _mm256_store_si256(&ymmBrightersFlags[0], _mm256_andnot_si256(ymm254, ymmBrightersFlags[0]));
                _mm256_store_si256(&ymmBrightersFlags[4], _mm256_andnot_si256(ymm254, ymmBrightersFlags[4]));
                _mm256_store_si256(&ymmBrightersFlags[8], _mm256_andnot_si256(ymm254, ymmBrightersFlags[8]));
                _mm256_store_si256(&ymmBrightersFlags[12], _mm256_andnot_si256(ymm254, ymmBrightersFlags[12]));
                // other values
                _mm256_store_si256(&ymmBrightersFlags[1], _mm256_andnot_si256(ymmBrightersFlags[1], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[2], _mm256_andnot_si256(ymmBrightersFlags[2], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[3], _mm256_andnot_si256(ymmBrightersFlags[3], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[5], _mm256_andnot_si256(ymmBrightersFlags[5], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[6], _mm256_andnot_si256(ymmBrightersFlags[6], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[7], _mm256_andnot_si256(ymmBrightersFlags[7], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[9], _mm256_andnot_si256(ymmBrightersFlags[9], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[10], _mm256_andnot_si256(ymmBrightersFlags[10], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[11], _mm256_andnot_si256(ymmBrightersFlags[11], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[13], _mm256_andnot_si256(ymmBrightersFlags[13], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[14], _mm256_andnot_si256(ymmBrightersFlags[14], ymmOnes));
                _mm256_store_si256(&ymmBrightersFlags[15], _mm256_andnot_si256(ymmBrightersFlags[15], ymmOnes));
                // add all flags
                _mm256_store_si256(&ymmBrightersFlags[0], _mm256_adds_epu8(ymmBrightersFlags[0], ymmBrightersFlags[1]));
                _mm256_store_si256(&ymmBrightersFlags[2], _mm256_adds_epu8(ymmBrightersFlags[2], ymmBrightersFlags[3]));
                _mm256_store_si256(&ymmBrightersFlags[4], _mm256_adds_epu8(ymmBrightersFlags[4], ymmBrightersFlags[5]));
                _mm256_store_si256(&ymmBrightersFlags[6], _mm256_adds_epu8(ymmBrightersFlags[6], ymmBrightersFlags[7]));
                _mm256_store_si256(&ymmBrightersFlags[8], _mm256_adds_epu8(ymmBrightersFlags[8], ymmBrightersFlags[9]));
                _mm256_store_si256(&ymmBrightersFlags[10], _mm256_adds_epu8(ymmBrightersFlags[10], ymmBrightersFlags[11]));
                _mm256_store_si256(&ymmBrightersFlags[12], _mm256_adds_epu8(ymmBrightersFlags[12], ymmBrightersFlags[13]));
                _mm256_store_si256(&ymmBrightersFlags[14], _mm256_adds_epu8(ymmBrightersFlags[14], ymmBrightersFlags[15]));
                _mm256_store_si256(&ymmBrightersFlags[0], _mm256_adds_epu8(ymmBrightersFlags[0], ymmBrightersFlags[2]));
                _mm256_store_si256(&ymmBrightersFlags[4], _mm256_adds_epu8(ymmBrightersFlags[4], ymmBrightersFlags[6]));
                _mm256_store_si256(&ymmBrightersFlags[8], _mm256_adds_epu8(ymmBrightersFlags[8], ymmBrightersFlags[10]));
                _mm256_store_si256(&ymmBrightersFlags[12], _mm256_adds_epu8(ymmBrightersFlags[12], ymmBrightersFlags[14]));
                _mm256_store_si256(&ymmBrightersFlags[0], _mm256_adds_epu8(ymmBrightersFlags[0], ymmBrightersFlags[4]));
                _mm256_store_si256(&ymmBrightersFlags[8], _mm256_adds_epu8(ymmBrightersFlags[8], ymmBrightersFlags[12]));
                _mm256_store_si256(&ymmBrightersFlags[0], _mm256_adds_epu8(ymmBrightersFlags[0], ymmBrightersFlags[8])); // sum is in ymmDarkersFlags[0]
                // Check the columns with at least N non-zero bits
                _mm256_store_si256(&ymm0, _mm256_cmpgt_epi8(ymmBrightersFlags[0], ymmNMinusOne));
                colBrightersFlags = _mm256_movemask_epi8(ymm0);
				loadB = (colBrightersFlags != 0);
            }

            if (loadD) {
                // Transpose
                COMPV_TRANSPOSE_I8_16X32_AVX2(
                    ymmDdarkers16x32[0], ymmDdarkers16x32[1], ymmDdarkers16x32[2], ymmDdarkers16x32[3],
                    ymmDdarkers16x32[4], ymmDdarkers16x32[5], ymmDdarkers16x32[6], ymmDdarkers16x32[7],
                    ymmDdarkers16x32[8], ymmDdarkers16x32[9], ymmDdarkers16x32[10], ymmDdarkers16x32[11],
                    ymmDdarkers16x32[12], ymmDdarkers16x32[13], ymmDdarkers16x32[14], ymmDdarkers16x32[15],
                    ymm0);
                // Flags
                fdarkers16[0] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[0], ymmZeros), ymmFF));
                fdarkers16[1] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[1], ymmZeros), ymmFF));
                fdarkers16[2] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[2], ymmZeros), ymmFF));
                fdarkers16[3] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[3], ymmZeros), ymmFF));
                fdarkers16[4] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[4], ymmZeros), ymmFF));
                fdarkers16[5] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[5], ymmZeros), ymmFF));
                fdarkers16[6] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[6], ymmZeros), ymmFF));
                fdarkers16[7] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[7], ymmZeros), ymmFF));
                fdarkers16[8] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[8], ymmZeros), ymmFF));
                fdarkers16[9] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[9], ymmZeros), ymmFF));
                fdarkers16[10] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[10], ymmZeros), ymmFF));
                fdarkers16[11] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[11], ymmZeros), ymmFF));
                fdarkers16[12] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[12], ymmZeros), ymmFF));
                fdarkers16[13] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[13], ymmZeros), ymmFF));
                fdarkers16[14] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[14], ymmZeros), ymmFF));
                fdarkers16[15] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDdarkers16x32[15], ymmZeros), ymmFF));
            }

            if (loadB) {
                // Transpose
                COMPV_TRANSPOSE_I8_16X32_AVX2(
                    ymmDbrighters16x32[0], ymmDbrighters16x32[1], ymmDbrighters16x32[2], ymmDbrighters16x32[3],
                    ymmDbrighters16x32[4], ymmDbrighters16x32[5], ymmDbrighters16x32[6], ymmDbrighters16x32[7],
                    ymmDbrighters16x32[8], ymmDbrighters16x32[9], ymmDbrighters16x32[10], ymmDbrighters16x32[11],
                    ymmDbrighters16x32[12], ymmDbrighters16x32[13], ymmDbrighters16x32[14], ymmDbrighters16x32[15],
                    ymm1);
                // Flags
                fbrighters16[0] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[0], ymmZeros), ymmFF));
                fbrighters16[1] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[1], ymmZeros), ymmFF));
                fbrighters16[2] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[2], ymmZeros), ymmFF));
                fbrighters16[3] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[3], ymmZeros), ymmFF));
                fbrighters16[4] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[4], ymmZeros), ymmFF));
                fbrighters16[5] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[5], ymmZeros), ymmFF));
                fbrighters16[6] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[6], ymmZeros), ymmFF));
                fbrighters16[7] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[7], ymmZeros), ymmFF));
                fbrighters16[8] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[8], ymmZeros), ymmFF));
                fbrighters16[9] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[9], ymmZeros), ymmFF));
                fbrighters16[10] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[10], ymmZeros), ymmFF));
                fbrighters16[11] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[11], ymmZeros), ymmFF));
                fbrighters16[12] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[12], ymmZeros), ymmFF));
                fbrighters16[13] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[13], ymmZeros), ymmFF));
                fbrighters16[14] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[14], ymmZeros), ymmFF));
                fbrighters16[15] = _mm256_movemask_epi8(_mm256_andnot_si256(_mm256_cmpeq_epi8(ymmDbrighters16x32[15], ymmZeros), ymmFF));
            }
			if (colBrightersFlags || colDarkersFlags) {
				FastStrengths32(colBrightersFlags, colDarkersFlags, (const uint8_t*)ymmDbrighters16x32, (const uint8_t*)ymmDdarkers16x32, &fbrighters16, &fdarkers16, (uint8_t*)ymmStrengths, N);
			}
        }
next:
        IP += 32;
        if (IPprev) {
            IPprev += 32;
        }
		ymmStrengths += 1;
    } // for i

    _mm256_zeroupper();
}

// Code Not used yet: AVX/SSE transition issue
// Use VPHMINPOSUW in ASM (no intrinsic)
#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void FastStrengths32_Intrin_AVX2(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(AVX) const uint8_t* dbrighters16x32, COMPV_ALIGNED(AVX) const uint8_t* ddarkers16x32, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv_scalar_t N)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // AVX/SSE transition penalties

	_mm256_zeroupper();

	__m256i ymm0, ymm1, ymmFastXFlags, ymmFLow, ymmFHigh;
	int r0, mask, maxnLow, maxnHigh, lowMin, highMin;
	uint32_t rb = (uint32_t)rbrighters, rd = (uint32_t)rdarkers;
	const uint16_t(&FastXFlags)[16] = N == 9 ? Fast9Flags : Fast12Flags;
	const uint8_t(&kFastArcs)[16][16] = (N == 9 ? kFast9Arcs : kFast12Arcs);
	__m128i xmm0, xmm1, xmmZeros;

	// FAST hard-coded flags
	_mm256_store_si256(&ymmFastXFlags, _mm256_load_si256((__m256i*)(FastXFlags)));

	_mm_store_si128(&xmmZeros, _mm_setzero_si128());

	// xmm0 contains the u8 values
	// xmm1 is used as temp register and will be trashed
#define COMPV_HORIZ_MIN(r, i, maxn) \
	if (r & (1 << i)) { \
		_mm_store_si128(&xmm1, _mm_shuffle_epi8(xmm0, _mm_load_si128((__m128i*)kFastArcs[i]))); /* eliminate zeros and duplicate first matching non-zero */ \
		lowMin = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_unpacklo_epi8(xmm1, xmmZeros))); \
		highMin = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_unpackhi_epi8(xmm1, xmmZeros))); \
		/* _mm_minpos_epu16 must set to zero the remaining bits but this doesn't look to happen or I missed something */ \
		lowMin &= 0xFFFF; \
		highMin &= 0xFFFF; \
		maxn = std::max(std::min(lowMin, highMin), (int)maxn); \
		}

	mask = (1 << 0) | (1 << 16); // (1 << p) | (1 << g)

	for (unsigned p = 0, g = 16; p < 16; ++p, ++g) {
		maxnLow = maxnHigh = 0;
		// Brighters
		if (rb & mask) {
			// brighters flags
			_mm256_store_si256(&ymmFLow, _mm256_set1_epi16((short)(*fbrighters16)[p]));
			_mm256_store_si256(&ymmFHigh, _mm256_set1_epi16((short)((*fbrighters16)[p] >> 16)));

			_mm256_store_si256(&ymm0, _mm256_and_si256(ymmFLow, ymmFastXFlags));
			_mm256_store_si256(&ymm0, _mm256_cmpeq_epi16(ymm0, ymmFastXFlags));
			_mm256_store_si256(&ymm1, _mm256_and_si256(ymmFHigh, ymmFastXFlags));
			_mm256_store_si256(&ymm1, _mm256_cmpeq_epi16(ymm1, ymmFastXFlags));
			r0 = _mm256_movemask_epi8(compv_avx2_packs_epi16(ymm0, ymm1)); // r0's popcnt is equal to N as FastXFlags contains values with popcnt==N
			if (r0) {
				_mm256_store_si256(&ymm0, _mm256_load_si256((__m256i*)dbrighters16x32));
				// Compute minimum hz (low)
				if (r0 & 0xFFFF) {
					_mm_store_si128(&xmm0, _mm256_extractf128_si256(ymm0, 0));
					COMPV_HORIZ_MIN(r0, 0, maxnLow) COMPV_HORIZ_MIN(r0, 1, maxnLow) COMPV_HORIZ_MIN(r0, 2, maxnLow) COMPV_HORIZ_MIN(r0, 3, maxnLow)
						COMPV_HORIZ_MIN(r0, 4, maxnLow) COMPV_HORIZ_MIN(r0, 5, maxnLow) COMPV_HORIZ_MIN(r0, 6, maxnLow) COMPV_HORIZ_MIN(r0, 7, maxnLow)
						COMPV_HORIZ_MIN(r0, 8, maxnLow) COMPV_HORIZ_MIN(r0, 9, maxnLow) COMPV_HORIZ_MIN(r0, 10, maxnLow) COMPV_HORIZ_MIN(r0, 11, maxnLow)
						COMPV_HORIZ_MIN(r0, 12, maxnLow) COMPV_HORIZ_MIN(r0, 13, maxnLow) COMPV_HORIZ_MIN(r0, 14, maxnLow) COMPV_HORIZ_MIN(r0, 15, maxnLow)
				}
				// Compute minimum hz (high)
				if (r0 & 0xFFFF0000) {
					r0 >>= 16;
					_mm_store_si128(&xmm0, _mm256_extractf128_si256(ymm0, 1));
					COMPV_HORIZ_MIN(r0, 0, maxnHigh) COMPV_HORIZ_MIN(r0, 1, maxnHigh) COMPV_HORIZ_MIN(r0, 2, maxnHigh) COMPV_HORIZ_MIN(r0, 3, maxnHigh)
					COMPV_HORIZ_MIN(r0, 4, maxnHigh) COMPV_HORIZ_MIN(r0, 5, maxnHigh) COMPV_HORIZ_MIN(r0, 6, maxnHigh) COMPV_HORIZ_MIN(r0, 7, maxnHigh)
					COMPV_HORIZ_MIN(r0, 8, maxnHigh) COMPV_HORIZ_MIN(r0, 9, maxnHigh) COMPV_HORIZ_MIN(r0, 10, maxnHigh) COMPV_HORIZ_MIN(r0, 11, maxnHigh)
					COMPV_HORIZ_MIN(r0, 12, maxnHigh) COMPV_HORIZ_MIN(r0, 13, maxnHigh) COMPV_HORIZ_MIN(r0, 14, maxnHigh) COMPV_HORIZ_MIN(r0, 15, maxnHigh)
				}
			}
		}

		// Darkers
		if (rd & mask) {
			// darkers flags
			_mm256_store_si256(&ymmFLow, _mm256_set1_epi16((short)(*fdarkers16)[p]));
			_mm256_store_si256(&ymmFHigh, _mm256_set1_epi16((short)((*fdarkers16)[p] >> 16)));

			_mm256_store_si256(&ymm0, _mm256_and_si256(ymmFLow, ymmFastXFlags));
			_mm256_store_si256(&ymm0, _mm256_cmpeq_epi16(ymm0, ymmFastXFlags));
			_mm256_store_si256(&ymm1, _mm256_and_si256(ymmFHigh, ymmFastXFlags));
			_mm256_store_si256(&ymm1, _mm256_cmpeq_epi16(ymm1, ymmFastXFlags));
			r0 = _mm256_movemask_epi8(compv_avx2_packs_epi16(ymm0, ymm1)); // r0's popcnt is equal to N as FastXFlags contains values with popcnt==N
			if (r0) {
				_mm256_store_si256(&ymm0, _mm256_load_si256((__m256i*)ddarkers16x32));
				// Compute minimum hz (low)
				if (r0 & 0xFFFF) {
					_mm_store_si128(&xmm0, _mm256_extractf128_si256(ymm0, 0));
					COMPV_HORIZ_MIN(r0, 0, maxnLow) COMPV_HORIZ_MIN(r0, 1, maxnLow) COMPV_HORIZ_MIN(r0, 2, maxnLow) COMPV_HORIZ_MIN(r0, 3, maxnLow)
					COMPV_HORIZ_MIN(r0, 4, maxnLow) COMPV_HORIZ_MIN(r0, 5, maxnLow) COMPV_HORIZ_MIN(r0, 6, maxnLow) COMPV_HORIZ_MIN(r0, 7, maxnLow)
					COMPV_HORIZ_MIN(r0, 8, maxnLow) COMPV_HORIZ_MIN(r0, 9, maxnLow) COMPV_HORIZ_MIN(r0, 10, maxnLow) COMPV_HORIZ_MIN(r0, 11, maxnLow)
					COMPV_HORIZ_MIN(r0, 12, maxnLow) COMPV_HORIZ_MIN(r0, 13, maxnLow) COMPV_HORIZ_MIN(r0, 14, maxnLow) COMPV_HORIZ_MIN(r0, 15, maxnLow)
				}

				// Compute minimum hz (high)
				if (r0 & 0xFFFF0000) {
					r0 >>= 16;
					_mm_store_si128(&xmm0, _mm256_extractf128_si256(ymm0, 1));
					COMPV_HORIZ_MIN(r0, 0, maxnHigh) COMPV_HORIZ_MIN(r0, 1, maxnHigh) COMPV_HORIZ_MIN(r0, 2, maxnHigh) COMPV_HORIZ_MIN(r0, 3, maxnHigh)
					COMPV_HORIZ_MIN(r0, 4, maxnHigh) COMPV_HORIZ_MIN(r0, 5, maxnHigh) COMPV_HORIZ_MIN(r0, 6, maxnHigh) COMPV_HORIZ_MIN(r0, 7, maxnHigh)
					COMPV_HORIZ_MIN(r0, 8, maxnHigh) COMPV_HORIZ_MIN(r0, 9, maxnHigh) COMPV_HORIZ_MIN(r0, 10, maxnHigh) COMPV_HORIZ_MIN(r0, 11, maxnHigh)
					COMPV_HORIZ_MIN(r0, 12, maxnHigh) COMPV_HORIZ_MIN(r0, 13, maxnHigh) COMPV_HORIZ_MIN(r0, 14, maxnHigh) COMPV_HORIZ_MIN(r0, 15, maxnHigh)
				}
			}
		}

		strengths32[p] = (uint8_t)maxnLow;
		strengths32[g] = (uint8_t)maxnHigh;

		dbrighters16x32 += 32;
		ddarkers16x32 += 32;

		mask <<= 1;
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
