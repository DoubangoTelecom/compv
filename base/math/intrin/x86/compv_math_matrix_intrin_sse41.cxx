/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_matrix_intrin_sse41.h"
#include "compv/base/compv_debug.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void COMPV_DEPRECATED(CompVMathMatrixMulABt_64f_Intrin_SSE41)(const COMPV_ALIGNED(SSE) compv_float64_t* A, compv_uscalar_t aRows, COMPV_ALIGNED(SSE) compv_uscalar_t aStrideInBytes, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t bRows, compv_uscalar_t bCols, COMPV_ALIGNED(SSE) compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, COMPV_ALIGNED(SSE) compv_uscalar_t rStrideInBytes)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Not optimized at all and deprecated");
	const compv_float64_t* B0;
	compv_scalar_t k, bColsSigned = static_cast<compv_scalar_t>(bCols);

	compv_uscalar_t i, j;
	__m128d vec0, vec1, vec2, vec3, vecSum;

	for (i = 0; i < aRows; ++i) {
		B0 = B;
		for (j = 0; j < bRows; ++j) {
			vecSum = _mm_setzero_pd();
			for (k = 0; k < bColsSigned - 7; k += 8) {
				vec0 = _mm_dp_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0]), 0x31);
				vec1 = _mm_dp_pd(_mm_load_pd(&A[k + 2]), _mm_load_pd(&B0[k + 2]), 0x31);
				vec2 = _mm_dp_pd(_mm_load_pd(&A[k + 4]), _mm_load_pd(&B0[k + 4]), 0x31);
				vec3 = _mm_dp_pd(_mm_load_pd(&A[k + 6]), _mm_load_pd(&B0[k + 6]), 0x31);
				vec0 = _mm_add_pd(_mm_shuffle_pd(vec0, vec1, 0x0), _mm_shuffle_pd(vec2, vec3, 0x0));
				vecSum = _mm_add_pd(vecSum, vec0);
			}
			if (k < bColsSigned - 3) {
				vec0 = _mm_dp_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0]), 0x31);
				vec1 = _mm_dp_pd(_mm_load_pd(&A[k + 2]), _mm_load_pd(&B0[k + 2]), 0x31);
				vec0 = _mm_shuffle_pd(vec0, vec1, 0x0);
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

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

