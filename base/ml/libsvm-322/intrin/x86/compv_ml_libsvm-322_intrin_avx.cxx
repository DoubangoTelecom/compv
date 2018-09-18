/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/ml/libsvm-322/intrin/x86/compv_ml_libsvm-322_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Training function, no need for ASM implementation
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVLibSVM322KernelRbf0Out_64f64f_AVX(const double& gamma, const double* xSquarePtr, const double* dotMatPtr, double* outPtr, const size_t count)
{
	/* AVX instrutions */
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const size_t count16 = count & -16;
	const size_t count4 = count & -4;
	size_t i = 0;
	const __m256d vecGammaTimes2Minus = _mm256_set1_pd(-(2.0 * gamma));

	for (; i < count16; i += 16) {
		__m256d vec0 = _mm256_sub_pd(_mm256_loadu_pd(&xSquarePtr[i]), _mm256_loadu_pd(&dotMatPtr[i]));
		__m256d vec1 = _mm256_sub_pd(_mm256_loadu_pd(&xSquarePtr[i + 4]), _mm256_loadu_pd(&dotMatPtr[i + 4]));
		__m256d vec2 = _mm256_sub_pd(_mm256_loadu_pd(&xSquarePtr[i + 8]), _mm256_loadu_pd(&dotMatPtr[i + 8]));
		__m256d vec3 = _mm256_sub_pd(_mm256_loadu_pd(&xSquarePtr[i + 12]), _mm256_loadu_pd(&dotMatPtr[i + 12]));
		vec0 = _mm256_mul_pd(vec0, vecGammaTimes2Minus);
		vec1 = _mm256_mul_pd(vec1, vecGammaTimes2Minus);
		vec2 = _mm256_mul_pd(vec2, vecGammaTimes2Minus);
		vec3 = _mm256_mul_pd(vec3, vecGammaTimes2Minus);
		_mm256_storeu_pd(&outPtr[i], vec0);
		_mm256_storeu_pd(&outPtr[i + 4], vec1);
		_mm256_storeu_pd(&outPtr[i + 8], vec2);
		_mm256_storeu_pd(&outPtr[i + 12], vec3);
	}
	for (; i < count4; i += 4) {
		__m256d vec0 = _mm256_sub_pd(_mm256_loadu_pd(&xSquarePtr[i]), _mm256_loadu_pd(&dotMatPtr[i]));
		vec0 = _mm256_mul_pd(vec0, vecGammaTimes2Minus);
		_mm256_storeu_pd(&outPtr[i], vec0);
	}
	_mm256_zeroupper();

	/* SSE instrutions */
	for (; i < count; i += 1) {
		__m128d vec0 = _mm_sub_sd(_mm_load_sd(&xSquarePtr[i]), _mm_load_sd(&dotMatPtr[i]));
		vec0 = _mm_mul_sd(vec0, _mm256_castpd256_pd128(vecGammaTimes2Minus)); // "_mm256_castpd256_pd128" is a nop -> no AVX/SSE transition issues
		_mm_store_sd(&outPtr[i], vec0);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
