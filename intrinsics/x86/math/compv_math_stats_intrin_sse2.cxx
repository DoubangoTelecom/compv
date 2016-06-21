/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_stats_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void MathStatsNormalize2DHartley_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	compv_scalar_t i, numPoints_ = static_cast<compv_uscalar_t>(numPoints);

	__m128d xmm0, xmm1, xmm2, xmm3;
	__m128d xmmTx = _mm_setzero_pd();
	__m128d xmmTy = _mm_setzero_pd();
	__m128d xmmMagnitude = _mm_setzero_pd();
	const __m128d xmmOneOverNumPoints = _mm_div_pd(_mm_set1_pd(1.), _mm_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m128d xmmSqrt2 = _mm_set1_pd(COMPV_MATH_SQRT_2);

	/* TX and TY */

	for (i = 0; i < numPoints_ - 7; i += 8) {
		xmmTx = _mm_add_pd(_mm_add_pd(_mm_add_pd(_mm_load_pd(&x[i]), _mm_load_pd(&x[i + 2])),
			_mm_add_pd(_mm_load_pd(&x[i + 4]), _mm_load_pd(&x[i + 6]))), xmmTx);
		xmmTy = _mm_add_pd(_mm_add_pd(_mm_add_pd(_mm_load_pd(&y[i]), _mm_load_pd(&y[i + 2])),
			_mm_add_pd(_mm_load_pd(&y[i + 4]), _mm_load_pd(&y[i + 6]))), xmmTy);
	}
	if (i < numPoints_ - 3) {
		xmmTx = _mm_add_pd(_mm_add_pd(_mm_load_pd(&x[i]), _mm_load_pd(&x[i + 2])), xmmTx);
		xmmTy = _mm_add_pd(_mm_add_pd(_mm_load_pd(&y[i]), _mm_load_pd(&y[i + 2])), xmmTy);
		i += 4;
	}
	if (i < numPoints_ - 1) {
		xmmTx = _mm_add_pd(_mm_load_pd(&x[i]), xmmTx);
		xmmTy = _mm_add_pd(_mm_load_pd(&y[i]), xmmTy);
		i += 2;
	}
	if (numPoints_ & 1) {
		xmmTx = _mm_add_sd(xmmTx, _mm_load_sd(&x[i]));
		xmmTy = _mm_add_sd(xmmTy, _mm_load_sd(&y[i]));
	}
	xmmTx = _mm_mul_sd(_mm_add_sd(xmmTx, _mm_shuffle_pd(xmmTx, xmmTx, 0x1)), xmmOneOverNumPoints); // horiz add and result in first double
	xmmTy = _mm_mul_sd(_mm_add_sd(xmmTy, _mm_shuffle_pd(xmmTy, xmmTy, 0x1)), xmmOneOverNumPoints); // horiz add and result in first double
	xmmTx = _mm_shuffle_pd(xmmTx, xmmTx, 0x0); // duplicate first double
	xmmTy = _mm_shuffle_pd(xmmTy, xmmTy, 0x0); // duplicate first double

	/* Magnitude */

	for (i = 0; i < numPoints_ - 3; i += 4) {
		xmm0 = _mm_sub_pd(_mm_load_pd(&x[i]), xmmTx);
		xmm1 = _mm_sub_pd(_mm_load_pd(&y[i]), xmmTy);
		xmm2 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(xmm0, xmm0), _mm_mul_pd(xmm1, xmm1)));
		xmm0 = _mm_sub_pd(_mm_load_pd(&x[i + 2]), xmmTx);
		xmm1 = _mm_sub_pd(_mm_load_pd(&y[i + 2]), xmmTy);
		xmm3 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(xmm0, xmm0), _mm_mul_pd(xmm1, xmm1)));
		xmmMagnitude = _mm_add_pd(xmm2, xmmMagnitude);
		xmmMagnitude = _mm_add_pd(xmm3, xmmMagnitude);
	}
	if (i < numPoints_ - 1) {
		xmm0 = _mm_sub_pd(_mm_load_pd(&x[i]), xmmTx);
		xmm1 = _mm_sub_pd(_mm_load_pd(&y[i]), xmmTy);
		xmm2 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(xmm0, xmm0), _mm_mul_pd(xmm1, xmm1)));
		xmmMagnitude = _mm_add_pd(xmm2, xmmMagnitude);
		i += 2;
	}
	if (numPoints_ & 1) {
		// asm: notice the "sd" for all functions -> the high double must be ignored (replaced with zero)
		xmm0 = _mm_sub_sd(_mm_load_sd(&x[i]), xmmTx);
		xmm1 = _mm_sub_sd(_mm_load_sd(&y[i]), xmmTy);
		xmm2 = _mm_add_sd(_mm_mul_sd(xmm0, xmm0), _mm_mul_sd(xmm1, xmm1));
		xmm3 = _mm_sqrt_sd(xmm2, xmm2);
		xmmMagnitude = _mm_add_sd(xmmMagnitude, xmm3);
	}
	xmmMagnitude = _mm_mul_sd(_mm_add_sd(xmmMagnitude, _mm_shuffle_pd(xmmMagnitude, xmmMagnitude, 0x1)), xmmOneOverNumPoints); // horiz add and result in first double
	xmmMagnitude = _mm_div_sd(xmmSqrt2, xmmMagnitude);
	
	_mm_store_sd(tx1, xmmTx);
	_mm_store_sd(ty1, xmmTy);
	_mm_store_sd(s1, xmmMagnitude);
}

