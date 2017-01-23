/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_utils_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void MathUtilsSum2_32i32i_Intrin_SSE2(COMPV_ALIGNED(SSE) const int32_t* a, COMPV_ALIGNED(SSE) const int32_t* b, COMPV_ALIGNED(SSE) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): add ASM
    COMPV_DEBUG_INFO_CHECK_SSE2();

    compv_uscalar_t j;
    compv_scalar_t i;
    compv_scalar_t width_ = static_cast<compv_scalar_t>(width);

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width_ - 15; i += 16) {
            _mm_store_si128(reinterpret_cast<__m128i*>(&s[i]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i]))));
            _mm_store_si128(reinterpret_cast<__m128i*>(&s[i + 4]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i + 4])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i + 4]))));
            _mm_store_si128(reinterpret_cast<__m128i*>(&s[i + 8]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i + 8])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i + 8]))));
            _mm_store_si128(reinterpret_cast<__m128i*>(&s[i + 12]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i + 12])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i + 12]))));
        }
        for (; i < width_; i += 4) {
            _mm_store_si128(reinterpret_cast<__m128i*>(&s[i]), _mm_add_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&a[i])), _mm_load_si128(reinterpret_cast<const __m128i*>(&b[i]))));
        }
        a += stride;
        b += stride;
        s += stride;
    }
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
