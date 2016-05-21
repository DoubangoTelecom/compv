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
#include "compv/intrinsics/x86/features/orb/compv_feature_orb_desc_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/compv_simd_globals.h"
#include "compv/compv_math_utils.h"
#include "compv/compv_bits.h"
#include "compv/compv_cpu.h"

COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AX[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AY[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BX[256];
COMPV_EXTERNC const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BY[256];

COMPV_NAMESPACE_BEGIN()


// TODO(dmi): add ASM version
#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void Brief256_31_Intrin_AVX2(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Visual Studio inserts SSE code
    _mm256_zeroupper();

    int i, u8_index;
    COMPV_ALIGN_AVX2() int32_t ymmIndex[8];
    COMPV_ALIGN_AVX2() uint8_t ymmA[32];
    COMPV_ALIGN_AVX2() uint8_t ymmB[32];
    __m256i ymmX, ymmY, ymmStride, ymmR, ymm128;

    __m256 ymmCosT, ymmSinT, ymmXF, ymmYF;

    uint32_t* outPtr = (uint32_t*)out;

    const float* Brief256Pattern31AX = &kBrief256Pattern31AX[0];
    const float* Brief256Pattern31AY = &kBrief256Pattern31AY[0];
    const float* Brief256Pattern31BX = &kBrief256Pattern31BX[0];
    const float* Brief256Pattern31BY = &kBrief256Pattern31BY[0];

    _mm256_store_si256(&ymm128, _mm256_load_si256((__m256i*)k128_u8));
    _mm256_store_si256(&ymmStride, _mm256_set1_epi32((int)img_stride));
    _mm256_store_ps((float*)&ymmCosT, _mm256_set1_ps(*cos1));
    _mm256_store_ps((float*)&ymmSinT, _mm256_set1_ps(*sin1));

    u8_index = 0;

    for (i = 0; i < 256; i += 8) {
        // xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
        _mm256_store_ps((float*)&ymmXF, _mm256_sub_ps(_mm256_mul_ps(_mm256_load_ps(Brief256Pattern31AX), ymmCosT), _mm256_mul_ps(_mm256_load_ps(Brief256Pattern31AY), ymmSinT)));
        // yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
        _mm256_store_ps((float*)&ymmYF, _mm256_add_ps(_mm256_mul_ps(_mm256_load_ps(Brief256Pattern31AX), ymmSinT), _mm256_mul_ps(_mm256_load_ps(Brief256Pattern31AY), ymmCosT)));
        // x = COMPV_MATH_ROUNDF_2_INT(xf, int);
        _mm256_store_si256(&ymmX, _mm256_cvtps_epi32(ymmXF));
        // y = COMPV_MATH_ROUNDF_2_INT(yf, int);
        _mm256_store_si256(&ymmY, _mm256_cvtps_epi32(ymmYF));
        // a = img_center[(y * img_stride) + x];
        _mm256_store_si256((__m256i*)ymmIndex, _mm256_add_epi32(_mm256_mullo_epi32(ymmY, ymmStride), ymmX));
        ymmA[u8_index + 0] = img_center[ymmIndex[0]];
        ymmA[u8_index + 1] = img_center[ymmIndex[1]];
        ymmA[u8_index + 2] = img_center[ymmIndex[2]];
        ymmA[u8_index + 3] = img_center[ymmIndex[3]];
        ymmA[u8_index + 4] = img_center[ymmIndex[4]];
        ymmA[u8_index + 5] = img_center[ymmIndex[5]];
        ymmA[u8_index + 6] = img_center[ymmIndex[6]];
        ymmA[u8_index + 7] = img_center[ymmIndex[7]];

        // xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
        _mm256_store_ps((float*)&ymmXF, _mm256_sub_ps(_mm256_mul_ps(_mm256_load_ps(Brief256Pattern31BX), ymmCosT), _mm256_mul_ps(_mm256_load_ps(Brief256Pattern31BY), ymmSinT)));
        // yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
        _mm256_store_ps((float*)&ymmYF, _mm256_add_ps(_mm256_mul_ps(_mm256_load_ps(Brief256Pattern31BX), ymmSinT), _mm256_mul_ps(_mm256_load_ps(Brief256Pattern31BY), ymmCosT)));
        // x = COMPV_MATH_ROUNDF_2_INT(xf, int);
        _mm256_store_si256(&ymmX, _mm256_cvtps_epi32(ymmXF));
        // y = COMPV_MATH_ROUNDF_2_INT(yf, int);
        _mm256_store_si256(&ymmY, _mm256_cvtps_epi32(ymmYF));
        // b = img_center[(y * img_stride) + x];
        _mm256_store_si256((__m256i*)ymmIndex, _mm256_add_epi32(_mm256_mullo_epi32(ymmY, ymmStride), ymmX)); // _mm_mullo_epi32 is SSE4.1
        ymmB[u8_index + 0] = img_center[ymmIndex[0]];
        ymmB[u8_index + 1] = img_center[ymmIndex[1]];
        ymmB[u8_index + 2] = img_center[ymmIndex[2]];
        ymmB[u8_index + 3] = img_center[ymmIndex[3]];
        ymmB[u8_index + 4] = img_center[ymmIndex[4]];
        ymmB[u8_index + 5] = img_center[ymmIndex[5]];
        ymmB[u8_index + 6] = img_center[ymmIndex[6]];
        ymmB[u8_index + 7] = img_center[ymmIndex[7]];

        if ((u8_index += 8) == 32) {
            // _out[0] |= (a < b) ? (u64_1 << j) : 0;
            // Important: '_mm256_cmplt_epi8' doesn't exist -> we use '_mm256_cmpgt_epi8' and swap (a, b)
            _mm256_store_si256(&ymmR, _mm256_cmpgt_epi8(_mm256_sub_epi8(_mm256_load_si256((__m256i*)ymmB), ymm128), _mm256_sub_epi8(_mm256_load_si256((__m256i*)ymmA), ymm128))); // _mm256_cmpgt_epu8 does exist
            *outPtr = _mm256_movemask_epi8(ymmR);

            u8_index = 0;
            outPtr += 1;
        }

        Brief256Pattern31AX += 8;
        Brief256Pattern31AY += 8;
        Brief256Pattern31BX += 8;
        Brief256Pattern31BY += 8;
    }
    _mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
