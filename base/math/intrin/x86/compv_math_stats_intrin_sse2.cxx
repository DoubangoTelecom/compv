/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_stats_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathStatsNormalize2DHartley_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i, numPoints_ = static_cast<compv_scalar_t>(numPoints);

	__m128d vec0, vec1, vec2, vec3;
	__m128d vecTx = _mm_setzero_pd();
	__m128d vecTy = _mm_setzero_pd();
	__m128d vecMagnitude = _mm_setzero_pd();
	const __m128d vecOneOverNumPoints = _mm_div_pd(_mm_set1_pd(1.), _mm_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m128d vecSqrt2 = _mm_set1_pd(COMPV_MATH_SQRT_2);

	/*** TX and TY ***/

	for (i = 0; i < numPoints_ - 7; i += 8) {
		vecTx = _mm_add_pd(_mm_add_pd(_mm_add_pd(_mm_load_pd(&x[i]), _mm_load_pd(&x[i + 2])),
			_mm_add_pd(_mm_load_pd(&x[i + 4]), _mm_load_pd(&x[i + 6]))), vecTx);
		vecTy = _mm_add_pd(_mm_add_pd(_mm_add_pd(_mm_load_pd(&y[i]), _mm_load_pd(&y[i + 2])),
			_mm_add_pd(_mm_load_pd(&y[i + 4]), _mm_load_pd(&y[i + 6]))), vecTy);
	}
	if (i < numPoints_ - 3) {
		vecTx = _mm_add_pd(_mm_add_pd(_mm_load_pd(&x[i]), _mm_load_pd(&x[i + 2])), vecTx);
		vecTy = _mm_add_pd(_mm_add_pd(_mm_load_pd(&y[i]), _mm_load_pd(&y[i + 2])), vecTy);
		i += 4;
	}
	if (i < numPoints_ - 1) {
		vecTx = _mm_add_pd(_mm_load_pd(&x[i]), vecTx);
		vecTy = _mm_add_pd(_mm_load_pd(&y[i]), vecTy);
		i += 2;
	}
	if (numPoints_ & 1) {
		vecTx = _mm_add_sd(vecTx, _mm_load_sd(&x[i]));
		vecTy = _mm_add_sd(vecTy, _mm_load_sd(&y[i]));
	}
	vecTx = _mm_mul_sd(_mm_add_sd(vecTx, _mm_shuffle_pd(vecTx, vecTx, 0x1)), vecOneOverNumPoints); // horiz add and result in first double
	vecTy = _mm_mul_sd(_mm_add_sd(vecTy, _mm_shuffle_pd(vecTy, vecTy, 0x1)), vecOneOverNumPoints); // horiz add and result in first double
	vecTx = _mm_shuffle_pd(vecTx, vecTx, 0x0); // duplicate first double
	vecTy = _mm_shuffle_pd(vecTy, vecTy, 0x0); // duplicate first double

	/*** Magnitude ***/

	for (i = 0; i < numPoints_ - 3; i += 4) {
		vec0 = _mm_sub_pd(_mm_load_pd(&x[i]), vecTx);
		vec1 = _mm_sub_pd(_mm_load_pd(&y[i]), vecTy);
		vec2 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(vec0, vec0), _mm_mul_pd(vec1, vec1)));
		vec0 = _mm_sub_pd(_mm_load_pd(&x[i + 2]), vecTx);
		vec1 = _mm_sub_pd(_mm_load_pd(&y[i + 2]), vecTy);
		vec3 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(vec0, vec0), _mm_mul_pd(vec1, vec1)));
		vecMagnitude = _mm_add_pd(vec2, vecMagnitude);
		vecMagnitude = _mm_add_pd(vec3, vecMagnitude);
	}
	if (i < numPoints_ - 1) {
		vec0 = _mm_sub_pd(_mm_load_pd(&x[i]), vecTx);
		vec1 = _mm_sub_pd(_mm_load_pd(&y[i]), vecTy);
		vec2 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(vec0, vec0), _mm_mul_pd(vec1, vec1)));
		vecMagnitude = _mm_add_pd(vec2, vecMagnitude);
		i += 2;
	}
	if (numPoints_ & 1) {
		// asm: notice the "sd" for all functions -> the high double must be ignored (replaced with zero)
		vec0 = _mm_sub_sd(_mm_load_sd(&x[i]), vecTx);
		vec1 = _mm_sub_sd(_mm_load_sd(&y[i]), vecTy);
		vec2 = _mm_add_sd(_mm_mul_sd(vec0, vec0), _mm_mul_sd(vec1, vec1));
		vec3 = _mm_sqrt_sd(vec2, vec2);
		vecMagnitude = _mm_add_sd(vecMagnitude, vec3);
	}
	vecMagnitude = _mm_mul_sd(_mm_add_sd(vecMagnitude, _mm_shuffle_pd(vecMagnitude, vecMagnitude, 0x1)), vecOneOverNumPoints); // horiz add and result in first double
	vecMagnitude = _mm_div_sd(vecSqrt2, vecMagnitude);

	_mm_store_sd(tx1, vecTx);
	_mm_store_sd(ty1, vecTy);
	_mm_store_sd(s1, vecMagnitude);
}

