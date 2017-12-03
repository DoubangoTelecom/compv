/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_utils_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "strideInBytes" must be AVX-aligned
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathUtilsSumAbs_16s16u_Intrin_AVX2(const COMPV_ALIGNED(AVX) int16_t* a, const COMPV_ALIGNED(AVX) int16_t* b, COMPV_ALIGNED(AVX) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
    _mm256_zeroupper();
    compv_uscalar_t j;
    __m256i vec0, vec1, vec2, vec3;
    compv_scalar_t i, widthSigned = static_cast<compv_scalar_t>(width);

    for (j = 0; j < height; ++j) {
        for (i = 0; i < widthSigned - 63; i += 64) {
            vec0 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i))));
            vec1 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i + 16))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i + 16))));
            vec2 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i + 32))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i + 32))));
            vec3 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i + 48))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i + 48))));
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i), vec0);
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i + 16), vec1);
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i + 32), vec2);
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i + 48), vec3);
        }
        for (; i < widthSigned; i += 16) {
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i), _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i)))));
        }
        r += stride;
        a += stride;
        b += stride;
    }
    _mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
