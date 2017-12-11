/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_stats_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

//!\\ TODO(dmi): This file contains deprecated code not fully optimized yet and not used

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathStatsNormalize2DHartley_64f_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* x, const COMPV_ALIGNED(AVX) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();
	compv_scalar_t i, numPoints_ = static_cast<compv_uscalar_t>(numPoints);

	__m256d vec0, vec1, vec2, vec3;
	__m256d vecTx = _mm256_setzero_pd();
	__m256d vecTy = _mm256_setzero_pd();
	__m256d vecMagnitude = _mm256_setzero_pd();
	const __m256d vecOneOverNumPoints = _mm256_div_pd(_mm256_set1_pd(1.), _mm256_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m256d vecSqrt2 = _mm256_set1_pd(COMPV_MATH_SQRT_2);
	const __m256i vecMaskToExtractFirst64Bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXMaskstore_0_64u)); // not need for asm
	const __m256i vecMaskToExtractFirst128Bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXMaskstore_0_1_64u)); // not need for asm
	const __m256d vecMaskToHideLast128Bits = _mm256_load_pd(reinterpret_cast<const compv_float64_t*>(kAVXMaskzero_2_3_64u)); // not need for asm
	const __m256d vecMaskToHideLast192Bits = _mm256_load_pd(reinterpret_cast<const compv_float64_t*>(kAVXMaskzero_1_2_3_64u)); // not need for asm

	/* TX and TY */

	for (i = 0; i < numPoints_ - 15; i += 16) {
		vecTx = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&x[i]), _mm256_load_pd(&x[i + 4])),
			_mm256_add_pd(_mm256_load_pd(&x[i + 8]), _mm256_load_pd(&x[i + 12]))), vecTx);
		vecTy = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&y[i]), _mm256_load_pd(&y[i + 4])),
			_mm256_add_pd(_mm256_load_pd(&y[i + 8]), _mm256_load_pd(&y[i + 12]))), vecTy);
	}
	if (i < numPoints_ - 7) {
		vecTx = _mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&x[i]), _mm256_load_pd(&x[i + 4])), vecTx);
		vecTy = _mm256_add_pd(_mm256_add_pd(_mm256_load_pd(&y[i]), _mm256_load_pd(&y[i + 4])), vecTy);
		i += 8;
	}
	if (i < numPoints_ - 3) {
		vecTx = _mm256_add_pd(_mm256_load_pd(&x[i]), vecTx);
		vecTy = _mm256_add_pd(_mm256_load_pd(&y[i]), vecTy);
		i += 4;
	}
	if (i < numPoints_ - 1) {
		vecTx = _mm256_add_pd(_mm256_maskload_pd(&x[i], vecMaskToExtractFirst128Bits), vecTx);
		vecTy = _mm256_add_pd(_mm256_maskload_pd(&y[i], vecMaskToExtractFirst128Bits), vecTy);
		i += 2;
	}
	if (numPoints_ & 1) {
		vecTx = _mm256_add_pd(vecTx, _mm256_maskload_pd(&x[i], vecMaskToExtractFirst64Bits));
		vecTy = _mm256_add_pd(vecTy, _mm256_maskload_pd(&y[i], vecMaskToExtractFirst64Bits));
	}
	vecTx = _mm256_hadd_pd(vecTx, vecTx);
	vecTx = _mm256_add_pd(vecTx, _mm256_permute2f128_pd(vecTx, vecTx, 0x11)); // hadd and result in first double
	vecTx = _mm256_mul_pd(vecTx, vecOneOverNumPoints);
	vecTx = _mm256_permute2f128_pd(vecTx, vecTx, 0x0); // duplicate first double
	vecTy = _mm256_hadd_pd(vecTy, vecTy);
	vecTy = _mm256_add_pd(vecTy, _mm256_permute2f128_pd(vecTy, vecTy, 0x11)); // hadd and result in first double
	vecTy = _mm256_mul_pd(vecTy, vecOneOverNumPoints);
	vecTy = _mm256_permute2f128_pd(vecTy, vecTy, 0x0); // duplicate first double

	/* Magnitude */

	for (i = 0; i < numPoints_ - 7; i += 8) {
		vec0 = _mm256_sub_pd(_mm256_load_pd(&x[i]), vecTx);
		vec1 = _mm256_sub_pd(_mm256_load_pd(&y[i]), vecTy);
		vec2 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(vec0, vec0), _mm256_mul_pd(vec1, vec1)));
		vec0 = _mm256_sub_pd(_mm256_load_pd(&x[i + 4]), vecTx);
		vec1 = _mm256_sub_pd(_mm256_load_pd(&y[i + 4]), vecTy);
		vec3 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(vec0, vec0), _mm256_mul_pd(vec1, vec1)));
		vecMagnitude = _mm256_add_pd(vec2, vecMagnitude);
		vecMagnitude = _mm256_add_pd(vec3, vecMagnitude);
	}
	if (i < numPoints_ - 3) {
		vec0 = _mm256_sub_pd(_mm256_load_pd(&x[i]), vecTx);
		vec1 = _mm256_sub_pd(_mm256_load_pd(&y[i]), vecTy);
		vec2 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(vec0, vec0), _mm256_mul_pd(vec1, vec1)));
		vecMagnitude = _mm256_add_pd(vec2, vecMagnitude);
		i += 4;
	}
	if (i < numPoints_ - 1) {
		vec0 = _mm256_sub_pd(_mm256_maskload_pd(&x[i], vecMaskToExtractFirst128Bits), vecTx);
		vec1 = _mm256_sub_pd(_mm256_maskload_pd(&y[i], vecMaskToExtractFirst128Bits), vecTy);
		vec2 = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(vec0, vec0), _mm256_mul_pd(vec1, vec1)));
		vecMagnitude = _mm256_add_pd(_mm256_and_pd(vecMaskToHideLast128Bits, vec2), vecMagnitude);
		i += 2;
	}
	if (numPoints_ & 1) {
		vec0 = _mm256_sub_pd(_mm256_maskload_pd(&x[i], vecMaskToExtractFirst64Bits), vecTx);
		vec1 = _mm256_sub_pd(_mm256_maskload_pd(&y[i], vecMaskToExtractFirst64Bits), vecTy);
		vec2 = _mm256_add_pd(_mm256_mul_pd(vec0, vec0), _mm256_mul_pd(vec1, vec1));
		vec3 = _mm256_sqrt_pd(vec2);
		vecMagnitude = _mm256_add_pd(_mm256_and_pd(vecMaskToHideLast192Bits, vec3), vecMagnitude);
	}
	vecMagnitude = _mm256_hadd_pd(vecMagnitude, vecMagnitude);
	vecMagnitude = _mm256_add_pd(vecMagnitude, _mm256_permute2f128_pd(vecMagnitude, vecMagnitude, 0x11)); // hadd and result in first double
	vecMagnitude = _mm256_mul_pd(vecMagnitude, vecOneOverNumPoints);
	vecMagnitude = _mm256_div_pd(vecSqrt2, vecMagnitude);

	/* Result */

	_mm256_maskstore_pd(tx1, vecMaskToExtractFirst64Bits, vecTx);
	_mm256_maskstore_pd(ty1, vecMaskToExtractFirst64Bits, vecTy);
	_mm256_maskstore_pd(s1, vecMaskToExtractFirst64Bits, vecMagnitude);

	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
