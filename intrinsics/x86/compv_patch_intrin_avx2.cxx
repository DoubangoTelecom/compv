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
#include "compv/intrinsics/x86/compv_patch_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void Moments0110_Intrin_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* top, COMPV_ALIGNED(AVX2)const uint8_t* bottom, COMPV_ALIGNED(AVX2)const int16_t* x, COMPV_ALIGNED(AVX2) const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
    COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // AVX-SSE transition penalities

    __m256i ymmTop, ymmBottom, ymmT, ymmB, ymm0, ymm1, ymm2, ymm3, ymm4, ymmX, ymmY, ymmZero;

    compv_scalar_t s01_ = *s01;
    compv_scalar_t s10_ = *s10;

    ymmZero = _mm256_setzero_si256();

    // TODO(dmi): FMA3 for AVX

    // max(x|y) = 15 (patch radius)
    // max(top|bottom) = 255
    // -> (x|y) * (top|bottom +- (top|bottom)) is within [-32640, +32640] = [-0x7F80, +0x7F80]
    // -> we can use "_mm256_mullo_epi16" without overflow

    // top, bottom, x, y are allocated with padding which means you can read up to align_fwd(count, 32)
    for (compv_scalar_t i = 0; i < count; i += 32) {
        ymmTop = _mm256_permute4x64_epi64(_mm256_load_si256((__m256i*)&top[i]), COMPV_MM_SHUFFLE(3, 1, 2, 0));
        ymmBottom = _mm256_permute4x64_epi64(_mm256_load_si256((__m256i*)&bottom[i]), COMPV_MM_SHUFFLE(3, 1, 2, 0));

        // s10_ += *x * (*top + *bottom) or (x * top) + (x * bottom)
        // s01_ += *y * (*top - *bottom) or (y * top) - (y * bottom)

        ymmT = _mm256_unpacklo_epi8(ymmTop, ymmZero); // SSE4.1: _mm256_cvtepi8_epi16
        ymmB = _mm256_unpacklo_epi8(ymmBottom, ymmZero);
        ymmX = _mm256_load_si256((__m256i*)&x[i]);
        ymmY = _mm256_load_si256((__m256i*)&y[i]);

        ymm2 = _mm256_mullo_epi16(ymmX, _mm256_add_epi16(ymmT, ymmB));
        ymm3 = _mm256_srai_epi32(_mm256_unpacklo_epi16(ymmZero, ymm2), 16); // Convert from I16 to I32 while shifting in sign bits, ASM: use '_mm256_cvtepi16_epi32' which is SSE4.1
        ymm4 = _mm256_srai_epi32(_mm256_unpackhi_epi16(ymmZero, ymm2), 16);
        ymm0 = _mm256_hadd_epi32(ymm3, ymm4);

        ymm2 = _mm256_mullo_epi16(ymmY, _mm256_sub_epi16(ymmT, ymmB));
        ymm3 = _mm256_srai_epi32(_mm256_unpacklo_epi16(ymmZero, ymm2), 16);
        ymm4 = _mm256_srai_epi32(_mm256_unpackhi_epi16(ymmZero, ymm2), 16);
        ymm1 = _mm256_hadd_epi32(ymm3, ymm4);

        ymm0 = _mm256_hadd_epi32(ymm0, ymm1);

        ymmT = _mm256_unpackhi_epi8(ymmTop, ymmZero);
        ymmB = _mm256_unpackhi_epi8(ymmBottom, ymmZero);
        ymmX = _mm256_load_si256((__m256i*)&x[i + 16]);
        ymmY = _mm256_load_si256((__m256i*)&y[i + 16]);

        ymm2 = _mm256_mullo_epi16(ymmX, _mm256_add_epi16(ymmT, ymmB));
        ymm3 = _mm256_srai_epi32(_mm256_unpacklo_epi16(ymmZero, ymm2), 16);
        ymm4 = _mm256_srai_epi32(_mm256_unpackhi_epi16(ymmZero, ymm2), 16);
        ymm1 = _mm256_hadd_epi32(ymm3, ymm4);

        ymm2 = _mm256_mullo_epi16(ymmY, _mm256_sub_epi16(ymmT, ymmB));
        ymm3 = _mm256_srai_epi32(_mm256_unpacklo_epi16(ymmZero, ymm2), 16);
        ymm4 = _mm256_srai_epi32(_mm256_unpackhi_epi16(ymmZero, ymm2), 16);
        ymm3 = _mm256_hadd_epi32(ymm3, ymm4);

        ymm1 = _mm256_hadd_epi32(ymm1, ymm3);

        ymm0 = _mm256_hadd_epi32(ymm0, ymm1);
#if 0
        s10_ += (int32_t)_mm256_extract_epi32(ymm0, 0);
        s01_ += (int32_t)_mm256_extract_epi32(ymm0, 1);
        s10_ += (int32_t)_mm256_extract_epi32(ymm0, 2);
        s01_ += (int32_t)_mm256_extract_epi32(ymm0, 3);
#else
        // AVX-SSE transition issue
        COMPV_DEBUG_INFO_CODE_FOR_TESTING();
        __m128i xmm0 = _mm256_extracti128_si256(ymm0, 0);
        s10_ += (int32_t)_mm_extract_epi32(xmm0, 0);
        s01_ += (int32_t)_mm_extract_epi32(xmm0, 1);
        s10_ += (int32_t)_mm_extract_epi32(xmm0, 2);
        s01_ += (int32_t)_mm_extract_epi32(xmm0, 3);
        xmm0 = _mm256_extracti128_si256(ymm0, 1);
        s10_ += (int32_t)_mm_extract_epi32(xmm0, 0);
        s01_ += (int32_t)_mm_extract_epi32(xmm0, 1);
        s10_ += (int32_t)_mm_extract_epi32(xmm0, 2);
        s01_ += (int32_t)_mm_extract_epi32(xmm0, 3);
#endif
    }

    *s01 = s01_;
    *s10 = s10_;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
