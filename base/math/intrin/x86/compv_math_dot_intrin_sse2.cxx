/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_dot_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Must not require memory alignment (random access from SVM)
void CompVMathDotDot_64f64f_Intrin_SSE2(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	
	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width2 = width & -2;
	compv_uscalar_t i;
	__m128d vecSum0 = _mm_setzero_pd();
	__m128d vecSum1 = _mm_setzero_pd();

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			// TODO(dmi): Add FMA implementation
			__m128d vec0 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i]), _mm_loadu_pd(&ptrB[i]));
			__m128d vec1 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 2]), _mm_loadu_pd(&ptrB[i + 2]));
			__m128d vec2 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 4]), _mm_loadu_pd(&ptrB[i + 4]));
			__m128d vec3 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 6]), _mm_loadu_pd(&ptrB[i + 6]));
			__m128d vec4 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 8]), _mm_loadu_pd(&ptrB[i + 8]));
			__m128d vec5 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 10]), _mm_loadu_pd(&ptrB[i + 10]));
			__m128d vec6 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 12]), _mm_loadu_pd(&ptrB[i + 12]));
			__m128d vec7 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i + 14]), _mm_loadu_pd(&ptrB[i + 14]));
			vec0 = _mm_add_pd(vec0, vec2);
			vec4 = _mm_add_pd(vec4, vec6);
			vec1 = _mm_add_pd(vec1, vec3);
			vec5 = _mm_add_pd(vec5, vec7);
			vec0 = _mm_add_pd(vec0, vec4);
			vec1 = _mm_add_pd(vec1, vec5);
			vecSum0 = _mm_add_pd(vecSum0, vec0);
			vecSum1 = _mm_add_pd(vecSum1, vec1);
		}
		for (; i < width2; i += 2) {
			__m128d vec0 = _mm_mul_pd(_mm_loadu_pd(&ptrA[i]), _mm_loadu_pd(&ptrB[i]));
			vecSum0 = _mm_add_pd(vecSum0, vec0);
		}
		for (; i < width; i += 1) {
			__m128d vec0 = _mm_mul_sd(_mm_load_sd(&ptrA[i]), _mm_load_sd(&ptrB[i]));
			vecSum0 = _mm_add_sd(vecSum0, vec0);
		}
		ptrA += strideA;
		ptrB += strideB;
	}

	vecSum0 = _mm_add_pd(vecSum0, vecSum1);
	vecSum0 = _mm_add_sd(vecSum0, _mm_shuffle_pd(vecSum0, vecSum0, 0xff));
	_mm_store_sd(ret, vecSum0);
}

// Must not require memory alignment (random access from SVM)
void CompVMathDotDotSub_64f64f_Intrin_SSE2(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Use AVX2+FMA3 ASM implementation");

	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width2 = width & -2;
	compv_uscalar_t i;
	__m128d vecSum0 = _mm_setzero_pd();
	__m128d vecSum1 = _mm_setzero_pd();

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			__m128d vec0 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i]), _mm_loadu_pd(&ptrB[i]));
			__m128d vec1 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i + 2]), _mm_loadu_pd(&ptrB[i + 2]));
			__m128d vec2 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i + 4]), _mm_loadu_pd(&ptrB[i + 4]));
			__m128d vec3 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i + 6]), _mm_loadu_pd(&ptrB[i + 6]));
			__m128d vec4 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i + 8]), _mm_loadu_pd(&ptrB[i + 8]));
			__m128d vec5 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i + 10]), _mm_loadu_pd(&ptrB[i + 10]));
			__m128d vec6 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i + 12]), _mm_loadu_pd(&ptrB[i + 12]));
			__m128d vec7 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i + 14]), _mm_loadu_pd(&ptrB[i + 14]));
			// TODO(dmi): Add FMA implementation
			vec0 = _mm_mul_pd(vec0, vec0);
			vec2 = _mm_mul_pd(vec2, vec2);
			vec4 = _mm_mul_pd(vec4, vec4);
			vec6 = _mm_mul_pd(vec6, vec6);
			vec1 = _mm_mul_pd(vec1, vec1);
			vec3 = _mm_mul_pd(vec3, vec3);
			vec5 = _mm_mul_pd(vec5, vec5);
			vec7 = _mm_mul_pd(vec7, vec7);
			vec0 = _mm_add_pd(vec0, vec2);
			vec4 = _mm_add_pd(vec4, vec6);
			vec1 = _mm_add_pd(vec1, vec3);
			vec5 = _mm_add_pd(vec5, vec7);
			vec0 = _mm_add_pd(vec0, vec4);
			vec1 = _mm_add_pd(vec1, vec5);
			vecSum0 = _mm_add_pd(vecSum0, vec0);
			vecSum1 = _mm_add_pd(vecSum1, vec1);
		}
		for (; i < width2; i += 2) {
			__m128d vec0 = _mm_sub_pd(_mm_loadu_pd(&ptrA[i]), _mm_loadu_pd(&ptrB[i]));
			vec0 = _mm_mul_pd(vec0, vec0);
			vecSum0 = _mm_add_pd(vecSum0, vec0);
		}
		for (; i < width; i += 1) {
			__m128d vec0 = _mm_sub_sd(_mm_load_sd(&ptrA[i]), _mm_load_sd(&ptrB[i]));
			vec0 = _mm_mul_sd(vec0, vec0);
			vecSum0 = _mm_add_sd(vecSum0, vec0);
		}
		ptrA += strideA;
		ptrB += strideB;
	}

	vecSum0 = _mm_add_pd(vecSum0, vecSum1);
	vecSum0 = _mm_add_sd(vecSum0, _mm_shuffle_pd(vecSum0, vecSum0, 0xff));
	_mm_store_sd(ret, vecSum0);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
