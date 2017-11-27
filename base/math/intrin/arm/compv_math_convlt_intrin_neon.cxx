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
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): Not optiz -> on iOS (iPhone5, ARM32, HD image, single threaded, #1000 times), asm code: 11395.ms, intrin code: 12879.ms
// TODO(dmi): Not optiz -> on iOS (iPad2 Air, ARM64, HD image, single threaded, #1000 times), asm code: 4337.ms, intrin code: 4608.ms
// TODO(dmi): Not optiz -> on Android (Huawei MediaPad2, ARM64, HD image, single threaded, #1000 times), asm code: 8390.ms, intrin code: 9620.ms
void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Intrin_NEON(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width16 = width & -16;
	uint8x16_t vecInPtr;
	uint16x8_t vec0, vec1, vec2, vec3, vecSum0 = vdupq_n_u16(0), vecSum1;
	uint16_t coeff;
	COMPV_ALIGN_NEON() uint8_t mem[16];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			vecSum0 = veorq_u16(vecSum0, vecSum0);
			vecSum1 = veorq_u16(vecSum0, vecSum0);
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = vld1q_u8(&inPtr[i + k]);
				coeff = vthzKernPtr[row];
				vec2 = vmovl_u8(vget_low_u8(vecInPtr)); // epu8 -> epu16
				vec3 = vmovl_u8(vget_high_u8(vecInPtr)); // epu8 -> epu16
				vec0 = vmull_n_u16(vget_low_u16(vec2), coeff); // epu16 -> epu32 + multiply
				vec1 = vmull_n_u16(vget_high_u16(vec2), coeff); // epu16 -> epu32 + multiply
				vec2 = vmull_n_u16(vget_low_u16(vec3), coeff); // epu16 -> epu32 + multiply
				vec3 = vmull_n_u16(vget_high_u16(vec3), coeff); // epu16 -> epu32 + multiply
				vecSum0 = vqaddq_u16(vecSum0, vcombine_u16(vshrn_n_u32(vec0, 16), vshrn_n_u32(vec1, 16)));
				vecSum1 = vqaddq_u16(vecSum1, vcombine_u16(vshrn_n_u32(vec2, 16), vshrn_n_u32(vec3, 16)));
			}
			vec0 = vcombine_u8(vqmovn_u16(vecSum0), vqmovn_u16(vecSum1));
			if (i < width16) {
				vst1q_u8(&outPtr[i], vec0);
			}
			else {
				vst1q_u8(mem, vec0);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}
		inPtr += stride;
		outPtr += stride;
	}
}

// on iOS (iPhone5, ARM32, HD image, single threaded, no FMA, #1000 times), asm code: 13049.ms, intrin code: 13316.ms
void CompVMathConvlt1VtHz_8u32f8u_Intrin_NEON(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width16 = width & -16;
	uint8x16_t vecInPtr, vec0i, vec1i, vec2i, vec3i;
	float32x4_t vecSum0 = vdupq_n_f32(0.f), vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	compv_float32_t coeff;
	COMPV_ALIGN_NEON() uint8_t mem[16];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
            vecSum0 = veorq_s32(vecSum0, vecSum0);
            vecSum1 = veorq_s32(vecSum0, vecSum0);
            vecSum2 = veorq_s32(vecSum0, vecSum0);
            vecSum3 = veorq_s32(vecSum0, vecSum0);
            for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = vld1q_u8(&inPtr[i + k]);
				coeff = vthzKernPtr[row];
				vec2i = vmovl_u8(vget_low_u8(vecInPtr)); // epu8 -> epu16
				vec3i = vmovl_u8(vget_high_u8(vecInPtr)); // epu8 -> epu16
				vec0i = vmovl_u16(vget_low_u16(vec2i)); // epu16 -> epu32
				vec1i = vmovl_u16(vget_high_u16(vec2i)); // epu16 -> epu32
				vec2i = vmovl_u16(vget_low_u16(vec3i)); // epu16 -> epu32
				vec3i = vmovl_u16(vget_high_u16(vec3i)); // epu16 -> epu32
				vec0f = vcvtq_f32_u32(vec0i);
				vec1f = vcvtq_f32_u32(vec1i);
				vec2f = vcvtq_f32_u32(vec2i);
				vec3f = vcvtq_f32_u32(vec3i);
				vecSum0 = vmlaq_n_f32(vecSum0, vec0f, coeff);
				vecSum1 = vmlaq_n_f32(vecSum1, vec1f, coeff);
				vecSum2 = vmlaq_n_f32(vecSum2, vec2f, coeff);
				vecSum3 = vmlaq_n_f32(vecSum3, vec3f, coeff);
			}
			vec0i = vcvtq_u32_f32(vecSum0);
			vec1i = vcvtq_u32_f32(vecSum1);
			vec2i = vcvtq_u32_f32(vecSum2);
			vec3i = vcvtq_u32_f32(vecSum3);
			vec0i = vcombine_u16(vqmovn_u32(vec0i), vqmovn_u32(vec1i));
			vec2i = vcombine_u16(vqmovn_u32(vec2i), vqmovn_u32(vec3i));
			vec0i = vcombine_u8(vqmovn_u16(vec0i), vqmovn_u16(vec2i));
			if (i < width16) {
				vst1q_u8(&outPtr[i], vec0i);
			}
			else {
				vst1q_u8(mem, vec0i);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
}

