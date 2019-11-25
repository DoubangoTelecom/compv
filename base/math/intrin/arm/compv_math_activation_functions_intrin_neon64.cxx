/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_activation_functions_intrin_neon64.h"

#if COMPV_ARCH_ARM64 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"

COMPV_NAMESPACE_BEGIN()

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
void CompVMathActivationFunctionsTanh_64f64f_Intrin_NEON64(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("https://github.com/DoubangoTelecom/compv/issues/173");
	COMPV_DEBUG_INFO_CHECK_NEON();
	const float64x2_t vecMinus1 = vdupq_n_f64(-1.0);
	const float64x2_t vecPlus1 = vdupq_n_f64(1.0);
	const float64x2_t vecScale = vdupq_n_f64(*scale1);
	const float64x2_t vecZero = vdupq_n_f64(0.0);
	const uint32x2_t vecLut_length_minus1 = vdup_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		float64x2_t vecX = vld1q_f64(&in_ptr[i]);
		float64x2_t vecSign = vcltq_f64(vecX, vecZero);
		vecSign = vorrq_s64(vandq_s64(vecSign, vecMinus1), vbicq_s64(vecPlus1, vecSign));
		vecX = vmulq_f64(vmulq_f64(vecX, vecSign), vecScale);
		uint32x2_t vecIndex = vqmovn_u64(vcvtq_u64_f64(vecX));
		uint32x2_t vecIndexMask = vclt_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROD(vecIndexMask)) {
			const float64x2_t vecIndexRounded = vrndmq_f64(vecX); 
			vecIndex = vmin_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const uint32x2x2_t vecIndexMask32x2x2 = vzip_u32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const uint64x2_t vecIndexMask64x2 = vcombine_s32(vecIndexMask32x2x2.val[0], vecIndexMask32x2x2.val[1]); // _mm_unpacklo_epi32(vecIndexMask, vecIndexMask)
			const float64x1x2_t vecLUT0 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 0));
			const float64x1x2_t vecLUT1 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 1));
			const float64x2_t vecTanh_i0 = vcombine_f64(vecLUT0.val[0], vecLUT1.val[0]);
			const float64x2_t vecTanh_i1 = vcombine_f64(vecLUT0.val[1], vecLUT1.val[1]);
			const float64x2_t vecResult = vmulq_f64(
				vfmaq_f64(vecTanh_i0, vsubq_f64(vecTanh_i1, vecTanh_i0), vsubq_f64(vecX, vecIndexRounded)),
				vecSign
			);
			vst1q_f64(&out_ptr[i],
				vorrq_s64(vandq_s64(vecResult, vecIndexMask64x2), vbicq_s64(vecSign, vecIndexMask64x2))
			);
		}
		else {
			vst1q_f64(&out_ptr[i], vecSign);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
void CompVMathActivationFunctionsTanhMul_64f64f_Intrin_NEON64(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("https://github.com/DoubangoTelecom/compv/issues/173");
	COMPV_DEBUG_INFO_CHECK_NEON();
	const float64x2_t vecMinus1 = vdupq_n_f64(-1.0);
	const float64x2_t vecPlus1 = vdupq_n_f64(1.0);
	const float64x2_t vecScale = vdupq_n_f64(*scale1);
	const float64x2_t vecZero = vdupq_n_f64(0.0);
	const uint32x2_t vecLut_length_minus1 = vdup_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		float64x2_t vecX = vld1q_f64(&in_ptr[i]);
		float64x2_t vecSign = vcltq_f64(vecX, vecZero);
		vecSign = vorrq_s64(vandq_s64(vecSign, vecMinus1), vbicq_s64(vecPlus1, vecSign));
		vecX = vmulq_f64(vmulq_f64(vecX, vecSign), vecScale);
		uint32x2_t vecIndex = vqmovn_u64(vcvtq_u64_f64(vecX));
		uint32x2_t vecIndexMask = vclt_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROD(vecIndexMask)) { // TODO(dmi): https://github.com/DoubangoTelecom/compv/issues/173
			const float64x2_t vecIndexRounded = vrndmq_f64(vecX);
			vecIndex = vmin_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const uint32x2x2_t vecIndexMask32x2x2 = vzip_u32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const uint64x2_t vecIndexMask64x2 = vcombine_s32(vecIndexMask32x2x2.val[0], vecIndexMask32x2x2.val[1]); // _mm_unpacklo_epi32(vecIndexMask, vecIndexMask)
			const float64x1x2_t vecLUT0 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 0));
			const float64x1x2_t vecLUT1 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 1));
			const float64x2_t vecTanh_i0 = vcombine_f64(vecLUT0.val[0], vecLUT1.val[0]);
			const float64x2_t vecTanh_i1 = vcombine_f64(vecLUT0.val[1], vecLUT1.val[1]);
			const float64x2_t vecResult = vmulq_f64(
				vfmaq_f64(vecTanh_i0, vsubq_f64(vecTanh_i1, vecTanh_i0), vsubq_f64(vecX, vecIndexRounded)),
				vecSign
			);
			vst1q_f64(&out_ptr[i],
				vmulq_f64(
					vorrq_s64(vandq_s64(vecResult, vecIndexMask64x2), vbicq_s64(vecSign, vecIndexMask64x2)),
					vld1q_f64(&mul_ptr[i])
				)
			);
		}
		else {
			vst1q_f64(&out_ptr[i], 
				vmulq_f64(
					vecSign,
					vld1q_f64(&mul_ptr[i])
				)
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
void CompVMathActivationFunctionsLogistic_64f64f_Intrin_NEON64(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("https://github.com/DoubangoTelecom/compv/issues/173");
	COMPV_DEBUG_INFO_CHECK_NEON();
	const float64x2_t vecMinus1 = vdupq_n_f64(-1.0);
	const float64x2_t vecPlus1 = vdupq_n_f64(1.0);
	const float64x2_t vecScale = vdupq_n_f64(*scale1);
	const float64x2_t vecZero = vdupq_n_f64(0.0);
	const uint32x2_t vecLut_length_minus1 = vdup_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		float64x2_t vecX = vld1q_f64(&in_ptr[i]);
		const float64x2_t vecSignMask = vcltq_f64(vecX, vecZero);
		const float64x2_t vecSign = vorrq_s64(vandq_s64(vecSignMask, vecMinus1), vbicq_s64(vecPlus1, vecSignMask));
		vecX = vmulq_f64(vmulq_f64(vecX, vecSign), vecScale);
		uint32x2_t vecIndex = vqmovn_u64(vcvtq_u64_f64(vecX));
		uint32x2_t vecIndexMask = vclt_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROD(vecIndexMask)) {
			const float64x2_t vecIndexRounded = vrndmq_f64(vecX);
			vecIndex = vmin_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const uint32x2x2_t vecIndexMask32x2x2 = vzip_u32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const uint64x2_t vecIndexMask64x2 = vcombine_s32(vecIndexMask32x2x2.val[0], vecIndexMask32x2x2.val[1]); // _mm_unpacklo_epi32(vecIndexMask, vecIndexMask)
			const float64x1x2_t vecLUT0 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 0));
			const float64x1x2_t vecLUT1 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 1));
			const float64x2_t vec_l0 = vcombine_f64(vecLUT0.val[0], vecLUT1.val[0]);
			const float64x2_t vec_l1 = vcombine_f64(vecLUT0.val[1], vecLUT1.val[1]);
			const float64x2_t vecResult = vmulq_f64(
				vfmaq_f64(vec_l0, vsubq_f64(vec_l1, vec_l0), vsubq_f64(vecX, vecIndexRounded)),
				vecSign
			);
			vst1q_f64(&out_ptr[i],
				vaddq_f64(
					vorrq_s64(vandq_s64(vecResult, vecIndexMask64x2), vbicq_s64(vecSign, vecIndexMask64x2)),
					vandq_s64(vecSignMask, vecPlus1)
				)
			);
		}
		else {
			vst1q_f64(&out_ptr[i],
				vaddq_f64(
					vecSign,
					vandq_s64(vecSignMask, vecPlus1)
				)
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
void CompVMathActivationFunctionsLogisticMul_64f64f_Intrin_NEON64(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("https://github.com/DoubangoTelecom/compv/issues/173");
	COMPV_DEBUG_INFO_CHECK_NEON();
	const float64x2_t vecMinus1 = vdupq_n_f64(-1.0);
	const float64x2_t vecPlus1 = vdupq_n_f64(1.0);
	const float64x2_t vecScale = vdupq_n_f64(*scale1);
	const float64x2_t vecZero = vdupq_n_f64(0.0);
	const uint32x2_t vecLut_length_minus1 = vdup_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		float64x2_t vecX = vld1q_f64(&in_ptr[i]);
		const float64x2_t vecSignMask = vcltq_f64(vecX, vecZero);
		const float64x2_t vecSign = vorrq_s64(vandq_s64(vecSignMask, vecMinus1), vbicq_s64(vecPlus1, vecSignMask));
		vecX = vmulq_f64(vmulq_f64(vecX, vecSign), vecScale);
		uint32x2_t vecIndex = vqmovn_u64(vcvtq_u64_f64(vecX));
		uint32x2_t vecIndexMask = vclt_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROD(vecIndexMask)) {
			const float64x2_t vecIndexRounded = vrndmq_f64(vecX);
			vecIndex = vmin_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const uint32x2x2_t vecIndexMask32x2x2 = vzip_u32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const uint64x2_t vecIndexMask64x2 = vcombine_s32(vecIndexMask32x2x2.val[0], vecIndexMask32x2x2.val[1]); // _mm_unpacklo_epi32(vecIndexMask, vecIndexMask)
			const float64x1x2_t vecLUT0 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 0));
			const float64x1x2_t vecLUT1 = vld2_f64(lut_ptr + vget_lane_u32(vecIndex, 1));
			const float64x2_t vec_l0 = vcombine_f64(vecLUT0.val[0], vecLUT1.val[0]);
			const float64x2_t vec_l1 = vcombine_f64(vecLUT0.val[1], vecLUT1.val[1]);
			const float64x2_t vecResult = vmulq_f64(
				vfmaq_f64(vec_l0, vsubq_f64(vec_l1, vec_l0), vsubq_f64(vecX, vecIndexRounded)),
				vecSign
			);
			vst1q_f64(&out_ptr[i],
				vmulq_f64(
					vaddq_f64(
						vorrq_s64(vandq_s64(vecResult, vecIndexMask64x2), vbicq_s64(vecSign, vecIndexMask64x2)),
						vandq_s64(vecSignMask, vecPlus1)
					),
					vld1q_f64(&mul_ptr[i])
				)
			);
		}
		else {
			vst1q_f64(&out_ptr[i],
				vmulq_f64(
					vaddq_f64(
						vecSign,
						vandq_s64(vecSignMask, vecPlus1)
					),
					vld1q_f64(&mul_ptr[i])
				)
			);
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM64 && COMPV_INTRINSIC */
