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

#define yuv420p_uv_step				8
#define yuv422p_uv_step				8
#define yuv444p_uv_step				16

#define yuv420p_uv_load				_mm_loadl_epi64
#define yuv422p_uv_load				_mm_loadl_epi64
#define yuv444p_uv_load				_mm_load_si128

#define CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(nameYuv, nameRgbx, ssex) { \
	compv_uscalar_t i, j, k, l; \
	const compv_uscalar_t strideUV = (stride >> 1); /* no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes) */ \
	const compv_uscalar_t strideRGBx = (stride * nameRgbx##_bytes_per_sample); \
	__m128i vecYlow, vecYhigh, vecUlow, vecVlow, vecR, vecG, vecB; \
	__m128i vec0, vec1; \
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
			vecYlow = _mm_load_si128(reinterpret_cast<const __m128i*>(&yPtr[i])); /* #16 Y samples */ \
			vecUlow = nameYuv##_uv_load(reinterpret_cast<const __m128i*>(&uPtr[l])); /* #8 U samples, low mem */ \
			vecVlow = nameYuv##_uv_load(reinterpret_cast<const __m128i*>(&vPtr[l])); /* #8 V samples, low mem */ \
			 \
			/* Convert to I16 */ \
			vecYhigh = _mm_unpackhi_epi8(vecYlow, vecZero); \
			vecYlow = _mm_unpacklo_epi8(vecYlow, vecZero); \
			vecUlow = _mm_unpacklo_epi8(vecUlow, vecZero); \
			vecVlow = _mm_unpacklo_epi8(vecVlow, vecZero); \
			 \
			/* Compute Y', U', V' */\
			vecYlow = _mm_sub_epi16(vecYlow, vec16); \
			vecYhigh = _mm_sub_epi16(vecYhigh, vec16); \
			vecUlow = _mm_sub_epi16(vecUlow, vec127); \
			vecVlow = _mm_sub_epi16(vecVlow, vec127); \
			 \
			/* Compute (37Y'), (51V') and (65U') */ \
			vecYlow = _mm_mullo_epi16(vecYlow, vec37); \
			vecYhigh = _mm_mullo_epi16(vecYhigh, vec37); \
			vec0 = _mm_mullo_epi16(vecVlow, vec51); \
			vec1 = _mm_mullo_epi16(vecUlow, vec65); \
			 \
			/* Compute R = (37Y' + 0U' + 51V') >> 5 */ \
			vecR = _mm_packus_epi16(\
				_mm_srai_epi16(_mm_add_epi16(vecYlow, _mm_unpacklo_epi16(vec0, vec0)), 5), \
				_mm_srai_epi16(_mm_add_epi16(vecYhigh, _mm_unpackhi_epi16(vec0, vec0)), 5) \
			); \
			 \
			/* B = (37Y' + 65U' + 0V') >> 5 */ \
			vecB = _mm_packus_epi16(\
				_mm_srai_epi16(_mm_add_epi16(vecYlow, _mm_unpacklo_epi16(vec1, vec1)), 5), \
				_mm_srai_epi16(_mm_add_epi16(vecYhigh, _mm_unpackhi_epi16(vec1, vec1)), 5) \
			); \
			 \
			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */ \
			vec0 = _mm_madd_epi16(_mm_unpacklo_epi16(vecUlow, vecVlow), vec13_26); /* (13U' + 26V').low - I32 */ \
			vec1 = _mm_madd_epi16(_mm_unpackhi_epi16(vecUlow, vecVlow), vec13_26); /* (13U' + 26V').high - I32 */ \
			vec0 = _mm_packs_epi32(vec0, vec1); \
			vecG = _mm_packus_epi16(\
				_mm_srai_epi16(_mm_sub_epi16(vecYlow, _mm_unpacklo_epi16(vec0, vec0)), 5), \
				_mm_srai_epi16(_mm_sub_epi16(vecYhigh, _mm_unpackhi_epi16(vec0, vec0)), 5) \
			); \
			 \
			/* Store result */ \
			nameRgbx##_store_##ssex##(&rgbxPtr[k], vecR, vecG, vecB, vecA, vec0, vec1); \
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