// "numPoints = 4" -> Very common (Homography)
void CompVMathStatsNormalize2DHartley_4_64f_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* x, const COMPV_ALIGNED(AVX) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();
	const __m256d vecOneOverNumPoints = _mm256_div_pd(_mm256_set1_pd(1.), _mm256_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m256d vecSqrt2 = _mm256_set1_pd(COMPV_MATH_SQRT_2);
	const __m256i vecMaskToExtractFirst64Bits = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXMaskstore_0_64u)); // not need for asm

	__m256d vecX = _mm256_load_pd(&x[0]);
	__m256d vecY = _mm256_load_pd(&y[0]);

	__m256d vecTx = _mm256_hadd_pd(vecX, vecX);
	__m256d vecTy = _mm256_hadd_pd(vecY, vecY);
	vecTx = _mm256_add_pd(vecTx, _mm256_permute2f128_pd(vecTx, vecTx, 0x11)); // hadd and result in first double
	vecTy = _mm256_add_pd(vecTy, _mm256_permute2f128_pd(vecTy, vecTy, 0x11)); // hadd and result in first double
	vecTx = _mm256_mul_pd(vecTx, vecOneOverNumPoints);
	vecTy = _mm256_mul_pd(vecTy, vecOneOverNumPoints);
	vecTx = _mm256_permute2f128_pd(vecTx, vecTx, 0x0); // duplicate first double
	vecTy = _mm256_permute2f128_pd(vecTy, vecTy, 0x0); // duplicate first double

	vecX = _mm256_sub_pd(vecX, vecTx);
	vecY = _mm256_sub_pd(vecY, vecTy);
	__m256d vecMagnitude = _mm256_sqrt_pd(_mm256_add_pd(_mm256_mul_pd(vecX, vecX), _mm256_mul_pd(vecY, vecY)));
	vecMagnitude = _mm256_hadd_pd(vecMagnitude, vecMagnitude);
	vecMagnitude = _mm256_add_pd(vecMagnitude, _mm256_permute2f128_pd(vecMagnitude, vecMagnitude, 0x11)); // hadd and result in first double
	vecMagnitude = _mm256_mul_pd(vecMagnitude, vecOneOverNumPoints);
	vecMagnitude = _mm256_div_pd(vecSqrt2, vecMagnitude);

	_mm256_maskstore_pd(tx1, vecMaskToExtractFirst64Bits, vecTx);
	_mm256_maskstore_pd(ty1, vecMaskToExtractFirst64Bits, vecTy);
	_mm256_maskstore_pd(s1, vecMaskToExtractFirst64Bits, vecMagnitude);

	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathStatsMSE2DHomogeneous_64f_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* aX_h, const COMPV_ALIGNED(AVX) compv_float64_t* aY_h, const COMPV_ALIGNED(AVX) compv_float64_t* aZ_h, const COMPV_ALIGNED(AVX) compv_float64_t* bX, const COMPV_ALIGNED(AVX) compv_float64_t* bY, COMPV_ALIGNED(AVX) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();

	compv_scalar_t numPointsSigned = static_cast<compv_scalar_t>(numPoints);
	__m256d vecEX, vecEY, vecScale;
	const __m256d vecOne = _mm256_set1_pd(1.);

	for (compv_scalar_t i = 0; i < numPointsSigned; i += 4) { // memory aligned -> can read beyond the end of the data and up to stride
		vecScale = _mm256_div_pd(vecOne, _mm256_load_pd(&aZ_h[i]));
		vecEX = _mm256_sub_pd(_mm256_mul_pd(_mm256_load_pd(&aX_h[i]), vecScale), _mm256_load_pd(&bX[i]));
		vecEY = _mm256_sub_pd(_mm256_mul_pd(_mm256_load_pd(&aY_h[i]), vecScale), _mm256_load_pd(&bY[i]));
		_mm256_store_pd(&mse[i], _mm256_add_pd(_mm256_mul_pd(vecEX, vecEX), _mm256_mul_pd(vecEY, vecEY)));
	}

	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathStatsVariance_64f_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1)
{
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();

	compv_scalar_t countSigned = static_cast<compv_scalar_t>(count), i;
	__m256d vecDev;
	__m256d vecVar = _mm256_setzero_pd();
	const __m256d vecCountMinus1 = _mm256_set1_pd(static_cast<compv_float64_t>(count - 1)); // asm: ctv(epi32, double) and no need to duplicate
	const __m256d vecMean = _mm256_set1_pd(*mean1); // asm: vmovsd() then shufle
	for (i = 0; i < countSigned - 3; i += 4) {
		vecDev = _mm256_sub_pd(_mm256_load_pd(&data[i]), vecMean);
		vecVar = _mm256_add_pd(vecVar, _mm256_mul_pd(vecDev, vecDev));
	}
	if (i < countSigned) {
		compv_scalar_t remain = (countSigned - i);
		vecDev = _mm256_sub_pd(_mm256_load_pd(&data[i]), vecMean); // data is aligned on AVX which means we can read beyond the bounds and up to the stride
		if (remain == 3) {
			const __m256d vecMaskToHideLast64Bits = _mm256_load_pd(reinterpret_cast<const compv_float64_t*>(kAVXMaskzero_3_64u)); // not need for asm
			vecDev = _mm256_and_pd(vecDev, vecMaskToHideLast64Bits);
		}
		else if (remain == 2) {
			const __m256d vecMaskToHideLast128Bits = _mm256_load_pd(reinterpret_cast<const compv_float64_t*>(kAVXMaskzero_2_3_64u)); // not need for asm
			vecDev = _mm256_and_pd(vecDev, vecMaskToHideLast128Bits);
		}
		else {
			const __m256d vecMaskToHideLast192Bits = _mm256_load_pd(reinterpret_cast<const compv_float64_t*>(kAVXMaskzero_1_2_3_64u)); // not need for asm
			vecDev = _mm256_and_pd(vecDev, vecMaskToHideLast192Bits);
		}
		vecVar = _mm256_add_pd(vecVar, _mm256_mul_pd(vecDev, vecDev));
	}
	vecVar = _mm256_hadd_pd(vecVar, vecVar);
	vecVar = _mm256_add_pd(vecVar, _mm256_permute2f128_pd(vecVar, vecVar, 0x11)); // hadd and result in first double
	vecVar = _mm256_div_pd(vecVar, vecCountMinus1); // asm: vdivsd
	_mm256_maskstore_pd(var1, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXMaskstore_0_64u)), vecVar); // asm: vmovsd

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
