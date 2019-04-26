/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_histogram_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathHistogramBuildProjectionX_8u32s_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* ptrIn, COMPV_ALIGNED(NEON) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride)
{
	COMPV_ASSERT(width >= 16);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM faster");
	const compv_uscalar_t width16 = width & -16;
	compv_uscalar_t i;
	uint8x16_t vec0, vec1;
	uint32_t* ptrOut_ = reinterpret_cast<uint32_t*>(ptrOut);

	/* Copy first row (to avoid using memset(0)) */
	for (i = 0; i < width16; i += 16) {
		// int32_t <- uint8_t
		vec0 = vld1q_u8(&ptrIn[i]);
		vec1 = vmovl_u8(vget_high_u8(vec0));
		vec0 = vmovl_u8(vget_low_u8(vec0));
		vst1q_u32(&ptrOut_[i], vmovl_u16(vget_low_u16(vec0)));
		vst1q_u32(&ptrOut_[i + 4], vmovl_u16(vget_high_u16(vec0)));
		vst1q_u32(&ptrOut_[i + 8], vmovl_u16(vget_low_u16(vec1)));
		vst1q_u32(&ptrOut_[i + 12], vmovl_u16(vget_high_u16(vec1)));
	}
	for (; i < width; ++i) {
		ptrOut_[i] = ptrIn[i]; // int32_t <- uint8_t
	}
	ptrIn += stride;
	/* Other rows */
	for (compv_uscalar_t j = 1; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			vec0 = vld1q_u8(&ptrIn[i]);
			vec1 = vmovl_u8(vget_high_u8(vec0));
			vec0 = vmovl_u8(vget_low_u8(vec0));
			vst1q_u32(&ptrOut_[i],
				vaddw_u16(
					vld1q_u32(&ptrOut_[i]), // Perf issue, do not load ptrOut -> see ASM/AVX2 code
					vget_low_u16(vec0))
			);
			vst1q_u32(&ptrOut_[i + 4],
				vaddw_u16(
					vld1q_u32(&ptrOut_[i + 4]),
					vget_high_u16(vec0))
			);
			vst1q_u32(&ptrOut_[i + 8],
				vaddw_u16(
					vld1q_u32(&ptrOut_[i + 8]),
					vget_low_u16(vec1))
			);
			vst1q_u32(&ptrOut_[i + 12],
				vaddw_u16(
					vld1q_u32(&ptrOut_[i + 12]),
					vget_high_u16(vec1))
			);
		}
		for (; i < width; ++i) {
			ptrOut_[i] += ptrIn[i]; // int32_t <- uint8_t
		}
		ptrIn += stride;
	}
}

void CompVMathHistogramBuildProjectionY_8u32s_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* ptrIn, COMPV_ALIGNED(NEON) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_ASSERT(width >= 16);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	int32_t sum;
	compv_uscalar_t i;
	const compv_uscalar_t width16 = width & -16;
	uint8x16_t vec0, vec1, vec2, vec3, vec4, vec5;
	uint32x2_t vec0n;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		// int32_t <- uint8_t
		vec1 = vld1q_u8(&ptrIn[0]);
		vec3 = vmovl_u8(vget_high_u8(vec1));
		vec1 = vmovl_u8(vget_low_u8(vec1));
		vec0 = vmovl_u16(vget_low_u16(vec1));
		vec1 = vmovl_u16(vget_high_u16(vec1));
		vec2 = vmovl_u16(vget_low_u16(vec3));
		vec3 = vmovl_u16(vget_high_u16(vec3));
		for (i = 16; i < width16; i += 16) {
			vec4 = vld1q_u8(&ptrIn[i]);
			vec5 = vmovl_u8(vget_high_u8(vec4));
			vec4 = vmovl_u8(vget_low_u8(vec4));
			vec0 = vaddw_u16(vec0, vget_low_u16(vec4));
			vec1 = vaddw_u16(vec1, vget_high_u16(vec4));
			vec2 = vaddw_u16(vec2, vget_low_u16(vec5));
			vec3 = vaddw_u16(vec3, vget_high_u16(vec5));
		}
		vec0 = vaddq_u32(vec0, vec2);
		vec1 = vaddq_u32(vec1, vec3);
		vec0 = vaddq_u32(vec0, vec1);
		vec0n = vpadd_u32(vget_low_u32(vec0), vget_high_u32(vec0));
		vec0n = vpadd_u32(vec0n, vec0n);
		sum = vget_lane_u32(vec0n, 0);

		for (; i < width; ++i) {
			sum += ptrIn[i]; // int32_t <- uint8_t
		}

		ptrIn += stride;
		ptrOut[j] = sum;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
