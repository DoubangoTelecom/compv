/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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
//	- Up to the caller to make sure LUT is padded to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsTanh_32f32f_Intrin_SSE41(
	COMPV_ALIGNED(SSE) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	COMPV_ASSERT(!(in_out_length & 3));
	const __m128 vecZero = _mm_setzero_ps();
	const __m128 vecMinus1 = _mm_set1_ps(-1.0);
	const __m128 vecPlus1 = _mm_set1_ps(1.0);
	const __m128 vecScale = _mm_set1_ps(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m128 vecX = _mm_loadu_ps(&in_ptr[i]);
		__m128 vecSign = _mm_cmplt_ps(vecX, vecZero);
		vecSign = _mm_or_ps(_mm_and_ps(vecSign, vecMinus1), _mm_andnot_ps(vecSign, vecPlus1));
		vecX = _mm_mul_ps(_mm_mul_ps(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttps_epi32(vecX);
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask)) {
			const __m128 vecIndexRounded = _mm_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to float 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded 
			const __m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			const __m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vecTanh_i0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vecTanh_i1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			const __m128 vecResult = _mm_mul_ps(
				_mm_add_ps(vecTanh_i0, _mm_mul_ps(_mm_sub_ps(vecTanh_i1, vecTanh_i0), _mm_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_ps(&out_ptr[i], 
				_mm_or_ps(_mm_and_ps(vecResult, _mm_castsi128_ps(vecIndexMask)), _mm_andnot_ps(_mm_castsi128_ps(vecIndexMask), vecSign))
			);
		}
		else {
			_mm_storeu_ps(&out_ptr[i], vecSign);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsTanhMul_32f32f_Intrin_SSE41(
	COMPV_ALIGNED(SSE) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	const compv_float32_t* mul_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	COMPV_ASSERT(!(in_out_length & 3));
	const __m128 vecZero = _mm_setzero_ps();
	const __m128 vecMinus1 = _mm_set1_ps(-1.0);
	const __m128 vecPlus1 = _mm_set1_ps(1.0);
	const __m128 vecScale = _mm_set1_ps(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m128 vecX = _mm_loadu_ps(&in_ptr[i]);
		__m128 vecSign = _mm_cmplt_ps(vecX, vecZero);
		vecSign = _mm_or_ps(_mm_and_ps(vecSign, vecMinus1), _mm_andnot_ps(vecSign, vecPlus1));
		vecX = _mm_mul_ps(_mm_mul_ps(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttps_epi32(vecX);
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask)) {
			const __m128 vecIndexRounded = _mm_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to float 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded
			const __m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			const __m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vecTanh_i0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vecTanh_i1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			const __m128 vecResult = _mm_mul_ps(
				_mm_add_ps(vecTanh_i0, _mm_mul_ps(_mm_sub_ps(vecTanh_i1, vecTanh_i0), _mm_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_ps(&out_ptr[i],
				_mm_mul_ps(
					_mm_or_ps(_mm_and_ps(vecResult, _mm_castsi128_ps(vecIndexMask)), _mm_andnot_ps(_mm_castsi128_ps(vecIndexMask), vecSign)),
					_mm_loadu_ps(&mul_ptr[i])
				)
			);
		}
		else {
			_mm_storeu_ps(
				&out_ptr[i], 
				_mm_mul_ps(
					vecSign,
					_mm_loadu_ps(&mul_ptr[i])
				)
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsLogistic_32f32f_Intrin_SSE41(
	COMPV_ALIGNED(SSE) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	COMPV_ASSERT(!(in_out_length & 3));
	const __m128 vecZero = _mm_setzero_ps();
	const __m128 vecMinus1 = _mm_set1_ps(-1.0);
	const __m128 vecPlus1 = _mm_set1_ps(1.0);
	const __m128 vecScale = _mm_set1_ps(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m128 vecX = _mm_loadu_ps(&in_ptr[i]);
		const __m128 vecSignMask = _mm_cmplt_ps(vecX, vecZero);
		const __m128 vecSign = _mm_or_ps(_mm_and_ps(vecSignMask, vecMinus1), _mm_andnot_ps(vecSignMask, vecPlus1));
		vecX = _mm_mul_ps(_mm_mul_ps(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttps_epi32(vecX);
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask)) {
			const __m128 vecIndexRounded = _mm_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT must be padded
			const __m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			const __m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vec_l0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vec_l1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			const __m128 vecResult = _mm_mul_ps(
				_mm_add_ps(vec_l0, _mm_mul_ps(_mm_sub_ps(vec_l1, vec_l0), _mm_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_ps(&out_ptr[i],
				_mm_add_ps(
					_mm_or_ps(_mm_and_ps(vecResult, _mm_castsi128_ps(vecIndexMask)), _mm_andnot_ps(_mm_castsi128_ps(vecIndexMask), vecSign)),
					_mm_and_ps(vecSignMask, vecPlus1)
				)
			);
		}
		else {
			_mm_storeu_ps(&out_ptr[i], 
				_mm_add_ps(
					vecSign,
					_mm_and_ps(vecSignMask, vecPlus1)
				)
			);
		}
	}
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
// "_mm_min_epi32" and "_mm_extract_epi32" are SSE4.1
void CompVMathActivationFunctionsLogisticMul_32f32f_Intrin_SSE41(
	COMPV_ALIGNED(SSE) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(4) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	const compv_float32_t* mul_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	COMPV_ASSERT(!(in_out_length & 3));
	const __m128 vecZero = _mm_setzero_ps();
	const __m128 vecMinus1 = _mm_set1_ps(-1.0);
	const __m128 vecPlus1 = _mm_set1_ps(1.0);
	const __m128 vecScale = _mm_set1_ps(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m128 vecX = _mm_loadu_ps(&in_ptr[i]);
		const __m128 vecSignMask = _mm_cmplt_ps(vecX, vecZero);
		const __m128 vecSign = _mm_or_ps(_mm_and_ps(vecSignMask, vecMinus1), _mm_andnot_ps(vecSignMask, vecPlus1));
		vecX = _mm_mul_ps(_mm_mul_ps(vecX, vecSign), vecScale);
		__m128i vecIndex = _mm_cvttps_epi32(vecX);
		__m128i vecIndexMask = _mm_cmplt_epi32(vecIndex, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask)) {
			const __m128 vecIndexRounded = _mm_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT must be padded
			const __m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			const __m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vec_l0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vec_l1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			const __m128 vecResult = _mm_mul_ps(
				_mm_add_ps(vec_l0, _mm_mul_ps(_mm_sub_ps(vec_l1, vec_l0), _mm_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);
			_mm_storeu_ps(&out_ptr[i],
				_mm_mul_ps(
					_mm_add_ps(
						_mm_or_ps(_mm_and_ps(vecResult, _mm_castsi128_ps(vecIndexMask)), _mm_andnot_ps(_mm_castsi128_ps(vecIndexMask), vecSign)),
						_mm_and_ps(vecSignMask, vecPlus1)
					),
					_mm_loadu_ps(&mul_ptr[i])
				)
			);
		}
		else {
			_mm_storeu_ps(&out_ptr[i],
				_mm_mul_ps(
					_mm_add_ps(vecSign, _mm_and_ps(vecSignMask, vecPlus1)),
					_mm_loadu_ps(&mul_ptr[i])
				)
			);
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
