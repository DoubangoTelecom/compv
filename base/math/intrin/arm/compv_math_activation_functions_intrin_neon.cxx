/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_activation_functions_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"

COMPV_NAMESPACE_BEGIN()

const float kMaxSoftmaxActivation = -86;
extern float ___expf_c(float x);

// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 4
void CompVMathActivationFunctionsTanh_32f32f_Intrin_NEON(
	COMPV_ALIGNED(NEON) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_ASSERT(!(in_out_length & 3));
	const float32x4_t vecMinus1 = vdupq_n_f32(-1.f);
	const float32x4_t vecPlus1 = vdupq_n_f32(1.f);
	const float32x4_t vecScale = vdupq_n_f32(*scale1);
	const float32x4_t vecZero = vdupq_n_f32(0.f);
	const uint32x4_t vecLut_length_minus1 = vdupq_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		float32x4_t vecX = (float32x4_t)vld1q_f32(&in_ptr[i]);
		float32x4_t vecSign = (float32x4_t)vcltq_f32(vecX, vecZero);
		vecSign = (float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecSign, (int32x4_t)vecMinus1), vbicq_s32((int32x4_t)vecPlus1, (int32x4_t)vecSign));
		vecX = vmulq_f32(vmulq_f32(vecX, vecSign), vecScale);
		uint32x4_t vecIndex = vcvtq_u32_f32(vecX);
		uint32x4_t vecIndexMask = vcltq_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROQ(vecIndexMask)) {
			const float32x4_t vecIndexRounded = COMPV_ARM_NEON_FLOOR_F32(vecX);
			vecIndex = vminq_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const float32x2x2_t vecLUT0 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 0)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 1))  // index+1 | index
			); // {index+1[1] | index+1[0]} | {index[1] | index[0]}
			const float32x2x2_t vecLUT1 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 2)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 3))  // index+1 | index
			); // {index+1[3] | index+1[2]} | {index[3] | index[2]}
			
			const float32x4_t vecTanh_i0 = vcombine_f32(vecLUT0.val[0], vecLUT1.val[0]); // index[3] | index[2] | index[1] | index[0]
			const float32x4_t vecTanh_i1 = vcombine_f32(vecLUT0.val[1], vecLUT1.val[1]); // index[3] | index[2] | index[1] | index[0]
			
			const float32x4_t vecResult = vmulq_f32(
				COMPV_ARM_FMAQ_F32(
					vecTanh_i0, 
					vsubq_f32(vecTanh_i1, vecTanh_i0), 
					vsubq_f32(vecX, vecIndexRounded)
				),
				vecSign
			);
			vst1q_f32(&out_ptr[i],
				(float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecResult, (int32x4_t)vecIndexMask), vbicq_s32((int32x4_t)vecSign, (int32x4_t)vecIndexMask))
			);
		}
		else {
			vst1q_f32(&out_ptr[i], vecSign);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 4
void CompVMathActivationFunctionsTanhMul_32f32f_Intrin_NEON(
	COMPV_ALIGNED(SSE) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	const compv_float32_t* mul_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_ASSERT(!(in_out_length & 3));
	const float32x4_t vecMinus1 = vdupq_n_f32(-1.f);
	const float32x4_t vecPlus1 = vdupq_n_f32(1.f);
	const float32x4_t vecScale = vdupq_n_f32(*scale1);
	const float32x4_t vecZero = vdupq_n_f32(0.f);
	const uint32x4_t vecLut_length_minus1 = vdupq_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		float32x4_t vecX = (float32x4_t)vld1q_f32(&in_ptr[i]);
		float32x4_t vecSign = (float32x4_t)vcltq_f32(vecX, vecZero);
		vecSign = (float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecSign, (int32x4_t)vecMinus1), vbicq_s32((int32x4_t)vecPlus1, (int32x4_t)vecSign));
		vecX = vmulq_f32(vmulq_f32(vecX, vecSign), vecScale);
		uint32x4_t vecIndex = vcvtq_u32_f32(vecX);
		uint32x4_t vecIndexMask = vcltq_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROQ(vecIndexMask)) {
			const float32x4_t vecIndexRounded = COMPV_ARM_NEON_FLOOR_F32(vecX);
			vecIndex = vminq_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const float32x2x2_t vecLUT0 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 0)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 1))  // index+1 | index
			); // {index+1[1] | index+1[0]} | {index[1] | index[0]}
			const float32x2x2_t vecLUT1 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 2)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 3))  // index+1 | index
			); // {index+1[3] | index+1[2]} | {index[3] | index[2]}

			const float32x4_t vecTanh_i0 = vcombine_f32(vecLUT0.val[0], vecLUT1.val[0]); // index[3] | index[2] | index[1] | index[0]
			const float32x4_t vecTanh_i1 = vcombine_f32(vecLUT0.val[1], vecLUT1.val[1]); // index[3] | index[2] | index[1] | index[0]

			const float32x4_t vecResult = vmulq_f32(
				COMPV_ARM_FMAQ_F32(
					vecTanh_i0,
					vsubq_f32(vecTanh_i1, vecTanh_i0),
					vsubq_f32(vecX, vecIndexRounded)
				),
				vecSign
			);
			vst1q_f32(&out_ptr[i],
				vmulq_f32(
				(float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecResult, (int32x4_t)vecIndexMask), vbicq_s32((int32x4_t)vecSign, (int32x4_t)vecIndexMask)),
					vld1q_f32(&mul_ptr[i])
				)
			);
		}
		else {
			vst1q_f32(&out_ptr[i],
				vmulq_f32(
					vecSign,
					vld1q_f32(&mul_ptr[i])
				)
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 4
void CompVMathActivationFunctionsLogistic_32f32f_Intrin_NEON(
	COMPV_ALIGNED(SSE) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_ASSERT(!(in_out_length & 3));
	const float32x4_t vecMinus1 = vdupq_n_f32(-1.f);
	const float32x4_t vecPlus1 = vdupq_n_f32(1.f);
	const float32x4_t vecScale = vdupq_n_f32(*scale1);
	const float32x4_t vecZero = vdupq_n_f32(0.f);
	const uint32x4_t vecLut_length_minus1 = vdupq_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		float32x4_t vecX = vld1q_f32(&in_ptr[i]);
		const float32x4_t vecSignMask = (float32x4_t)vcltq_f32(vecX, vecZero);
		const float32x4_t vecSign = (float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecSignMask, (int32x4_t)vecMinus1), vbicq_s32((int32x4_t)vecPlus1, (int32x4_t)vecSignMask));
		vecX = vmulq_f32(vmulq_f32(vecX, vecSign), vecScale);
		uint32x4_t vecIndex = vcvtq_u32_f32(vecX);
		uint32x4_t vecIndexMask = vcltq_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROQ(vecIndexMask)) {
			const float32x4_t vecIndexRounded = COMPV_ARM_NEON_FLOOR_F32(vecX);
			vecIndex = vminq_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const float32x2x2_t vecLUT0 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 0)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 1))  // index+1 | index
			); // {index+1[1] | index+1[0]} | {index[1] | index[0]}
			const float32x2x2_t vecLUT1 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 2)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 3))  // index+1 | index
			); // {index+1[3] | index+1[2]} | {index[3] | index[2]}

			const float32x4_t vec_l0 = vcombine_f32(vecLUT0.val[0], vecLUT1.val[0]); // index[3] | index[2] | index[1] | index[0]
			const float32x4_t vec_l1 = vcombine_f32(vecLUT0.val[1], vecLUT1.val[1]); // index[3] | index[2] | index[1] | index[0]

			const float32x4_t vecResult = vmulq_f32(
				COMPV_ARM_FMAQ_F32(vec_l0, vsubq_f32(vec_l1, vec_l0), vsubq_f32(vecX, vecIndexRounded)),
				vecSign
			);
			vst1q_f32(&out_ptr[i],
				vaddq_f32(
					(float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecResult, (int32x4_t)vecIndexMask), vbicq_s32((int32x4_t)vecSign, (int32x4_t)vecIndexMask)),
					(float32x4_t)vandq_s32((int32x4_t)vecSignMask, (int32x4_t)vecPlus1)
				)
			);
		}
		else {
			vst1q_f32(&out_ptr[i],
				vaddq_f32(
					vecSign,
					(float32x4_t)vandq_s32((int32x4_t)vecSignMask, (int32x4_t)vecPlus1)
				)
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 4
void CompVMathActivationFunctionsLogisticMul_32f32f_Intrin_NEON(
	COMPV_ALIGNED(SSE) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	const compv_float32_t* mul_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_ASSERT(!(in_out_length & 3));
	const float32x4_t vecMinus1 = vdupq_n_f32(-1.f);
	const float32x4_t vecPlus1 = vdupq_n_f32(1.f);
	const float32x4_t vecScale = vdupq_n_f32(*scale1);
	const float32x4_t vecZero = vdupq_n_f32(0.f);
	const uint32x4_t vecLut_length_minus1 = vdupq_n_u32(static_cast<uint32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		float32x4_t vecX = vld1q_f32(&in_ptr[i]);
		const float32x4_t vecSignMask = (float32x4_t)vcltq_f32(vecX, vecZero);
		const float32x4_t vecSign = (float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecSignMask, (int32x4_t)vecMinus1), vbicq_s32((int32x4_t)vecPlus1, (int32x4_t)vecSignMask));
		vecX = vmulq_f32(vmulq_f32(vecX, vecSign), vecScale);
		uint32x4_t vecIndex = vcvtq_u32_f32(vecX);
		uint32x4_t vecIndexMask = vcltq_u32(vecIndex, vecLut_length_minus1);
		if (COMPV_ARM_NEON_NEQ_ZEROQ(vecIndexMask)) {
			const float32x4_t vecIndexRounded = COMPV_ARM_NEON_FLOOR_F32(vecX);
			vecIndex = vminq_u32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const float32x2x2_t vecLUT0 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 0)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 1))  // index+1 | index
			); // {index+1[1] | index+1[0]} | {index[1] | index[0]}
			const float32x2x2_t vecLUT1 = vzip_f32(
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 2)), // index+1 | index
				vld1_f32(lut_ptr + vgetq_lane_u32(vecIndex, 3))  // index+1 | index
			); // {index+1[3] | index+1[2]} | {index[3] | index[2]}

			const float32x4_t vec_l0 = vcombine_f32(vecLUT0.val[0], vecLUT1.val[0]); // index[3] | index[2] | index[1] | index[0]
			const float32x4_t vec_l1 = vcombine_f32(vecLUT0.val[1], vecLUT1.val[1]); // index[3] | index[2] | index[1] | index[0]

			const float32x4_t vecResult = vmulq_f32(
				COMPV_ARM_FMAQ_F32(vec_l0, vsubq_f32(vec_l1, vec_l0), vsubq_f32(vecX, vecIndexRounded)),
				vecSign
			);
			vst1q_f32(&out_ptr[i],
				vmulq_f32(
					vaddq_f32(
					(float32x4_t)vorrq_s32(vandq_s32((int32x4_t)vecResult, (int32x4_t)vecIndexMask), vbicq_s32((int32x4_t)vecSign, (int32x4_t)vecIndexMask)),
						(float32x4_t)vandq_s32((int32x4_t)vecSignMask, (int32x4_t)vecPlus1)
					),
					vld1q_f32(&mul_ptr[i])
				)
			);
		}
		else {
			vst1q_f32(&out_ptr[i],
				vmulq_f32(
					vaddq_f32(
						vecSign,
						(float32x4_t)vandq_s32((int32x4_t)vecSignMask, (int32x4_t)vecPlus1)
					),
					vld1q_f32(&mul_ptr[i])
				)
			);
		}
	}
}

