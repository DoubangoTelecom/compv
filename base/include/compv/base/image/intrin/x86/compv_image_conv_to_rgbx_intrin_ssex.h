/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_TO_RGBX_INTRIN_SSE_H_)
#define _COMPV_BASE_IMAGE_CONV_TO_RGBX_INTRIN_SSE_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#define rgb24_store_SSSE3(ptr, vecR, vecG, vecB, vecA, vectmp0, vectmp1) COMPV_VST3_U8_SSSE3(ptr, vecR, vecG, vecB, vectmp0, vectmp1)
#define rgba32_store_SSE2(ptr, vecR, vecG, vecB, vecA, vectmp0, vectmp1) COMPV_VST4_U8_SSE2(ptr, vecR, vecG, vecB, vecA, vectmp0, vectmp1)

#define rgb24_step					48 /* (16 * 3) */
#define rgba32_step					64 /* (16 * 4) */

#define rgb24_bytes_per_sample		3
#define rgba32_bytes_per_sample		4

#define yuv420p_checkAddStrideUV	if (j & 1)
#define yuv422p_checkAddStrideUV 
#define yuv444p_checkAddStrideUV

#define yuv420p_uv_step					8
#define yuv422p_uv_step					8
#define yuv444p_uv_step					16

#define yuv420p_uv_stride				(stride >> 1) /* no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes) */
#define yuv422p_uv_stride				(stride >> 1) /* no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes) */
#define yuv444p_uv_stride				(stride)

#define yuv420p_uv_load					_mm_loadl_epi64
#define yuv422p_uv_load					_mm_loadl_epi64
#define yuv444p_uv_load					_mm_load_si128

#define yuv420p_u_unpackhi				(void)(vecUhi)
#define yuv422p_u_unpackhi				(void)(vecUhi)
#define yuv444p_u_unpackhi				vecUhi = _mm_unpackhi_epi8(vecUlo, vecZero)

#define yuv420p_v_unpackhi				(void)(vecVhi)
#define yuv422p_v_unpackhi				(void)(vecVhi)
#define yuv444p_v_unpackhi				vecVhi = _mm_unpackhi_epi8(vecVlo, vecZero)

#define yuv420p_u_primehi				(void)(vecUhi)
#define yuv422p_u_primehi				(void)(vecUhi)
#define yuv444p_u_primehi				vecUhi = _mm_sub_epi16(vecUhi, vec127)

#define yuv420p_v_primehi				(void)(vecVhi)
#define yuv422p_v_primehi				(void)(vecVhi)
#define yuv444p_v_primehi				vecVhi = _mm_sub_epi16(vecVhi, vec127)

#define yuv420p_u_primehi65				(void)(vec1hi)
#define yuv422p_u_primehi65				(void)(vec1hi)
#define yuv444p_u_primehi65				vec1hi = _mm_mullo_epi16(vecUhi, vec65)

#define yuv420p_v_primehi51				(void)(vec0hi)
#define yuv422p_v_primehi51				(void)(vec0hi)
#define yuv444p_v_primehi51				vec0hi = _mm_mullo_epi16(vecVhi, vec51)

#define yuv420p_final_vec(vec, p)		vec##p = _mm_unpack##p##_epi16(vec##lo, vec##lo)
#define yuv422p_final_vec(vec, p)		vec##p = _mm_unpack##p##_epi16(vec##lo, vec##lo)
#define yuv444p_final_vec(vec, p)		(void)(vec##p)

#define yuv420p_g_high					(void)(vec0hi)
#define yuv422p_g_high					(void)(vec0hi)
#define yuv444p_g_high					vec1lo = _mm_madd_epi16(_mm_unpacklo_epi16(vecUhi, vecVhi), vec13_26); vec1hi = _mm_madd_epi16(_mm_unpackhi_epi16(vecUhi, vecVhi), vec13_26); vec0hi = _mm_packs_epi32(vec1lo, vec1hi)

