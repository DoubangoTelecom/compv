/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsTanh_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const __m256d vecZero = _mm256_setzero_pd();
	const __m256d vecMinus1 = _mm256_set1_pd(-1.0);
	const __m256d vecPlus1 = _mm256_set1_pd(1.0);
	const __m256d vecScale = _mm256_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m256d vecX = _mm256_loadu_pd(&in_ptr[i]);
		const __m256d vecSign = _mm256_blendv_pd(vecPlus1, vecMinus1, _mm256_cmp_pd(vecX, vecZero, _CMP_LT_OQ));
		vecX = _mm256_mul_pd(_mm256_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex128 = _mm256_cvttpd_epi32(vecX);
		const __m128i vecIndexMask128 = _mm_cmplt_epi32(vecIndex128, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask128)) {
			const __m256d vecIndexRounded = _mm256_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex128 = _mm_min_epi32(vecIndex128, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const __m256i vecIndexMask256 = _mm256_insertf128_si256(
				_mm256_castsi128_si256(_mm_unpacklo_epi32(vecIndexMask128, vecIndexMask128)),
				_mm_unpackhi_epi32(vecIndexMask128, vecIndexMask128), 0x1
			); // Convert low #4 32B -> #4 64B
			// Next code could be replaced with "_mm256_i32gather_pd" -> see AVX2 version
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 1));
			const __m128d vecLUT2 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 2));
			const __m128d vecLUT3 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 3));
			const __m256d vecTanh_i0 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpacklo_pd(vecLUT0, vecLUT1)), _mm_unpacklo_pd(vecLUT2, vecLUT3), 0x1);
			const __m256d vecTanh_i1 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpackhi_pd(vecLUT0, vecLUT1)), _mm_unpackhi_pd(vecLUT2, vecLUT3), 0x1);
#if 1
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_add_pd(vecTanh_i0, _mm256_mul_pd(_mm256_sub_pd(vecTanh_i1, vecTanh_i0), _mm256_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
#else // FMA version
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_fmadd_pd(_mm256_sub_pd(vecTanh_i1, vecTanh_i0), _mm256_sub_pd(vecX, vecIndexRounded), vecTanh_i0),
				vecSign
			);
#endif
			_mm256_storeu_pd(&out_ptr[i],
				_mm256_blendv_pd(vecSign, vecResult, _mm256_castsi256_pd(vecIndexMask256))
			);
		}
		else {
			_mm256_storeu_pd(&out_ptr[i], vecSign);
		}
	}
	_mm256_zeroupper();
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsTanhMul_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const __m256d vecZero = _mm256_setzero_pd();
	const __m256d vecMinus1 = _mm256_set1_pd(-1.0);
	const __m256d vecPlus1 = _mm256_set1_pd(1.0);
	const __m256d vecScale = _mm256_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m256d vecX = _mm256_loadu_pd(&in_ptr[i]);
		const __m256d vecSign = _mm256_blendv_pd(vecPlus1, vecMinus1, _mm256_cmp_pd(vecX, vecZero, _CMP_LT_OQ));
		vecX = _mm256_mul_pd(_mm256_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex128 = _mm256_cvttpd_epi32(vecX);
		const __m128i vecIndexMask128 = _mm_cmplt_epi32(vecIndex128, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask128)) {
			const __m256d vecIndexRounded = _mm256_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex128 = _mm_min_epi32(vecIndex128, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const __m256i vecIndexMask256 = _mm256_insertf128_si256(
				_mm256_castsi128_si256(_mm_unpacklo_epi32(vecIndexMask128, vecIndexMask128)),
				_mm_unpackhi_epi32(vecIndexMask128, vecIndexMask128), 0x1
			); // Convert low #4 32B -> #4 64B
			   // Next code could be replaced with "_mm256_i32gather_pd" -> see AVX2 version
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 1));
			const __m128d vecLUT2 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 2));
			const __m128d vecLUT3 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 3));
			const __m256d vecTanh_i0 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpacklo_pd(vecLUT0, vecLUT1)), _mm_unpacklo_pd(vecLUT2, vecLUT3), 0x1);
			const __m256d vecTanh_i1 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpackhi_pd(vecLUT0, vecLUT1)), _mm_unpackhi_pd(vecLUT2, vecLUT3), 0x1);