// This is an internal function
// up to the caller to make sure in_out_length >= 4
void CompVMathActivationFunctionsSoftmaxInPlace_32f32f_Intrin_NEON(
	const compv_uscalar_t& in_out_length,
	compv_float32_t* in_out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation for Exp");
	const compv_uscalar_t in_out_length4 = in_out_length & -4;
	compv_uscalar_t i;

	/* max */
	float32x4_t max_output = vld1q_f32(in_out_ptr);
	for (i = 4; i < in_out_length4; i += 4) {
		max_output = vmaxq_f32(max_output, vld1q_f32(&in_out_ptr[i]));
	}
#if COMPV_ARCH_ARM64
	float max_output_ss = vmaxvq_f32(max_output);
#else
	float32x2_t vv0__ = vmax_f32(vget_low_f32(max_output), vget_high_f32(max_output));
	vv0__ = vpmax_f32(vv0__, vv0__);
	float max_output_ss = vget_lane_f32(vv0__, 0);
#endif
	for (; i < in_out_length; i += 1) {
		max_output_ss = std::max(max_output_ss, in_out_ptr[i]);
	}
	max_output = vdupq_n_f32(max_output_ss);

	/* prob */
	const float32x4_t maxSoftmaxActivation = vdupq_n_f32(kMaxSoftmaxActivation);
	const float32x4_t zero = vdupq_n_f32(0.f);
	float32x4_t prob_total = vdupq_n_f32(0.f);
	for (i = 0; i < in_out_length4; i += 4) {
		float32x4_t prob = vsubq_f32(vld1q_f32(&in_out_ptr[i]), max_output);
		prob = vminq_f32(vmaxq_f32(maxSoftmaxActivation, prob), zero);
		// FIXE(dmi): Use Exp from https://github.com/DoubangoTelecom/compv/blob/607c0e78abaa21da110e47f4daddb9718a57dda3/base/math/intrin/arm/compv_math_exp_intrin_neon.cxx#L65
		prob = (float32x4_t){
			___expf_c(vgetq_lane_f32(prob, 0)),
			___expf_c(vgetq_lane_f32(prob, 1)),
			___expf_c(vgetq_lane_f32(prob, 2)),
			___expf_c(vgetq_lane_f32(prob, 3))
		};
		prob_total = vaddq_f32(prob_total, prob);
		vst1q_f32(&in_out_ptr[i], prob);
	}
#if COMPV_ARCH_ARM64
	float prob_total_ss = vaddvq_f32(prob_total);
#else
	vv0__ = vadd_f32(vget_low_f32(prob_total), vget_high_f32(prob_total));
	vv0__ = vpadd_f32(vv0__, vv0__);
	float prob_total_ss = vget_lane_f32(vv0__, 0);
#endif
	for (; i < in_out_length; i += 1) {
		float prob = in_out_ptr[i] - max_output_ss;
		prob = ___expf_c(std::min(std::max(kMaxSoftmaxActivation, prob), 0.f));
		prob_total_ss += prob;
		in_out_ptr[i] = prob;
	}
	if (prob_total_ss > 0.f && prob_total_ss != 1.f) {
		const float prob_total_scale_ss = 1.f / prob_total_ss;
		const float32x4_t prob_total_scale = vdupq_n_f32(prob_total_scale_ss);
		for (i = 0; i < in_out_length4; i += 4) {
			vst1q_f32(&in_out_ptr[i], vmulq_f32(vld1q_f32(&in_out_ptr[i]), prob_total_scale));
		}
		for (; i < in_out_length; i += 1) {
			in_out_ptr[i] *= prob_total_scale_ss;
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM64 && COMPV_INTRINSIC */
