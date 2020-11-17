/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_op_sub_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathOpSubSub_32f32f32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const compv_float32_t* Aptr,
	COMPV_ALIGNED(NEON) const compv_float32_t* Bptr,
	COMPV_ALIGNED(NEON) compv_float32_t* Rptr,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Astride,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Bstride,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	const compv_uscalar_t width16 = width & -16;
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			vst1q_f32(&Rptr[i], vsubq_f32(vld1q_f32(&Aptr[i]), vld1q_f32(&Bptr[i])));
			vst1q_f32(&Rptr[i + 4], vsubq_f32(vld1q_f32(&Aptr[i + 4]), vld1q_f32(&Bptr[i + 4])));
			vst1q_f32(&Rptr[i + 8], vsubq_f32(vld1q_f32(&Aptr[i + 8]), vld1q_f32(&Bptr[i + 8])));
			vst1q_f32(&Rptr[i + 12], vsubq_f32(vld1q_f32(&Aptr[i + 12]), vld1q_f32(&Bptr[i + 12])));
		}
		for (; i < width; i += 4) {
			vst1q_f32(&Rptr[i], vsubq_f32(vld1q_f32(&Aptr[i]), vld1q_f32(&Bptr[i])));
		}
		Aptr += Astride;
		Bptr += Bstride;
		Rptr += Rstride;
	}
}

void CompVMathOpSubSubMul_32f32f32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const compv_float32_t* Aptr,
	const compv_float32_t* subVal1,
	const compv_float32_t* mulVal1,
	COMPV_ALIGNED(NEON) compv_float32_t* Rptr,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Astride,
	COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code found");
	const compv_uscalar_t width16 = width & -16;
	const float32x4_t vecSubVal = vdupq_n_f32(*subVal1);
	const float32x4_t vecMulVal = vdupq_n_f32(*mulVal1);
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			vst1q_f32(&Rptr[i], vmulq_f32(vsubq_f32(vld1q_f32(&Aptr[i]), vecSubVal), vecMulVal));
			vst1q_f32(&Rptr[i + 4], vmulq_f32(vsubq_f32(vld1q_f32(&Aptr[i + 4]), vecSubVal), vecMulVal));
			vst1q_f32(&Rptr[i + 8], vmulq_f32(vsubq_f32(vld1q_f32(&Aptr[i + 8]), vecSubVal), vecMulVal));
			vst1q_f32(&Rptr[i + 12], vmulq_f32(vsubq_f32(vld1q_f32(&Aptr[i + 12]), vecSubVal), vecMulVal));
		}
		for (; i < width; i += 4) {
			vst1q_f32(&Rptr[i], vmulq_f32(vsubq_f32(vld1q_f32(&Aptr[i]), vecSubVal), vecMulVal));
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
