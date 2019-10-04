/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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
#	pragma intel optimization_parameter target_arch=avx
#endif
void COMPV_DEPRECATED(CompVMathMatrixMulABt_64f_Intrin_AVX)(const COMPV_ALIGNED(AVX) compv_float64_t* A, compv_uscalar_t aRows, COMPV_ALIGNED(AVX) compv_uscalar_t aStrideInBytes, const COMPV_ALIGNED(AVX) compv_float64_t* B, compv_uscalar_t bRows, compv_uscalar_t bCols, COMPV_ALIGNED(AVX) compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv_float64_t* R, COMPV_ALIGNED(AVX) compv_uscalar_t rStrideInBytes)
{
	COMPV_DEBUG_INFO_CHECK_AVX(); // AVX/SSE transition issues
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Not optimized at all and deprecated");
	_mm256_zeroupper();
	const compv_float64_t* B0;
	compv_scalar_t k, bColsSigned = static_cast<compv_scalar_t>(bCols);

	compv_uscalar_t i, j;
	__m256d vec0, vec1, vec2, vec3, vecSum;
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
			if (k < bColsSigned - 7) {
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

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathMatrixMulGA_64f_Intrin_AVX(COMPV_ALIGNED(AVX) compv_float64_t* ri, COMPV_ALIGNED(AVX) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_AVX(); // AVX/SSE transition issues
	_mm256_zeroupper();

	__m256d vecRI0, vecRI1, vecRI2, vecRI3, vecRJ0, vecRJ1, vecRJ2, vecRJ3;
	compv_scalar_t i, countSigned = static_cast<compv_scalar_t>(count);

	const __m256d vecC = _mm256_broadcast_sd(c1); // From Intel intrinsic guide _mm_load1_pd = 'movapd xmm, m128' which is not correct, should be 'shufpd movsd, movsd, 0x0'
	const __m256d vecS = _mm256_broadcast_sd(s1);

	// Case #16
	for (i = 0; i < countSigned - 15; i += 16) {
		vecRI0 = _mm256_load_pd(&ri[i + 0]);
		vecRI1 = _mm256_load_pd(&ri[i + 4]);
		vecRI2 = _mm256_load_pd(&ri[i + 8]);
		vecRI3 = _mm256_load_pd(&ri[i + 12]);
		vecRJ0 = _mm256_load_pd(&rj[i + 0]);
		vecRJ1 = _mm256_load_pd(&rj[i + 4]);
		vecRJ2 = _mm256_load_pd(&rj[i + 8]);
		vecRJ3 = _mm256_load_pd(&rj[i + 12]);
		_mm256_store_pd(&ri[i + 0], _mm256_add_pd(_mm256_mul_pd(vecRI0, vecC), _mm256_mul_pd(vecRJ0, vecS)));
		_mm256_store_pd(&ri[i + 4], _mm256_add_pd(_mm256_mul_pd(vecRI1, vecC), _mm256_mul_pd(vecRJ1, vecS)));
		_mm256_store_pd(&ri[i + 8], _mm256_add_pd(_mm256_mul_pd(vecRI2, vecC), _mm256_mul_pd(vecRJ2, vecS)));
		_mm256_store_pd(&ri[i + 12], _mm256_add_pd(_mm256_mul_pd(vecRI3, vecC), _mm256_mul_pd(vecRJ3, vecS)));
		_mm256_store_pd(&rj[i + 0], _mm256_sub_pd(_mm256_mul_pd(vecRJ0, vecC), _mm256_mul_pd(vecRI0, vecS)));
		_mm256_store_pd(&rj[i + 4], _mm256_sub_pd(_mm256_mul_pd(vecRJ1, vecC), _mm256_mul_pd(vecRI1, vecS)));
		_mm256_store_pd(&rj[i + 8], _mm256_sub_pd(_mm256_mul_pd(vecRJ2, vecC), _mm256_mul_pd(vecRI2, vecS)));
		_mm256_store_pd(&rj[i + 12], _mm256_sub_pd(_mm256_mul_pd(vecRJ3, vecC), _mm256_mul_pd(vecRI3, vecS)));
	}

	// Case #8
	if (i < countSigned - 7) { // countSigned is equal to #9 for homography
		vecRI0 = _mm256_load_pd(&ri[i + 0]);
		vecRI1 = _mm256_load_pd(&ri[i + 4]);
		vecRJ0 = _mm256_load_pd(&rj[i + 0]);
		vecRJ1 = _mm256_load_pd(&rj[i + 4]);
		_mm256_store_pd(&ri[i + 0], _mm256_add_pd(_mm256_mul_pd(vecRI0, vecC), _mm256_mul_pd(vecRJ0, vecS)));
		_mm256_store_pd(&ri[i + 4], _mm256_add_pd(_mm256_mul_pd(vecRI1, vecC), _mm256_mul_pd(vecRJ1, vecS)));
		_mm256_store_pd(&rj[i + 0], _mm256_sub_pd(_mm256_mul_pd(vecRJ0, vecC), _mm256_mul_pd(vecRI0, vecS)));
		_mm256_store_pd(&rj[i + 4], _mm256_sub_pd(_mm256_mul_pd(vecRJ1, vecC), _mm256_mul_pd(vecRI1, vecS)));
		i += 8;
	}

	// All other cases (7, 6... 1)
	for (; i < countSigned; i += 4) { // event if only #1 sample remains we can read beyond count (up to stride)
		vecRI0 = _mm256_load_pd(&ri[i + 0]);
		vecRJ0 = _mm256_load_pd(&rj[i + 0]);
		_mm256_store_pd(&ri[i + 0], _mm256_add_pd(_mm256_mul_pd(vecRI0, vecC), _mm256_mul_pd(vecRJ0, vecS)));
		_mm256_store_pd(&rj[i + 0], _mm256_sub_pd(_mm256_mul_pd(vecRJ0, vecC), _mm256_mul_pd(vecRI0, vecS)));
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
