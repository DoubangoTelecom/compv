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
#include "compv/intrinsics/x86/compv_hamming_intrin_sse.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_math.h"
#include "compv/compv_bits.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// popcnt available starting SSE4.2 but up to the caller to check its availability using CPU features
void HammingDistance_Intrin_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_scalar_t width, compv_scalar_t stride, compv_scalar_t height, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM code almost #2 times faster (VS2013)

    compv_scalar_t i, j;
    uint64_t cnt;
    uint64_t pop;
    __m128i xmm0;

    for (j = 0; j < height; ++j) {
        cnt = 0;
        i = 0;

        for (; i <= width - 16; i += 16) {
            xmm0 = _mm_xor_si128(_mm_load_si128((__m128i*)&dataPtr[i]), _mm_load_si128((__m128i*)&patch1xnPtr[i]));
#if COMPV_ARCH_X64
            cnt += compv_popcnt64((uint64_t)_mm_cvtsi128_si64(xmm0));
            cnt += compv_popcnt64((uint64_t)_mm_extract_epi64(xmm0, 1));
#else
            cnt += compv_popcnt32((uint32_t)_mm_cvtsi128_si32(xmm0));
            cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm0, 1));
            cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm0, 2));
            cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm0, 3));
#endif
        }
#if COMPV_ARCH_X64
        for (; i <= width - 8; i += 8) {
            pop = *((uint64_t*)&dataPtr[i]) ^ *((uint64_t*)&patch1xnPtr[i]);
            cnt += compv_popcnt64(pop);
        }
#endif
        for (; i <= width - 4; i += 4) {
            pop = *((uint32_t*)&dataPtr[i]) ^ *((uint32_t*)&patch1xnPtr[i]);
            cnt += compv_popcnt32((uint32_t)pop);
        }
        if (i <= width - 2) {
            pop = *((uint16_t*)&dataPtr[i]) ^ *((uint16_t*)&patch1xnPtr[i]);
            cnt += compv_popcnt16((uint16_t)pop);
            i += 2;
        }
        if (i <= width - 1) {
            pop = *((uint8_t*)&dataPtr[i]) ^ *((uint8_t*)&patch1xnPtr[i]);
            cnt += compv_popcnt16((uint16_t)pop);
            ++i;
        }
        dataPtr += stride;
        distPtr[j] = (int32_t)(cnt);
    }
}

// popcnt available starting SSE4.2 but up to the caller to check its availability using CPU features
void HammingDistance256_Intrin_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_scalar_t height, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM code almost #2 times faster (VS2013)

    uint64_t cnt;
    __m128i xmm0, xmm1;

    for (compv_scalar_t j = 0; j < height; ++j) {
        xmm0 = _mm_xor_si128(_mm_load_si128((__m128i*)&dataPtr[0]), _mm_load_si128((__m128i*)&patch1xnPtr[0]));
        xmm1 = _mm_xor_si128(_mm_load_si128((__m128i*)&dataPtr[16]), _mm_load_si128((__m128i*)&patch1xnPtr[16]));
#if COMPV_ARCH_X64
        cnt = compv_popcnt64((uint64_t)_mm_cvtsi128_si64(xmm0));
        cnt += compv_popcnt64((uint64_t)_mm_extract_epi64(xmm0, 1));
        cnt += compv_popcnt64((uint64_t)_mm_cvtsi128_si64(xmm1));
        cnt += compv_popcnt64((uint64_t)_mm_extract_epi64(xmm1, 1));
#else
        cnt = compv_popcnt32((uint32_t)_mm_cvtsi128_si32(xmm0));
        cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm0, 1));
        cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm0, 2));
        cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm0, 3));
        cnt += compv_popcnt32((uint32_t)_mm_cvtsi128_si32(xmm1));
        cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm1, 1));
        cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm1, 2));
        cnt += compv_popcnt32((uint32_t)_mm_extract_epi32(xmm1, 3));
#endif
        dataPtr += 32;
        distPtr[j] = (int32_t)(cnt);
    }
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
