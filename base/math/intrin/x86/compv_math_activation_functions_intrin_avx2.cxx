/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 8
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsTanh_32f32f_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(8) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_ASSERT(!(in_out_length & 7));
	_mm256_zeroupper();
	const __m256 vecZero = _mm256_setzero_ps();
	const __m128 vecZero128 = _mm_setzero_ps();
	const __m256 vecMinus1 = _mm256_set1_ps(-1.0);
	const __m256 vecPlus1 = _mm256_set1_ps(1.0);
	const __m256 vecScale = _mm256_set1_ps(*scale1);
	const __m256i vecLut_length_minus1 = _mm256_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 8) {
		__m256 vecX = _mm256_loadu_ps(&in_ptr[i]);
		const __m256 vecSign = _mm256_blendv_ps(vecPlus1, vecMinus1, _mm256_cmp_ps(vecX, vecZero, _CMP_LT_OQ));
		vecX = _mm256_mul_ps(_mm256_mul_ps(vecX, vecSign), vecScale);
		__m256i vecIndex = _mm256_cvttps_epi32(vecX);
		__m256i vecIndexMask = _mm256_cmpgt_epi32(vecLut_length_minus1, vecIndex);
		if (_mm256_movemask_epi8(vecIndexMask)) {
			const __m256 vecIndexRounded = _mm256_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to float 
			vecIndex = _mm256_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded 
			const __m128i vecIndex0 = _mm256_castsi256_si128(vecIndex);
			const __m128i vecIndex1 = _mm256_extractf128_si256(vecIndex, 1);
			__m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			__m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vecTanh_low_i0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vecTanh_low_i1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vecTanh_high_i0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vecTanh_high_i1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]

			// Concat(high,low)
			const __m256 vecTanh_i0 = _mm256_insertf128_ps(_mm256_castps128_ps256(vecTanh_low_i0), vecTanh_high_i0, 0x1);
			const __m256 vecTanh_i1 = _mm256_insertf128_ps(_mm256_castps128_ps256(vecTanh_low_i1), vecTanh_high_i1, 0x1);

			const __m256 vecResult = _mm256_mul_ps(
				_mm256_add_ps(vecTanh_i0, _mm256_mul_ps(_mm256_sub_ps(vecTanh_i1, vecTanh_i0), _mm256_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);
			_mm256_storeu_ps(&out_ptr[i],
				_mm256_blendv_ps(vecSign, vecResult, _mm256_castsi256_ps(vecIndexMask))
			);
		}
		else {
			_mm256_storeu_ps(&out_ptr[i], vecSign);
		}
	}
	_mm256_zeroupper();
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 8
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsTanhMul_32f32f_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(8) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	const compv_float32_t* mul_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	COMPV_ASSERT(!(in_out_length & 7));
	_mm256_zeroupper();
	const __m256 vecZero = _mm256_setzero_ps();
	const __m128 vecZero128 = _mm_setzero_ps();
	const __m256 vecMinus1 = _mm256_set1_ps(-1.0);
	const __m256 vecPlus1 = _mm256_set1_ps(1.0);
	const __m256 vecScale = _mm256_set1_ps(*scale1);
	const __m256i vecLut_length_minus1 = _mm256_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 8) {
		__m256 vecX = _mm256_loadu_ps(&in_ptr[i]);
		const __m256 vecSign = _mm256_blendv_ps(vecPlus1, vecMinus1, _mm256_cmp_ps(vecX, vecZero, _CMP_LT_OQ));
		vecX = _mm256_mul_ps(_mm256_mul_ps(vecX, vecSign), vecScale);
		__m256i vecIndex = _mm256_cvttps_epi32(vecX);
		__m256i vecIndexMask = _mm256_cmpgt_epi32(vecLut_length_minus1, vecIndex);
		if (_mm256_movemask_epi8(vecIndexMask)) {
			const __m256 vecIndexRounded = _mm256_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to float 
			vecIndex = _mm256_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded 
			const __m128i vecIndex0 = _mm256_castsi256_si128(vecIndex);
			const __m128i vecIndex1 = _mm256_extractf128_si256(vecIndex, 1);
			__m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			__m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vecTanh_low_i0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vecTanh_low_i1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vecTanh_high_i0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vecTanh_high_i1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]

			// Concat(high,low)
			const __m256 vecTanh_i0 = _mm256_insertf128_ps(_mm256_castps128_ps256(vecTanh_low_i0), vecTanh_high_i0, 0x1);
			const __m256 vecTanh_i1 = _mm256_insertf128_ps(_mm256_castps128_ps256(vecTanh_low_i1), vecTanh_high_i1, 0x1);

			const __m256 vecResult = _mm256_mul_ps(
				_mm256_add_ps(vecTanh_i0, _mm256_mul_ps(_mm256_sub_ps(vecTanh_i1, vecTanh_i0), _mm256_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);
			_mm256_storeu_ps(&out_ptr[i],
				_mm256_mul_ps(
					_mm256_blendv_ps(vecSign, vecResult, _mm256_castsi256_ps(vecIndexMask)),
					_mm256_loadu_ps(&mul_ptr[i])
				)
			);
		}
		else {
			_mm256_storeu_ps(&out_ptr[i],
				_mm256_mul_ps(
					vecSign,
					_mm256_loadu_ps(&mul_ptr[i])
				)
			);
		}
	}
	_mm256_zeroupper();
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 8
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsLogistic_32f32f_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(8) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	const __m256 vecZero = _mm256_setzero_ps();
	const __m128 vecZero128 = _mm_setzero_ps();
	const __m256 vecMinus1 = _mm256_set1_ps(-1.0);
	const __m256 vecPlus1 = _mm256_set1_ps(1.0);
	const __m256 vecScale = _mm256_set1_ps(*scale1);
	const __m256i vecLut_length_minus1 = _mm256_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 8) {
		__m256 vecX = _mm256_loadu_ps(&in_ptr[i]);
		const __m256 vecSignMask = _mm256_cmp_ps(vecX, vecZero, _CMP_LT_OQ);
		const __m256 vecSign = _mm256_blendv_ps(vecPlus1, vecMinus1, vecSignMask);
		vecX = _mm256_mul_ps(_mm256_mul_ps(vecX, vecSign), vecScale);
		__m256i vecIndex = _mm256_cvttps_epi32(vecX);
		const __m256i vecIndexMask = _mm256_cmpgt_epi32(vecLut_length_minus1, vecIndex);
		if (_mm256_movemask_epi8(vecIndexMask)) {
			const __m256 vecIndexRounded = _mm256_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm256_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double

			// Next code could be replaced with "_mm256_i32gather_ps" -> see AVX2 version
			const __m128i vecIndex0 = _mm256_castsi256_si128(vecIndex);
			const __m128i vecIndex1 = _mm256_extractf128_si256(vecIndex, 1);
			__m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			__m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vec_low_l0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vec_low_l1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vec_high_l0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vec_high_l1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]

			// Concat(high,low)
			const __m256 vec_l0 = _mm256_insertf128_ps(_mm256_castps128_ps256(vec_low_l0), vec_high_l0, 0x1);
			const __m256 vec_l1 = _mm256_insertf128_ps(_mm256_castps128_ps256(vec_low_l1), vec_high_l1, 0x1);

			const __m256 vecResult = _mm256_mul_ps(
				_mm256_add_ps(vec_l0, _mm256_mul_ps(_mm256_sub_ps(vec_l1, vec_l0), _mm256_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);
			_mm256_storeu_ps(&out_ptr[i],
				_mm256_add_ps(
					_mm256_blendv_ps(vecSign, vecResult, _mm256_castsi256_ps(vecIndexMask)),
					_mm256_and_ps(vecSignMask, vecPlus1)
				)
			);
		}
		else {
			_mm256_storeu_ps(&out_ptr[i],
				_mm256_add_ps(
					vecSign,
					_mm256_and_ps(vecSignMask, vecPlus1)
				)
			);
		}
	}
	_mm256_zeroupper();
}


