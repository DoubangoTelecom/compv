/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define RGBX_VLD3_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4, vec5) COMPV_VLD3_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4)
#define RGBX_VLD4_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4, vec5) COMPV_VLD4_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4, vec5)

#define CompVImageConvRgbxToHsv_Intrin_SSSE3(rgbxPtr, rgbxn) { \
	COMPV_DEBUG_INFO_CHECK_SSSE3(); \
	compv_uscalar_t i, j, k, strideRGBx; \
	const compv_uscalar_t rgbxStep = (rgbxn<<4); /* (16 * rgbxn) */\
	__m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9; \
	__m128 vec0f, vec1f, vec2f, vec3f; \
	static const __m128i vecZero = _mm_setzero_si128(); \
	static const __m128i vec85 = _mm_load_si128(reinterpret_cast<const __m128i*>(k85_8s)); \
	static const __m128i vec171 = _mm_load_si128(reinterpret_cast<const __m128i*>(k171_8u)); \
	static const __m128i vecFF = _mm_cmpeq_epi8(vec85, vec85); \
	static const __m128 vec43f = _mm_load_ps(k43_32f); \
	static const __m128 vec255f = _mm_load_ps(k255_32f); \
	strideRGBx = stride * rgbxn; /* from samples to bytes (width * 3) for RGB24 and (width * 4) for RGBA32 */ \
	width += (width << 1); /* from samples to bytes (width * 3)*/ \
	stride += (stride << 1); /* from samples to bytes (stride * 3) */ \
	 \
	for (j = 0; j < height; ++j) { \
		for (i = 0, k = 0; i < width; i += 48, k += rgbxStep) { /* 48 = hsvStep = (16*3) */ \
			/* R = vec0, G = vec1, B = vec2, A = vec3 */ \
			RGBX_VLD##rgbxn##_U8_SSSE3(&rgbxPtr[k], vec0, vec1, vec2, vec3, vec4, vec5); \
			 \
			vec3 = _mm_min_epu8(vec2, _mm_min_epu8(vec0, vec1)); /* vec3 = minVal */ \
			vec4 = _mm_max_epu8(vec2, _mm_max_epu8(vec0, vec1)); /* vec4 = maxVal = hsv[2].u8 */ \
			vec3 = _mm_subs_epu8(vec4, vec3); /* vec3 = minus */ \
			 \
			vec5 = _mm_cmpeq_epi8(vec4, vec0); /* m0 = (maxVal == r) */ \
			vec6 = _mm_andnot_si128(vec5, _mm_cmpeq_epi8(vec4, vec1)); /* vec6 = m1 = (maxVal == g) & ~m0 */ \
			vec7 = _mm_andnot_si128(_mm_or_si128(vec5, vec6), vecFF); /* vec7 = m2 = ~(m0 | m1) */ \
			 \
			vec9 = _mm_and_si128(vec7, _mm_sub_epi8(vec0, vec1)); \
			vec5 = _mm_and_si128(vec5, _mm_sub_epi8(vec1, vec2)); \
			vec8 = _mm_and_si128(vec6, _mm_sub_epi8(vec2, vec0)); \
			 \
			vec5 = _mm_or_si128(vec5, vec8); \
			vec5 = _mm_or_si128(vec5, vec9); /* vec5 = diff */ \
			 \
			/* convert minus to epi32 then to float32 */ \
			vec1 = _mm_unpacklo_epi8(vec3, vecZero); \
			vec3 = _mm_unpackhi_epi8(vec3, vecZero); \
			vec0 = _mm_unpacklo_epi16(vec1, vecZero); \
			vec1 = _mm_unpackhi_epi16(vec1, vecZero); \
			vec2 = _mm_unpacklo_epi16(vec3, vecZero); \
			vec3 = _mm_unpackhi_epi16(vec3, vecZero); \
			vec0 = _mm_castps_si128(_mm_cvtepi32_ps(vec0)); \
			vec1 = _mm_castps_si128(_mm_cvtepi32_ps(vec1)); \
			vec2 = _mm_castps_si128(_mm_cvtepi32_ps(vec2)); \
			vec3 = _mm_castps_si128(_mm_cvtepi32_ps(vec3)); \
			 \
			/* convert maxVal to epi32 then to float32 (unsigned values -> unpack/lo/hi) */ \
			vec1f = _mm_castsi128_ps(_mm_unpacklo_epi8(vec4, vecZero)); \
			vec3f = _mm_castsi128_ps(_mm_unpackhi_epi8(vec4, vecZero)); \
			vec0f = _mm_castsi128_ps(_mm_unpacklo_epi16(_mm_castps_si128(vec1f), vecZero)); \
			vec1f = _mm_castsi128_ps(_mm_unpackhi_epi16(_mm_castps_si128(vec1f), vecZero)); \
			vec2f = _mm_castsi128_ps(_mm_unpacklo_epi16(_mm_castps_si128(vec3f), vecZero)); \
			vec3f = _mm_castsi128_ps(_mm_unpackhi_epi16(_mm_castps_si128(vec3f), vecZero)); \
			vec0f = _mm_cvtepi32_ps(_mm_castps_si128(vec0f)); \
			vec1f = _mm_cvtepi32_ps(_mm_castps_si128(vec1f)); \
			vec2f = _mm_cvtepi32_ps(_mm_castps_si128(vec2f)); \
			vec3f = _mm_cvtepi32_ps(_mm_castps_si128(vec3f)); \
			 \
			/* compute scale = maxVal ? (1.f / maxVal) : 0.f */ \
			vec0f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec0f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec0f)))); \
			vec1f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec1f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec1f)))); \
			vec2f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec2f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec2f)))); \
			vec3f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec3f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec3f)))); \
			 \
			/* compute scales255 = (255 * scale) */ \
			vec0f = _mm_mul_ps(vec0f, vec255f); \
			vec1f = _mm_mul_ps(vec1f, vec255f); \
			vec2f = _mm_mul_ps(vec2f, vec255f); \
			vec3f = _mm_mul_ps(vec3f, vec255f); \
			 \
			/* hsv[1].float = static_cast<uint8_t>(round(scales255 * minus)) */ \
			vec0f = _mm_mul_ps(vec0f, _mm_castsi128_ps(vec0)); \
			vec1f = _mm_mul_ps(vec1f, _mm_castsi128_ps(vec1)); \
			vec2f = _mm_mul_ps(vec2f, _mm_castsi128_ps(vec2)); \
			vec3f = _mm_mul_ps(vec3f, _mm_castsi128_ps(vec3)); \
			vec0f = _mm_castsi128_ps(_mm_cvtps_epi32(vec0f)); \
			vec1f = _mm_castsi128_ps(_mm_cvtps_epi32(vec1f)); \
			vec2f = _mm_castsi128_ps(_mm_cvtps_epi32(vec2f)); \
			vec3f = _mm_castsi128_ps(_mm_cvtps_epi32(vec3f)); \
			vec0f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec0f), _mm_castps_si128(vec1f))); \
			vec2f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec2f), _mm_castps_si128(vec3f))); \
			vec8 = _mm_packus_epi16(_mm_castps_si128(vec0f), _mm_castps_si128(vec2f)); /* vec8 = hsv[1].u8 */ \
			vec0 = _mm_andnot_si128(_mm_cmpeq_epi32(vec0, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec0)))); \
			vec1 = _mm_andnot_si128(_mm_cmpeq_epi32(vec1, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec1)))); \
			vec2 = _mm_andnot_si128(_mm_cmpeq_epi32(vec2, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec2)))); \
			vec3 = _mm_andnot_si128(_mm_cmpeq_epi32(vec3, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec3)))); \
			 \
			/* compute scales43 = (43 * scale) */  \
			vec0 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec0), vec43f)); \
			vec1 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec1), vec43f)); \
			vec2 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec2), vec43f)); \
			vec3 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec3), vec43f)); \
			 \
			/* convert diff to epi32 then to float32 (signed values -> cannot unpack/zero) */ \
			vec9 = _mm_unpacklo_epi8(vec5, vec5); \
			vec5 = _mm_unpackhi_epi8(vec5, vec5); \
			vec0f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpacklo_epi16(vec9, vec9), 24)); \
			vec1f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpackhi_epi16(vec9, vec9), 24)); \
			vec2f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpacklo_epi16(vec5, vec5), 24)); \
			vec3f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpackhi_epi16(vec5, vec5), 24)); \
			vec0f = _mm_cvtepi32_ps(_mm_castps_si128(vec0f)); \
			vec1f = _mm_cvtepi32_ps(_mm_castps_si128(vec1f)); \
			vec2f = _mm_cvtepi32_ps(_mm_castps_si128(vec2f)); \
			vec3f = _mm_cvtepi32_ps(_mm_castps_si128(vec3f)); \
			 \
			/* compute static_cast<uint8_t>(round(diff * scales43)) + ((85 & m1) | (171 & m2)) */ \
			vec0f = _mm_mul_ps(vec0f, _mm_castsi128_ps(vec0)); \
			vec1f = _mm_mul_ps(vec1f, _mm_castsi128_ps(vec1)); \
			vec2f = _mm_mul_ps(vec2f, _mm_castsi128_ps(vec2)); \
			vec3f = _mm_mul_ps(vec3f, _mm_castsi128_ps(vec3)); \
			vec0f = _mm_castsi128_ps(_mm_cvtps_epi32(vec0f)); \
			vec1f = _mm_castsi128_ps(_mm_cvtps_epi32(vec1f)); \
			vec2f = _mm_castsi128_ps(_mm_cvtps_epi32(vec2f)); \
			vec3f = _mm_castsi128_ps(_mm_cvtps_epi32(vec3f)); \
			vec0f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec0f), _mm_castps_si128(vec1f))); \
			vec2f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec2f), _mm_castps_si128(vec3f))); \
			vec9 = _mm_packs_epi16(_mm_castps_si128(vec0f), _mm_castps_si128(vec2f)); \
			vec6 = _mm_and_si128(vec6, vec85); /* (85 & m1) */ \
			vec7 = _mm_and_si128(vec7, vec171); /* (171 & m2) */ \
			vec6 = _mm_or_si128(vec6, vec7); /* (85 & m1) | (171 & m2) */ \
			vec9 = _mm_adds_epi8(vec9, vec6); /* vec9 = hsv[0].u8 */ \
			\
			/* Store the result */ \
			COMPV_VST3_U8_SSSE3(&hsvPtr[i], vec9, vec8, vec4, vec0, vec1); \
			 \
		} /* End_Of for (i = 0; i < width; i += 48) */ \
		rgbxPtr += strideRGBx; \
		hsvPtr += stride; \
	} \
}

// width must be >= 16
void CompVImageConvRgba32ToHsv_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgba32Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	CompVImageConvRgbxToHsv_Intrin_SSSE3(rgba32Ptr, 4);
}

void CompVImageConvRgb24ToHsv_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb24Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	CompVImageConvRgbxToHsv_Intrin_SSSE3(rgb24Ptr, 3);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
