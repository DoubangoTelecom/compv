/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_utils_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "strideInBytes" must be AVX-aligned
#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MathUtilsAddAbs_16i16u_Intrin_AVX2(const COMPV_ALIGNED(AVX) int16_t* a, const COMPV_ALIGNED(AVX) int16_t* b, COMPV_ALIGNED(AVX) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();
	compv_uscalar_t i, j;

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
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
