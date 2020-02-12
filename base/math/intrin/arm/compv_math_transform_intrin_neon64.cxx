/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_transform_intrin_neon64.h"

#if COMPV_ARCH_ARM64 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "src" and "dst" must have the same stride and "strideInBytes" must be SSE-aligned
// AVX version not faster
void CompVMathTransformHomogeneousToCartesian2D_64f_Intrin_NEON64(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* srcZ, COMPV_ALIGNED(SSE) compv_float64_t* dstX, COMPV_ALIGNED(SSE) compv_float64_t* dstY, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementaion found"); // no ASM implementation: for now the function isn't used
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_scalar_t i, numPoints_ = static_cast<compv_scalar_t>(numPoints);

	float64x2_t vec0, vec1, vec2, vec3;
	const float64x2_t vecOne = vdupq_n_f64(1.);

	for (i = 0; i < numPoints_ - 7; i += 8) {
		vec0 = vdivq_f64(vecOne, vld1q_f64(&srcZ[i]));
		vec1 = vdivq_f64(vecOne, vld1q_f64(&srcZ[i + 2]));
		vec2 = vdivq_f64(vecOne, vld1q_f64(&srcZ[i + 4]));
		vec3 = vdivq_f64(vecOne, vld1q_f64(&srcZ[i + 6]));
		vst1q_f64(&dstX[i], vmulq_f64(vld1q_f64(&srcX[i]), vec0));
		vst1q_f64(&dstX[i + 2], vmulq_f64(vld1q_f64(&srcX[i + 2]), vec1));
		vst1q_f64(&dstX[i + 4], vmulq_f64(vld1q_f64(&srcX[i + 4]), vec2));
		vst1q_f64(&dstX[i + 6], vmulq_f64(vld1q_f64(&srcX[i + 6]), vec3));
		vst1q_f64(&dstY[i], vmulq_f64(vld1q_f64(&srcY[i]), vec0));
		vst1q_f64(&dstY[i + 2], vmulq_f64(vld1q_f64(&srcY[i + 2]), vec1));
		vst1q_f64(&dstY[i + 4], vmulq_f64(vld1q_f64(&srcY[i + 4]), vec2));
		vst1q_f64(&dstY[i + 6], vmulq_f64(vld1q_f64(&srcY[i + 6]), vec3));
	}
	if (i < numPoints_ - 3) {
		vec0 = vdivq_f64(vecOne, vld1q_f64(&srcZ[i]));
		vec1 = vdivq_f64(vecOne, vld1q_f64(&srcZ[i + 2]));
		vst1q_f64(&dstX[i], vmulq_f64(vld1q_f64(&srcX[i]), vec0));
		vst1q_f64(&dstX[i + 2], vmulq_f64(vld1q_f64(&srcX[i + 2]), vec1));
		vst1q_f64(&dstY[i], vmulq_f64(vld1q_f64(&srcY[i]), vec0));
		vst1q_f64(&dstY[i + 2], vmulq_f64(vld1q_f64(&srcY[i + 2]), vec1));
		i += 4;
	}
	if (i < numPoints_ - 1) {
		vec0 = vdivq_f64(vecOne, vld1q_f64(&srcZ[i]));
		vst1q_f64(&dstX[i], vmulq_f64(vld1q_f64(&srcX[i]), vec0));
		vst1q_f64(&dstY[i], vmulq_f64(vld1q_f64(&srcY[i]), vec0));
		i += 2;
	}
	if (numPoints_ & 1) {
		float64x1_t vec0n = vdiv_f64(vget_low_f64(vecOne), vld1_f64(&srcZ[i]));
		vst1_f64(&dstX[i], vmul_f64(vld1_f64(&srcX[i]), vec0n));
		vst1_f64(&dstY[i], vmul_f64(vld1_f64(&srcY[i]), vec0n));
	}
}

// numPoints = 4 (very common: transforming a rectangular box)
void CompVMathTransformHomogeneousToCartesian2D_4_64f_Intrin_NEON64(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* srcZ, COMPV_ALIGNED(SSE) compv_float64_t* dstX, COMPV_ALIGNED(SSE) compv_float64_t* dstY, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const float64x2_t vecOne = vld1q_f64(k1_64f);
	const float64x2_t vec0 = vdivq_f64(vecOne, vld1q_f64(&srcZ[0]));
	const float64x2_t vec1 = vdivq_f64(vecOne, vld1q_f64(&srcZ[2]));
	vst1q_f64(&dstX[0], vmulq_f64(vld1q_f64(&srcX[0]), vec0));
	vst1q_f64(&dstX[2], vmulq_f64(vld1q_f64(&srcX[2]), vec1));
	vst1q_f64(&dstY[0], vmulq_f64(vld1q_f64(&srcY[0]), vec0));
	vst1q_f64(&dstY[2], vmulq_f64(vld1q_f64(&srcY[2]), vec1));
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */