/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_stats_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MathStatsNormalize2DHartley_float64_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* x, const COMPV_ALIGNED(AVX) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
#if !defined(__AVX__)
	COMPV_DEBUG_INFO_CODE_AVX_SSE_MIX();
#endif
	_mm256_zeroupper();
	compv_scalar_t i, numPoints_ = static_cast<compv_uscalar_t>(numPoints);

	__m256d ymm0, ymm1, ymm2, ymm3;
	__m256d ymmTx = _mm256_setzero_pd();
	__m256d ymmTy = _mm256_setzero_pd();
	__m256d ymmMagnitude = _mm256_setzero_pd();
	const __m256d ymmOneOverNumPoints = _mm256_div_pd(_mm256_set1_pd(1.), _mm256_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m256d ymmSqrt2 = _mm256_set1_pd(COMPV_MATH_SQRT_2);
	const __m256i ymmMaskToExtractFirst64Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_u64); // not need for asm
	const __m256i ymmMaskToExtractFirst128Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_u64); // not need for asm
	const __m256d ymmMaskToHideLast128Bits = _mm256_load_pd((compv_float64_t*)kAVXMaskzero_2_3_u64); // not need for asm
	const __m256d ymmMaskToHideLast192Bits = _mm256_load_pd((compv_float64_t*)kAVXMaskzero_1_2_3_u64); // not need for asm

	/* TX and TY */

	for (i = 0; i < numPoints_ - 15; i += 16) {
		ymmTx = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&x[i]), _mm256_load_pd(&x[i + 4])),
			_mm256_add_pd(_mm256_load_pd(&x[i + 8]), _mm256_load_pd(&x[i + 12]))), ymmTx);
		ymmTy = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&y[i]), _mm256_load_pd(&y[i + 4])),
			_mm256_add_pd(_mm256_load_pd(&y[i + 8]), _mm256_load_pd(&y[i + 12]))), ymmTy);
	}
	if (i < numPoints_ - 7) {
		ymmTx = _mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&x[i]), _mm256_load_pd(&x[i + 4])), ymmTx);
		ymmTy = _mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&y[i]), _mm256_load_pd(&y[i + 4])), ymmTy);
		i += 8;
	}
	if (i < numPoints_ - 3) {
		ymmTx = _mm256_add_pd(_mm256_load_pd(&x[i]), ymmTx);
		ymmTy = _mm256_add_pd(_mm256_load_pd(&y[i]), ymmTy);
		i += 4;
	}
	if (i < numPoints_ - 1) {
		ymmTx = _mm256_add_pd(_mm256_maskload_pd(&x[i], ymmMaskToExtractFirst128Bits), ymmTx);
		ymmTy = _mm256_add_pd(_mm256_maskload_pd(&y[i], ymmMaskToExtractFirst128Bits), ymmTy);
		i += 2;
	}
	if (numPoints_ & 1) {
		ymmTx = _mm256_add_pd(ymmTx, _mm256_maskload_pd(&x[i], ymmMaskToExtractFirst64Bits));
		ymmTy = _mm256_add_pd(ymmTy, _mm256_maskload_pd(&y[i], ymmMaskToExtractFirst64Bits));
	}
	ymmTx = _mm256_hadd_pd(ymmTx, ymmTx);
	ymmTx = _mm256_add_pd(ymmTx, _mm256_permute2f128_pd(ymmTx, ymmTx, 0x11)); // hadd and result in first double
	ymmTx = _mm256_mul_pd(ymmTx, ymmOneOverNumPoints);
	ymmTx = _mm256_permute2f128_pd(ymmTx, ymmTx, 0x0); // duplicate first double
	ymmTy = _mm256_hadd_pd(ymmTy, ymmTy);
	ymmTy = _mm256_add_pd(ymmTy, _mm256_permute2f128_pd(ymmTy, ymmTy, 0x11)); // hadd and result in first double
	ymmTy = _mm256_mul_pd(ymmTy, ymmOneOverNumPoints);
	ymmTy = _mm256_permute2f128_pd(ymmTy, ymmTy, 0x0); // duplicate first double

	/* Magnitude */

	for (i = 0; i < numPoints_ - 7; i += 8) {
		ymm0 = _mm256_sub_pd(_mm256_load_pd(&x[i]), ymmTx);
		ymm1 = _mm256_sub_pd(_mm256_load_pd(&y[i]), ymmTy);
		ymm2 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(ymm0, ymm0), _mm256_mul_pd(ymm1, ymm1)));
		ymm0 = _mm256_sub_pd(_mm256_load_pd(&x[i + 4]), ymmTx);
		ymm1 = _mm256_sub_pd(_mm256_load_pd(&y[i + 4]), ymmTy);
		ymm3 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(ymm0, ymm0), _mm256_mul_pd(ymm1, ymm1)));
		ymmMagnitude = _mm256_add_pd(ymm2, ymmMagnitude);
		ymmMagnitude = _mm256_add_pd(ymm3, ymmMagnitude);
	}
	if (i < numPoints_ - 3) {
		ymm0 = _mm256_sub_pd(_mm256_load_pd(&x[i]), ymmTx);
		ymm1 = _mm256_sub_pd(_mm256_load_pd(&y[i]), ymmTy);
		ymm2 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(ymm0, ymm0), _mm256_mul_pd(ymm1, ymm1)));
		ymmMagnitude = _mm256_add_pd(ymm2, ymmMagnitude);
		i += 4;
	}
	if (i < numPoints_ - 1) {
		ymm0 = _mm256_sub_pd(_mm256_maskload_pd(&x[i], ymmMaskToExtractFirst128Bits), ymmTx);
		ymm1 = _mm256_sub_pd(_mm256_maskload_pd(&y[i], ymmMaskToExtractFirst128Bits), ymmTy);
		ymm2 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(ymm0, ymm0), _mm256_mul_pd(ymm1, ymm1)));
		ymmMagnitude = _mm256_add_pd(_mm256_and_pd(ymmMaskToHideLast128Bits, ymm2), ymmMagnitude);
		i += 2;
	}
	if (numPoints_ & 1) {
		ymm0 = _mm256_sub_pd(_mm256_maskload_pd(&x[i], ymmMaskToExtractFirst64Bits), ymmTx);
		ymm1 = _mm256_sub_pd(_mm256_maskload_pd(&y[i], ymmMaskToExtractFirst64Bits), ymmTy);
		ymm2 = _mm256_add_pd(_mm256_mul_pd(ymm0, ymm0), _mm256_mul_pd(ymm1, ymm1));
		ymm3 = _mm256_sqrt_pd(ymm2);
		ymmMagnitude = _mm256_add_pd(_mm256_and_pd(ymmMaskToHideLast192Bits, ymm3), ymmMagnitude);
	}
	ymmMagnitude = _mm256_hadd_pd(ymmMagnitude, ymmMagnitude);
	ymmMagnitude = _mm256_add_pd(ymmMagnitude, _mm256_permute2f128_pd(ymmMagnitude, ymmMagnitude, 0x11)); // hadd and result in first double
	ymmMagnitude = _mm256_mul_pd(ymmMagnitude, ymmOneOverNumPoints);
	ymmMagnitude = _mm256_div_pd(ymmSqrt2, ymmMagnitude);

	/* Result */

	_mm256_maskstore_pd(tx1, ymmMaskToExtractFirst64Bits, ymmTx);
	_mm256_maskstore_pd(ty1, ymmMaskToExtractFirst64Bits, ymmTy);
	_mm256_maskstore_pd(s1, ymmMaskToExtractFirst64Bits, ymmMagnitude);
	
	_mm256_zeroupper();
}

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
// "numPoints = 4" -> Very common (Homography)
void MathStatsNormalize2DHartley_4_float64_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* x, const COMPV_ALIGNED(AVX) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
#if !defined(__AVX__)
	COMPV_DEBUG_INFO_CODE_AVX_SSE_MIX();