// TODO(dmi): optiz issue -> no perf gain compared to asm impl (iPhone5, ARM32) - no FMA
void CompVMathConvlt1VtHz_8u32f32f_Intrin_NEON(const uint8_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width16 = width & -16;
	uint8x16_t vecInPtr, vec0i, vec1i, vec2i, vec3i;
	float32x4_t vecSum0 = vdupq_n_f32(0.f), vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	compv_float32_t coeff;
	COMPV_ALIGN_NEON() compv_float32_t mem[16];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
            vecSum0 = veorq_s32(vecSum0, vecSum0);
            vecSum1 = veorq_s32(vecSum0, vecSum0);
            vecSum2 = veorq_s32(vecSum0, vecSum0);
            vecSum3 = veorq_s32(vecSum0, vecSum0);
            for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = vld1q_u8(&inPtr[i + k]);
				coeff = vthzKernPtr[row];
				vec2i = vmovl_u8(vget_low_u8(vecInPtr)); // epu8 -> epu16
				vec3i = vmovl_u8(vget_high_u8(vecInPtr)); // epu8 -> epu16
				vec0i = vmovl_u16(vget_low_u16(vec2i)); // epu16 -> epu32
				vec1i = vmovl_u16(vget_high_u16(vec2i)); // epu16 -> epu32
				vec2i = vmovl_u16(vget_low_u16(vec3i)); // epu16 -> epu32
				vec3i = vmovl_u16(vget_high_u16(vec3i)); // epu16 -> epu32
				vec0f = vcvtq_f32_u32(vec0i);
				vec1f = vcvtq_f32_u32(vec1i);
				vec2f = vcvtq_f32_u32(vec2i);
				vec3f = vcvtq_f32_u32(vec3i);
				vecSum0 = vmlaq_n_f32(vecSum0, vec0f, coeff);
				vecSum1 = vmlaq_n_f32(vecSum1, vec1f, coeff);
				vecSum2 = vmlaq_n_f32(vecSum2, vec2f, coeff);
				vecSum3 = vmlaq_n_f32(vecSum3, vec3f, coeff);
			}
			if (i < width16) {
				vst1q_f32(&outPtr[i], vecSum0);
				vst1q_f32(&outPtr[i + 4], vecSum1);
				vst1q_f32(&outPtr[i + 8], vecSum2);
				vst1q_f32(&outPtr[i + 12], vecSum3);
			}
			else {
				vst1q_f32(&mem[0], vecSum0);
				vst1q_f32(&mem[4], vecSum1);
				vst1q_f32(&mem[8], vecSum2);
				vst1q_f32(&mem[12], vecSum3);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
}