// "numPoints = 4" -> Very common (Homography)
void CompVMathStatsNormalize2DHartley_4_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128d vecOneOverNumPoints = _mm_div_pd(_mm_set1_pd(1.), _mm_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m128d vecSqrt2 = _mm_set1_pd(COMPV_MATH_SQRT_2);

	__m128d vecTx = _mm_add_pd(_mm_load_pd(&x[0]), _mm_load_pd(&x[2]));
	__m128d vecTy = _mm_add_pd(_mm_load_pd(&y[0]), _mm_load_pd(&y[2]));
	vecTx = _mm_mul_sd(_mm_add_sd(vecTx, _mm_shuffle_pd(vecTx, vecTx, 0x1)), vecOneOverNumPoints);
	vecTy = _mm_mul_sd(_mm_add_sd(vecTy, _mm_shuffle_pd(vecTy, vecTy, 0x1)), vecOneOverNumPoints);
	vecTx = _mm_shuffle_pd(vecTx, vecTx, 0x0);
	vecTy = _mm_shuffle_pd(vecTy, vecTy, 0x0);

	__m128d vec0 = _mm_sub_pd(_mm_load_pd(&x[0]), vecTx);
	__m128d vec1 = _mm_sub_pd(_mm_load_pd(&y[0]), vecTy);
	__m128d vecMagnitude = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(vec0, vec0), _mm_mul_pd(vec1, vec1)));
	vec0 = _mm_sub_pd(_mm_load_pd(&x[2]), vecTx);
	vec1 = _mm_sub_pd(_mm_load_pd(&y[2]), vecTy);
	__m128d vec2 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(vec0, vec0), _mm_mul_pd(vec1, vec1)));
	vecMagnitude = _mm_add_pd(vec2, vecMagnitude);
	vecMagnitude = _mm_mul_sd(_mm_add_sd(vecMagnitude, _mm_shuffle_pd(vecMagnitude, vecMagnitude, 0x1)), vecOneOverNumPoints);
	vecMagnitude = _mm_div_sd(vecSqrt2, vecMagnitude);

	_mm_store_sd(tx1, vecTx);
	_mm_store_sd(ty1, vecTy);
	_mm_store_sd(s1, vecMagnitude);
}