// This is an internal function
//	- Up to the caller to make sure LUT is padded
//	- Make sure in_out_length is multiple of 8
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsLogisticMul_32f32f_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const compv_float32_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float32_t* scale1,
	COMPV_ALIGNED(8) const compv_uscalar_t& in_out_length,
	const compv_float32_t* in_ptr,
	const compv_float32_t* mul_ptr,
	compv_float32_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	const __m256 vecZero = _mm256_setzero_ps();
	const __m128 vecZero128 = _mm_setzero_ps();
	const __m256 vecMinus1 = _mm256_set1_ps(-1.0);
	const __m256 vecPlus1 = _mm256_set1_ps(1.0);
	const __m256 vecScale = _mm256_set1_ps(*scale1);
	const __m256i vecLut_length_minus1 = _mm256_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 8) {
		__m256 vecX = _mm256_loadu_ps(&in_ptr[i]);
		const __m256 vecSignMask = _mm256_cmp_ps(vecX, vecZero, _CMP_LT_OQ);
		const __m256 vecSign = _mm256_blendv_ps(vecPlus1, vecMinus1, vecSignMask);
		vecX = _mm256_mul_ps(_mm256_mul_ps(vecX, vecSign), vecScale);
		__m256i vecIndex = _mm256_cvttps_epi32(vecX);
		const __m256i vecIndexMask = _mm256_cmpgt_epi32(vecLut_length_minus1, vecIndex);
		if (_mm256_movemask_epi8(vecIndexMask)) {
			const __m256 vecIndexRounded = _mm256_round_ps(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex = _mm256_min_epi32(vecIndex, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double

			// Next code could be replaced with "_mm256_i32gather_ps" -> see AVX2 version
			const __m128i vecIndex0 = _mm256_castsi256_si128(vecIndex);
			const __m128i vecIndex1 = _mm256_extractf128_si256(vecIndex, 1);
			__m128 vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			__m128 vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex0, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vec_low_l0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vec_low_l1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]
			vecLUT0 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 0))), // 0 | 0 | index+1 | index
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 1)))  // 0 | 0 | index+1 | index
			); // index+1[1] | index+1[0] | index[1] | index[0]
			vecLUT1 = _mm_unpacklo_ps(
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 2))), // 0 | 0 | index+1 | index 
				_mm_loadl_pi(vecZero128, reinterpret_cast<const __m64*>(lut_ptr + _mm_extract_epi32(vecIndex1, 3)))  // 0 | 0 | index+1 | index
			); // index+1[3] | index+1[2] | index[3] | index[2]
			const __m128 vec_high_l0 = _mm_castsi128_ps(_mm_unpacklo_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index[3]   | index[2]   | index[1]   | index[0]
			const __m128 vec_high_l1 = _mm_castsi128_ps(_mm_unpackhi_epi64(_mm_castps_si128(vecLUT0), _mm_castps_si128(vecLUT1))); // index+1[3] | index+1[2] | index+1[1] | index+1[0]

			// Concat(high,low)
			const __m256 vec_l0 = _mm256_insertf128_ps(_mm256_castps128_ps256(vec_low_l0), vec_high_l0, 0x1);
			const __m256 vec_l1 = _mm256_insertf128_ps(_mm256_castps128_ps256(vec_low_l1), vec_high_l1, 0x1);

			const __m256 vecResult = _mm256_mul_ps(
				_mm256_add_ps(vec_l0, _mm256_mul_ps(_mm256_sub_ps(vec_l1, vec_l0), _mm256_sub_ps(vecX, vecIndexRounded))),
				vecSign
			);

			_mm256_storeu_ps(&out_ptr[i],
				_mm256_mul_ps(
					_mm256_add_ps(
						_mm256_blendv_ps(vecSign, vecResult, _mm256_castsi256_ps(vecIndexMask)),
						_mm256_and_ps(vecSignMask, vecPlus1)
					),
					_mm256_loadu_ps(&mul_ptr[i])
				)
			);
		}
		else {
			_mm256_storeu_ps(&out_ptr[i],
				_mm256_mul_ps(
					_mm256_add_ps(
						vecSign,
						_mm256_and_ps(vecSignMask, vecPlus1)
					),
					_mm256_loadu_ps(&mul_ptr[i])
				)
			);
		}
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
