/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/ml/libsvm-322/intrin/x86/compv_ml_libsvm-322_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Training function, no need for ASM implementation
void CompVLibSVM322KernelRbf0Out_64f64f_SSE2(const double& gamma, const double* xSquarePtr, const double* dotMatPtr, double* outPtr, const size_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const size_t count8 = count & -8;
	const size_t count2 = count & -2;
	size_t i = 0;
	const __m128d vecGammaTimes2Minus = _mm_set1_pd(-(2.0 * gamma));

	for (; i < count8; i += 8) {		
		__m128d vec0 = _mm_sub_pd(_mm_loadu_pd(&xSquarePtr[i]), _mm_loadu_pd(&dotMatPtr[i]));
		__m128d vec1 = _mm_sub_pd(_mm_loadu_pd(&xSquarePtr[i + 2]), _mm_loadu_pd(&dotMatPtr[i + 2]));
		__m128d vec2 = _mm_sub_pd(_mm_loadu_pd(&xSquarePtr[i + 4]), _mm_loadu_pd(&dotMatPtr[i + 4]));
		__m128d vec3 = _mm_sub_pd(_mm_loadu_pd(&xSquarePtr[i + 6]), _mm_loadu_pd(&dotMatPtr[i + 6]));
		vec0 = _mm_mul_pd(vec0, vecGammaTimes2Minus);
		vec1 = _mm_mul_pd(vec1, vecGammaTimes2Minus);
		vec2 = _mm_mul_pd(vec2, vecGammaTimes2Minus);
		vec3 = _mm_mul_pd(vec3, vecGammaTimes2Minus);
		_mm_storeu_pd(&outPtr[i], vec0);
		_mm_storeu_pd(&outPtr[i + 2], vec1);
		_mm_storeu_pd(&outPtr[i + 4], vec2);
		_mm_storeu_pd(&outPtr[i + 6], vec3);
	}
	for (; i < count2; i += 2) {
		__m128d vec0 = _mm_sub_pd(_mm_loadu_pd(&xSquarePtr[i]), _mm_loadu_pd(&dotMatPtr[i]));
		vec0 = _mm_mul_pd(vec0, vecGammaTimes2Minus);
		_mm_storeu_pd(&outPtr[i], vec0);
	}
	for (; i < count; i += 1) {
		__m128d vec0 = _mm_sub_sd(_mm_load_sd(&xSquarePtr[i]), _mm_load_sd(&dotMatPtr[i]));
		vec0 = _mm_mul_sd(vec0, vecGammaTimes2Minus);
		_mm_store_sd(&outPtr[i], vec0);
	}
}

