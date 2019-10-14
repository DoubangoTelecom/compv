/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/x86/compv_mem_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM version
// size must be > 16 and it's up to the caller to check it
// size should be multiple of 16, if not the remaining will be ignored
void CompVMemCopy_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size)
{
    COMPV_DEBUG_INFO_CHECK_SSE2();
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

void CompVMemCopyNTA_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size)
{
    COMPV_DEBUG_INFO_CHECK_SSE2();
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

void CompVMemZeroNTA_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dstPtr, compv_uscalar_t size)
{
    COMPV_DEBUG_INFO_CHECK_SSE2();
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

void CompVMemPack2_Intrin_SSE2(
	COMPV_ALIGNED(SSE) compv_uint8x2_t* dstPtr, COMPV_ALIGNED(SSE) const uint8_t* srcPt0, COMPV_ALIGNED(SSE) const uint8_t* srcPt1,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) { // strided/SSE-aligned -> can write beyond width
			const __m128i vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&srcPt0[i])); // UVUVUVUVUVUVUVUV
			const __m128i vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&srcPt1[i])); // UVUVUVUVUVUVUVUV
			_mm_store_si128(reinterpret_cast<__m128i*>(&dstPtr[i]), _mm_unpacklo_epi8(vec0, vec1)); // UUUUUUUUUUUUUUUU
			_mm_store_si128(reinterpret_cast<__m128i*>(&dstPtr[i + 8]), _mm_unpackhi_epi8(vec0, vec1)); // VVVVVVVVVVVVVVVV
		}
		dstPtr += stride;
		srcPt0 += stride;
		srcPt1 += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