#define CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(nameYuv, nameRgbx, ssex) { \
	compv_uscalar_t i, j, k, l; \
	const compv_uscalar_t strideUV = nameYuv##_uv_stride;  \
	const compv_uscalar_t strideRGBx = (stride * nameRgbx##_bytes_per_sample); \
	__m128i vecYlo, vecYhi, vecUlo, vecUhi, vecVlo, vecVhi, vecR, vecG, vecB; \
	__m128i vec0lo, vec0hi, vec1lo, vec1hi; \
	static const __m128i vecZero = _mm_setzero_si128(); \
	static const __m128i vec16 = _mm_load_si128(reinterpret_cast<const __m128i*>(k16_i16)); \
	static const __m128i vec37 = _mm_load_si128(reinterpret_cast<const __m128i*>(k37_i16)); \
	static const __m128i vec51 = _mm_load_si128(reinterpret_cast<const __m128i*>(k51_i16)); \
	static const __m128i vec65 = _mm_load_si128(reinterpret_cast<const __m128i*>(k65_i16)); \
	static const __m128i vec127 = _mm_load_si128(reinterpret_cast<const __m128i*>(k127_i16)); \
	static const __m128i vec13_26 = _mm_load_si128(reinterpret_cast<const __m128i*>(k13_26_i16)); /* 13, 26, 13, 26 ... */ \
	static const __m128i vecA = _mm_cmpeq_epi8(vecZero, vecZero); /* FF FF FF FF... */ \
	 \
	for (j = 0; j < height; ++j) { \
		for (i = 0, k = 0, l = 0; i < width; i += 16, k += nameRgbx##_step, l += nameYuv##_uv_step) { \
			/* Load samples */ \
			vecYlo = _mm_load_si128(reinterpret_cast<const __m128i*>(&yPtr[i])); /* #16 Y samples */ \
			vecUlo = nameYuv##_uv_load(reinterpret_cast<const __m128i*>(&uPtr[l])); /* #8 or #16 U samples, lo mem */ \
			vecVlo = nameYuv##_uv_load(reinterpret_cast<const __m128i*>(&vPtr[l])); /* #8 or #16 V samples, lo mem */ \
			 \
			/* Convert to I16 */ \
			vecYhi = _mm_unpackhi_epi8(vecYlo, vecZero); \
			vecYlo = _mm_unpacklo_epi8(vecYlo, vecZero); \
			nameYuv##_u_unpackhi; \
			vecUlo = _mm_unpacklo_epi8(vecUlo, vecZero); \
			nameYuv##_v_unpackhi; \
			vecVlo = _mm_unpacklo_epi8(vecVlo, vecZero); \
			 \
			/* Compute Y', U', V' */\
			vecYlo = _mm_sub_epi16(vecYlo, vec16); \
			vecYhi = _mm_sub_epi16(vecYhi, vec16); \
			vecUlo = _mm_sub_epi16(vecUlo, vec127); \
			nameYuv##_u_primehi; \
			vecVlo = _mm_sub_epi16(vecVlo, vec127); \
			nameYuv##_v_primehi; \
			 \
			/* Compute (37Y'), (51V') and (65U') */ \
			vecYlo = _mm_mullo_epi16(vecYlo, vec37); \
			vecYhi = _mm_mullo_epi16(vecYhi, vec37); \
			vec0lo = _mm_mullo_epi16(vecVlo, vec51); \
			nameYuv##_v_primehi51; \
			vec1lo = _mm_mullo_epi16(vecUlo, vec65); \
			nameYuv##_u_primehi65; \
			 \
			/* Compute R = (37Y' + 0U' + 51V') >> 5 */ \
			nameYuv##_final_vec(vec0, hi); \
			nameYuv##_final_vec(vec0, lo); \
			vecR = _mm_packus_epi16(\
				_mm_srai_epi16(_mm_add_epi16(vecYlo, vec0lo), 5), \
				_mm_srai_epi16(_mm_add_epi16(vecYhi, vec0hi), 5) \
			); \
			 \
			/* B = (37Y' + 65U' + 0V') >> 5 */ \
			nameYuv##_final_vec(vec1, hi); \
			nameYuv##_final_vec(vec1, lo); \
			vecB = _mm_packus_epi16(\
				_mm_srai_epi16(_mm_add_epi16(vecYlo, vec1lo), 5), \
				_mm_srai_epi16(_mm_add_epi16(vecYhi, vec1hi), 5) \
			); \
			 \
			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */ \
			vec0lo = _mm_madd_epi16(_mm_unpacklo_epi16(vecUlo, vecVlo), vec13_26); /* (13U' + 26V').lo - I32 */ \
			vec0hi = _mm_madd_epi16(_mm_unpackhi_epi16(vecUlo, vecVlo), vec13_26); /* (13U' + 26V').hi - I32 */ \
			vec0lo = _mm_packs_epi32(vec0lo, vec0hi); \
			nameYuv##_g_high; \
			nameYuv##_final_vec(vec0, hi); \
			nameYuv##_final_vec(vec0, lo); \
			vecG = _mm_packus_epi16(\
				_mm_srai_epi16(_mm_sub_epi16(vecYlo, vec0lo), 5), \
				_mm_srai_epi16(_mm_sub_epi16(vecYhi, vec0hi), 5) \
			); \
			 \
			/* Store result */ \
			nameRgbx##_store_##ssex##(&rgbxPtr[k], vecR, vecG, vecB, vecA, vec0lo, vec1lo); \
			 \
		} /* End_Of for (i = 0; i < width; i += 16) */ \
		yPtr += stride; \
		rgbxPtr += strideRGBx; \
		nameYuv##_checkAddStrideUV { \
			uPtr += strideUV; \
			vPtr += strideUV; \
		} \
	} /* End_Of for (j = 0; j < height; ++j) */ \
}


COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_CONV_TO_RGBX_INTRIN_SSE_H_ */
