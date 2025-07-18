/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_fma3_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_bits.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathDistanceSquaredL2Row_32f_Intrin_FMA3_AVX(COMPV_ALIGNED(AVX) const float* dataset, COMPV_ALIGNED(AVX) const float* vectors, float* result1, const compv_uscalar_t& cols)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	COMPV_DEBUG_INFO_CHECK_FMA3();

	_mm256_zeroupper(); // Must to avoid AVX/SSE transition issues

	const size_t cols32 = cols & -32;
	const size_t cols8 = cols & -8;
	size_t col = 0;

	__m256 sum0 = _mm256_setzero_ps();
	__m256 sum1 = _mm256_setzero_ps();
	__m256 sum2 = _mm256_setzero_ps();
	__m256 sum3 = _mm256_setzero_ps();

	// loop-32
	for (; col < cols32; col += 32) {
		__m256 vec0 = _mm256_sub_ps(_mm256_load_ps(&dataset[col]), _mm256_load_ps(&vectors[col]));
		__m256 vec1 = _mm256_sub_ps(_mm256_load_ps(&dataset[col + 8]), _mm256_load_ps(&vectors[col + 8]));
		__m256 vec2 = _mm256_sub_ps(_mm256_load_ps(&dataset[col + 16]), _mm256_load_ps(&vectors[col + 16]));
		__m256 vec3 = _mm256_sub_ps(_mm256_load_ps(&dataset[col + 24]), _mm256_load_ps(&vectors[col + 24]));

		sum0 = _mm256_fmadd_ps(vec0, vec0, sum0);
		sum1 = _mm256_fmadd_ps(vec1, vec1, sum1);
		sum2 = _mm256_fmadd_ps(vec2, vec2, sum2);
		sum3 = _mm256_fmadd_ps(vec3, vec3, sum3);
	}

	// loop-8
	for (; col < cols8; col += 8) {
		__m256 vec0 = _mm256_sub_ps(_mm256_load_ps(&dataset[col]), _mm256_load_ps(&vectors[col]));
		sum0 = _mm256_fmadd_ps(vec0, vec0, sum0);
	}

	// loop-1
	__m128 sum0n = _mm_setzero_ps();
	for (; col < cols; col += 1) {
		__m128 vec0 = _mm_sub_ss(_mm_load_ss(&dataset[col]), _mm_load_ss(&vectors[col]));
		sum0n = _mm_fmadd_ss(vec0, vec0, sum0n);
	}

	sum0 = _mm256_add_ps(sum0, sum1);
	sum2 = _mm256_add_ps(sum2, sum3);
	sum0 = _mm256_add_ps(sum0, sum2);

	sum0n = _mm_add_ps(
		sum0n,
		_mm_add_ps(_mm256_castps256_ps128(sum0), _mm256_extractf128_ps(sum0, 0x1))
	);
	sum0n = _mm_add_ps(sum0n, _mm_shuffle_ps(sum0n, sum0n, 0xEE));
	sum0n = _mm_add_ss(sum0n, _mm_shuffle_ps(sum0n, sum0n, 0x55));

	_mm_store_ss(result1, sum0n);

	_mm256_zeroupper(); // To avoid AVX/SSE transition issues
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