// on iOS (iPhone5, ARM32, HD image, single threaded, no FMA, #1000 times) small perf gain, asm code: 19433.ms, intrin code: 20240.ms
void CompVMathConvlt1VtHz_32f32f32f_Intrin_NEON(const compv_float32_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width16 = width & -16;
	float32x4_t vecSum0 = vdupq_n_f32(0.f), vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	compv_float32_t coeff;
	COMPV_ALIGN_NEON() compv_float32_t mem[16];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
            vecSum0 = veorq_s32(vecSum0, vecSum0);
            vecSum1 = veorq_s32(vecSum0, vecSum0);
            vecSum2 = veorq_s32(vecSum0, vecSum0);
            vecSum3 = veorq_s32(vecSum0, vecSum0);
            for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0f = vld1q_f32(&inPtr[i + k]);
				vec1f = vld1q_f32(&inPtr[i + k + 4]);
				vec2f = vld1q_f32(&inPtr[i + k + 8]);
				vec3f = vld1q_f32(&inPtr[i + k + 12]);
				coeff = vthzKernPtr[row];
				vecSum0 = vmlaq_n_f32(vecSum0, vec0f, coeff);
				vecSum1 = vmlaq_n_f32(vecSum1, vec1f, coeff);
				vecSum2 = vmlaq_n_f32(vecSum2, vec2f, coeff);
				vecSum3 = vmlaq_n_f32(vecSum3, vec3f, coeff);
			}
			if (i < width16) {
				vst1q_f32(&outPtr[i], vecSum0);
				vst1q_f32(&outPtr[i + 4], vecSum1);
				vst1q_f32(&outPtr[i + 8], vecSum2);
				vst1q_f32(&outPtr[i + 12], vecSum3);
			}
			else {
				vst1q_f32(&mem[0], vecSum0);
				vst1q_f32(&mem[4], vecSum1);
				vst1q_f32(&mem[8], vecSum2);
				vst1q_f32(&mem[12], vecSum3);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
}

// on iOS (iPhone5, ARM32, HD image, single threaded, no FMA, #1000 times) small perf gain, asm code: 13962.ms, intrin code: 14270.ms
void CompVMathConvlt1VtHz_32f32f8u_Intrin_NEON(const compv_float32_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = (width + pad);
	const compv_uscalar_t width16 = width & -16;
	float32x4_t vecSum0 = vdupq_n_f32(0.f), vecSum1, vecSum2, vecSum3, vec0f, vec1f, vec2f, vec3f;
	compv_float32_t coeff;
	COMPV_ALIGN_SSE() uint8_t mem[16];

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
            vecSum0 = veorq_s32(vecSum0, vecSum0);
            vecSum1 = veorq_s32(vecSum0, vecSum0);
            vecSum2 = veorq_s32(vecSum0, vecSum0);
            vecSum3 = veorq_s32(vecSum0, vecSum0);
            for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec0f = vld1q_f32(&inPtr[i + k]);
				vec1f = vld1q_f32(&inPtr[i + k + 4]);
				vec2f = vld1q_f32(&inPtr[i + k + 8]);
				vec3f = vld1q_f32(&inPtr[i + k + 12]);
				coeff = vthzKernPtr[row];
				vecSum0 = vmlaq_n_f32(vecSum0, vec0f, coeff);
				vecSum1 = vmlaq_n_f32(vecSum1, vec1f, coeff);
				vecSum2 = vmlaq_n_f32(vecSum2, vec2f, coeff);
				vecSum3 = vmlaq_n_f32(vecSum3, vec3f, coeff);
			}
			vec0f = vcvtq_s32_f32(vecSum0);
			vec1f = vcvtq_s32_f32(vecSum1);
			vec2f = vcvtq_s32_f32(vecSum2);
			vec3f = vcvtq_s32_f32(vecSum3);
			vec0f = vcombine_s16(vqmovn_s32(vec0f), vqmovn_s32(vec1f));
			vec2f = vcombine_s16(vqmovn_s32(vec2f), vqmovn_s32(vec3f));
			vec0f = vcombine_u8(vqmovun_s16(vec0f), vqmovun_s16(vec2f));
			if (i < width16) {
				vst1q_u8(&outPtr[i], vec0f);
			}
			else {
				vst1q_u8(mem, vec0f);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
}