void CompVMathStatsMSE2DHomogeneous_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t numPointsSigned = static_cast<compv_scalar_t>(numPoints), i;
	__m128d vecEX0, vecEY0, vecScale0, vecEX1, vecEY1, vecScale1;
	const __m128d vecOne = _mm_set1_pd(1.);

	for (i = 0; i < numPointsSigned - 3; i += 4) {
		vecScale0 = _mm_div_pd(vecOne, _mm_load_pd(&aZ_h[i]));
		vecScale1 = _mm_div_pd(vecOne, _mm_load_pd(&aZ_h[i + 2]));
		vecEX0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[i]), vecScale0), _mm_load_pd(&bX[i]));
		vecEX1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[i + 2]), vecScale1), _mm_load_pd(&bX[i + 2]));
		vecEY0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[i]), vecScale0), _mm_load_pd(&bY[i]));
		vecEY1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[i + 2]), vecScale1), _mm_load_pd(&bY[i + 2]));
		_mm_store_pd(&mse[i], _mm_add_pd(_mm_mul_pd(vecEX0, vecEX0), _mm_mul_pd(vecEY0, vecEY0)));
		_mm_store_pd(&mse[i + 2], _mm_add_pd(_mm_mul_pd(vecEX1, vecEX1), _mm_mul_pd(vecEY1, vecEY1)));
	}
	if (i < numPointsSigned - 1) {
		vecScale0 = _mm_div_pd(vecOne, _mm_load_pd(&aZ_h[i]));
		vecEX0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[i]), vecScale0), _mm_load_pd(&bX[i]));
		vecEY0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[i]), vecScale0), _mm_load_pd(&bY[i]));
		_mm_store_pd(&mse[i], _mm_add_pd(_mm_mul_pd(vecEX0, vecEX0), _mm_mul_pd(vecEY0, vecEY0)));
		i += 2;
	}
	if (numPointsSigned & 1) {
		vecScale0 = _mm_div_sd(vecOne, _mm_load_sd(&aZ_h[i]));
		vecEX0 = _mm_sub_sd(_mm_mul_sd(_mm_load_sd(&aX_h[i]), vecScale0), _mm_load_sd(&bX[i]));
		vecEY0 = _mm_sub_sd(_mm_mul_sd(_mm_load_sd(&aY_h[i]), vecScale0), _mm_load_sd(&bY[i]));
		_mm_store_sd(&mse[i], _mm_add_sd(_mm_mul_sd(vecEX0, vecEX0), _mm_mul_sd(vecEY0, vecEY0)));
	}
}

// "numPoints = 4" -> Very common (Homography)
void CompVMathStatsMSE2DHomogeneous_4_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	__m128d vecEX0, vecEY0, vecScale0, vecEX1, vecEY1, vecScale1;
	const __m128d vecOne = _mm_set1_pd(1.);

	vecScale0 = _mm_div_pd(vecOne, _mm_load_pd(&aZ_h[0]));
	vecScale1 = _mm_div_pd(vecOne, _mm_load_pd(&aZ_h[2]));
	vecEX0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[0]), vecScale0), _mm_load_pd(&bX[0]));
	vecEX1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[2]), vecScale1), _mm_load_pd(&bX[2]));
	vecEY0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[0]), vecScale0), _mm_load_pd(&bY[0]));
	vecEY1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[2]), vecScale1), _mm_load_pd(&bY[2]));
	_mm_store_pd(&mse[0], _mm_add_pd(_mm_mul_pd(vecEX0, vecEX0), _mm_mul_pd(vecEY0, vecEY0)));
	_mm_store_pd(&mse[2], _mm_add_pd(_mm_mul_pd(vecEX1, vecEX1), _mm_mul_pd(vecEY1, vecEY1)));
}

void CompVMathStatsVariance_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t countSigned = static_cast<compv_scalar_t>(count), i;
	__m128d vecDev0, vecDev1;
	__m128d vecVar = _mm_setzero_pd();
	const __m128d vecCountMinus1 = _mm_set1_pd(static_cast<compv_float64_t>(count - 1)); // asm: ctv(epi32, double) and no need to duplicate
	const __m128d vecMean = _mm_set1_pd(*mean1); // asm: load_sd() then shufle
	for (i = 0; i < countSigned - 3; i += 4) {
		vecDev0 = _mm_sub_pd(_mm_load_pd(&data[i]), vecMean);
		vecDev1 = _mm_sub_pd(_mm_load_pd(&data[i + 2]), vecMean);
		vecVar = _mm_add_pd(vecVar, _mm_mul_pd(vecDev0, vecDev0));
		vecVar = _mm_add_pd(vecVar, _mm_mul_pd(vecDev1, vecDev1));
	}
	if (i < countSigned - 1) {
		vecDev0 = _mm_sub_pd(_mm_load_pd(&data[i]), vecMean);
		vecVar = _mm_add_pd(vecVar, _mm_mul_pd(vecDev0, vecDev0));
		i += 2;
	}
	if (countSigned & 1) {
		vecDev0 = _mm_sub_sd(_mm_load_sd(&data[i]), vecMean);
		vecVar = _mm_add_sd(vecVar, _mm_mul_sd(vecDev0, vecDev0));
	}
	vecVar = _mm_add_sd(vecVar, _mm_shuffle_pd(vecVar, vecVar, 0x1));
	vecVar = _mm_div_sd(vecVar, vecCountMinus1);
	_mm_store_sd(var1, vecVar);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