#endif
	_mm256_zeroupper();
	const __m256d ymmOneOverNumPoints = _mm256_div_pd(_mm256_set1_pd(1.), _mm256_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m256d ymmSqrt2 = _mm256_set1_pd(COMPV_MATH_SQRT_2);
	const __m256i ymmMaskToExtractFirst64Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_u64); // not need for asm

	__m256d xmmX = _mm256_load_pd(&x[0]);
	__m256d xmmY = _mm256_load_pd(&y[0]);
	
	__m256d ymmTx = _mm256_hadd_pd(xmmX, xmmX);
	__m256d ymmTy = _mm256_hadd_pd(xmmY, xmmY);
	ymmTx = _mm256_add_pd(ymmTx, _mm256_permute2f128_pd(ymmTx, ymmTx, 0x11)); // hadd and result in first double
	ymmTy = _mm256_add_pd(ymmTy, _mm256_permute2f128_pd(ymmTy, ymmTy, 0x11)); // hadd and result in first double
	ymmTx = _mm256_mul_pd(ymmTx, ymmOneOverNumPoints);
	ymmTy = _mm256_mul_pd(ymmTy, ymmOneOverNumPoints);
	ymmTx = _mm256_permute2f128_pd(ymmTx, ymmTx, 0x0); // duplicate first double	
	ymmTy = _mm256_permute2f128_pd(ymmTy, ymmTy, 0x0); // duplicate first double

	xmmX = _mm256_sub_pd(xmmX, ymmTx);
	xmmY = _mm256_sub_pd(xmmY, ymmTy);
	__m256d ymmMagnitude = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(xmmX, xmmX), _mm256_mul_pd(xmmY, xmmY)));
	ymmMagnitude = _mm256_hadd_pd(ymmMagnitude, ymmMagnitude);
	ymmMagnitude = _mm256_add_pd(ymmMagnitude, _mm256_permute2f128_pd(ymmMagnitude, ymmMagnitude, 0x11)); // hadd and result in first double
	ymmMagnitude = _mm256_mul_pd(ymmMagnitude, ymmOneOverNumPoints);
	ymmMagnitude = _mm256_div_pd(ymmSqrt2, ymmMagnitude);

	_mm256_maskstore_pd(tx1, ymmMaskToExtractFirst64Bits, ymmTx);
	_mm256_maskstore_pd(ty1, ymmMaskToExtractFirst64Bits, ymmTy);
	_mm256_maskstore_pd(s1, ymmMaskToExtractFirst64Bits, ymmMagnitude);

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