// on iOS (iPhone5, ARM32, HD image, single threaded, #1000 times) small perf gain, asm code: 11785.ms, intrin code: 12305.ms
void CompVMathConvlt1VtHz_8u16s16s_Intrin_NEON(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = width + pad;
	const compv_uscalar_t width16 = width & -16;
	int32x4_t vecSum0 = vdupq_n_s32(0), vecSum1, vecSum2, vecSum3;
	int16x8_t vec2, vec3;
	uint8x16_t vecInPtr;
	int16_t coeff;
	COMPV_ALIGN_SSE() int16_t mem[16];
    
	// Using int32_t as accumulator to avoid overflow

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			vecSum0 = veorq_s32(vecSum0, vecSum0);
			vecSum1 = veorq_s32(vecSum0, vecSum0);
			vecSum2 = veorq_s32(vecSum0, vecSum0);
			vecSum3 = veorq_s32(vecSum0, vecSum0);
			for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vecInPtr = vld1q_u8(&inPtr[i + k]);
				coeff = vthzKernPtr[row];
				vec2 = vmovl_u8(vget_low_u8(vecInPtr)); // epu8 -> epu16
				vec3 = vmovl_u8(vget_high_u8(vecInPtr)); // epu8 -> epu16
				vecSum0 = vmlal_n_s16(vecSum0, vget_low_s16(vec2), coeff); // epi16 -> epi32 + mul + add
				vecSum1 = vmlal_n_s16(vecSum1, vget_high_s16(vec2), coeff);
				vecSum2 = vmlal_n_s16(vecSum2, vget_low_s16(vec3), coeff);
				vecSum3 = vmlal_n_s16(vecSum3, vget_high_s16(vec3), coeff);
			}
			vecSum0 = vcombine_s16(vqmovn_s32(vecSum0), vqmovn_s32(vecSum1));
			vecSum2 = vcombine_s16(vqmovn_s32(vecSum2), vqmovn_s32(vecSum3));
			if (i < width16) {
				vst1q_s16(&outPtr[i], vecSum0);
				vst1q_s16(&outPtr[i + 8], vecSum2);
			}
			else {
				vst1q_s16(&mem[0], vecSum0);
				vst1q_s16(&mem[8], vecSum2);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
}

// on iOS (iPhone5, ARM32, HD image, single threaded, #1000 times) small perf gain, asm code: 11801.ms, intrin code: 12328.ms
void CompVMathConvlt1VtHz_16s16s16s_Intrin_NEON(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, k, row;
	const compv_uscalar_t stride = width + pad;
	const compv_uscalar_t width16 = width & -16;
	int32x4_t vecSum0 = vdupq_n_s32(0), vecSum1, vecSum2, vecSum3;
	int16x8_t vec2, vec3;
	int16_t coeff;
	COMPV_ALIGN_SSE() int16_t mem[16];

	// Using int32_t as accumulator to avoid overflow

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
            vecSum0 = veorq_s32(vecSum0, vecSum0);
            vecSum1 = veorq_s32(vecSum0, vecSum0);
            vecSum2 = veorq_s32(vecSum0, vecSum0);
            vecSum3 = veorq_s32(vecSum0, vecSum0);
            for (row = 0, k = 0; row < kernSize; ++row, k += step) {
				vec2 = vld1q_s16(&inPtr[i + k]);
				vec3 = vld1q_s16(&inPtr[i + k + 8]);
				coeff = vthzKernPtr[row];
				vecSum0 = vmlal_n_s16(vecSum0, vget_low_s16(vec2), coeff); // epi16 -> epi32 + mul + add
				vecSum1 = vmlal_n_s16(vecSum1, vget_high_s16(vec2), coeff);
				vecSum2 = vmlal_n_s16(vecSum2, vget_low_s16(vec3), coeff);
				vecSum3 = vmlal_n_s16(vecSum3, vget_high_s16(vec3), coeff);
			}
			vecSum0 = vcombine_s16(vqmovn_s32(vecSum0), vqmovn_s32(vecSum1));
			vecSum2 = vcombine_s16(vqmovn_s32(vecSum2), vqmovn_s32(vecSum3));
			if (i < width16) {
				vst1q_s16(&outPtr[i], vecSum0);
				vst1q_s16(&outPtr[i + 8], vecSum2);
			}
			else {
				vst1q_s16(&mem[0], vecSum0);
				vst1q_s16(&mem[8], vecSum2);
				for (k = 0; i < width; ++i, ++k) {
					outPtr[i] = mem[k];
				}
			}
		}

		inPtr += stride;
		outPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