// "numPoints = 4" -> Very common (Homography)
void MathStatsNormalize2DHartley_4_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	const __m128d xmmOneOverNumPoints = _mm_div_pd(_mm_set1_pd(1.), _mm_set1_pd(static_cast<compv_float64_t>(numPoints)));
	const __m128d xmmSqrt2 = _mm_set1_pd(COMPV_MATH_SQRT_2);

	__m128d xmmTx = _mm_add_pd(_mm_load_pd(&x[0]), _mm_load_pd(&x[2]));
	__m128d xmmTy = _mm_add_pd(_mm_load_pd(&y[0]), _mm_load_pd(&y[2]));
	xmmTx = _mm_mul_sd(_mm_add_sd(xmmTx, _mm_shuffle_pd(xmmTx, xmmTx, 0x1)), xmmOneOverNumPoints);
	xmmTy = _mm_mul_sd(_mm_add_sd(xmmTy, _mm_shuffle_pd(xmmTy, xmmTy, 0x1)), xmmOneOverNumPoints);
	xmmTx = _mm_shuffle_pd(xmmTx, xmmTx, 0x0);
	xmmTy = _mm_shuffle_pd(xmmTy, xmmTy, 0x0);

	__m128d xmm0 = _mm_sub_pd(_mm_load_pd(&x[0]), xmmTx);
	__m128d xmm1 = _mm_sub_pd(_mm_load_pd(&y[0]), xmmTy);
	__m128d xmmMagnitude = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(xmm0, xmm0), _mm_mul_pd(xmm1, xmm1)));
	xmm0 = _mm_sub_pd(_mm_load_pd(&x[2]), xmmTx);
	xmm1 = _mm_sub_pd(_mm_load_pd(&y[2]), xmmTy);
	__m128d xmm2 = _mm_sqrt_pd(_mm_add_pd(_mm_mul_pd(xmm0, xmm0), _mm_mul_pd(xmm1, xmm1)));
	xmmMagnitude = _mm_add_pd(xmm2, xmmMagnitude);
	xmmMagnitude = _mm_mul_sd(_mm_add_sd(xmmMagnitude, _mm_shuffle_pd(xmmMagnitude, xmmMagnitude, 0x1)), xmmOneOverNumPoints);
	xmmMagnitude = _mm_div_sd(xmmSqrt2, xmmMagnitude);

	_mm_store_sd(tx1, xmmTx);
	_mm_store_sd(ty1, xmmTy);
	_mm_store_sd(s1, xmmMagnitude);
}

