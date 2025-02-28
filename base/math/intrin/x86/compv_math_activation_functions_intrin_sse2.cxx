/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

const float kMaxSoftmaxActivation = -86;
extern float ___expf_c(float x);

// This is an internal function
// up to the caller to make sure in_out_length >= 4
void CompVMathActivationFunctionsSoftmaxInPlace_32f32f_Intrin_SSE2(
	const compv_uscalar_t& in_out_length,
	compv_float32_t* in_out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation for Exp");
	const compv_uscalar_t in_out_length4 = in_out_length & -4;
	compv_uscalar_t i;

	/* max */
	__m128 max_output = _mm_loadu_ps(in_out_ptr);
	for (i = 4; i < in_out_length4; i += 4) {
		max_output = _mm_max_ps(max_output, _mm_loadu_ps(&in_out_ptr[i]));
	}
	for (; i < in_out_length; i += 1) {
		max_output = _mm_max_ss(max_output, _mm_load_ss(&in_out_ptr[i]));
	}
	// ARM: vpmax_f32 
	max_output = _mm_max_ps(max_output, _mm_shuffle_ps(max_output, max_output, 0x0E));
	max_output = _mm_max_ps(max_output, _mm_shuffle_ps(max_output, max_output, 0x01));
	max_output = _mm_shuffle_ps(max_output, max_output, 0x0);

	/* prob */
	const __m128 maxSoftmaxActivation = _mm_set1_ps(kMaxSoftmaxActivation);
	const __m128 zero = _mm_setzero_ps();
	__m128 prob_total = _mm_setzero_ps();
	for (i = 0; i < in_out_length4; i += 4) {
		__m128 prob = _mm_sub_ps(_mm_loadu_ps(&in_out_ptr[i]), max_output);
		prob = _mm_min_ps(_mm_max_ps(maxSoftmaxActivation, prob), zero);
		// FIXE(dmi): Use Exp from https://github.com/DoubangoTelecom/compv/blob/607c0e78abaa21da110e47f4daddb9718a57dda3/base/math/intrin/x86/compv_math_exp_intrin_sse2.cxx#L71
		COMPV_ALIGN(16) const float exps[] = { 
			___expf_c(_mm_cvtss_f32(_mm_shuffle_ps(prob, prob, _MM_SHUFFLE(0, 0, 0, 0)))),
			___expf_c(_mm_cvtss_f32(_mm_shuffle_ps(prob, prob, _MM_SHUFFLE(1, 1, 1, 1)))),
			___expf_c(_mm_cvtss_f32(_mm_shuffle_ps(prob, prob, _MM_SHUFFLE(2, 2, 2, 2)))),
			___expf_c(_mm_cvtss_f32(_mm_shuffle_ps(prob, prob, _MM_SHUFFLE(3, 3, 3, 3))))
		};
		prob = _mm_load_ps(exps);
		prob_total = _mm_add_ps(prob_total, prob);
		_mm_storeu_ps(&in_out_ptr[i], prob);
	}
	for (; i < in_out_length; i += 1) {
		__m128 prob = _mm_sub_ss(_mm_load_ss(&in_out_ptr[i]), max_output);
		prob = _mm_min_ss(zero, _mm_max_ss(maxSoftmaxActivation, prob));
		prob = _mm_set_ss(___expf_c(_mm_cvtss_f32(prob)));
		prob_total = _mm_add_ss(prob_total, prob);
		_mm_store_ss(&in_out_ptr[i], prob);
	}
	// ARM: vpadd_f32 
	prob_total = _mm_add_ps(prob_total, _mm_shuffle_ps(prob_total, prob_total, 0x0E));
	prob_total = _mm_add_ps(prob_total, _mm_shuffle_ps(prob_total, prob_total, 0x01));
	const float prob_total_ss = _mm_cvtss_f32(prob_total);
	if (prob_total_ss > 0.f && prob_total_ss != 1.f) {
		const __m128 prob_total_scale = _mm_set1_ps(1.f / prob_total_ss);
		for (i = 0; i < in_out_length4; i += 4) {
			_mm_storeu_ps(&in_out_ptr[i], _mm_mul_ps(_mm_loadu_ps(&in_out_ptr[i]), prob_total_scale));
		}
		for (; i < in_out_length; i += 1) {
			_mm_store_ss(&in_out_ptr[i], _mm_mul_ss(_mm_load_ss(&in_out_ptr[i]), prob_total_scale));
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
