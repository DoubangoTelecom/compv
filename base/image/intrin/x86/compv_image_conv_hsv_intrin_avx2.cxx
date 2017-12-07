/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define RGBX_VLD3_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4, vec5) (void)(vec5); COMPV_VLD3_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4)
#define RGBX_VLD4_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4, vec5) COMPV_VLD4_U8_SSSE3(ptr, vec0, vec1, vec2, vec3, vec4, vec5)

#define CompVImageConvRgbxToHsv_Intrin_AVX2(rgbxPtr, rgbxn) { \
	COMPV_DEBUG_INFO_CHECK_AVX2(); \
	_mm256_zeroupper(); \
	compv_uscalar_t i, j, k, strideRGBx; \
	const compv_uscalar_t rgbxStep = (rgbxn<<5)/* (32*rgbxn) */; \
	__m256i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9; \
	__m128i vec0n, vec1n, vec2n, vec3n, vec4n, vec5n; \
	__m256 vec0f, vec1f, vec2f, vec3f; \
	static const __m256i vecZero = _mm256_setzero_si256(); \
	static const __m256i vec85 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k85_i8)); \
	static const __m256i vec171 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k171_u8)); \
	static const __m256i vecFF = _mm256_cmpeq_epi8(vec85, vec85); \
	static const __m256 vec43f = _mm256_load_ps(k43_f32); \
	static const __m256 vec255f = _mm256_load_ps(k255_f32); \
	\
	strideRGBx = stride * rgbxn; /* from samples to bytes (width * 3) for RGB24 and (width * 4) for RGBA32 */ \
	width += (width << 1); /* from samples to bytes (width * 3) */ \
	stride += (stride << 1); /* from samples to bytes (stride * 3) */ \
	\
	for (j = 0; j < height; ++j) { \
		for (i = 0, k = 0; i < width; i += 96, k += rgbxStep) { /* 96 = (32 * 3) */ \
			/* R = vec0, G = vec1, B = vec2, vec3 and vec4 used as temp variables */ \
			RGBX_VLD##rgbxn##_U8_SSSE3(&rgbxPtr[k], vec0n, vec1n, vec2n, vec3n, vec4n, vec5n); \
			vec0 = _mm256_castsi128_si256(vec0n); \
			vec1 = _mm256_castsi128_si256(vec1n); \
			vec2 = _mm256_castsi128_si256(vec2n); \
			vec3 = _mm256_castsi128_si256(vec3n); \
			RGBX_VLD##rgbxn##_U8_SSSE3(&rgbxPtr[k + (rgbxn<<4)/*(16*rgbxn)*/], vec0n, vec1n, vec2n, vec3n, vec4n, vec5n); \
			vec0 = _mm256_insertf128_si256(vec0, vec0n, 0x1); \
			vec1 = _mm256_insertf128_si256(vec1, vec1n, 0x1); \
			vec2 = _mm256_insertf128_si256(vec2, vec2n, 0x1); \
			vec3 = _mm256_insertf128_si256(vec3, vec3n, 0x1); \
			\
			vec3 = _mm256_min_epu8(vec2, _mm256_min_epu8(vec0, vec1)); /* vec3 = minVal */ \
			vec4 = _mm256_max_epu8(vec2, _mm256_max_epu8(vec0, vec1)); /* vec4 = maxVal = hsv[2].u8 */ \
			vec3 = _mm256_subs_epu8(vec4, vec3); /* vec3 = minus */ \
			\
			vec5 = _mm256_cmpeq_epi8(vec4, vec0); /* m0 = (maxVal == r) */ \
			vec6 = _mm256_andnot_si256(vec5, _mm256_cmpeq_epi8(vec4, vec1)); /* vec6 = m1 = (maxVal == g) & ~m0 */ \
			vec7 = _mm256_andnot_si256(_mm256_or_si256(vec5, vec6), vecFF); /* vec7 = m2 = ~(m0 | m1) */ \
			\
			vec9 = _mm256_and_si256(vec7, _mm256_sub_epi8(vec0, vec1)); \
			vec5 = _mm256_and_si256(vec5, _mm256_sub_epi8(vec1, vec2)); \
			vec8 = _mm256_and_si256(vec6, _mm256_sub_epi8(vec2, vec0)); \
			\
			vec5 = _mm256_or_si256(vec5, vec8); \
			vec5 = _mm256_or_si256(vec5, vec9); /* vec5 = diff */ \
			\
			/* convert minus to epi32 then to float32 */ \
			vec1 = _mm256_unpacklo_epi8(vec3, vecZero); \
			vec3 = _mm256_unpackhi_epi8(vec3, vecZero); \
			vec0 = _mm256_unpacklo_epi16(vec1, vecZero); \
			vec1 = _mm256_unpackhi_epi16(vec1, vecZero); \
			vec2 = _mm256_unpacklo_epi16(vec3, vecZero); \
			vec3 = _mm256_unpackhi_epi16(vec3, vecZero); \
			vec0 = _mm256_castps_si256(_mm256_cvtepi32_ps(vec0)); \
			vec1 = _mm256_castps_si256(_mm256_cvtepi32_ps(vec1)); \
			vec2 = _mm256_castps_si256(_mm256_cvtepi32_ps(vec2)); \
			vec3 = _mm256_castps_si256(_mm256_cvtepi32_ps(vec3)); \
			 \
			/* convert maxVal to epi32 then to float32 (unsigned values -> unpack/lo/hi) */ \
			vec1f = _mm256_castsi256_ps(_mm256_unpacklo_epi8(vec4, vecZero)); \
			vec3f = _mm256_castsi256_ps(_mm256_unpackhi_epi8(vec4, vecZero)); \
			vec0f = _mm256_castsi256_ps(_mm256_unpacklo_epi16(_mm256_castps_si256(vec1f), vecZero)); \
			vec1f = _mm256_castsi256_ps(_mm256_unpackhi_epi16(_mm256_castps_si256(vec1f), vecZero)); \
			vec2f = _mm256_castsi256_ps(_mm256_unpacklo_epi16(_mm256_castps_si256(vec3f), vecZero)); \
			vec3f = _mm256_castsi256_ps(_mm256_unpackhi_epi16(_mm256_castps_si256(vec3f), vecZero)); \
			vec0f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec0f)); \
			vec1f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec1f)); \
			vec2f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec2f)); \
			vec3f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec3f)); \
			 \
			/* compute scale = maxVal ? (1.f / maxVal) : 0.f */ \
			vec0f = _mm256_castsi256_ps(_mm256_andnot_si256(_mm256_cmpeq_epi32(_mm256_castps_si256(vec0f), vecZero), _mm256_castps_si256(_mm256_rcp_ps(vec0f)))); \
			vec1f = _mm256_castsi256_ps(_mm256_andnot_si256(_mm256_cmpeq_epi32(_mm256_castps_si256(vec1f), vecZero), _mm256_castps_si256(_mm256_rcp_ps(vec1f)))); \
			vec2f = _mm256_castsi256_ps(_mm256_andnot_si256(_mm256_cmpeq_epi32(_mm256_castps_si256(vec2f), vecZero), _mm256_castps_si256(_mm256_rcp_ps(vec2f)))); \
			vec3f = _mm256_castsi256_ps(_mm256_andnot_si256(_mm256_cmpeq_epi32(_mm256_castps_si256(vec3f), vecZero), _mm256_castps_si256(_mm256_rcp_ps(vec3f)))); \
			 \
			/* compute scales255 = (255 * scale) */ \
			vec0f = _mm256_mul_ps(vec0f, vec255f); \
			vec1f = _mm256_mul_ps(vec1f, vec255f); \
			vec2f = _mm256_mul_ps(vec2f, vec255f); \
			vec3f = _mm256_mul_ps(vec3f, vec255f); \
			 \
			/* hsv[1].float = static_cast<uint8_t>(round(scales255 * minus)) */ \
			vec0f = _mm256_mul_ps(vec0f, _mm256_castsi256_ps(vec0)); \
			vec1f = _mm256_mul_ps(vec1f, _mm256_castsi256_ps(vec1)); \
			vec2f = _mm256_mul_ps(vec2f, _mm256_castsi256_ps(vec2)); \
			vec3f = _mm256_mul_ps(vec3f, _mm256_castsi256_ps(vec3)); \
			vec0f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec0f)); \
			vec1f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec1f)); \
			vec2f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec2f)); \
			vec3f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec3f)); \
			vec0f = _mm256_castsi256_ps(_mm256_packs_epi32(_mm256_castps_si256(vec0f), _mm256_castps_si256(vec1f))); \
			vec2f = _mm256_castsi256_ps(_mm256_packs_epi32(_mm256_castps_si256(vec2f), _mm256_castps_si256(vec3f))); \
			vec8 = _mm256_packus_epi16(_mm256_castps_si256(vec0f), _mm256_castps_si256(vec2f)); /* vec8 = hsv[1].u8 */ \
			\
			/* compute scale = minus ? (1.f / minus) : 0.f */ \
			vec0 = _mm256_andnot_si256(_mm256_cmpeq_epi32(vec0, vecZero), _mm256_castps_si256(_mm256_rcp_ps(_mm256_castsi256_ps(vec0)))); \
			vec1 = _mm256_andnot_si256(_mm256_cmpeq_epi32(vec1, vecZero), _mm256_castps_si256(_mm256_rcp_ps(_mm256_castsi256_ps(vec1)))); \
			vec2 = _mm256_andnot_si256(_mm256_cmpeq_epi32(vec2, vecZero), _mm256_castps_si256(_mm256_rcp_ps(_mm256_castsi256_ps(vec2)))); \
			vec3 = _mm256_andnot_si256(_mm256_cmpeq_epi32(vec3, vecZero), _mm256_castps_si256(_mm256_rcp_ps(_mm256_castsi256_ps(vec3)))); \
			\
			/* compute scales43 = (43 * scale) */ \
			vec0 = _mm256_castps_si256(_mm256_mul_ps(_mm256_castsi256_ps(vec0), vec43f)); \
			vec1 = _mm256_castps_si256(_mm256_mul_ps(_mm256_castsi256_ps(vec1), vec43f)); \
			vec2 = _mm256_castps_si256(_mm256_mul_ps(_mm256_castsi256_ps(vec2), vec43f)); \
			vec3 = _mm256_castps_si256(_mm256_mul_ps(_mm256_castsi256_ps(vec3), vec43f)); \
			\
			/* convert diff to epi32 then to float32 (signed values -> cannot unpack/zero) */ \
			vec9 = _mm256_unpacklo_epi8(vec5, vec5); \
			vec5 = _mm256_unpackhi_epi8(vec5, vec5); \
			vec0f = _mm256_castsi256_ps(_mm256_srai_epi32(_mm256_unpacklo_epi16(vec9, vec9), 24)); \
			vec1f = _mm256_castsi256_ps(_mm256_srai_epi32(_mm256_unpackhi_epi16(vec9, vec9), 24)); \
			vec2f = _mm256_castsi256_ps(_mm256_srai_epi32(_mm256_unpacklo_epi16(vec5, vec5), 24)); \
			vec3f = _mm256_castsi256_ps(_mm256_srai_epi32(_mm256_unpackhi_epi16(vec5, vec5), 24)); \
			vec0f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec0f)); \
			vec1f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec1f)); \
			vec2f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec2f)); \
			vec3f = _mm256_cvtepi32_ps(_mm256_castps_si256(vec3f)); \
			\
			/* compute static_cast<uint8_t>(round(diff * scales43)) + ((85 & m1) | (171 & m2)) */ \
			vec0f = _mm256_mul_ps(vec0f, _mm256_castsi256_ps(vec0)); \
			vec1f = _mm256_mul_ps(vec1f, _mm256_castsi256_ps(vec1)); \
			vec2f = _mm256_mul_ps(vec2f, _mm256_castsi256_ps(vec2)); \
			vec3f = _mm256_mul_ps(vec3f, _mm256_castsi256_ps(vec3)); \
			vec0f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec0f)); \
			vec1f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec1f)); \
			vec2f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec2f)); \
			vec3f = _mm256_castsi256_ps(_mm256_cvtps_epi32(vec3f)); \
			vec0f = _mm256_castsi256_ps(_mm256_packs_epi32(_mm256_castps_si256(vec0f), _mm256_castps_si256(vec1f))); \
			vec2f = _mm256_castsi256_ps(_mm256_packs_epi32(_mm256_castps_si256(vec2f), _mm256_castps_si256(vec3f))); \
			vec9 = _mm256_packs_epi16(_mm256_castps_si256(vec0f), _mm256_castps_si256(vec2f)); \
			vec6 = _mm256_and_si256(vec6, vec85); /* (85 & m1) */ \
			vec7 = _mm256_and_si256(vec7, vec171); /* (171 & m2) */ \
			vec6 = _mm256_or_si256(vec6, vec7); /* (85 & m1) | (171 & m2) */ \
			vec9 = _mm256_adds_epi8(vec9, vec6); /* vec9 = hsv[0].u8 */ \
			\
			/* Store the result */ \
			/*!\\ AVX<->SSE transition issues if code not compiled with /AVX2 flag. */ \
			/*		Even if this is done, you should not trust the compiler, enable ASM to make sure. */ \
			vec0n = _mm256_castsi256_si128(vec9); \
			vec1n = _mm256_castsi256_si128(vec8); \
			vec2n = _mm256_castsi256_si128(vec4); \
			COMPV_VST3_U8_SSSE3(&hsvPtr[i], vec0n, vec1n, vec2n, vec3n, vec4n); \
			vec0n = _mm256_extractf128_si256(vec9, 0x1); \
			vec1n = _mm256_extractf128_si256(vec8, 0x1); \
			vec2n = _mm256_extractf128_si256(vec4, 0x1); \
			COMPV_VST3_U8_SSSE3(&hsvPtr[i + 48], vec0n, vec1n, vec2n, vec3n, vec4n); \
			\
		} /* End_Of for (i = 0; i < width; i += 96) */ \
		rgbxPtr += strideRGBx; \
		hsvPtr += stride; \
	} \
	_mm256_zeroupper(); \
}


void CompVImageConvRgb24ToHsv_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgb24Ptr, COMPV_ALIGNED(AVX) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	CompVImageConvRgbxToHsv_Intrin_AVX2(rgb24Ptr, 3);
}

void CompVImageConvRgba32ToHsv_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgba32Ptr, COMPV_ALIGNED(AVX) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	CompVImageConvRgbxToHsv_Intrin_AVX2(rgba32Ptr, 4);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
