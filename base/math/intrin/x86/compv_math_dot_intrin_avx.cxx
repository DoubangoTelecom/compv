/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_dot_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathDotDot_64f64f_Intrin_AVX(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_TODO("Add ASM implementation");
	COMPV_ASSERT(false);
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathDotDotSub_64f64f_Intrin_AVX(COMPV_ALIGNED(AVX) const compv_float64_t* ptrA, COMPV_ALIGNED(AVX) const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t strideA, COMPV_ALIGNED(AVX) const compv_uscalar_t strideB, compv_float64_t* ret)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Use AVX2+FMA3 ASM implementation (faster)");
	COMPV_DEBUG_INFO_CODE_TODO("Add ASM implementation");

	_mm256_zeroupper();

	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width2 = width & -2;
	compv_uscalar_t i;
	__m256d vecSum = _mm256_setzero_pd();

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			__m256d vec0 = _mm256_sub_pd(_mm256_loadu_pd(&ptrA[i]), _mm256_loadu_pd(&ptrB[i]));
			__m256d vec1 = _mm256_sub_pd(_mm256_loadu_pd(&ptrA[i + 4]), _mm256_loadu_pd(&ptrB[i + 4]));
			__m256d vec2 = _mm256_sub_pd(_mm256_loadu_pd(&ptrA[i + 8]), _mm256_loadu_pd(&ptrB[i + 8]));
			__m256d vec3 = _mm256_sub_pd(_mm256_loadu_pd(&ptrA[i + 12]), _mm256_loadu_pd(&ptrB[i + 12]));
			// TODO(dmi): Add FMA implementation
			vec0 = _mm256_mul_pd(vec0, vec0);
			vec1 = _mm256_mul_pd(vec1, vec1);
			vec2 = _mm256_mul_pd(vec2, vec2);
			vec3 = _mm256_mul_pd(vec3, vec3);			
			vec0 = _mm256_add_pd(vec0, vec1);
			vec2 = _mm256_add_pd(vec2, vec3);
			vec0 = _mm256_add_pd(vec0, vec2);
			vecSum = _mm256_add_pd(vecSum, vec0);
		}
		__m128d vecSum0 = _mm256_castpd256_pd128(vecSum); // TODO(dmi): Not needed (see ASM code)
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
		vecSum = _mm256_insertf128_pd(vecSum, vecSum0, 0); // TODO(dmi): Not needed (see ASM code)
		ptrA += strideA;
		ptrB += strideB;
	}

	__m128d vecSum0 = _mm_add_pd(_mm256_castpd256_pd128(vecSum), _mm256_extractf128_pd(vecSum, 1));
	vecSum0 = _mm_add_sd(vecSum0, _mm_shuffle_pd(vecSum0, vecSum0, 0xff));
	_mm_store_sd(ret, vecSum0);

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
