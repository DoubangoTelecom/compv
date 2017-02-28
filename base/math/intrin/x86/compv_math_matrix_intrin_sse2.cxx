/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_matrix_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathMatrixMulABt_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, compv_uscalar_t aRows, COMPV_ALIGNED(SSE) compv_uscalar_t aStrideInBytes, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t bRows, compv_uscalar_t bCols, COMPV_ALIGNED(SSE) compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, COMPV_ALIGNED(SSE) compv_uscalar_t rStrideInBytes)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_float64_t* B0;
	compv_scalar_t k, bColsSigned = static_cast<compv_scalar_t>(bCols);

	compv_uscalar_t i, j;
	__m128d vec0, vec1, vec2, vec3, vecSum;
	
	for (i = 0; i < aRows; ++i) {
		B0 = B;
		for (j = 0; j < bRows; ++j) {
			vecSum = _mm_setzero_pd();
			for (k = 0; k < bColsSigned - 7; k += 8) {
				vec0 = _mm_mul_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0]));
				vec1 = _mm_mul_pd(_mm_load_pd(&A[k + 2]), _mm_load_pd(&B0[k + 2]));
				vec2 = _mm_mul_pd(_mm_load_pd(&A[k + 4]), _mm_load_pd(&B0[k + 4]));
				vec3 = _mm_mul_pd(_mm_load_pd(&A[k + 6]), _mm_load_pd(&B0[k + 6]));
				vec0 = _mm_add_pd(vec0, vec1);
				vec2 = _mm_add_pd(vec2, vec3);
				vecSum = _mm_add_pd(vecSum, vec0);
				vecSum = _mm_add_pd(vecSum, vec2);
			}
			if (k < bColsSigned - 3) {
				vec0 = _mm_mul_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0]));
				vec1 = _mm_mul_pd(_mm_load_pd(&A[k + 2]), _mm_load_pd(&B0[k + 2]));
				vec0 = _mm_add_pd(vec0, vec1);
				vecSum = _mm_add_pd(vecSum, vec0);
				k += 4;
			}
			if (k < bColsSigned - 1) {
				vecSum = _mm_add_pd(vecSum, _mm_mul_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0])));
				k += 2;
			}
			if (bColsSigned & 1) {
				vecSum = _mm_add_sd(vecSum, _mm_mul_sd(_mm_load_sd(&A[k + 0]), _mm_load_sd(&B0[k + 0])));
			}

			vecSum = _mm_add_pd(vecSum, _mm_shuffle_pd(vecSum, vecSum, 0x1));
			_mm_store_sd(&R[j], vecSum);

			B0 = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(B0) + bStrideInBytes);
		}
		A = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(A) + aStrideInBytes);
		R = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(R) + rStrideInBytes);
	}
}

// We'll read beyond the end of the data which means ri and rj must be strided and SSE-aligned
void CompVMathMatrixMulGA_64f_Intrin_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	__m128d vecRI0, vecRI1, vecRI2, vecRI3, vecRJ0, vecRJ1, vecRJ2, vecRJ3;
	compv_scalar_t i, countSigned = static_cast<compv_scalar_t>(count);

	const __m128d vecC = _mm_load1_pd(c1); // From Intel intrinsic guide _mm_load1_pd = 'movapd xmm, m128' which is not correct, should be 'shufpd movsd, movsd, 0x0'
	const __m128d vecS = _mm_load1_pd(s1);

	// Case #8
	for (i = 0; i < countSigned - 7; i += 8) {
		vecRI0 = _mm_load_pd(&ri[i + 0]);
		vecRI1 = _mm_load_pd(&ri[i + 2]);
		vecRI2 = _mm_load_pd(&ri[i + 4]);
		vecRI3 = _mm_load_pd(&ri[i + 6]);
		vecRJ0 = _mm_load_pd(&rj[i + 0]);
		vecRJ1 = _mm_load_pd(&rj[i + 2]);
		vecRJ2 = _mm_load_pd(&rj[i + 4]);
		vecRJ3 = _mm_load_pd(&rj[i + 6]);
		_mm_store_pd(&ri[i + 0], _mm_add_pd(_mm_mul_pd(vecRI0, vecC), _mm_mul_pd(vecRJ0, vecS)));
		_mm_store_pd(&ri[i + 2], _mm_add_pd(_mm_mul_pd(vecRI1, vecC), _mm_mul_pd(vecRJ1, vecS)));
		_mm_store_pd(&ri[i + 4], _mm_add_pd(_mm_mul_pd(vecRI2, vecC), _mm_mul_pd(vecRJ2, vecS)));
		_mm_store_pd(&ri[i + 6], _mm_add_pd(_mm_mul_pd(vecRI3, vecC), _mm_mul_pd(vecRJ3, vecS)));
		_mm_store_pd(&rj[i + 0], _mm_sub_pd(_mm_mul_pd(vecRJ0, vecC), _mm_mul_pd(vecRI0, vecS)));
		_mm_store_pd(&rj[i + 2], _mm_sub_pd(_mm_mul_pd(vecRJ1, vecC), _mm_mul_pd(vecRI1, vecS)));
		_mm_store_pd(&rj[i + 4], _mm_sub_pd(_mm_mul_pd(vecRJ2, vecC), _mm_mul_pd(vecRI2, vecS)));
		_mm_store_pd(&rj[i + 6], _mm_sub_pd(_mm_mul_pd(vecRJ3, vecC), _mm_mul_pd(vecRI3, vecS)));
	}

	// Case #4
	if (i < countSigned - 3) {
		vecRI0 = _mm_load_pd(&ri[i + 0]);
		vecRI1 = _mm_load_pd(&ri[i + 2]);
		vecRJ0 = _mm_load_pd(&rj[i + 0]);
		vecRJ1 = _mm_load_pd(&rj[i + 2]);
		_mm_store_pd(&ri[i + 0], _mm_add_pd(_mm_mul_pd(vecRI0, vecC), _mm_mul_pd(vecRJ0, vecS)));
		_mm_store_pd(&ri[i + 2], _mm_add_pd(_mm_mul_pd(vecRI1, vecC), _mm_mul_pd(vecRJ1, vecS)));
		_mm_store_pd(&rj[i + 0], _mm_sub_pd(_mm_mul_pd(vecRJ0, vecC), _mm_mul_pd(vecRI0, vecS)));
		_mm_store_pd(&rj[i + 2], _mm_sub_pd(_mm_mul_pd(vecRJ1, vecC), _mm_mul_pd(vecRI1, vecS)));
		i += 4;
	}

	// Cases #2 and #1
	for (; i < countSigned; i += 2) { // event if only #1 sample remains we can read beyond count (up to stride)
		vecRI0 = _mm_load_pd(&ri[i + 0]);
		vecRJ0 = _mm_load_pd(&rj[i + 0]);
		_mm_store_pd(&ri[i + 0], _mm_add_pd(_mm_mul_pd(vecRI0, vecC), _mm_mul_pd(vecRJ0, vecS)));
		_mm_store_pd(&rj[i + 0], _mm_sub_pd(_mm_mul_pd(vecRJ0, vecC), _mm_mul_pd(vecRI0, vecS)));
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
