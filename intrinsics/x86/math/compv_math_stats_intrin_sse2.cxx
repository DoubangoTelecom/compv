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
		xmmTx = _mm_add_pd(_mm_load_sd(&x[i]), xmmTx);
		xmmTy = _mm_add_pd(_mm_load_sd(&y[i]), xmmTy);
	}
	xmmTx = _mm_mul_sd(_mm_add_pd(xmmTx, _mm_shuffle_pd(xmmTx, xmmTx, 0x1)), xmmOneOverNumPoints); // horiz add and result in first double
	xmmTy = _mm_mul_sd(_mm_add_pd(xmmTy, _mm_shuffle_pd(xmmTy, xmmTy, 0x1)), xmmOneOverNumPoints); // horiz add and result in first double
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
		// asm: notice the "sd" for all functions except the last "addpd" -> the high double must be ignored (replaced with zero)
		xmm0 = _mm_sub_sd(_mm_load_sd(&x[i]), xmmTx);
		xmm1 = _mm_sub_sd(_mm_load_sd(&y[i]), xmmTy);
		xmm2 = _mm_add_sd(_mm_mul_sd(xmm0, xmm0), _mm_mul_sd(xmm1, xmm1));
		xmm3 = _mm_sqrt_sd(xmm2, xmm2);
		xmmMagnitude = _mm_add_pd(xmm3, xmmMagnitude);
	}
	xmmMagnitude = _mm_mul_sd(_mm_add_pd(xmmMagnitude, _mm_shuffle_pd(xmmMagnitude, xmmMagnitude, 0x1)), xmmOneOverNumPoints); // horiz add and result in first double
	xmmMagnitude = _mm_div_sd(xmmSqrt2, xmmMagnitude);
	
	_mm_store_sd(tx1, xmmTx);
	_mm_store_sd(ty1, xmmTy);
	_mm_store_sd(s1, xmmMagnitude);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
