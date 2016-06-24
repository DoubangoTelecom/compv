/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_transform_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "src" and "dst" must have the same stride and "strideInBytes" must be SSE-aligned
// AVX version not faster
void TransformHomogeneousToCartesian2D_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* srcZ, COMPV_ALIGNED(SSE) compv_float64_t* dstX, COMPV_ALIGNED(SSE) compv_float64_t* dstY, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): add ASM
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i, numPoints_ = static_cast<compv_uscalar_t>(numPoints);

	__m128d xmm0, xmm1, xmm2, xmm3;
	const __m128d xmmOne = _mm_set1_pd(1.);

	for (i = 0; i < numPoints_ - 7; i += 8) {
		xmm0 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[i]));
		xmm1 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[i + 2]));
		xmm2 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[i + 4]));
		xmm3 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[i + 6]));
		_mm_store_pd(&dstX[i], _mm_mul_pd(_mm_load_pd(&srcX[i]), xmm0));
		_mm_store_pd(&dstX[i + 2], _mm_mul_pd(_mm_load_pd(&srcX[i + 2]), xmm1));
		_mm_store_pd(&dstX[i + 4], _mm_mul_pd(_mm_load_pd(&srcX[i + 4]), xmm2));
		_mm_store_pd(&dstX[i + 6], _mm_mul_pd(_mm_load_pd(&srcX[i + 6]), xmm3));
		_mm_store_pd(&dstY[i], _mm_mul_pd(_mm_load_pd(&srcY[i]), xmm0));
		_mm_store_pd(&dstY[i + 2], _mm_mul_pd(_mm_load_pd(&srcY[i + 2]), xmm1));
		_mm_store_pd(&dstY[i + 4], _mm_mul_pd(_mm_load_pd(&srcY[i + 4]), xmm2));
		_mm_store_pd(&dstY[i + 6], _mm_mul_pd(_mm_load_pd(&srcY[i + 6]), xmm3));
	}
	if (i < numPoints_ - 3) {
		xmm0 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[i]));
		xmm1 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[i + 2]));
		_mm_store_pd(&dstX[i], _mm_mul_pd(_mm_load_pd(&srcX[i]), xmm0));
		_mm_store_pd(&dstX[i + 2], _mm_mul_pd(_mm_load_pd(&srcX[i + 2]), xmm1));
		_mm_store_pd(&dstY[i], _mm_mul_pd(_mm_load_pd(&srcY[i]), xmm0));
		_mm_store_pd(&dstY[i + 2], _mm_mul_pd(_mm_load_pd(&srcY[i + 2]), xmm1));
		i += 4;
	}
	if (i < numPoints_ - 1) {
		xmm0 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[i]));
		_mm_store_pd(&dstX[i], _mm_mul_pd(_mm_load_pd(&srcX[i]), xmm0));
		_mm_store_pd(&dstY[i], _mm_mul_pd(_mm_load_pd(&srcY[i]), xmm0));
		i += 2;
	}
	if (numPoints_ & 1) {
		xmm0 = _mm_div_sd(xmmOne, _mm_load_sd(&srcZ[i]));
		_mm_store_sd(&dstX[i], _mm_mul_sd(_mm_load_sd(&srcX[i]), xmm0));
		_mm_store_sd(&dstY[i], _mm_mul_pd(_mm_load_sd(&srcY[i]), xmm0));
	}
}

// numPoints = 4 (very common: transforming a rectangular box)
void TransformHomogeneousToCartesian2D_4_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* srcZ, COMPV_ALIGNED(SSE) compv_float64_t* dstX, COMPV_ALIGNED(SSE) compv_float64_t* dstY, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): add ASM
	COMPV_DEBUG_INFO_CHECK_SSE2();

	const __m128d xmmOne = _mm_load_pd(k1_f64);
	__m128d xmm0 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[0]));
	__m128d xmm1 = _mm_div_pd(xmmOne, _mm_load_pd(&srcZ[2]));
	_mm_store_pd(&dstX[0], _mm_mul_pd(_mm_load_pd(&srcX[0]), xmm0));
	_mm_store_pd(&dstX[2], _mm_mul_pd(_mm_load_pd(&srcX[2]), xmm1));
	_mm_store_pd(&dstY[0], _mm_mul_pd(_mm_load_pd(&srcY[0]), xmm0));
	_mm_store_pd(&dstY[2], _mm_mul_pd(_mm_load_pd(&srcY[2]), xmm1));
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