#if 1
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_add_pd(vecTanh_i0, _mm256_mul_pd(_mm256_sub_pd(vecTanh_i1, vecTanh_i0), _mm256_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
#else // FMA version
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_fmadd_pd(_mm256_sub_pd(vecTanh_i1, vecTanh_i0), _mm256_sub_pd(vecX, vecIndexRounded), vecTanh_i0),
				vecSign
			);
#endif
			_mm256_storeu_pd(&out_ptr[i],
				_mm256_mul_pd(
					_mm256_blendv_pd(vecSign, vecResult, _mm256_castsi256_pd(vecIndexMask256)),
					_mm256_loadu_pd(&mul_ptr[i])
				)
			);
		}
		else {
			_mm256_storeu_pd(&out_ptr[i], 
				_mm256_mul_pd(
					vecSign,
					_mm256_loadu_pd(&mul_ptr[i])
				)
			);
		}
	}
	_mm256_zeroupper();
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsLogistic_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const __m256d vecZero = _mm256_setzero_pd();
	const __m256d vecMinus1 = _mm256_set1_pd(-1.0);
	const __m256d vecPlus1 = _mm256_set1_pd(1.0);
	const __m256d vecScale = _mm256_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m256d vecX = _mm256_loadu_pd(&in_ptr[i]);
		const __m256d vecSignMask = _mm256_cmp_pd(vecX, vecZero, _CMP_LT_OQ);
		const __m256d vecSign = _mm256_blendv_pd(vecPlus1, vecMinus1, vecSignMask);
		vecX = _mm256_mul_pd(_mm256_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex128 = _mm256_cvttpd_epi32(vecX);
		const __m128i vecIndexMask128 = _mm_cmplt_epi32(vecIndex128, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask128)) {
			const __m256d vecIndexRounded = _mm256_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex128 = _mm_min_epi32(vecIndex128, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const __m256i vecIndexMask256 = _mm256_insertf128_si256(
				_mm256_castsi128_si256(_mm_unpacklo_epi32(vecIndexMask128, vecIndexMask128)),
				_mm_unpackhi_epi32(vecIndexMask128, vecIndexMask128), 0x1
			); // Convert low #4 32B -> #4 64B
			   // Next code could be replaced with "_mm256_i32gather_pd" -> see AVX2 version
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 1));
			const __m128d vecLUT2 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 2));
			const __m128d vecLUT3 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 3));
			const __m256d vec_l0 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpacklo_pd(vecLUT0, vecLUT1)), _mm_unpacklo_pd(vecLUT2, vecLUT3), 0x1);
			const __m256d vec_l1 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpackhi_pd(vecLUT0, vecLUT1)), _mm_unpackhi_pd(vecLUT2, vecLUT3), 0x1);
#if 1
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_add_pd(vec_l0, _mm256_mul_pd(_mm256_sub_pd(vec_l1, vec_l0), _mm256_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
#else // FMA version
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_fmadd_pd(_mm256_sub_pd(vec_l1, vec_l0), _mm256_sub_pd(vecX, vecIndexRounded), vec_l0),
				vecSign
			);
#endif
			_mm256_storeu_pd(&out_ptr[i],
				_mm256_add_pd(
					_mm256_blendv_pd(vecSign, vecResult, _mm256_castsi256_pd(vecIndexMask256)),
					_mm256_and_pd(vecSignMask, vecPlus1)
				)
			);
		}
		else {
			_mm256_storeu_pd(&out_ptr[i], 
				_mm256_add_pd(
					vecSign,
					_mm256_and_pd(vecSignMask, vecPlus1)
				)
			);
		}
	}
	_mm256_zeroupper();
}

