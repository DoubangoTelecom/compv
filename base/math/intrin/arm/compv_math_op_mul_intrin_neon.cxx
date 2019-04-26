/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_op_mul_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathOpMulMulABt_32f32f32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const compv_float32_t* Aptr,
	COMPV_ALIGNED(NEON) const compv_float32_t* Bptr,
	COMPV_ALIGNED(NEON) compv_float32_t* Rptr,
	const compv_uscalar_t Bcols,
	const compv_uscalar_t Arows,
	const compv_uscalar_t Brows,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Astride,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Bstride,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Use ASM code with support for NEON and FMA");
	const compv_uscalar_t Bcols16 = Bcols & -16;
	const compv_uscalar_t Bcols4 = Bcols & -4;
	compv_uscalar_t k;
	for (compv_uscalar_t i = 0; i < Arows; ++i) {
		const compv_float32_t* B0ptr = Bptr;
		for (compv_uscalar_t j = 0; j < Brows; ++j) {
			float32x4_t vec0 = vdupq_n_f32(0); // TODO(dmi): for ASM, use xor to reset the register (faster)
			float32x4_t vec1 = vdupq_n_f32(0);
			float32x4_t vec2 = vdupq_n_f32(0);
			float32x4_t vec3 = vdupq_n_f32(0);
			for (k = 0; k < Bcols16; k += 16) {
				vec0 = vmlaq_f32(vec0, vld1q_f32(&Aptr[k]), vld1q_f32(&B0ptr[k]));
				vec1 = vmlaq_f32(vec1, vld1q_f32(&Aptr[k + 4]), vld1q_f32(&B0ptr[k + 4]));
				vec2 = vmlaq_f32(vec2, vld1q_f32(&Aptr[k + 8]), vld1q_f32(&B0ptr[k + 8]));
				vec3 = vmlaq_f32(vec3, vld1q_f32(&Aptr[k + 12]), vld1q_f32(&B0ptr[k + 12]));
			}
			for (; k < Bcols4; k += 4) {
				vec0 = vmlaq_f32(vec0, vld1q_f32(&Aptr[k]), vld1q_f32(&B0ptr[k]));
			}
			vec0 = vaddq_f32(vec0, vec1);
			vec2 = vaddq_f32(vec2, vec3);
			vec0 = vaddq_f32(vec0, vec2);
			float32x2_t vec0n = vadd_f32(vget_low_f32(vec0), vget_high_f32(vec0));
			vec0n = vpadd_f32(vec0n, vec0n);
			compv_float32_t sum = vget_lane_f32(vec0n, 0);
			for (; k < Bcols; k += 1) {
				sum += (Aptr[k] * B0ptr[k]);
			}
			Rptr[j] = sum;
			B0ptr += Bstride;
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
