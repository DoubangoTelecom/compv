/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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

// Training function, no need for ASM implementation
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVLibSVM322KernelRbf1Out_Step1_64f64f_AVX(const double& gamma, const double& x_squarei, const double* xSquarejPtr, const double* dotMatPtr, double* outPtr, const size_t count)
{
	/* AVX instrutions */
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const size_t count16 = count & -16;
	const size_t count4 = count & -4;
	size_t i = 0;
	const __m256d vecGammaMinus = _mm256_set1_pd(-gamma);
	const __m256d vecTwo = _mm256_set1_pd(2.0);
	const __m256d vecXSquarei = _mm256_set1_pd(x_squarei);

	for (; i < count16; i += 16) {
		const __m256d veca0 = _mm256_mul_pd(vecTwo, _mm256_loadu_pd(&dotMatPtr[i]));
		const __m256d veca1 = _mm256_mul_pd(vecTwo, _mm256_loadu_pd(&dotMatPtr[i + 4]));
		const __m256d veca2 = _mm256_mul_pd(vecTwo, _mm256_loadu_pd(&dotMatPtr[i + 8]));
		const __m256d veca3 = _mm256_mul_pd(vecTwo, _mm256_loadu_pd(&dotMatPtr[i + 12]));
		__m256d vecb0 = _mm256_add_pd(vecXSquarei, _mm256_loadu_pd(&xSquarejPtr[i]));
		__m256d vecb1 = _mm256_add_pd(vecXSquarei, _mm256_loadu_pd(&xSquarejPtr[i + 4]));
		__m256d vecb2 = _mm256_add_pd(vecXSquarei, _mm256_loadu_pd(&xSquarejPtr[i + 8]));
		__m256d vecb3 = _mm256_add_pd(vecXSquarei, _mm256_loadu_pd(&xSquarejPtr[i + 12]));
		vecb0 = _mm256_sub_pd(vecb0, veca0);
		vecb1 = _mm256_sub_pd(vecb1, veca1);
		vecb2 = _mm256_sub_pd(vecb2, veca2);
		vecb3 = _mm256_sub_pd(vecb3, veca3);
		vecb0 = _mm256_mul_pd(vecb0, vecGammaMinus);
		vecb1 = _mm256_mul_pd(vecb1, vecGammaMinus);
		vecb2 = _mm256_mul_pd(vecb2, vecGammaMinus);
		vecb3 = _mm256_mul_pd(vecb3, vecGammaMinus);
		_mm256_storeu_pd(&outPtr[i], vecb0);
		_mm256_storeu_pd(&outPtr[i + 4], vecb1);
		_mm256_storeu_pd(&outPtr[i + 8], vecb2);
		_mm256_storeu_pd(&outPtr[i + 12], vecb3);
	}
	for (; i < count4; i += 4) {
		const __m256d veca0 = _mm256_mul_pd(vecTwo, _mm256_loadu_pd(&dotMatPtr[i]));
		__m256d vecb0 = _mm256_add_pd(vecXSquarei, _mm256_loadu_pd(&xSquarejPtr[i]));
		vecb0 = _mm256_sub_pd(vecb0, veca0);
		vecb0 = _mm256_mul_pd(vecb0, vecGammaMinus);
		_mm256_storeu_pd(&outPtr[i], vecb0);
	}
	_mm256_zeroupper();

	/* SSE instrutions */
	for (; i < count; i += 1) {
		const __m128d veca0 = _mm_mul_sd(_mm256_castpd256_pd128(vecTwo), _mm_load_sd(&dotMatPtr[i])); // "_mm256_castpd256_pd128" is a nop -> no AVX/SSE transition issues
		__m128d vecb0 = _mm_add_sd(_mm256_castpd256_pd128(vecXSquarei), _mm_load_sd(&xSquarejPtr[i])); // "_mm256_castpd256_pd128" is a nop -> no AVX/SSE transition issues
		vecb0 = _mm_sub_sd(vecb0, veca0);
		vecb0 = _mm_mul_sd(vecb0, _mm256_castpd256_pd128(vecGammaMinus)); // "_mm256_castpd256_pd128" is a nop -> no AVX/SSE transition issues
		_mm_store_sd(&outPtr[i], vecb0);
	}
}

// Training function, no need for ASM implementation
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVLibSVM322KernelRbf1Out_Step2_64f32f_AVX(const double& yi, const double* yjPtr, const double* outStep1Ptr, float* outPtr, const size_t count)
{
	/* AVX instrutions mixed with SSE ones (transition issue if this file not built with "/AVX" flags) */
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const size_t count16 = count & -16;
	const size_t count4 = count & -4;
	const size_t count2 = count & -2;
	size_t i = 0;
	const __m256d vecYi = _mm256_set1_pd(yi);
	for (; i < count16; i += 16) {
		__m256d vec0 = _mm256_mul_pd(vecYi, _mm256_loadu_pd(&yjPtr[i]));
		__m256d vec1 = _mm256_mul_pd(vecYi, _mm256_loadu_pd(&yjPtr[i + 4]));
		__m256d vec2 = _mm256_mul_pd(vecYi, _mm256_loadu_pd(&yjPtr[i + 8]));
		__m256d vec3 = _mm256_mul_pd(vecYi, _mm256_loadu_pd(&yjPtr[i + 12]));
		vec0 = _mm256_mul_pd(vec0, _mm256_loadu_pd(&outStep1Ptr[i]));
		vec1 = _mm256_mul_pd(vec1, _mm256_loadu_pd(&outStep1Ptr[i + 4]));
		vec2 = _mm256_mul_pd(vec2, _mm256_loadu_pd(&outStep1Ptr[i + 8]));
		vec3 = _mm256_mul_pd(vec3, _mm256_loadu_pd(&outStep1Ptr[i + 12]));
		_mm_storeu_ps(&outPtr[i], _mm256_cvtpd_ps(vec0));
		_mm_storeu_ps(&outPtr[i + 4], _mm256_cvtpd_ps(vec1));
		_mm_storeu_ps(&outPtr[i + 8], _mm256_cvtpd_ps(vec2));
		_mm_storeu_ps(&outPtr[i + 12], _mm256_cvtpd_ps(vec3));
	}
	for (; i < count4; i += 4) {
		__m256d vec0 = _mm256_mul_pd(vecYi, _mm256_loadu_pd(&yjPtr[i]));
		vec0 = _mm256_mul_pd(vec0, _mm256_loadu_pd(&outStep1Ptr[i]));
		_mm_storeu_ps(&outPtr[i], _mm256_cvtpd_ps(vec0));
	}
	_mm256_zeroupper();

	/* SSE instrutions */
	for (; i < count2; i += 2) {
		__m128d vec0 = _mm_mul_pd(_mm256_castpd256_pd128(vecYi), _mm_loadu_pd(&yjPtr[i])); // "_mm256_castpd256_pd128" is a nop -> no AVX/SSE transition issues
		vec0 = _mm_mul_pd(vec0, _mm_loadu_pd(&outStep1Ptr[i]));
		_mm_storel_pd(reinterpret_cast<double*>(&outPtr[i]), _mm_castps_pd(_mm_cvtpd_ps(vec0))); // no "_mm_storel_ps"
	}

	/* C++ instructions */
	for (; i < count; i += 1) {
		outPtr[i] = static_cast<float>(yi * yjPtr[i] * outStep1Ptr[i]);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
