/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsTanh_64f64f_Intrin_SSE41(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	const __m128d vecZero = _mm_setzero_pd();
	const __m128d vecMinus1 = _mm_set1_pd(-1.0);
	const __m128d vecPlus1 = _mm_set1_pd(1.0);
	const __m128d vecScale = _mm_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		__m128d vecX = _mm_load_pd(&in_ptr[i]);
		__m128d vecSign = _mm_cmplt_pd(vecX, vecZero);
		vecSign = _mm_or_pd(_mm_and_pd(vecSign, vecMinus1), _mm_andnot_pd(vecSign, vecPlus1));
		vecX = _mm_mul_pd(_mm_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttpd_epi32(vecX); // upper values set to 0 -> https://www.felixcloutier.com/x86/cvtpd2dq
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1); // '0 < Lut_length_minus1' -> upper values set to 0xFF
		if (_mm_movemask_epi8(vecIndexMask) & 0xff) { // only lower #2 32B
			const __m128d vecIndexRounded = _mm_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			vecIndexMask = _mm_unpacklo_epi32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 1));
			const __m128d vecTanh_i0 = _mm_unpacklo_pd(vecLUT0, vecLUT1);
			const __m128d vecTanh_i1 = _mm_unpackhi_pd(vecLUT0, vecLUT1);
			const __m128d vecResult = _mm_mul_pd(
				_mm_add_pd(vecTanh_i0, _mm_mul_pd(_mm_sub_pd(vecTanh_i1, vecTanh_i0), _mm_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_pd(&out_ptr[i], 
				_mm_or_pd(_mm_and_pd(vecResult, _mm_castsi128_pd(vecIndexMask)), _mm_andnot_pd(_mm_castsi128_pd(vecIndexMask), vecSign))
			);
		}
		else {
			_mm_storeu_pd(&out_ptr[i], vecSign);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsTanhMul_64f64f_Intrin_SSE41(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	const __m128d vecZero = _mm_setzero_pd();
	const __m128d vecMinus1 = _mm_set1_pd(-1.0);
	const __m128d vecPlus1 = _mm_set1_pd(1.0);
	const __m128d vecScale = _mm_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		__m128d vecX = _mm_load_pd(&in_ptr[i]);
		__m128d vecSign = _mm_cmplt_pd(vecX, vecZero);
		vecSign = _mm_or_pd(_mm_and_pd(vecSign, vecMinus1), _mm_andnot_pd(vecSign, vecPlus1));
		vecX = _mm_mul_pd(_mm_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttpd_epi32(vecX); // upper values set to 0 -> https://www.felixcloutier.com/x86/cvtpd2dq
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1); // '0 < Lut_length_minus1' -> upper values set to 0xFF
		if (_mm_movemask_epi8(vecIndexMask) & 0xff) { // only lower #2 32B
			const __m128d vecIndexRounded = _mm_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			vecIndexMask = _mm_unpacklo_epi32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 1));
			const __m128d vecTanh_i0 = _mm_unpacklo_pd(vecLUT0, vecLUT1);
			const __m128d vecTanh_i1 = _mm_unpackhi_pd(vecLUT0, vecLUT1);
			const __m128d vecResult = _mm_mul_pd(
				_mm_add_pd(vecTanh_i0, _mm_mul_pd(_mm_sub_pd(vecTanh_i1, vecTanh_i0), _mm_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_pd(&out_ptr[i],
				_mm_mul_pd(
					_mm_or_pd(_mm_and_pd(vecResult, _mm_castsi128_pd(vecIndexMask)), _mm_andnot_pd(_mm_castsi128_pd(vecIndexMask), vecSign)),
					_mm_loadu_pd(&mul_ptr[i])
				)
			);
		}
		else {
			_mm_storeu_pd(&out_ptr[i], 
				_mm_mul_pd(vecSign, _mm_loadu_pd(&mul_ptr[i]))
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsLogistic_64f64f_Intrin_SSE41(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	const __m128d vecZero = _mm_setzero_pd();
	const __m128d vecMinus1 = _mm_set1_pd(-1.0);
	const __m128d vecPlus1 = _mm_set1_pd(1.0);
	const __m128d vecScale = _mm_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		__m128d vecX = _mm_load_pd(&in_ptr[i]);
		const __m128d vecSignMask = _mm_cmplt_pd(vecX, vecZero);
		const __m128d vecSign = _mm_or_pd(_mm_and_pd(vecSignMask, vecMinus1), _mm_andnot_pd(vecSignMask, vecPlus1));
		vecX = _mm_mul_pd(_mm_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttpd_epi32(vecX); // upper values set to 0 -> https://www.felixcloutier.com/x86/cvtpd2dq
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1); // '0 < Lut_length_minus1' -> upper values set to 0xFF
		if (_mm_movemask_epi8(vecIndexMask) & 0xff) { // only lower #2 32B
			const __m128d vecIndexRounded = _mm_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			vecIndexMask = _mm_unpacklo_epi32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 1));
			const __m128d vec_l0 = _mm_unpacklo_pd(vecLUT0, vecLUT1);
			const __m128d vec_l1 = _mm_unpackhi_pd(vecLUT0, vecLUT1);
			const __m128d vecResult = _mm_mul_pd(
				_mm_add_pd(vec_l0, _mm_mul_pd(_mm_sub_pd(vec_l1, vec_l0), _mm_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_pd(&out_ptr[i],
				_mm_add_pd(
					_mm_or_pd(_mm_and_pd(vecResult, _mm_castsi128_pd(vecIndexMask)), _mm_andnot_pd(_mm_castsi128_pd(vecIndexMask), vecSign)),
					_mm_and_pd(vecSignMask, vecPlus1)
				)
			);
		}
		else {
			_mm_storeu_pd(&out_ptr[i], 
				_mm_add_pd(
					vecSign,
					_mm_and_pd(vecSignMask, vecPlus1)
				)
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 2
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsLogisticMul_64f64f_Intrin_SSE41(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	const __m128d vecZero = _mm_setzero_pd();
	const __m128d vecMinus1 = _mm_set1_pd(-1.0);
	const __m128d vecPlus1 = _mm_set1_pd(1.0);
	const __m128d vecScale = _mm_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 2) {
		__m128d vecX = _mm_load_pd(&in_ptr[i]);
		const __m128d vecSignMask = _mm_cmplt_pd(vecX, vecZero);
		const __m128d vecSign = _mm_or_pd(_mm_and_pd(vecSignMask, vecMinus1), _mm_andnot_pd(vecSignMask, vecPlus1));
		vecX = _mm_mul_pd(_mm_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttpd_epi32(vecX); // upper values set to 0 -> https://www.felixcloutier.com/x86/cvtpd2dq
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1); // '0 < Lut_length_minus1' -> upper values set to 0xFF
		if (_mm_movemask_epi8(vecIndexMask) & 0xff) { // only lower #2 32B
			const __m128d vecIndexRounded = _mm_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			vecIndexMask = _mm_unpacklo_epi32(vecIndexMask, vecIndexMask); // Convert low #2 32B -> #2 64B
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex, 1));
			const __m128d vec_l0 = _mm_unpacklo_pd(vecLUT0, vecLUT1);
			const __m128d vec_l1 = _mm_unpackhi_pd(vecLUT0, vecLUT1);
			const __m128d vecResult = _mm_mul_pd(
				_mm_add_pd(vec_l0, _mm_mul_pd(_mm_sub_pd(vec_l1, vec_l0), _mm_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_pd(&out_ptr[i],
				_mm_mul_pd(
					_mm_add_pd(
						_mm_or_pd(_mm_and_pd(vecResult, _mm_castsi128_pd(vecIndexMask)), _mm_andnot_pd(_mm_castsi128_pd(vecIndexMask), vecSign)),
						_mm_and_pd(vecSignMask, vecPlus1)
					),
					_mm_loadu_pd(&mul_ptr[i])
				)
			);
		}
		else {
			_mm_storeu_pd(&out_ptr[i],
				_mm_mul_pd(
					_mm_add_pd(vecSign, _mm_and_pd(vecSignMask, vecPlus1)),
					_mm_loadu_pd(&mul_ptr[i])
				)
			);
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
