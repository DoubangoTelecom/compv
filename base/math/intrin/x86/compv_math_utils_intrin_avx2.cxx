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
#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MathUtilsSumAbs_16i16u_Intrin_AVX2(const COMPV_ALIGNED(AVX) int16_t* a, const COMPV_ALIGNED(AVX) int16_t* b, COMPV_ALIGNED(AVX) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
    COMPV_DEBUG_INFO_CHECK_AVX();

    _mm256_zeroupper();
    compv_uscalar_t j;
    __m256i ymm0, ymm1, ymm2, ymm3;
    compv_scalar_t i, width_ = static_cast<compv_scalar_t>(width);

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width_ - 63; i += 64) {
            ymm0 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i))));
            ymm1 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i + 16))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i + 16))));
            ymm2 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i + 32))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i + 32))));
            ymm3 = _mm256_adds_epu16(_mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(a + i + 48))), _mm256_abs_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(b + i + 48))));
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i), ymm0);
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i + 16), ymm1);
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i + 32), ymm2);
            _mm256_store_si256(reinterpret_cast<__m256i*>(r + i + 48), ymm3);
        }
        for (; i < width_; i += 16) {
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
