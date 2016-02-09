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
#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_sse.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"
#include "compv/compv_mathutils.h"
#include "compv/compv_bits.h"
#include "compv/compv_cpu.h"

#include <algorithm>

extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast9Arcs[16][16];
extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast12Arcs[16][16];

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): ASM version
// TODO(dmi): add AVX
compv_scalar_t FastData_Intrin_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, COMPV_ALIGNED(SSE) int16_t(&ddarkers16)[16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16)[16])
{
    int32_t sum;
    COMPV_ALIGN_SSE() uint8_t temp16[16];

    int16_t brighter = (int16_t)(dataPtr[0] + threshold);
    int16_t darker = (int16_t)(dataPtr[0] - threshold);

    bool popcntHard = CompVCpu::isSupported(kCpuFlagPOPCNT);

    // compare I1 and I7
    temp16[0] = dataPtr[pixels16[0]];
    temp16[8] = dataPtr[pixels16[8]];
    ddarkers16[0] = (darker - temp16[0]);
    ddarkers16[8] = (darker - temp16[8]);
    dbrighters16[0] = (temp16[0] - brighter);
    dbrighters16[8] = (temp16[8] - brighter);

    sum = (dbrighters16[0] > 0 || ddarkers16[0] > 0) + (dbrighters16[8] > 0 || ddarkers16[8] > 0);

    /*  Speed-Test-1 */
    if (N != 12 || sum > 0) {
        // compare I5 and I13
        temp16[4] = dataPtr[pixels16[4]];
        temp16[12] = dataPtr[pixels16[12]];
        ddarkers16[4] = (darker - temp16[4]); // I5-darkness
        ddarkers16[12] = (darker - temp16[12]); // I13-darkness
        dbrighters16[4] = (temp16[4] - brighter); // I5-brightness
        dbrighters16[12] = (temp16[12] - brighter); // I13-brightness

        sum += (dbrighters16[4] > 0 || ddarkers16[4] > 0) + (dbrighters16[12] > 0 || ddarkers16[12] > 0);
        /*  Speed-Test-2 */
        if ((sum >= 2 && (N != 12 || sum >= 3))) {
            temp16[1] = dataPtr[pixels16[1]];
            temp16[2] = dataPtr[pixels16[2]];
            temp16[3] = dataPtr[pixels16[3]];
            temp16[5] = dataPtr[pixels16[5]];
            temp16[6] = dataPtr[pixels16[6]];
            temp16[7] = dataPtr[pixels16[7]];
            temp16[9] = dataPtr[pixels16[9]];
            temp16[10] = dataPtr[pixels16[10]];
            temp16[11] = dataPtr[pixels16[11]];
            temp16[13] = dataPtr[pixels16[13]];
            temp16[14] = dataPtr[pixels16[14]];
            temp16[15] = dataPtr[pixels16[15]];

            __m128i xmmTemp16, xmmDdarkers16, xmmDbrighters16, xmmDarker, xmmBrighter, xmmZeros;

            _mm_store_si128(&xmmZeros, _mm_setzero_si128());

            _mm_store_si128(&xmmTemp16, _mm_load_si128((__m128i*)temp16));
            _mm_store_si128(&xmmDarker, _mm_set1_epi8((int8_t)darker));
            _mm_store_si128(&xmmBrighter, _mm_set1_epi8((int8_t)brighter));

            _mm_store_si128(&xmmDdarkers16, _mm_subs_epu8(xmmDarker, xmmTemp16));
            _mm_store_si128(&xmmDbrighters16, _mm_subs_epu8(xmmTemp16, xmmBrighter));
            // _mm_cmpgt_epi8 uses signed integers while we're using unsigned values and there is no _mm_cmpneq_epi8.
            *pfdarkers = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDdarkers16, xmmZeros));
            *pfbrighters = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDbrighters16, xmmZeros));

            // The flags contain int values with the highest bits always set -> we must use popcnt16 or at least popcnt32(flag&0xFFFF)
            compv_scalar_t popcnt0 = compv_popcnt16(popcntHard, (unsigned short)*pfdarkers);
            compv_scalar_t popcnt1 = compv_popcnt16(popcntHard, (unsigned short)*pfbrighters);
            if (popcnt0 >= N || popcnt1 >= N) {
                // Convert ddarkers16 and dbrighters16 from epu8 to epi16
                _mm_store_si128((__m128i*)&ddarkers16[0], _mm_unpacklo_epi8(xmmDdarkers16, xmmZeros));
                _mm_store_si128((__m128i*)&ddarkers16[8], _mm_unpackhi_epi8(xmmDdarkers16, xmmZeros));
                _mm_store_si128((__m128i*)&dbrighters16[0], _mm_unpacklo_epi8(xmmDbrighters16, xmmZeros));
                _mm_store_si128((__m128i*)&dbrighters16[8], _mm_unpackhi_epi8(xmmDbrighters16, xmmZeros));
                return 1;
            }
        }
    }
    return 0;
}