// This is an internal function
//	- Up to the caller to make sure LUT is padded with at least #1 double to alow reading beyond valid data
//	- Make sure in_out_length is multiple of 4
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathActivationFunctionsLogisticMul_64f64f_Intrin_AVX(
	const compv_float64_t* lut_ptr,
	const compv_uscalar_t& lut_length_minus1,
	const compv_float64_t* scale1,
	const compv_uscalar_t& in_out_length,
	const compv_float64_t* in_ptr,
	const compv_float64_t* mul_ptr,
	compv_float64_t* out_ptr
)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const __m256d vecZero = _mm256_setzero_pd();
	const __m256d vecMinus1 = _mm256_set1_pd(-1.0);
	const __m256d vecPlus1 = _mm256_set1_pd(1.0);
	const __m256d vecScale = _mm256_set1_pd(*scale1);
	const __m128i vecLut_length_minus1 = _mm_set1_epi32(static_cast<int32_t>(lut_length_minus1));
	for (compv_uscalar_t i = 0; i < in_out_length; i += 4) {
		__m256d vecX = _mm256_loadu_pd(&in_ptr[i]);
		const __m256d vecSignMask = _mm256_cmp_pd(vecX, vecZero, _CMP_LT_OQ);
		const __m256d vecSign = _mm256_blendv_pd(vecPlus1, vecMinus1, vecSignMask);
		vecX = _mm256_mul_pd(_mm256_mul_pd(vecX, vecSign), vecScale);
		__m128i vecIndex128 = _mm256_cvttpd_epi32(vecX);
		const __m128i vecIndexMask128 = _mm_cmplt_epi32(vecIndex128, vecLut_length_minus1);
		if (_mm_movemask_epi8(vecIndexMask128)) {
			const __m256d vecIndexRounded = _mm256_round_pd(vecX, _MM_FROUND_FLOOR); // same as converting vecIndex to double 
			vecIndex128 = _mm_min_epi32(vecIndex128, vecLut_length_minus1); // Clip indices to avoid reading beyond valid data. LUT should be padded with at lease #1 double
			const __m256i vecIndexMask256 = _mm256_insertf128_si256(
				_mm256_castsi128_si256(_mm_unpacklo_epi32(vecIndexMask128, vecIndexMask128)),
				_mm_unpackhi_epi32(vecIndexMask128, vecIndexMask128), 0x1
			); // Convert low #4 32B -> #4 64B
			   // Next code could be replaced with "_mm256_i32gather_pd" -> see AVX2 version
			const __m128d vecLUT0 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 0));
			const __m128d vecLUT1 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 1));
			const __m128d vecLUT2 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 2));
			const __m128d vecLUT3 = _mm_loadu_pd(lut_ptr + _mm_extract_epi32(vecIndex128, 3));
			const __m256d vec_l0 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpacklo_pd(vecLUT0, vecLUT1)), _mm_unpacklo_pd(vecLUT2, vecLUT3), 0x1);
			const __m256d vec_l1 = _mm256_insertf128_pd(_mm256_castpd128_pd256(_mm_unpackhi_pd(vecLUT0, vecLUT1)), _mm_unpackhi_pd(vecLUT2, vecLUT3), 0x1);
#if 1
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_add_pd(vec_l0, _mm256_mul_pd(_mm256_sub_pd(vec_l1, vec_l0), _mm256_sub_pd(vecX, vecIndexRounded))),
				vecSign
			);
#else // FMA version
			const __m256d vecResult = _mm256_mul_pd(
				_mm256_fmadd_pd(_mm256_sub_pd(vec_l1, vec_l0), _mm256_sub_pd(vecX, vecIndexRounded), vec_l0),
				vecSign
			);
#endif
			_mm256_storeu_pd(&out_ptr[i],
				_mm256_mul_pd(
					_mm256_add_pd(
						_mm256_blendv_pd(vecSign, vecResult, _mm256_castsi256_pd(vecIndexMask256)),
						_mm256_and_pd(vecSignMask, vecPlus1)
					),
					_mm256_loadu_pd(&mul_ptr[i])
				)
			);
		}
		else {
			_mm256_storeu_pd(&out_ptr[i],
				_mm256_mul_pd(
					_mm256_add_pd(
						vecSign,
						_mm256_and_pd(vecSignMask, vecPlus1)
					),
					_mm256_loadu_pd(&mul_ptr[i])
				)
			);
		}
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
