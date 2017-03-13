/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_utils_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "strideInBytes" must be SSE-aligned
void MathUtilsSumAbs_16i16u_Intrin_SSSE3(const COMPV_ALIGNED(SSE) int16_t* a, const COMPV_ALIGNED(SSE) int16_t* b, COMPV_ALIGNED(SSE) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): add ASM SSSE3 version (use "pabsw")
    COMPV_DEBUG_INFO_CHECK_SSSE3();
    compv_uscalar_t j;
    __m128i xmm0, xmm1, xmm2, xmm3;
    compv_scalar_t i, width_ = static_cast<compv_scalar_t>(width);

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width_ - 31; i += 32) {
            xmm0 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i))));
            xmm1 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i + 8))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i + 8))));
            xmm2 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i + 16))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i + 16))));
            xmm3 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i + 24))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i + 24))));
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i), xmm0);
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i + 8), xmm1);
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i + 16), xmm2);
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i + 24), xmm3);
        }
        for (; i < width_; i += 8) {
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i), _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i)))));
        }

        r += stride;
        a += stride;
        b += stride;
    }
}

// Doesn't work with "signed int8"
// FIXME: this is SSE2 function
void MathUtilsSum_8u32u_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* data, compv_uscalar_t count, uint32_t *sum1)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
    COMPV_DEBUG_INFO_CHECK_SSE2();
    __m128i xmmSum = _mm_setzero_si128(), xmm0;
    const __m128i xmmZero = _mm_setzero_si128();
    compv_scalar_t count_ = static_cast<compv_scalar_t>(count), i;
    for (i = 0; i < count_ - 15; i += 16) {
        xmm0 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + i));
        // conversion from "uint8" to "uint16" using unpack will lose sign -> doesn't work with "signed int8"
        // SSE4.1 "_mm_cvtepi8_epi16" would work
        // Same remark apply to conversion from epi16 to epi32
        xmm0 = _mm_add_epi16(_mm_unpacklo_epi8(xmm0, xmmZero), _mm_unpackhi_epi8(xmm0, xmmZero));
        xmm0 = _mm_add_epi32(_mm_unpacklo_epi16(xmm0, xmmZero), _mm_unpackhi_epi16(xmm0, xmmZero));
        xmmSum = _mm_add_epi32(xmmSum, xmm0);
    }
    if (i < count_ - 7) {
        xmm0 = _mm_unpacklo_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(data + i)), xmmZero);
        xmm0 = _mm_add_epi32(_mm_unpacklo_epi16(xmm0, xmmZero), _mm_unpackhi_epi16(xmm0, xmmZero));
        xmmSum = _mm_add_epi32(xmmSum, xmm0);
        i += 8;
    }
    if (i < count_ - 3) {
        xmm0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(data + i)), xmmZero);
        xmmSum = _mm_add_epi32(xmmSum, _mm_unpacklo_epi16(xmm0, xmmZero));
        i += 4;
    }

    xmmSum = _mm_hadd_epi32(xmmSum, xmmSum);
    xmmSum = _mm_hadd_epi32(xmmSum, xmmSum);
    uint32_t sum = (uint32_t)_mm_cvtsi128_si32(xmmSum);

    if (i < count_ - 1) {
        sum += data[i] + data[i + 1];
        i += 2;
    }
    if (count_ & 1) {
        sum += data[i];
    }
    *sum1 = sum;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
