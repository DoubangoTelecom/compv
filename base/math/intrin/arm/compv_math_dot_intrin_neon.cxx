/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_dot_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ARCH_ARM64

// Must not require memory alignment (random access from SVM)
void CompVMathDotDot_64f64f_Intrin_NEON64(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM implementation faster");
	
	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width2 = width & -2;
	compv_uscalar_t i;
	float64x2_t vecSum0 = vdupq_n_f64(0.0);
	float64x2_t vecSum1 = vdupq_n_f64(0.0);

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) { // test "width16, width16"
			// TODO(dmi): Add FMA implementation (see ASM code)
			float64x2_t vec0 = vmulq_f64(vld1q_f64(&ptrA[i]), vld1q_f64(&ptrB[i])); // unaligned load
			float64x2_t vec1 = vmulq_f64(vld1q_f64(&ptrA[i + 2]), vld1q_f64(&ptrB[i + 2]));
			float64x2_t vec2 = vmulq_f64(vld1q_f64(&ptrA[i + 4]), vld1q_f64(&ptrB[i + 4]));
			float64x2_t vec3 = vmulq_f64(vld1q_f64(&ptrA[i + 6]), vld1q_f64(&ptrB[i + 6]));
			float64x2_t vec4 = vmulq_f64(vld1q_f64(&ptrA[i + 8]), vld1q_f64(&ptrB[i + 8]));
			float64x2_t vec5 = vmulq_f64(vld1q_f64(&ptrA[i + 10]), vld1q_f64(&ptrB[i + 10]));
			float64x2_t vec6 = vmulq_f64(vld1q_f64(&ptrA[i + 12]), vld1q_f64(&ptrB[i + 12]));
			float64x2_t vec7 = vmulq_f64(vld1q_f64(&ptrA[i + 14]), vld1q_f64(&ptrB[i + 14]));
			vec0 = vaddq_f64(vec0, vec2);
			vec4 = vaddq_f64(vec4, vec6);
			vec1 = vaddq_f64(vec1, vec3);
			vec5 = vaddq_f64(vec5, vec7);
			vec0 = vaddq_f64(vec0, vec4);
			vec1 = vaddq_f64(vec1, vec5);
			vecSum0 = vaddq_f64(vecSum0, vec0);
			vecSum1 = vaddq_f64(vecSum1, vec1);
		}
		for (; i < width2; i += 2) { // not "test width2, width2" but "cmp i, width2"
			float64x2_t vec0 = vmulq_f64(vld1q_f64(&ptrA[i]), vld1q_f64(&ptrB[i])); // unaligned load
			vecSum0 = vaddq_f64(vecSum0, vec0);
		}
		if (width != width2) {
			float64x1_t vec0 = vmul_f64(vld1_f64(&ptrA[i]), vld1_f64(&ptrB[i])); // unaligned load
			vec0 = vadd_f64(vec0, vget_low_f64(vecSum0));
			vecSum0 = vcombine_f64(vec0, vget_high_f64(vecSum0));
		}
		ptrA += strideA;
		ptrB += strideB;
	}

	vecSum0 = vaddq_f64(vecSum0, vecSum1);
	const float64x1_t vec0 = vadd_f64(vget_low_f64(vecSum0), vget_high_f64(vecSum0));
	vst1_f64(ret, vec0);
}

// Must not require memory alignment (random access from SVM)
void CompVMathDotDotSub_64f64f_Intrin_NEON64(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM implementation faster");
	
	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t width2 = width & -2;
	compv_uscalar_t i;
	float64x2_t vecSum0 = vdupq_n_f64(0.0);
	float64x2_t vecSum1 = vdupq_n_f64(0.0);

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			float64x2_t vec0 = vsubq_f64(vld1q_f64(&ptrA[i]), vld1q_f64(&ptrB[i]));
			float64x2_t vec1 = vsubq_f64(vld1q_f64(&ptrA[i + 2]), vld1q_f64(&ptrB[i + 2]));
			float64x2_t vec2 = vsubq_f64(vld1q_f64(&ptrA[i + 4]), vld1q_f64(&ptrB[i + 4]));
			float64x2_t vec3 = vsubq_f64(vld1q_f64(&ptrA[i + 6]), vld1q_f64(&ptrB[i + 6]));
			float64x2_t vec4 = vsubq_f64(vld1q_f64(&ptrA[i + 8]), vld1q_f64(&ptrB[i + 8]));
			float64x2_t vec5 = vsubq_f64(vld1q_f64(&ptrA[i + 10]), vld1q_f64(&ptrB[i + 10]));
			float64x2_t vec6 = vsubq_f64(vld1q_f64(&ptrA[i + 12]), vld1q_f64(&ptrB[i + 12]));
			float64x2_t vec7 = vsubq_f64(vld1q_f64(&ptrA[i + 14]), vld1q_f64(&ptrB[i + 14]));
			// TODO(dmi): Add FMA implementation (see ASM code)
			vec0 = vmulq_f64(vec0, vec0);
			vec2 = vmulq_f64(vec2, vec2);
			vec4 = vmulq_f64(vec4, vec4);
			vec6 = vmulq_f64(vec6, vec6);
			vec1 = vmulq_f64(vec1, vec1);
			vec3 = vmulq_f64(vec3, vec3);
			vec5 = vmulq_f64(vec5, vec5);
			vec7 = vmulq_f64(vec7, vec7);
			vec0 = vaddq_f64(vec0, vec2);
			vec4 = vaddq_f64(vec4, vec6);
			vec1 = vaddq_f64(vec1, vec3);
			vec5 = vaddq_f64(vec5, vec7);
			vec0 = vaddq_f64(vec0, vec4);
			vec1 = vaddq_f64(vec1, vec5);
			vecSum0 = vaddq_f64(vecSum0, vec0);
			vecSum1 = vaddq_f64(vecSum1, vec1);
		}
		for (; i < width2; i += 2) {
			float64x2_t vec0 = vsubq_f64(vld1q_f64(&ptrA[i]), vld1q_f64(&ptrB[i]));
			vec0 = vmulq_f64(vec0, vec0);
			vecSum0 = vaddq_f64(vecSum0, vec0);
		}
		if (width != width2) {
			float64x1_t vec0 = vsub_f64(vld1_f64(&ptrA[i]), vld1_f64(&ptrB[i]));
			vec0 = vmul_f64(vec0, vec0);
			vec0 = vadd_f64(vec0, vget_low_f64(vecSum0));
			vecSum0 = vcombine_f64(vec0, vget_high_f64(vecSum0));
		}
		ptrA += strideA;
		ptrB += strideB;
	}

	vecSum0 = vaddq_f64(vecSum0, vecSum1);
	const float64x1_t vec0 = vadd_f64(vget_low_f64(vecSum0), vget_high_f64(vecSum0));
	vst1_f64(ret, vec0);
	
}

#endif /* COMPV_ARCH_ARM64 */

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
