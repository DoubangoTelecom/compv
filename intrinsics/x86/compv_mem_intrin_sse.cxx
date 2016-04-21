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
#include "compv/intrinsics/x86/compv_mem_intrin_sse.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/compv_simd_globals.h"
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM version
// size must be > 16 and it's up to the caller to check it
// size should be multiple of 16, if not the remaining will be ignored
void MemCopy_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size)
{
    const __m128i* xmmSrc = (const __m128i*)dataSrcPtr;
    __m128i* xmmDst = (__m128i*)dataDstPtr;
    compv_uscalar_t count16x16 = (size >> 8);
    compv_uscalar_t count16x1 = (size >> 4);

    if (count16x16 > 0) {
        count16x1 = ((size - (count16x16 << 8))) >> 4;
        for (compv_uscalar_t i = 0; i < count16x16; ++i) {
            _mm_store_si128(&xmmDst[0], _mm_load_si128(&xmmSrc[0]));
            _mm_store_si128(&xmmDst[1], _mm_load_si128(&xmmSrc[1]));
            _mm_store_si128(&xmmDst[2], _mm_load_si128(&xmmSrc[2]));
            _mm_store_si128(&xmmDst[3], _mm_load_si128(&xmmSrc[3]));
            _mm_store_si128(&xmmDst[4], _mm_load_si128(&xmmSrc[4]));
            _mm_store_si128(&xmmDst[5], _mm_load_si128(&xmmSrc[5]));
            _mm_store_si128(&xmmDst[6], _mm_load_si128(&xmmSrc[6]));
            _mm_store_si128(&xmmDst[7], _mm_load_si128(&xmmSrc[7]));
            _mm_store_si128(&xmmDst[8], _mm_load_si128(&xmmSrc[8]));
            _mm_store_si128(&xmmDst[9], _mm_load_si128(&xmmSrc[9]));
            _mm_store_si128(&xmmDst[10], _mm_load_si128(&xmmSrc[10]));
            _mm_store_si128(&xmmDst[11], _mm_load_si128(&xmmSrc[11]));
            _mm_store_si128(&xmmDst[12], _mm_load_si128(&xmmSrc[12]));
            _mm_store_si128(&xmmDst[13], _mm_load_si128(&xmmSrc[13]));
            _mm_store_si128(&xmmDst[14], _mm_load_si128(&xmmSrc[14]));
            _mm_store_si128(&xmmDst[15], _mm_load_si128(&xmmSrc[15]));
            xmmDst += 16;
            xmmSrc += 16;
        }
    }

    for (compv_uscalar_t i = 0; i < count16x1; ++i) {
        _mm_store_si128(xmmDst, _mm_load_si128(xmmSrc));
        ++xmmSrc;
        ++xmmDst;
    }
}

void MemCopyNTA_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size)
{
    const __m128i* xmmSrc = (const __m128i*)dataSrcPtr;
    __m128i* xmmDst = (__m128i*)dataDstPtr;
    compv_uscalar_t count16x16 = (size >> 8);
    compv_uscalar_t count16x1 = (size >> 4);

    if (count16x16 > 0) {
        count16x1 = ((size - (count16x16 << 8))) >> 4;
        for (compv_uscalar_t i = 0; i < count16x16; ++i) {
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 0), _MM_HINT_NTA);
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 1), _MM_HINT_NTA);
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 2), _MM_HINT_NTA);
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 3), _MM_HINT_NTA);
            _mm_stream_si128(&xmmDst[0], _mm_load_si128(&xmmSrc[0]));
            _mm_stream_si128(&xmmDst[1], _mm_load_si128(&xmmSrc[1]));
            _mm_stream_si128(&xmmDst[2], _mm_load_si128(&xmmSrc[2]));
            _mm_stream_si128(&xmmDst[3], _mm_load_si128(&xmmSrc[3]));
            _mm_stream_si128(&xmmDst[4], _mm_load_si128(&xmmSrc[4]));
            _mm_stream_si128(&xmmDst[5], _mm_load_si128(&xmmSrc[5]));
            _mm_stream_si128(&xmmDst[6], _mm_load_si128(&xmmSrc[6]));
            _mm_stream_si128(&xmmDst[7], _mm_load_si128(&xmmSrc[7]));
            _mm_stream_si128(&xmmDst[8], _mm_load_si128(&xmmSrc[8]));
            _mm_stream_si128(&xmmDst[9], _mm_load_si128(&xmmSrc[9]));
            _mm_stream_si128(&xmmDst[10], _mm_load_si128(&xmmSrc[10]));
            _mm_stream_si128(&xmmDst[11], _mm_load_si128(&xmmSrc[11]));
            _mm_stream_si128(&xmmDst[12], _mm_load_si128(&xmmSrc[12]));
            _mm_stream_si128(&xmmDst[13], _mm_load_si128(&xmmSrc[13]));
            _mm_stream_si128(&xmmDst[14], _mm_load_si128(&xmmSrc[14]));
            _mm_stream_si128(&xmmDst[15], _mm_load_si128(&xmmSrc[15]));
            xmmDst += 16;
            xmmSrc += 16;
        }
    }

    for (compv_uscalar_t i = 0; i < count16x1; ++i) {
        _mm_stream_si128(xmmDst, _mm_load_si128(xmmSrc));
        ++xmmSrc;
        ++xmmDst;
    }
    _mm_mfence(); // flush latest WC (Write Combine) buffers to memory
}

void MemZeroNTA_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dstPtr, compv_uscalar_t size)
{
    __m128i *xmmDst = (__m128i*)dstPtr, xmmZero;
    compv_uscalar_t count16x16 = (size >> 8);
    compv_uscalar_t count16x1 = (size >> 4);

    _mm_store_si128(&xmmZero, _mm_setzero_si128());

    if (count16x16 > 0) {
        count16x1 = ((size - (count16x16 << 8))) >> 4;
        for (compv_uscalar_t i = 0; i < count16x16; ++i) {
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 0), _MM_HINT_NTA);
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 1), _MM_HINT_NTA);
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 2), _MM_HINT_NTA);
            _mm_prefetch(((const char*)xmmDst) + 256 + (64 * 3), _MM_HINT_NTA);
            _mm_stream_si128(&xmmDst[0], xmmZero);
            _mm_stream_si128(&xmmDst[1], xmmZero);
            _mm_stream_si128(&xmmDst[2], xmmZero);
            _mm_stream_si128(&xmmDst[3], xmmZero);
            _mm_stream_si128(&xmmDst[4], xmmZero);
            _mm_stream_si128(&xmmDst[5], xmmZero);
            _mm_stream_si128(&xmmDst[6], xmmZero);
            _mm_stream_si128(&xmmDst[7], xmmZero);
            _mm_stream_si128(&xmmDst[8], xmmZero);
            _mm_stream_si128(&xmmDst[9], xmmZero);
            _mm_stream_si128(&xmmDst[10], xmmZero);
            _mm_stream_si128(&xmmDst[11], xmmZero);
            _mm_stream_si128(&xmmDst[12], xmmZero);
            _mm_stream_si128(&xmmDst[13], xmmZero);
            _mm_stream_si128(&xmmDst[14], xmmZero);
            _mm_stream_si128(&xmmDst[15], xmmZero);
            xmmDst += 16;
        }
    }

    for (compv_uscalar_t i = 0; i < count16x1; ++i) {
        _mm_stream_si128(xmmDst, xmmZero);
        ++xmmDst;
    }
    _mm_mfence(); // flush latest WC (Write Combine) buffers to memory
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