// TODO(dmi): Fast9 and Fast12 for asm
// TODO(dmi): ASM version
// TODO(dmi): add AVX
compv_scalar_t FastData16_Intrin_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16])
{
    compv_scalar_t r = 0, sum, sumd, sumb, d0, d1, b0, b1;
    __m128i xmm0, xmm1, xmmBrighter, xmmDarker, xmmThreshold, xmmZeros;
    bool popcntHard = CompVCpu::isSupported(kCpuFlagPOPCNT);

    // ddarkers16x16 and ddarkers16x16 are int16 arrays but we want to use there memory to store uint8[] temp variables until the end of the process then we convert them
    // These arrays are int16 to make sure the CPP code won't need to sature all operations
    __m128i (&xmmDdarkers16x16)[16][2] = (__m128i (&)[16][2])ddarkers16x16;
    __m128i (&xmmDbrighters16x16)[16][2] = (__m128i (&)[16][2])dbrighters16x16;

    _mm_store_si128(&xmmZeros, _mm_setzero_si128());

    _mm_store_si128(&xmmThreshold, _mm_set1_epi8((uint8_t)threshold));
    _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)dataPtr));
    _mm_store_si128(&xmmBrighter, _mm_adds_epu8(xmm0, xmmThreshold));
    _mm_store_si128(&xmmDarker, _mm_subs_epu8(xmm0, xmmThreshold));

    // compare I1 and I7 aka 0 and 8
    _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[0]]));
    _mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[8]]));
    _mm_store_si128(&xmmDdarkers16x16[0][0], _mm_subs_epu8(xmmDarker, xmm0));
    _mm_store_si128(&xmmDdarkers16x16[8][0], _mm_subs_epu8(xmmDarker, xmm1));
    _mm_store_si128(&xmmDbrighters16x16[0][0], _mm_subs_epu8(xmm0, xmmBrighter));
    _mm_store_si128(&xmmDbrighters16x16[8][0], _mm_subs_epu8(xmm1, xmmBrighter));
    /*  Speed-Test-1 */
    d0 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDdarkers16x16[0][0], xmmZeros));
    d1 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDdarkers16x16[8][0], xmmZeros));
    b0 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDbrighters16x16[0][0], xmmZeros));
    b1 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDbrighters16x16[8][0], xmmZeros));
    b0 &= 0xffff; //FIXME: remove
    b1 &= 0xffff; //FIXME: remove
    d0 &= 0xffff; //FIXME: remove
    d1 &= 0xffff; //FIXME: remove
    sumb = compv_popcnt16(popcntHard, (unsigned short)b0) + compv_popcnt16(popcntHard, (unsigned short)b1);
    sumd = compv_popcnt16(popcntHard, (unsigned short)d0) + compv_popcnt16(popcntHard, (unsigned short)d1);
    b0 |= d0; // I1 is too brighter or too darker
    b1 |= d1; // I7 is too brighter or too darker
    // The flags contain int values with the highest bits always set -> we must use popcnt16 or at least popcnt32(flag&0xFFFF)
    b0 = compv_popcnt16(popcntHard, (unsigned short)b0);
    b1 = compv_popcnt16(popcntHard, (unsigned short)b1);

    sum = b0 + b1;

    if (N != 12 || sum > 0) {
        // compare I5 and I13 aka 4 and 12
        _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[4]]));
        _mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[12]]));
        _mm_store_si128(&xmmDdarkers16x16[4][0], _mm_subs_epu8(xmmDarker, xmm0));
        _mm_store_si128(&xmmDdarkers16x16[12][0], _mm_subs_epu8(xmmDarker, xmm1));
        _mm_store_si128(&xmmDbrighters16x16[4][0], _mm_subs_epu8(xmm0, xmmBrighter));
        _mm_store_si128(&xmmDbrighters16x16[12][0], _mm_subs_epu8(xmm1, xmmBrighter));

        /*if ((sum < 2 || (N == 12 && sum < 3)))*/ { // re-check sum only if we've chance not to reach 3 (for N=12) or 2 (for N=9)
            b0 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDbrighters16x16[4][0], xmmZeros));
            b1 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDbrighters16x16[12][0], xmmZeros));
            d0 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDdarkers16x16[4][0], xmmZeros));
            d1 = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDdarkers16x16[12][0], xmmZeros));
            b0 &= 0xffff; //FIXME: remove
            b1 &= 0xffff; //FIXME: remove
            d0 &= 0xffff; //FIXME: remove
            d1 &= 0xffff; //FIXME: remove
            sumb += compv_popcnt16(popcntHard, (unsigned short)b0) + compv_popcnt16(popcntHard, (unsigned short)b1);
            sumd += compv_popcnt16(popcntHard, (unsigned short)d0) + compv_popcnt16(popcntHard, (unsigned short)d1);

            b0 |= d0; // I5 is too brighter or too darker
            b1 |= d1; // I13 is too brighter or too darker
            // The flags contain int values with the highest bits always set -> we must use popcnt16 or at least popcnt32(flag&0xFFFF)
            b0 = compv_popcnt16(popcntHard, (unsigned short)b0);
            b1 = compv_popcnt16(popcntHard, (unsigned short)b1);
            sum += b0 + b1;
        }
        /*  Speed-Test-2 */
        if (N == 12 ? (sum >= 3) : (sum >= 2)) {
            __m128i xmm2, xmm3, xmm4, xmm5;
            bool loadB = (N == 12 ? (sumb >= 3) : (sumb >= 2));
            bool loadD = (N == 12 ? (sumd >= 3) : (sumd >= 2));

            // 1 2 3 5 6 7
            _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[1]]));
            _mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[2]]));
            _mm_store_si128(&xmm2, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[3]]));
            _mm_store_si128(&xmm3, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[5]]));
            _mm_store_si128(&xmm4, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[6]]));
            _mm_store_si128(&xmm5, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[7]]));
            if (loadD) {
                _mm_store_si128(&xmmDdarkers16x16[1][0], _mm_subs_epu8(xmmDarker, xmm0));
                _mm_store_si128(&xmmDdarkers16x16[2][0], _mm_subs_epu8(xmmDarker, xmm1));
                _mm_store_si128(&xmmDdarkers16x16[3][0], _mm_subs_epu8(xmmDarker, xmm2));
                _mm_store_si128(&xmmDdarkers16x16[5][0], _mm_subs_epu8(xmmDarker, xmm3));
                _mm_store_si128(&xmmDdarkers16x16[6][0], _mm_subs_epu8(xmmDarker, xmm4));
                _mm_store_si128(&xmmDdarkers16x16[7][0], _mm_subs_epu8(xmmDarker, xmm5));
            }
            if (loadB) {
                _mm_store_si128(&xmmDbrighters16x16[1][0], _mm_subs_epu8(xmm0, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[2][0], _mm_subs_epu8(xmm1, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[3][0], _mm_subs_epu8(xmm2, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[5][0], _mm_subs_epu8(xmm3, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[6][0], _mm_subs_epu8(xmm4, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[7][0], _mm_subs_epu8(xmm5, xmmBrighter));
            }
            // 9 10 11 13 14 15
            _mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[9]]));
            _mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[10]]));
            _mm_store_si128(&xmm2, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[11]]));
            _mm_store_si128(&xmm3, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[13]]));
            _mm_store_si128(&xmm4, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[14]]));
            _mm_store_si128(&xmm5, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[15]]));
            if (loadD) {
                _mm_store_si128(&xmmDdarkers16x16[9][0], _mm_subs_epu8(xmmDarker, xmm0));
                _mm_store_si128(&xmmDdarkers16x16[10][0], _mm_subs_epu8(xmmDarker, xmm1));
                _mm_store_si128(&xmmDdarkers16x16[11][0], _mm_subs_epu8(xmmDarker, xmm2));
                _mm_store_si128(&xmmDdarkers16x16[13][0], _mm_subs_epu8(xmmDarker, xmm3));
                _mm_store_si128(&xmmDdarkers16x16[14][0], _mm_subs_epu8(xmmDarker, xmm4));
                _mm_store_si128(&xmmDdarkers16x16[15][0], _mm_subs_epu8(xmmDarker, xmm5));
            }
            if (loadB) {
                _mm_store_si128(&xmmDbrighters16x16[9][0], _mm_subs_epu8(xmm0, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[10][0], _mm_subs_epu8(xmm1, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[11][0], _mm_subs_epu8(xmm2, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[13][0], _mm_subs_epu8(xmm3, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[14][0], _mm_subs_epu8(xmm4, xmmBrighter));
                _mm_store_si128(&xmmDbrighters16x16[15][0], _mm_subs_epu8(xmm5, xmmBrighter));
            }

            // Transpose, Flags, Convert from epi8 to epi16, Build the return value
            if (loadD) {
                // Transpose
                COMPV_TRANSPOSE_I8_16X16_SSE2(
                    xmmDdarkers16x16[0][0], xmmDdarkers16x16[1][0], xmmDdarkers16x16[2][0], xmmDdarkers16x16[3][0],
                    xmmDdarkers16x16[4][0], xmmDdarkers16x16[5][0], xmmDdarkers16x16[6][0], xmmDdarkers16x16[7][0],
                    xmmDdarkers16x16[8][0], xmmDdarkers16x16[9][0], xmmDdarkers16x16[10][0], xmmDdarkers16x16[11][0],
                    xmmDdarkers16x16[12][0], xmmDdarkers16x16[13][0], xmmDdarkers16x16[14][0], xmmDdarkers16x16[15][0],
                    xmm0);
                // Flags
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDdarkers16x16[0][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDdarkers16x16[1][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDdarkers16x16[2][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDdarkers16x16[3][0], xmmZeros));
                pfdarkers16[0] = ~_mm_movemask_epi8(xmm0);
                pfdarkers16[1] = ~_mm_movemask_epi8(xmm1);
                pfdarkers16[2] = ~_mm_movemask_epi8(xmm2);
                pfdarkers16[3] = ~_mm_movemask_epi8(xmm3);
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDdarkers16x16[4][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDdarkers16x16[5][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDdarkers16x16[6][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDdarkers16x16[7][0], xmmZeros));
                pfdarkers16[4] = ~_mm_movemask_epi8(xmm0);
                pfdarkers16[5] = ~_mm_movemask_epi8(xmm1);
                pfdarkers16[6] = ~_mm_movemask_epi8(xmm2);
                pfdarkers16[7] = ~_mm_movemask_epi8(xmm3);
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDdarkers16x16[8][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDdarkers16x16[9][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDdarkers16x16[10][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDdarkers16x16[11][0], xmmZeros));
                pfdarkers16[8] = ~_mm_movemask_epi8(xmm0);
                pfdarkers16[9] = ~_mm_movemask_epi8(xmm1);
                pfdarkers16[10] = ~_mm_movemask_epi8(xmm2);
                pfdarkers16[11] = ~_mm_movemask_epi8(xmm3);
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDdarkers16x16[12][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDdarkers16x16[13][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDdarkers16x16[14][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDdarkers16x16[15][0], xmmZeros));
                pfdarkers16[12] = ~_mm_movemask_epi8(xmm0);
                pfdarkers16[13] = ~_mm_movemask_epi8(xmm1);
                pfdarkers16[14] = ~_mm_movemask_epi8(xmm2);
                pfdarkers16[15] = ~_mm_movemask_epi8(xmm3);
                // Convert from epi8 to epi16
#define COMPV_DI16(i) _mm_store_si128(&xmmDdarkers16x16[i][1], _mm_unpackhi_epi8(xmmDdarkers16x16[i][0], xmmZeros));  _mm_store_si128(&xmmDdarkers16x16[i][0], _mm_unpacklo_epi8(xmmDdarkers16x16[i][0], xmmZeros));
                COMPV_DI16(0) COMPV_DI16(1) COMPV_DI16(2) COMPV_DI16(3) COMPV_DI16(4) COMPV_DI16(5) COMPV_DI16(6) COMPV_DI16(7)
                COMPV_DI16(8) COMPV_DI16(9) COMPV_DI16(10) COMPV_DI16(11) COMPV_DI16(12) COMPV_DI16(13) COMPV_DI16(14) COMPV_DI16(15)
                // Build the return value
#define COMPV_BUILD_RD(i) d0 = compv_popcnt16(popcntHard, (unsigned short)pfdarkers16[i]); if (d0 >= N) r |= ((compv_scalar_t)1 << i);
                COMPV_BUILD_RD(0) COMPV_BUILD_RD(1) COMPV_BUILD_RD(2) COMPV_BUILD_RD(3) COMPV_BUILD_RD(4) COMPV_BUILD_RD(5) COMPV_BUILD_RD(6) COMPV_BUILD_RD(7)
                COMPV_BUILD_RD(8) COMPV_BUILD_RD(9) COMPV_BUILD_RD(10) COMPV_BUILD_RD(11) COMPV_BUILD_RD(12) COMPV_BUILD_RD(13) COMPV_BUILD_RD(14) COMPV_BUILD_RD(15)
            }
            else {
                pfdarkers16[0] = pfdarkers16[1] = pfdarkers16[2] = pfdarkers16[3] = pfdarkers16[4] = pfdarkers16[5] = pfdarkers16[6]
                                                  = pfdarkers16[7] = pfdarkers16[8] = pfdarkers16[9] = pfdarkers16[10] = pfdarkers16[11] = pfdarkers16[12] = pfdarkers16[13]
                                                          = pfdarkers16[14] = pfdarkers16[15] = 0;
            }

            // Transpose, Flags, Convert from epi8 to epi16, Build the return value
            if (loadB) {
                // Transpose
                COMPV_TRANSPOSE_I8_16X16_SSE2(
                    xmmDbrighters16x16[0][0], xmmDbrighters16x16[1][0], xmmDbrighters16x16[2][0], xmmDbrighters16x16[3][0],
                    xmmDbrighters16x16[4][0], xmmDbrighters16x16[5][0], xmmDbrighters16x16[6][0], xmmDbrighters16x16[7][0],
                    xmmDbrighters16x16[8][0], xmmDbrighters16x16[9][0], xmmDbrighters16x16[10][0], xmmDbrighters16x16[11][0],
                    xmmDbrighters16x16[12][0], xmmDbrighters16x16[13][0], xmmDbrighters16x16[14][0], xmmDbrighters16x16[15][0],
                    xmm1);
                // Flags
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDbrighters16x16[0][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDbrighters16x16[1][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDbrighters16x16[2][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDbrighters16x16[3][0], xmmZeros));
                pfbrighters16[0] = ~_mm_movemask_epi8(xmm0);
                pfbrighters16[1] = ~_mm_movemask_epi8(xmm1);
                pfbrighters16[2] = ~_mm_movemask_epi8(xmm2);
                pfbrighters16[3] = ~_mm_movemask_epi8(xmm3);
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDbrighters16x16[4][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDbrighters16x16[5][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDbrighters16x16[6][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDbrighters16x16[7][0], xmmZeros));
                pfbrighters16[4] = ~_mm_movemask_epi8(xmm0);
                pfbrighters16[5] = ~_mm_movemask_epi8(xmm1);
                pfbrighters16[6] = ~_mm_movemask_epi8(xmm2);
                pfbrighters16[7] = ~_mm_movemask_epi8(xmm3);
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDbrighters16x16[8][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDbrighters16x16[9][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDbrighters16x16[10][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDbrighters16x16[11][0], xmmZeros));
                pfbrighters16[8] = ~_mm_movemask_epi8(xmm0);
                pfbrighters16[9] = ~_mm_movemask_epi8(xmm1);
                pfbrighters16[10] = ~_mm_movemask_epi8(xmm2);
                pfbrighters16[11] = ~_mm_movemask_epi8(xmm3);
                _mm_store_si128(&xmm0, _mm_cmpeq_epi8(xmmDbrighters16x16[12][0], xmmZeros));
                _mm_store_si128(&xmm1, _mm_cmpeq_epi8(xmmDbrighters16x16[13][0], xmmZeros));
                _mm_store_si128(&xmm2, _mm_cmpeq_epi8(xmmDbrighters16x16[14][0], xmmZeros));
                _mm_store_si128(&xmm3, _mm_cmpeq_epi8(xmmDbrighters16x16[15][0], xmmZeros));
                pfbrighters16[12] = ~_mm_movemask_epi8(xmm0);
                pfbrighters16[13] = ~_mm_movemask_epi8(xmm1);
                pfbrighters16[14] = ~_mm_movemask_epi8(xmm2);
                pfbrighters16[15] = ~_mm_movemask_epi8(xmm3);
                // Convert from epi8 to epi16
#define COMPV_BI16(i) _mm_store_si128(&xmmDbrighters16x16[i][1], _mm_unpackhi_epi8(xmmDbrighters16x16[i][0], xmmZeros)); _mm_store_si128(&xmmDbrighters16x16[i][0], _mm_unpacklo_epi8(xmmDbrighters16x16[i][0], xmmZeros));
                COMPV_BI16(0) COMPV_BI16(1) COMPV_BI16(2) COMPV_BI16(3) COMPV_BI16(4) COMPV_BI16(5) COMPV_BI16(6) COMPV_BI16(7)
                COMPV_BI16(8) COMPV_BI16(9) COMPV_BI16(10) COMPV_BI16(11) COMPV_BI16(12) COMPV_BI16(13) COMPV_BI16(14) COMPV_BI16(15)
                // Build the return value
#define COMPV_BUILD_RB(i) b0 = compv_popcnt16(popcntHard, (unsigned short)pfbrighters16[i]); if (b0 >= N) r |= ((compv_scalar_t)1 << i);
                COMPV_BUILD_RB(0) COMPV_BUILD_RB(1) COMPV_BUILD_RB(2) COMPV_BUILD_RB(3) COMPV_BUILD_RB(4) COMPV_BUILD_RB(5) COMPV_BUILD_RB(6) COMPV_BUILD_RB(7)
                COMPV_BUILD_RB(8) COMPV_BUILD_RB(9) COMPV_BUILD_RB(10) COMPV_BUILD_RB(11) COMPV_BUILD_RB(12) COMPV_BUILD_RB(13) COMPV_BUILD_RB(14) COMPV_BUILD_RB(15)
            }
            else {
                pfbrighters16[0] = pfbrighters16[1] = pfbrighters16[2] = pfbrighters16[3] = pfbrighters16[4] = pfbrighters16[5] = pfbrighters16[6]
                                                      = pfbrighters16[7] = pfbrighters16[8] = pfbrighters16[9] = pfbrighters16[10] = pfbrighters16[11] = pfbrighters16[12] = pfbrighters16[13]
                                                              = pfbrighters16[14] = pfbrighters16[15] = 0;
            }

#undef COMPV_DI16
#undef COMPV_BI16
#undef COMPV_BUILD_RD
#undef COMPV_BUILD_RB
        }
    }

    return (int)r;
}

// TODO(dmi): ASM version
// TODO(dmi): add AVX
compv_scalar_t FastStrengths_SSE2(COMPV_ALIGNED(SSE) const int16_t (&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t (&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t (&FastXFlags)[16])
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // FastStrengths_(ASM/INTRIN)_SSE41 is the best choice

    __m128i xmm0, xmm1, xmmFbrighters, xmmFdarkers, xmmZeros, xmmFastXFlagsLow, xmmFastXFlagsHigh;
    int r0 = 0, r1 = 0;
    unsigned i, j, k;
    int strength, maxnbrighter = 0, maxndarker = 0;

    _mm_store_si128(&xmmZeros, _mm_setzero_si128());

    // brighters and darkers flags
    _mm_store_si128(&xmmFbrighters, _mm_set1_epi16((short)fbrighters));
    _mm_store_si128(&xmmFdarkers, _mm_set1_epi16((short)fdarkers));

    // FAST hard-coded flags
    _mm_store_si128(&xmmFastXFlagsLow, _mm_load_si128((__m128i*)(FastXFlags + 0)));
    _mm_store_si128(&xmmFastXFlagsHigh, _mm_load_si128((__m128i*)(FastXFlags + 8)));

    // Brighters
    if (fbrighters) {
        _mm_store_si128(&xmm0, _mm_and_si128(xmmFbrighters, xmmFastXFlagsLow));
        _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
        _mm_store_si128(&xmm1, _mm_and_si128(xmmFbrighters, xmmFastXFlagsHigh));
        _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
        // xmm0 and xmm1 contain zeros and 0xFF values, if packed and satured we'll end with all-zeros -> perform a ">> 1" which keep sign bit
        _mm_store_si128(&xmm0, _mm_srli_epi16(xmm0, 1));
        _mm_store_si128(&xmm1, _mm_srli_epi16(xmm1, 1));
        r0 = _mm_movemask_epi8(_mm_packus_epi16(xmm0, xmm1)); // r0's popcnt is equal to N as FastXFlags contains values with popcnt==N
        if (r0) {
            int16_t nbrighter;
            for (i = 0; i < 16; ++i) {
                if (r0 & (1 << i)) {
                    // Compute Horizontal minimum (TODO: Find SSE2 method)
                    nbrighter = 255;
                    k = unsigned(i + N);
                    for (j = i; j < k; ++j) {
                        if (dbrighters[j & 15] < nbrighter) {
                            nbrighter = dbrighters[j & 15];
                        }
                    }
                    maxnbrighter = std::max((int)nbrighter, (int)maxnbrighter);
                }
            }
        }
    }

    // Darkers
    if (fdarkers) {
        _mm_store_si128(&xmm0, _mm_and_si128(xmmFdarkers, xmmFastXFlagsLow));
        _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
        _mm_store_si128(&xmm1, _mm_and_si128(xmmFdarkers, xmmFastXFlagsHigh));
        _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
        // xmm0 and xmm1 contain zeros and 0xFF values, if packed and satured we'll end with all-zeros -> perform a ">> 1" which keep sign bit
        _mm_store_si128(&xmm0, _mm_srli_epi16(xmm0, 1));
        _mm_store_si128(&xmm1, _mm_srli_epi16(xmm1, 1));
        r1 = _mm_movemask_epi8(_mm_packus_epi16(xmm0, xmm1)); // r1's popcnt is equal to N as FastXFlags contains values with popcnt==N
        if (r1) {
            int16_t ndarker;
            for (i = 0; i < 16; ++i) {
                if (r1 & (1 << i)) {
                    // Compute Horizontal minimum
                    ndarker = 255;
                    k = unsigned(i + N);
                    for (j = i; j < k; ++j) {
                        if (ddarkers[j & 15] < ndarker) {
                            ndarker = ddarkers[j & 15];
                        }
                    }
                    maxndarker = std::max((int)ndarker, (int)maxndarker);
                }
            }
        }
        else if (!r0) {
            return 0;
        }
    }

    strength = std::max(maxndarker, maxnbrighter);

    return compv_scalar_t(strength);
}

// TODO(dmi): add AVX
compv_scalar_t FastStrengths_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
{
    __m128i xmm0, xmm1, xmmFastXFlagsLow, xmmFastXFlagsHigh;
    int r0 = 0, r1 = 0;
    int strength, maxnbrighter = 0, maxndarker = 0;

    // FAST hard-coded flags
    _mm_store_si128(&xmmFastXFlagsLow, _mm_load_si128((__m128i*)(FastXFlags + 0)));
    _mm_store_si128(&xmmFastXFlagsHigh, _mm_load_si128((__m128i*)(FastXFlags + 8)));

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

    // Brighters
    if (fbrighters) {
        __m128i xmmFbrighters;

        // brighters flags
        _mm_store_si128(&xmmFbrighters, _mm_set1_epi16((short)fbrighters));

        _mm_store_si128(&xmm0, _mm_and_si128(xmmFbrighters, xmmFastXFlagsLow));
        _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
        _mm_store_si128(&xmm1, _mm_and_si128(xmmFbrighters, xmmFastXFlagsHigh));
        _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
        // clear the high bit in the epi16, otherwise will be considered as the sign bit when saturated to u8
        _mm_store_si128(&xmm0, _mm_srli_epi16(xmm0, 1));
        _mm_store_si128(&xmm1, _mm_srli_epi16(xmm1, 1));
        r0 = _mm_movemask_epi8(_mm_packus_epi16(xmm0, xmm1)); // r0's popcnt is equal to N as FastXFlags contains values with popcnt==N
        if (r0) {
            // most of the time we only have brighters or darkers not both, this is why the xmmZeros is duplicated
            int lowMin, highMin;
            __m128i xmmZeros;
            const uint8_t(&kFastArcs)[16][16] = (N == 12 ? kFast12Arcs : kFast9Arcs);

            _mm_store_si128(&xmmZeros, _mm_setzero_si128());
            // Load dbrighters and convert it from i16 to u8 and saturate
            _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)(dbrighters + 0)));
            _mm_store_si128(&xmm1, _mm_load_si128((__m128i*)(dbrighters + 8)));
            _mm_store_si128(&xmm0, _mm_packus_epi16(xmm0, xmm1));
            // Compute minimum hz
            COMPV_HORIZ_MIN(r0, 0, maxnbrighter) COMPV_HORIZ_MIN(r0, 1, maxnbrighter) COMPV_HORIZ_MIN(r0, 2, maxnbrighter) COMPV_HORIZ_MIN(r0, 3, maxnbrighter)
            COMPV_HORIZ_MIN(r0, 4, maxnbrighter) COMPV_HORIZ_MIN(r0, 5, maxnbrighter) COMPV_HORIZ_MIN(r0, 6, maxnbrighter) COMPV_HORIZ_MIN(r0, 7, maxnbrighter)
            COMPV_HORIZ_MIN(r0, 8, maxnbrighter) COMPV_HORIZ_MIN(r0, 9, maxnbrighter) COMPV_HORIZ_MIN(r0, 10, maxnbrighter) COMPV_HORIZ_MIN(r0, 11, maxnbrighter)
            COMPV_HORIZ_MIN(r0, 12, maxnbrighter) COMPV_HORIZ_MIN(r0, 13, maxnbrighter) COMPV_HORIZ_MIN(r0, 14, maxnbrighter) COMPV_HORIZ_MIN(r0, 15, maxnbrighter)
        }
    }

    // Darkers
    if (fdarkers) {
        __m128i xmmFdarkers;

        // darkers flags
        _mm_store_si128(&xmmFdarkers, _mm_set1_epi16((short)fdarkers));

        _mm_store_si128(&xmm0, _mm_and_si128(xmmFdarkers, xmmFastXFlagsLow));
        _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
        _mm_store_si128(&xmm1, _mm_and_si128(xmmFdarkers, xmmFastXFlagsHigh));
        _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
        // clear the high bit in the epi16, otherwise will be considered as the sign bit when saturated to u8
        _mm_store_si128(&xmm0, _mm_srli_epi16(xmm0, 1));
        _mm_store_si128(&xmm1, _mm_srli_epi16(xmm1, 1));
        r1 = _mm_movemask_epi8(_mm_packus_epi16(xmm0, xmm1)); // r1's popcnt is equal to N as FastXFlags contains values with popcnt==N
        if (r1) {
            int lowMin, highMin;
            __m128i xmmZeros;
            const uint8_t(&kFastArcs)[16][16] = (N == 12 ? kFast12Arcs : kFast9Arcs);

            _mm_store_si128(&xmmZeros, _mm_setzero_si128());
            // Load ddarkers and convert it from i16 to u8 and saturate
            _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)(ddarkers + 0)));
            _mm_store_si128(&xmm1, _mm_load_si128((__m128i*)(ddarkers + 8)));
            _mm_store_si128(&xmm0, _mm_packus_epi16(xmm0, xmm1));
            // Compute minimum hz
            COMPV_HORIZ_MIN(r1, 0, maxndarker) COMPV_HORIZ_MIN(r1, 1, maxndarker) COMPV_HORIZ_MIN(r1, 2, maxndarker) COMPV_HORIZ_MIN(r1, 3, maxndarker)
            COMPV_HORIZ_MIN(r1, 4, maxndarker) COMPV_HORIZ_MIN(r1, 5, maxndarker) COMPV_HORIZ_MIN(r1, 6, maxndarker) COMPV_HORIZ_MIN(r1, 7, maxndarker)
            COMPV_HORIZ_MIN(r1, 8, maxndarker) COMPV_HORIZ_MIN(r1, 9, maxndarker) COMPV_HORIZ_MIN(r1, 10, maxndarker) COMPV_HORIZ_MIN(r1, 11, maxndarker)
            COMPV_HORIZ_MIN(r1, 12, maxndarker) COMPV_HORIZ_MIN(r1, 13, maxndarker) COMPV_HORIZ_MIN(r1, 14, maxndarker) COMPV_HORIZ_MIN(r1, 15, maxndarker)
        }
        else if (!r0) {
            return 0;
        }
    }

    strength = std::max(maxndarker, maxnbrighter);

    return compv_scalar_t(strength);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
