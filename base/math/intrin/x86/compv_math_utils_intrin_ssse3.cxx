/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
