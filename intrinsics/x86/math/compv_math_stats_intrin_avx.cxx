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

	__m256d ymmX = _mm256_load_pd(&x[0]);
	__m256d ymmY = _mm256_load_pd(&y[0]);
	
	__m256d ymmTx = _mm256_hadd_pd(ymmX, ymmX);
	__m256d ymmTy = _mm256_hadd_pd(ymmY, ymmY);
	ymmTx = _mm256_add_pd(ymmTx, _mm256_permute2f128_pd(ymmTx, ymmTx, 0x11)); // hadd and result in first double
	ymmTy = _mm256_add_pd(ymmTy, _mm256_permute2f128_pd(ymmTy, ymmTy, 0x11)); // hadd and result in first double
	ymmTx = _mm256_mul_pd(ymmTx, ymmOneOverNumPoints);
	ymmTy = _mm256_mul_pd(ymmTy, ymmOneOverNumPoints);
	ymmTx = _mm256_permute2f128_pd(ymmTx, ymmTx, 0x0); // duplicate first double	
	ymmTy = _mm256_permute2f128_pd(ymmTy, ymmTy, 0x0); // duplicate first double

	ymmX = _mm256_sub_pd(ymmX, ymmTx);
	ymmY = _mm256_sub_pd(ymmY, ymmTy);
	__m256d ymmMagnitude = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(ymmX, ymmX), _mm256_mul_pd(ymmY, ymmY)));
	ymmMagnitude = _mm256_hadd_pd(ymmMagnitude, ymmMagnitude);
	ymmMagnitude = _mm256_add_pd(ymmMagnitude, _mm256_permute2f128_pd(ymmMagnitude, ymmMagnitude, 0x11)); // hadd and result in first double
	ymmMagnitude = _mm256_mul_pd(ymmMagnitude, ymmOneOverNumPoints);
	ymmMagnitude = _mm256_div_pd(ymmSqrt2, ymmMagnitude);

	_mm256_maskstore_pd(tx1, ymmMaskToExtractFirst64Bits, ymmTx);
	_mm256_maskstore_pd(ty1, ymmMaskToExtractFirst64Bits, ymmTy);
	_mm256_maskstore_pd(s1, ymmMaskToExtractFirst64Bits, ymmMagnitude);

	_mm256_zeroupper();
}

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MathStatsMSE2DHomogeneous_float64_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* aX_h, const COMPV_ALIGNED(AVX) compv_float64_t* aY_h, const COMPV_ALIGNED(AVX) compv_float64_t* aZ_h, const COMPV_ALIGNED(AVX) compv_float64_t* bX, const COMPV_ALIGNED(AVX) compv_float64_t* bY, COMPV_ALIGNED(AVX) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
#if !defined(__AVX__)
	COMPV_DEBUG_INFO_CODE_AVX_SSE_MIX();
#endif
	_mm256_zeroupper();

	compv_scalar_t numPointsSigned = static_cast<compv_scalar_t>(numPoints);
	__m256d ymmEX, ymmEY, ymmScale;
	const __m256d ymmOne = _mm256_set1_pd(1.);

	for (compv_scalar_t i = 0; i < numPointsSigned; i += 4) { // memory aligned -> can read beyond the end of the data and up to stride
		ymmScale = _mm256_div_pd(ymmOne, _mm256_load_pd(&aZ_h[i]));
		ymmEX = _mm256_sub_pd(_mm256_mul_pd(_mm256_load_pd(&aX_h[i]), ymmScale), _mm256_load_pd(&bX[i]));
		ymmEY = _mm256_sub_pd(_mm256_mul_pd(_mm256_load_pd(&aY_h[i]), ymmScale), _mm256_load_pd(&bY[i]));
		_mm256_store_pd(&mse[i], _mm256_add_pd(_mm256_mul_pd(ymmEX, ymmEX), _mm256_mul_pd(ymmEY, ymmEY)));
	}

	_mm256_zeroupper();
}

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MathStatsVariance_float64_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
#if !defined(__AVX__)
	COMPV_DEBUG_INFO_CODE_AVX_SSE_MIX();
#endif
	_mm256_zeroupper();

	compv_scalar_t countSigned = static_cast<compv_scalar_t>(count), i;
	__m256d ymmDev;
	__m256d ymmVar = _mm256_setzero_pd();
	const __m256d ymmCountMinus1 = _mm256_set1_pd(static_cast<compv_float64_t>(count - 1)); // asm: ctv(epi32, double) and no need to duplicate
	const __m256d ymmMean = _mm256_set1_pd(*mean1); // asm: vmovsd() then shufle
	for (i = 0; i < countSigned - 3; i += 4) {
		ymmDev = _mm256_sub_pd(_mm256_load_pd(&data[i]), ymmMean);
		ymmVar = _mm256_add_pd(ymmVar, _mm256_mul_pd(ymmDev, ymmDev));
	}
	if (i < countSigned) {
		compv_scalar_t remain = (countSigned - i);
		ymmDev = _mm256_sub_pd(_mm256_load_pd(&data[i]), ymmMean); // data is aligned on AVX which means we can read beyond the bounds and up to the stride
		if (remain == 3) {
			const __m256d ymmMaskToHideLast64Bits = _mm256_load_pd((compv_float64_t*)kAVXMaskzero_3_u64); // not need for asm
			ymmDev = _mm256_and_pd(ymmDev, ymmMaskToHideLast64Bits);
		}
		else if (remain == 2) {
			const __m256d ymmMaskToHideLast128Bits = _mm256_load_pd((compv_float64_t*)kAVXMaskzero_2_3_u64); // not need for asm
			ymmDev = _mm256_and_pd(ymmDev, ymmMaskToHideLast128Bits);
		}
		else {
			const __m256d ymmMaskToHideLast192Bits = _mm256_load_pd((compv_float64_t*)kAVXMaskzero_1_2_3_u64); // not need for asm
			ymmDev = _mm256_and_pd(ymmDev, ymmMaskToHideLast192Bits);
		}
		ymmVar = _mm256_add_pd(ymmVar, _mm256_mul_pd(ymmDev, ymmDev));
	}
	ymmVar = _mm256_hadd_pd(ymmVar, ymmVar);
	ymmVar = _mm256_add_pd(ymmVar, _mm256_permute2f128_pd(ymmVar, ymmVar, 0x11)); // hadd and result in first double
	ymmVar = _mm256_div_pd(ymmVar, ymmCountMinus1); // asm: vdivsd
	_mm256_maskstore_pd(var1, _mm256_load_si256((__m256i*)kAVXMaskstore_0_u64), ymmVar); // asm: vmovsd

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
