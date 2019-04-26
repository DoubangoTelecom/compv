/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_scale_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathScaleScale_32f32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* ptrIn, COMPV_ALIGNED(NEON) compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride, const compv_float32_t* s1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");

	const compv_uscalar_t width32 = width & -32;
	const float32x4_t vecScale = vld1q_dup_f32(s1); // vld1.32 {d0[]}, [r0]
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width32; i += 32) { // test "width32, width32"
			vst1q_f32(&ptrOut[i], vmulq_f32(vld1q_f32(&ptrIn[i]), vecScale));
			vst1q_f32(&ptrOut[i + 4], vmulq_f32(vld1q_f32(&ptrIn[i + 4]), vecScale));
			vst1q_f32(&ptrOut[i + 8], vmulq_f32(vld1q_f32(&ptrIn[i + 8]), vecScale));
			vst1q_f32(&ptrOut[i + 12], vmulq_f32(vld1q_f32(&ptrIn[i + 12]), vecScale));
			vst1q_f32(&ptrOut[i + 16], vmulq_f32(vld1q_f32(&ptrIn[i + 16]), vecScale));
			vst1q_f32(&ptrOut[i + 20], vmulq_f32(vld1q_f32(&ptrIn[i + 20]), vecScale));
			vst1q_f32(&ptrOut[i + 24], vmulq_f32(vld1q_f32(&ptrIn[i + 24]), vecScale));
			vst1q_f32(&ptrOut[i + 28], vmulq_f32(vld1q_f32(&ptrIn[i + 28]), vecScale));
		}
		for (; i < width; i += 4) { // write beyond width and up to stride (requires aligned data + corrctly strided)
			vst1q_f32(&ptrOut[i], vmulq_f32(vld1q_f32(&ptrIn[i]), vecScale));
		}
		
		ptrIn += stride;
		ptrOut += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
