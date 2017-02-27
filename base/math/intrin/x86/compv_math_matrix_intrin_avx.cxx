/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_matrix_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Useful only if bCols >= 8, otherwise use SSE version
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVMathMatrixMulABt_64f_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* A, compv_uscalar_t aRows, COMPV_ALIGNED(AVX) compv_uscalar_t aStrideInBytes, const COMPV_ALIGNED(AVX) compv_float64_t* B, compv_uscalar_t bRows, compv_uscalar_t bCols, COMPV_ALIGNED(AVX) compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv_float64_t* R, COMPV_ALIGNED(AVX) compv_uscalar_t rStrideInBytes)
{
	COMPV_DEBUG_INFO_CHECK_AVX(); // AVX/SSE transition issues
	_mm256_zeroupper();
	const compv_float64_t* B0;
	compv_scalar_t k, bColsSigned = static_cast<compv_scalar_t>(bCols);

	compv_uscalar_t i, j;
	__m256d vec0, vec1, vec2, vec3, vecSum;
	const __m256d vecZero = _mm256_setzero_pd();
	__m128d vec0n;
	
	for (i = 0; i < aRows; ++i) {
		B0 = B;
		for (j = 0; j < bRows; ++j) {
			vecSum = _mm256_setzero_pd();
			for (k = 0; k < bColsSigned - 15; k += 16) {
				vec0 = _mm256_mul_pd(_mm256_load_pd(&A[k + 0]), _mm256_load_pd(&B0[k + 0]));
				vec1 = _mm256_mul_pd(_mm256_load_pd(&A[k + 4]), _mm256_load_pd(&B0[k + 4]));
				vec2 = _mm256_mul_pd(_mm256_load_pd(&A[k + 8]), _mm256_load_pd(&B0[k + 8]));
				vec3 = _mm256_mul_pd(_mm256_load_pd(&A[k + 12]), _mm256_load_pd(&B0[k + 12]));
				vec0 = _mm256_add_pd(vec0, vec1);
				vec2 = _mm256_add_pd(vec2, vec3);
				vecSum = _mm256_add_pd(vecSum, vec0);
				vecSum = _mm256_add_pd(vecSum, vec2);
			}
			if (k < bColsSigned - 8) {
				vec0 = _mm256_mul_pd(_mm256_load_pd(&A[k + 0]), _mm256_load_pd(&B0[k + 0]));
				vec1 = _mm256_mul_pd(_mm256_load_pd(&A[k + 4]), _mm256_load_pd(&B0[k + 4]));
				vec0 = _mm256_add_pd(vec0, vec1);
				vecSum = _mm256_add_pd(vecSum, vec0);
				k += 8;
			}
			if (k < bColsSigned - 3) {
				vecSum = _mm256_add_pd(vecSum, _mm256_mul_pd(_mm256_load_pd(&A[k + 0]), _mm256_load_pd(&B0[k + 0])));
				k += 4;
			}
			if (k < bColsSigned - 1) {
				vec0n = _mm_add_pd(_mm256_castpd256_pd128(vecSum), _mm_mul_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0])));
				vecSum = _mm256_insertf128_pd(vecSum, vec0n, 0x0);
				k += 2;
			}
			if (bColsSigned & 1) {
				vec0n = _mm_add_sd(_mm256_castpd256_pd128(vecSum), _mm_mul_sd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0])));
				vecSum = _mm256_insertf128_pd(vecSum, vec0n, 0x0);
			}

			vecSum = _mm256_add_pd(vecSum, _mm256_permute2f128_pd(vecSum, vecSum, 0x11));
			vec0n = _mm256_castpd256_pd128(vecSum);
			vec0n = _mm_add_pd(vec0n, _mm_shuffle_pd(vec0n, vec0n, 0x1));
			_mm_store_sd(&R[j], vec0n);

			B0 = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(B0) + bStrideInBytes);
		}
		A = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(A) + aStrideInBytes);
		R = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(R) + rStrideInBytes);
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