void MathStatsMSE2DHomogeneous_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	compv_scalar_t numPointsSigned = static_cast<compv_scalar_t>(numPoints), i;
	__m128d xmmEX0, xmmEY0, xmmScale0, xmmEX1, xmmEY1, xmmScale1;
	const __m128d xmmOne = _mm_set1_pd(1.);

	for (i = 0; i < numPointsSigned - 3; i += 4) {
		xmmScale0 = _mm_div_pd(xmmOne, _mm_load_pd(&aZ_h[i]));
		xmmScale1 = _mm_div_pd(xmmOne, _mm_load_pd(&aZ_h[i + 2]));
		xmmEX0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[i]), xmmScale0), _mm_load_pd(&bX[i]));
		xmmEX1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[i + 2]), xmmScale1), _mm_load_pd(&bX[i + 2]));
		xmmEY0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[i]), xmmScale0), _mm_load_pd(&bY[i]));
		xmmEY1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[i + 2]), xmmScale1), _mm_load_pd(&bY[i + 2]));
		_mm_store_pd(&mse[i], _mm_add_pd(_mm_mul_pd(xmmEX0, xmmEX0), _mm_mul_pd(xmmEY0, xmmEY0)));
		_mm_store_pd(&mse[i + 2], _mm_add_pd(_mm_mul_pd(xmmEX1, xmmEX1), _mm_mul_pd(xmmEY1, xmmEY1)));
	}
	if (i < numPointsSigned - 1) {
		xmmScale0 = _mm_div_pd(xmmOne, _mm_load_pd(&aZ_h[i]));
		xmmEX0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[i]), xmmScale0), _mm_load_pd(&bX[i]));
		xmmEY0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[i]), xmmScale0), _mm_load_pd(&bY[i]));
		_mm_store_pd(&mse[i], _mm_add_pd(_mm_mul_pd(xmmEX0, xmmEX0), _mm_mul_pd(xmmEY0, xmmEY0)));
		i += 2;
	}
	if (numPointsSigned & 1) {
		xmmScale0 = _mm_div_sd(xmmOne, _mm_load_sd(&aZ_h[i]));
		xmmEX0 = _mm_sub_sd(_mm_mul_sd(_mm_load_sd(&aX_h[i]), xmmScale0), _mm_load_sd(&bX[i]));
		xmmEY0 = _mm_sub_sd(_mm_mul_sd(_mm_load_sd(&aY_h[i]), xmmScale0), _mm_load_sd(&bY[i]));
		_mm_store_sd(&mse[i], _mm_add_sd(_mm_mul_sd(xmmEX0, xmmEX0), _mm_mul_sd(xmmEY0, xmmEY0)));
	}
}

// "numPoints = 4" -> Very common (Homography)
void MathStatsMSE2DHomogeneous_4_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM
	__m128d xmmEX0, xmmEY0, xmmScale0, xmmEX1, xmmEY1, xmmScale1;
	const __m128d xmmOne = _mm_set1_pd(1.);
	
	xmmScale0 = _mm_div_pd(xmmOne, _mm_load_pd(&aZ_h[0]));
	xmmScale1 = _mm_div_pd(xmmOne, _mm_load_pd(&aZ_h[2]));
	xmmEX0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[0]), xmmScale0), _mm_load_pd(&bX[0]));
	xmmEX1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aX_h[2]), xmmScale1), _mm_load_pd(&bX[2]));
	xmmEY0 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[0]), xmmScale0), _mm_load_pd(&bY[0]));
	xmmEY1 = _mm_sub_pd(_mm_mul_pd(_mm_load_pd(&aY_h[2]), xmmScale1), _mm_load_pd(&bY[2]));
	_mm_store_pd(&mse[0], _mm_add_pd(_mm_mul_pd(xmmEX0, xmmEX0), _mm_mul_pd(xmmEY0, xmmEY0)));
	_mm_store_pd(&mse[2], _mm_add_pd(_mm_mul_pd(xmmEX1, xmmEX1), _mm_mul_pd(xmmEY1, xmmEY1)));	
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
