/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_transform_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "src" and "dst" must have the same stride and "strideInBytes" must be SSE-aligned
// AVX version not faster
void CompVMathTransformHomogeneousToCartesian2D_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* srcZ, COMPV_ALIGNED(SSE) compv_float64_t* dstX, COMPV_ALIGNED(SSE) compv_float64_t* dstY, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementaion found"); // no ASM implementation: for now the function isn't used
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_scalar_t i, numPoints_ = static_cast<compv_scalar_t>(numPoints);

	__m128d vec0, vec1, vec2, vec3;
	const __m128d vecOne = _mm_set1_pd(1.);

	for (i = 0; i < numPoints_ - 7; i += 8) {
		vec0 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[i]));
		vec1 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[i + 2]));
		vec2 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[i + 4]));
		vec3 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[i + 6]));
		_mm_store_pd(&dstX[i], _mm_mul_pd(_mm_load_pd(&srcX[i]), vec0));
		_mm_store_pd(&dstX[i + 2], _mm_mul_pd(_mm_load_pd(&srcX[i + 2]), vec1));
		_mm_store_pd(&dstX[i + 4], _mm_mul_pd(_mm_load_pd(&srcX[i + 4]), vec2));
		_mm_store_pd(&dstX[i + 6], _mm_mul_pd(_mm_load_pd(&srcX[i + 6]), vec3));
		_mm_store_pd(&dstY[i], _mm_mul_pd(_mm_load_pd(&srcY[i]), vec0));
		_mm_store_pd(&dstY[i + 2], _mm_mul_pd(_mm_load_pd(&srcY[i + 2]), vec1));
		_mm_store_pd(&dstY[i + 4], _mm_mul_pd(_mm_load_pd(&srcY[i + 4]), vec2));
		_mm_store_pd(&dstY[i + 6], _mm_mul_pd(_mm_load_pd(&srcY[i + 6]), vec3));
	}
	if (i < numPoints_ - 3) {
		vec0 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[i]));
		vec1 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[i + 2]));
		_mm_store_pd(&dstX[i], _mm_mul_pd(_mm_load_pd(&srcX[i]), vec0));
		_mm_store_pd(&dstX[i + 2], _mm_mul_pd(_mm_load_pd(&srcX[i + 2]), vec1));
		_mm_store_pd(&dstY[i], _mm_mul_pd(_mm_load_pd(&srcY[i]), vec0));
		_mm_store_pd(&dstY[i + 2], _mm_mul_pd(_mm_load_pd(&srcY[i + 2]), vec1));
		i += 4;
	}
	if (i < numPoints_ - 1) {
		vec0 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[i]));
		_mm_store_pd(&dstX[i], _mm_mul_pd(_mm_load_pd(&srcX[i]), vec0));
		_mm_store_pd(&dstY[i], _mm_mul_pd(_mm_load_pd(&srcY[i]), vec0));
		i += 2;
	}
	if (numPoints_ & 1) {
		vec0 = _mm_div_sd(vecOne, _mm_load_sd(&srcZ[i]));
		_mm_store_sd(&dstX[i], _mm_mul_sd(_mm_load_sd(&srcX[i]), vec0));
		_mm_store_sd(&dstY[i], _mm_mul_sd(_mm_load_sd(&srcY[i]), vec0));
	}
}

// numPoints = 4 (very common: transforming a rectangular box)
void CompVMathTransformHomogeneousToCartesian2D_4_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* srcZ, COMPV_ALIGNED(SSE) compv_float64_t* dstX, COMPV_ALIGNED(SSE) compv_float64_t* dstY, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128d vecOne = _mm_load_pd(k1_64f);
	const __m128d vec0 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[0]));
	const __m128d vec1 = _mm_div_pd(vecOne, _mm_load_pd(&srcZ[2]));
	_mm_store_pd(&dstX[0], _mm_mul_pd(_mm_load_pd(&srcX[0]), vec0));
	_mm_store_pd(&dstX[2], _mm_mul_pd(_mm_load_pd(&srcX[2]), vec1));
	_mm_store_pd(&dstY[0], _mm_mul_pd(_mm_load_pd(&srcY[0]), vec0));
	_mm_store_pd(&dstY[2], _mm_mul_pd(_mm_load_pd(&srcY[2]), vec1));
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */