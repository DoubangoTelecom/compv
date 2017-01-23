/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_matrix_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
// A and B must have same rows, cols and alignment
void MatrixIsEqual_64f_Intrin_AVX2(const COMPV_ALIGNED(AVX) compv_float64_t* A, const COMPV_ALIGNED(AVX) compv_float64_t* B, compv_uscalar_t rows, compv_uscalar_t cols, compv_uscalar_t strideInBytes, compv_scalar_t *equal)
{
    COMPV_DEBUG_INFO_CHECK_AVX();
    COMPV_DEBUG_INFO_CHECK_AVX2();

    _mm256_zeroupper();
    // TODO(dmi): add ASM (not urgent, function used rarely)
    compv_uscalar_t i, j;
    *equal = 0;

    // _mm_cmpeq_epi8: Latency = 1, Throughput = 0.5
    // _mm_cmpeq_pd: Latency = 3, Throughput = 0.5
    // -> use binary comparison which is faster

    const uint8_t* a = reinterpret_cast<const uint8_t*>(A);
    const uint8_t* b = reinterpret_cast<const uint8_t*>(B);
    __m256i ymmMaskToExtractFirst64Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_u64);

    cols <<= 3; // float64 to bytes

    for (j = 0; j < rows; ++j) {
        i = 0;
        for (; i < cols - 31; i += 32) {
            if (0xffffffff != _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_load_si256(reinterpret_cast<const __m256i*>(&a[i])), _mm256_load_si256(reinterpret_cast<const __m256i*>(&b[i]))))) {
                return;
            }
        }
        for (; i < cols - 7; i += 8) {
            if (0xffffffff != _mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&a[i]), ymmMaskToExtractFirst64Bits), _mm256_maskload_epi64(reinterpret_cast<const int64_t*>(&b[i]), ymmMaskToExtractFirst64Bits)))) {
                return;
            }
        }
        a += strideInBytes;
        b += strideInBytes;
    }

    *equal = 1;
    _mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
