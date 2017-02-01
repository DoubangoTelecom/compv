/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_convlt_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathConvlt1VtHz_8u32f8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row, stride = width + pad;
	uint8x16_t vecInPtr, vec0i, vec1i, vec2i, vec3i;
	float32x4_t vecCoeff, vecSum0, vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	float32x2_t vecSum0n, vecCoeffn;
	bool bOutptrIs4BytesAligned = COMPV_IS_ALIGNED(outPtr, 4); // to avoid bus error when casting as 'uint32_t'

	for (j = 0; j < height; ++j) {
		/* Per #16 bytes */
		for (i = 0; i < width - 15; i += 16) {
			vecSum0 = vdupq_n_f32(0);
			vecSum1 = vdupq_n_f32(0);
			vecSum2 = vdupq_n_f32(0);
			vecSum3 = vdupq_n_f32(0);
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = vld1q_u8(&inPtr[i + k]);
				vecCoeff = vdupq_n_f32(vthzKernPtr[row]);
				vec2i = vmovl_u8(vget_low_u8(vecInPtr)); // epi8 -> epi16
				vec3i = vmovl_u8(vget_high_u8(vecInPtr)); // epi8 -> epi16
				vec0i = vmovl_u16(vget_low_u16(vec2i)); // epi16 -> epi32
				vec1i = vmovl_u16(vget_high_u16(vec2i)); // epi16 -> epi32
				vec2i = vmovl_u16(vget_low_u16(vec3i)); // epi16 -> epi32
				vec3i = vmovl_u16(vget_high_u16(vec3i)); // epi16 -> epi32
				vec0f = vcvtq_f32_u32(vec0i);
				vec1f = vcvtq_f32_u32(vec1i);
				vec2f = vcvtq_f32_u32(vec2i);
				vec3f = vcvtq_f32_u32(vec3i);
				vecSum0 = vmlaq_f32(vecSum0, vec0f, vecCoeff);
				vecSum1 = vmlaq_f32(vecSum1, vec1f, vecCoeff);
				vecSum2 = vmlaq_f32(vecSum2, vec2f, vecCoeff);
				vecSum3 = vmlaq_f32(vecSum3, vec3f, vecCoeff);
			}
			vec0i = vcvtq_u32_f32(vecSum0);
			vec1i = vcvtq_u32_f32(vecSum1);
			vec2i = vcvtq_u32_f32(vecSum2);
			vec3i = vcvtq_u32_f32(vecSum3);
			vec0i = vcombine_u16(vmovn_u32(vec0i), vmovn_u32(vec1i));
			vec2i = vcombine_u16(vmovn_u32(vec2i), vmovn_u32(vec3i));
			vec0i = vcombine_u8(vmovn_u16(vec0i), vmovn_u16(vec2i));
			vst1q_u8(&outPtr[i], vec0i);
		}

		/* Per #4 bytes */
		for (; i < width - 3; i += 4) {
			vecSum0 = vdupq_n_f32(0);
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = vld1q_u8(&inPtr[i + k]);
				vecCoeff = vdupq_n_f32(vthzKernPtr[row]);
				vec0f = vcvtq_f32_u32(vmovl_u16(vget_low_u16(vmovl_u8(vget_low_u8(vecInPtr)))));
				vecSum0 = vmlaq_f32(vecSum0, vec0f, vecCoeff);
			}
			vec0i = vcvtq_u32_f32(vecSum0);
			vec0i = vcombine_u16(vmovn_u32(vec0i), vmovn_u32(vec0i));
			vec0i = vcombine_u8(vmovn_u16(vec0i), vmovn_u16(vec0i));
			if (bOutptrIs4BytesAligned) {
				*reinterpret_cast<uint32_t*>(&outPtr[i]) = vgetq_lane_u32(vec0i, 0);
			}
			else {
				outPtr[i + 0] = vgetq_lane_u8(vec0i, 0);
				outPtr[i + 1] = vgetq_lane_u8(vec0i, 1);
				outPtr[i + 2] = vgetq_lane_u8(vec0i, 2);
				outPtr[i + 3] = vgetq_lane_u8(vec0i, 3);
			}
		}

		/* Per #1 bytes */
		for (; i < width; i += 1) {
			vecSum0n = vdup_n_f32(0);
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecCoeffn = vdup_n_f32(vthzKernPtr[row]);
				vecSum0n = vmla_n_f32(vecSum0n, vecCoeffn, static_cast<compv_float32_t>(inPtr[i + k]));
			}
			outPtr[i] = static_cast<uint8_t>(vget_lane_f32(vecSum0n, 0));
		}

		inPtr += stride;
		outPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
