/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_utils_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "strideInBytes" must be SSE-aligned
void CompVMathUtilsSumAbs_16s16u_Intrin_SSSE3(const COMPV_ALIGNED(SSE) int16_t* a, const COMPV_ALIGNED(SSE) int16_t* b, COMPV_ALIGNED(SSE) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CHECK_SSSE3();
    compv_uscalar_t j;
    __m128i vec0, vec1, vec2, vec3;
    compv_scalar_t i, widthSigned = static_cast<compv_scalar_t>(width);

    for (j = 0; j < height; ++j) {
        for (i = 0; i < widthSigned - 31; i += 32) {
            vec0 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i))));
            vec1 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i + 8))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i + 8))));
            vec2 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i + 16))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i + 16))));
            vec3 = _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i + 24))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i + 24))));
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i), vec0);
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i + 8), vec1);
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i + 16), vec2);
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i + 24), vec3);
        }
        for (; i < widthSigned; i += 8) { // can read beyond width as data is strided
            _mm_store_si128(reinterpret_cast<__m128i*>(r + i), _mm_adds_epu16(_mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(a + i))), _mm_abs_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(b + i)))));
        }

        r += stride;
        a += stride;
        b += stride;
    }
}

// Doesn't work with "signed int8"
void CompVMathUtilsSum_8u32u_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* data, compv_uscalar_t count, uint32_t *sum1)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
    COMPV_DEBUG_INFO_CHECK_SSE2();
    __m128i vecSum = _mm_setzero_si128(), vec0;
    const __m128i vecZero = _mm_setzero_si128();
    compv_scalar_t count_ = static_cast<compv_scalar_t>(count), i;
    for (i = 0; i < count_ - 15; i += 16) {
        vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + i));
        // conversion from "uint8" to "uint16" using unpack will lose sign -> doesn't work with "signed int8"
        // SSE4.1 "_mm_cvtepi8_epi16" would work
        // Same remark apply to conversion from epi16 to epi32
        vec0 = _mm_add_epi16(_mm_unpacklo_epi8(vec0, vecZero), _mm_unpackhi_epi8(vec0, vecZero));
        vec0 = _mm_add_epi32(_mm_unpacklo_epi16(vec0, vecZero), _mm_unpackhi_epi16(vec0, vecZero));
        vecSum = _mm_add_epi32(vecSum, vec0);
    }
    if (i < count_ - 7) {
        vec0 = _mm_unpacklo_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(data + i)), vecZero);
        vec0 = _mm_add_epi32(_mm_unpacklo_epi16(vec0, vecZero), _mm_unpackhi_epi16(vec0, vecZero));
        vecSum = _mm_add_epi32(vecSum, vec0);
        i += 8;
    }
    if (i < count_ - 3) {
        vec0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(data + i)), vecZero);
        vecSum = _mm_add_epi32(vecSum, _mm_unpacklo_epi16(vec0, vecZero));
        i += 4;
    }

    vecSum = _mm_hadd_epi32(vecSum, vecSum);
    vecSum = _mm_hadd_epi32(vecSum, vecSum);
    uint32_t sum = (uint32_t)_mm_cvtsi128_si32(vecSum);

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