void CompVLibSVM322KernelRbf1Out_Step1_64f64f_SSE2(const double& gamma, const double& x_squarei, const double* xSquarejPtr, const double* dotMatPtr, double* outPtr, const size_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const size_t count8 = count & -8;
	const size_t count2 = count & -2;
	size_t i = 0;
	const __m128d vecGammaMinus = _mm_set1_pd(-gamma);
	const __m128d vecTwo = _mm_set1_pd(2.0);
	const __m128d vecXSquarei = _mm_set1_pd(x_squarei);

	for (; i < count8; i += 8) {
		const __m128d veca0 = _mm_mul_pd(vecTwo, _mm_loadu_pd(&dotMatPtr[i]));
		const __m128d veca1 = _mm_mul_pd(vecTwo, _mm_loadu_pd(&dotMatPtr[i + 2]));
		const __m128d veca2 = _mm_mul_pd(vecTwo, _mm_loadu_pd(&dotMatPtr[i + 4]));
		const __m128d veca3 = _mm_mul_pd(vecTwo, _mm_loadu_pd(&dotMatPtr[i + 6]));
		__m128d vecb0 = _mm_add_pd(vecXSquarei, _mm_loadu_pd(&xSquarejPtr[i]));
		__m128d vecb1 = _mm_add_pd(vecXSquarei, _mm_loadu_pd(&xSquarejPtr[i + 2]));
		__m128d vecb2 = _mm_add_pd(vecXSquarei, _mm_loadu_pd(&xSquarejPtr[i + 4]));
		__m128d vecb3 = _mm_add_pd(vecXSquarei, _mm_loadu_pd(&xSquarejPtr[i + 6]));
		vecb0 = _mm_sub_pd(vecb0, veca0);
		vecb1 = _mm_sub_pd(vecb1, veca1);
		vecb2 = _mm_sub_pd(vecb2, veca2);
		vecb3 = _mm_sub_pd(vecb3, veca3);
		vecb0 = _mm_mul_pd(vecb0, vecGammaMinus);
		vecb1 = _mm_mul_pd(vecb1, vecGammaMinus);
		vecb2 = _mm_mul_pd(vecb2, vecGammaMinus);
		vecb3 = _mm_mul_pd(vecb3, vecGammaMinus);
		_mm_storeu_pd(&outPtr[i], vecb0);
		_mm_storeu_pd(&outPtr[i + 2], vecb1);
		_mm_storeu_pd(&outPtr[i + 4], vecb2);
		_mm_storeu_pd(&outPtr[i + 6], vecb3);
	}
	for (; i < count2; i += 2) {
		const __m128d veca0 = _mm_mul_pd(vecTwo, _mm_loadu_pd(&dotMatPtr[i]));
		__m128d vecb0 = _mm_add_pd(vecXSquarei, _mm_loadu_pd(&xSquarejPtr[i]));
		vecb0 = _mm_sub_pd(vecb0, veca0);
		vecb0 = _mm_mul_pd(vecb0, vecGammaMinus);
		_mm_storeu_pd(&outPtr[i], vecb0);
	}
	for (; i < count; i += 1) {
		const __m128d veca0 = _mm_mul_sd(vecTwo, _mm_load_sd(&dotMatPtr[i]));
		__m128d vecb0 = _mm_add_sd(vecXSquarei, _mm_load_sd(&xSquarejPtr[i]));
		vecb0 = _mm_sub_sd(vecb0, veca0);
		vecb0 = _mm_mul_sd(vecb0, vecGammaMinus);
		_mm_store_sd(&outPtr[i], vecb0);
	}
}

void CompVLibSVM322KernelRbf1Out_Step2_64f32f_SSE2(const double& yi, const double* yjPtr, const double* outStep1Ptr, float* outPtr, const size_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const size_t count8 = count & -8;
	const size_t count2 = count & -2;
	size_t i = 0;
	const __m128d vecYi = _mm_set1_pd(yi);
	for (; i < count8; i += 8) {
		__m128d vec0 = _mm_mul_pd(vecYi, _mm_loadu_pd(&yjPtr[i]));
		__m128d vec1 = _mm_mul_pd(vecYi, _mm_loadu_pd(&yjPtr[i + 2]));
		__m128d vec2 = _mm_mul_pd(vecYi, _mm_loadu_pd(&yjPtr[i + 4]));
		__m128d vec3 = _mm_mul_pd(vecYi, _mm_loadu_pd(&yjPtr[i + 6]));
		vec0 = _mm_mul_pd(vec0, _mm_loadu_pd(&outStep1Ptr[i]));
		vec1 = _mm_mul_pd(vec1, _mm_loadu_pd(&outStep1Ptr[i + 2]));
		vec2 = _mm_mul_pd(vec2, _mm_loadu_pd(&outStep1Ptr[i + 4]));
		vec3 = _mm_mul_pd(vec3, _mm_loadu_pd(&outStep1Ptr[i + 6]));
		_mm_storeu_ps(&outPtr[i], _mm_shuffle_ps(_mm_cvtpd_ps(vec0), _mm_cvtpd_ps(vec1), 0x44));
		_mm_storeu_ps(&outPtr[i + 4], _mm_shuffle_ps(_mm_cvtpd_ps(vec2), _mm_cvtpd_ps(vec3), 0x44));
	}
	for (; i < count2; i += 2) {
		__m128d vec0 = _mm_mul_pd(vecYi, _mm_loadu_pd(&yjPtr[i]));
		vec0 = _mm_mul_pd(vec0, _mm_loadu_pd(&outStep1Ptr[i]));
		_mm_storel_pd(reinterpret_cast<double*>(&outPtr[i]), _mm_castps_pd(_mm_cvtpd_ps(vec0))); // no "_mm_storel_ps"
	}
	for (; i < count; i += 1) {
		outPtr[i] = static_cast<float>(yi * yjPtr[i] * outStep1Ptr[i]);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
