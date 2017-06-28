/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
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

// width must be >= 16
// "scales43" and "scales255" are recomputed(faster) instead of trying to access values using indexes (not cache-friendly)
void CompVImageConvRgba32ToHsv_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgba32Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride
	, const compv_float32_t(*scales43)[256], const compv_float32_t(*scales255)[256])
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Code not clean, see RGB24 impl");

	compv_uscalar_t i, j, k, strideRGBA;
	__m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9;
	__m128 vec0f, vec1f, vec2f, vec3f;
	__m128 vechsv0, vechsv1, vechsv2; // FIXME(dmi): remove
	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vec85 = _mm_set1_epi8(85);
	static const __m128i vec171 = _mm_set1_epi8((uint8_t)171);
	static const __m128i vecFF = _mm_cmpeq_epi8(vec85, vec85);
	static const __m128 vec43f = _mm_set1_ps(43.f);
	static const __m128 vec255f = _mm_set1_ps(255.f);

	width = (width << 2); // from samples to bytes (width * 4)
	strideRGBA = (stride << 2); // from samples to bytes
	stride += (stride << 1); // from samples to bytes (stride * 3)

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("reciprocal neon ok? -> http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka14282.html");

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("use _mm_rcp_ps instead of _mm_rcp_ps");
#define _mm_rcp_ps(x) _mm_rcp_ps(x)

	for (j = 0; j < height; ++j) {
		for (i = 0, k = 0; i < width; i += 64, k += 48) { // 64 = (16 * 4), 48 = (16 * 3)
			// R = vec0, G = vec1, B = vec2, A = vec3 / vec4 and vec5 used as temp variables
			COMPV_VLD4_U8_SSSE3(&rgba32Ptr[i], vec0, vec1, vec2, vec3, vec4, vec5);

			vec3 = _mm_min_epu8(vec2, _mm_min_epu8(vec0, vec1)); // vec3 = minVal
			vec4 = _mm_max_epu8(vec2, _mm_max_epu8(vec0, vec1)); // vec4 = maxVal = hsv[2].u8

			vec3 = _mm_subs_epu8(vec4, vec3); // vec3 = minus

#if 1
			for (size_t y = 0; y < 16; ++y) {
				if (j == 0 && i == 624 && y == 8) {
					COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				}
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				int r = vec0.m128i_u8[y];
				int g = vec1.m128i_u8[y];
				int b = vec2.m128i_u8[y];
				int maxVal = vec4.m128i_u8[y];
				int m0 = -(maxVal == r); // (maxVal == r) ? 0xff : 0x00;
				int m1 = -(maxVal == g) & ~m0; // ((maxVal == r) ? 0xff : 0x00) & ~m0
				int m2 = ~(m0 | m1);
				int diff = ((g - b) & m0) | ((b - r) & m1) | ((r - g) & m2);
				int minus = vec3.m128i_u8[y];
				compv_float32_t s43 = (diff * (*scales43)[minus]);
				compv_float32_t s255 = (minus * (*scales255)[maxVal]);
				vechsv0.m128_u8[y] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s43, uint8_t) // Important round "SIGNED"
					+ ((85 & m1) | (171 & m2));
				vechsv1.m128_u8[y] = COMPV_MATH_ROUNDF_2_NEAREST_INT(s255, uint8_t);
				vechsv2.m128_u8[y] = static_cast<uint8_t>(maxVal);
			}
#endif

			vec5 = _mm_cmpeq_epi8(vec4, vec0); // m0 = (maxVal == r)
			vec6 = _mm_andnot_si128(vec5, _mm_cmpeq_epi8(vec4, vec1)); // m1 = (maxVal == g) & ~m0
			vec7 = _mm_andnot_si128(_mm_or_si128(vec5, vec6), vecFF); // m2 = ~(m0 | m1)

			vec5 = _mm_and_si128(vec5, _mm_sub_epi8(vec1, vec2));
			vec8 = _mm_and_si128(vec6, _mm_sub_epi8(vec2, vec0));
			vec9 = _mm_and_si128(vec7, _mm_sub_epi8(vec0, vec1));
			vec5 = _mm_or_si128(vec5, vec8);
			vec5 = _mm_or_si128(vec5, vec9); // vec5 = diff

			// convert minus to epi32 then to float32
			vec1 = _mm_unpacklo_epi8(vec3, vecZero);
			vec3 = _mm_unpackhi_epi8(vec3, vecZero);
			vec0 = _mm_unpacklo_epi16(vec1, vecZero);
			vec1 = _mm_unpackhi_epi16(vec1, vecZero);
			vec2 = _mm_unpacklo_epi16(vec3, vecZero);
			vec3 = _mm_unpackhi_epi16(vec3, vecZero);
			vec0 = _mm_castps_si128(_mm_cvtepi32_ps(vec0));
			vec1 = _mm_castps_si128(_mm_cvtepi32_ps(vec1));
			vec2 = _mm_castps_si128(_mm_cvtepi32_ps(vec2));
			vec3 = _mm_castps_si128(_mm_cvtepi32_ps(vec3));

			// convert maxVal to epi32 then to float32 (positive values -> unpack/lo/hi)
			vec8 = _mm_unpacklo_epi8(vec4, vecZero);
			vec9 = _mm_unpackhi_epi8(vec4, vecZero);
			vec0f = _mm_castsi128_ps(_mm_unpacklo_epi16(vec8, vecZero));
			vec1f = _mm_castsi128_ps(_mm_unpackhi_epi16(vec8, vecZero));
			vec2f = _mm_castsi128_ps(_mm_unpacklo_epi16(vec9, vecZero));
			vec3f = _mm_castsi128_ps(_mm_unpackhi_epi16(vec9, vecZero));
			vec0f = _mm_cvtepi32_ps(_mm_castps_si128(vec0f));
			vec1f = _mm_cvtepi32_ps(_mm_castps_si128(vec1f));
			vec2f = _mm_cvtepi32_ps(_mm_castps_si128(vec2f));
			vec3f = _mm_cvtepi32_ps(_mm_castps_si128(vec3f));

			// compute scale = maxVal ? (1.f / maxVal) : 0.f
			vec0f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec0f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec0f))));
			vec1f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec1f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec1f))));
			vec2f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec2f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec2f))));
			vec3f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec3f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec3f))));

			// compute scales255 = (255 * scale)
			vec0f = _mm_mul_ps(vec0f, vec255f);
			vec1f = _mm_mul_ps(vec1f, vec255f);
			vec2f = _mm_mul_ps(vec2f, vec255f);
			vec3f = _mm_mul_ps(vec3f, vec255f);

			// hsv[1].float = static_cast<uint8_t>(round(scales255 * minus))
			// FIXME(dmi): AVX/NEON: FMA
			vec0f = _mm_mul_ps(vec0f, _mm_castsi128_ps(vec0));
			vec1f = _mm_mul_ps(vec1f, _mm_castsi128_ps(vec1));
			vec2f = _mm_mul_ps(vec2f, _mm_castsi128_ps(vec2));
			vec3f = _mm_mul_ps(vec3f, _mm_castsi128_ps(vec3));
			vec0f = _mm_castsi128_ps(_mm_cvtps_epi32(vec0f));
			vec1f = _mm_castsi128_ps(_mm_cvtps_epi32(vec1f));
			vec2f = _mm_castsi128_ps(_mm_cvtps_epi32(vec2f));
			vec3f = _mm_castsi128_ps(_mm_cvtps_epi32(vec3f));
			vec0f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec0f), _mm_castps_si128(vec1f)));
			vec2f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec2f), _mm_castps_si128(vec3f)));
			vec8 = _mm_packus_epi16(_mm_castps_si128(vec0f), _mm_castps_si128(vec2f)); // vec8 = hsv[1].u8

#if 0
			for (size_t y = 0; y < 16; ++y) {
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				if ((i + (y << 2)) < width && std::abs(vec8.m128i_u8[y] - vechsv1.m128_u8[y]) > 0) {
					int kaka = 0;
				}
			}
#endif

			// compute scale = minus ? (1.f / minus) : 0.f
			vec0 = _mm_andnot_si128(_mm_cmpeq_epi32(vec0, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec0))));
			vec1 = _mm_andnot_si128(_mm_cmpeq_epi32(vec1, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec1))));
			vec2 = _mm_andnot_si128(_mm_cmpeq_epi32(vec2, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec2))));
			vec3 = _mm_andnot_si128(_mm_cmpeq_epi32(vec3, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec3))));

			// compute scales43 = (43 * scale)
			vec0 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec0), vec43f));
			vec1 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec1), vec43f));
			vec2 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec2), vec43f));
			vec3 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec3), vec43f));

#if 0
			if (j == 402 && i == 1792) {
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				vec2f = _mm_castsi128_ps(vec2);
				uint8_t kiki = (uint8_t)static_cast<int>(((-3 * 14)));
				uint8_t koko = (uint8_t)static_cast<int>(((253 * 14)));
				uint8_t nana = (uint8_t)static_cast<int>(((3542)));
				int kaka = 0;
			}
#endif

			//if (j == 402 && i == 1792) {
			//	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
			//}

			// convert diff to epi32 then to float32
#if 0
			vec9 = _mm_unpacklo_epi8(vec5, vecZero);
			vec5 = _mm_unpackhi_epi8(vec5, vecZero);
			vec0f = _mm_castsi128_ps(_mm_unpacklo_epi16(vec9, vecZero));
			vec1f = _mm_castsi128_ps(_mm_unpackhi_epi16(vec9, vecZero));
			vec2f = _mm_castsi128_ps(_mm_unpacklo_epi16(vec5, vecZero));
			vec3f = _mm_castsi128_ps(_mm_unpackhi_epi16(vec5, vecZero));
#else
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("SSE41");
			vec0f = _mm_castsi128_ps(_mm_cvtepi8_epi32(vec5));
			vec5 = _mm_srli_si128(vec5, 4);
			vec1f = _mm_castsi128_ps(_mm_cvtepi8_epi32(vec5));
			vec5 = _mm_srli_si128(vec5, 4);
			vec2f = _mm_castsi128_ps(_mm_cvtepi8_epi32(vec5));
			vec5 = _mm_srli_si128(vec5, 4);
			vec3f = _mm_castsi128_ps(_mm_cvtepi8_epi32(vec5));
#endif
			vec0f = _mm_cvtepi32_ps(_mm_castps_si128(vec0f));
			vec1f = _mm_cvtepi32_ps(_mm_castps_si128(vec1f));
			vec2f = _mm_cvtepi32_ps(_mm_castps_si128(vec2f));
			vec3f = _mm_cvtepi32_ps(_mm_castps_si128(vec3f));

			// compute static_cast<uint8_t>(round(diff * scales43)) + ((85 & m1) | (171 & m2))
			// FIXME(dmi): AVX/NEON -> FMA
			vec0f = _mm_mul_ps(vec0f, _mm_castsi128_ps(vec0));
			vec1f = _mm_mul_ps(vec1f, _mm_castsi128_ps(vec1));
			vec2f = _mm_mul_ps(vec2f, _mm_castsi128_ps(vec2));
			vec3f = _mm_mul_ps(vec3f, _mm_castsi128_ps(vec3));
			vec0f = _mm_castsi128_ps(_mm_cvtps_epi32(vec0f));
			vec1f = _mm_castsi128_ps(_mm_cvtps_epi32(vec1f));
			vec2f = _mm_castsi128_ps(_mm_cvtps_epi32(vec2f));
			vec3f = _mm_castsi128_ps(_mm_cvtps_epi32(vec3f));
			vec0f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec0f), _mm_castps_si128(vec1f)));
			vec2f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec2f), _mm_castps_si128(vec3f)));
			vec9 = _mm_packs_epi16(_mm_castps_si128(vec0f), _mm_castps_si128(vec2f));
			vec6 = _mm_and_si128(vec6, vec85); // (85 & m1)
			vec7 = _mm_and_si128(vec7, vec171); // (171 & m2)
			vec6 = _mm_or_si128(vec6, vec7); // (85 & m1) | (171 & m2)
			vec9 = _mm_adds_epi8(vec9, vec6); // // vec9 = hsv[0].u8

#if 1
			for (size_t y = 0; y < 16; ++y) {
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				if ((i + (y * 4)) < width && std::abs(vec9.m128i_u8[y] - vechsv0.m128_u8[y]) > 1) {
					assert(0);
				}
			}

			for (size_t y = 0; y < 16; ++y) {
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				if ((i + (y * 4)) < width && std::abs(vec8.m128i_u8[y] - vechsv1.m128_u8[y]) > 1) {
					assert(0);
				}
			}

			for (size_t y = 0; y < 16; ++y) {
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				if ((i + (y * 4)) < width && std::abs(vec4.m128i_u8[y] - vechsv2.m128_u8[y]) > 1) {
					assert(0);
				}
			}
#endif

			for (size_t y = 0; y < 16; ++y) {
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				//printf("%u, ", vec9.m128i_u8[y]);
			}

			COMPV_VST3_U8_SSSE3(&hsvPtr[k], vec9, vec8, vec4, vec0, vec1);



			// _mm_castps_si128 , _mm_castsi128_ps

			//_mm_rcp_ps()

			//compv_float32_t scale;
			//for (int b = 1; b < 256; ++b) {
			//	scale = 1.f / static_cast<compv_float32_t>(b);
			//	__hsv_scales43[b] = 43 * scale;
			//	__hsv_scales255[b] = 255 * scale;
			//}

			//m0 = -(maxVal == r);
			//m1 = -(maxVal == g) & ~m0;
			//m2 = ~(m0 | m1);
			//diff = ((g - b) & m0) | ((b - r) & m1) | ((r - g) & m2);
			//minus = maxVal - minVal;
			//hsv[0] = static_cast<uint8_t>(diff * (*scales43)[minus])
			//	+ ((85 & m1) | (171 & m2));
			//hsv[1] = static_cast<uint8_t>(minus * (*scales255)[maxVal]);
			//hsv[2] = static_cast<uint8_t>(maxVal);



		} // End_Of for (i = 0; i < width; i += 64)
		rgba32Ptr += strideRGBA;
		hsvPtr += stride;
	}
#undef _mm_rcp_ps
}

void CompVImageConvRgb24ToHsv_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb24Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride
	, const compv_float32_t(*scales43)[256], const compv_float32_t(*scales255)[256])
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();

	compv_uscalar_t i, j;
	__m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9;
	__m128 vec0f, vec1f, vec2f, vec3f;
	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vec85 = _mm_load_si128(reinterpret_cast<const __m128i*>(k85_i8));
	static const __m128i vec171 = _mm_load_si128(reinterpret_cast<const __m128i*>(k171_u8));
	static const __m128i vecFF = _mm_cmpeq_epi8(vec85, vec85);
	static const __m128 vec43f = _mm_load_ps(k43_f32);
	static const __m128 vec255f = _mm_load_ps(k255_f32);

	width += (width << 1); // from samples to bytes (width * 3)
	stride += (stride << 1); // from samples to bytes (stride * 3)

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 48) { // 48 = (16 * 3)
			// R = vec0, G = vec1, B = vec2, vec3 and vec4 used as temp variables
			COMPV_VLD3_U8_SSSE3(&rgb24Ptr[i], vec0, vec1, vec2, vec3, vec4);

			vec3 = _mm_min_epu8(vec2, _mm_min_epu8(vec0, vec1)); // vec3 = minVal
			vec4 = _mm_max_epu8(vec2, _mm_max_epu8(vec0, vec1)); // vec4 = maxVal = hsv[2].u8
			vec3 = _mm_subs_epu8(vec4, vec3); // vec3 = minus

			vec5 = _mm_cmpeq_epi8(vec4, vec0); // m0 = (maxVal == r)
			vec6 = _mm_andnot_si128(vec5, _mm_cmpeq_epi8(vec4, vec1)); // vec6 = m1 = (maxVal == g) & ~m0
			vec7 = _mm_andnot_si128(_mm_or_si128(vec5, vec6), vecFF); // vec7 = m2 = ~(m0 | m1)

			vec9 = _mm_and_si128(vec7, _mm_sub_epi8(vec0, vec1));
			vec5 = _mm_and_si128(vec5, _mm_sub_epi8(vec1, vec2));
			vec8 = _mm_and_si128(vec6, _mm_sub_epi8(vec2, vec0));
			
			vec5 = _mm_or_si128(vec5, vec8);
			vec5 = _mm_or_si128(vec5, vec9); // vec5 = diff

			// convert minus to epi32 then to float32
			vec1 = _mm_unpacklo_epi8(vec3, vecZero);
			vec3 = _mm_unpackhi_epi8(vec3, vecZero);
			vec0 = _mm_unpacklo_epi16(vec1, vecZero);
			vec1 = _mm_unpackhi_epi16(vec1, vecZero);
			vec2 = _mm_unpacklo_epi16(vec3, vecZero);
			vec3 = _mm_unpackhi_epi16(vec3, vecZero);
			vec0 = _mm_castps_si128(_mm_cvtepi32_ps(vec0));
			vec1 = _mm_castps_si128(_mm_cvtepi32_ps(vec1));
			vec2 = _mm_castps_si128(_mm_cvtepi32_ps(vec2));
			vec3 = _mm_castps_si128(_mm_cvtepi32_ps(vec3));

			// convert maxVal to epi32 then to float32 (unsigned values -> unpack/lo/hi)
			vec1f = _mm_castsi128_ps(_mm_unpacklo_epi8(vec4, vecZero));
			vec3f = _mm_castsi128_ps(_mm_unpackhi_epi8(vec4, vecZero));
			vec0f = _mm_castsi128_ps(_mm_unpacklo_epi16(_mm_castps_si128(vec1f), vecZero));
			vec1f = _mm_castsi128_ps(_mm_unpackhi_epi16(_mm_castps_si128(vec1f), vecZero));
			vec2f = _mm_castsi128_ps(_mm_unpacklo_epi16(_mm_castps_si128(vec3f), vecZero));
			vec3f = _mm_castsi128_ps(_mm_unpackhi_epi16(_mm_castps_si128(vec3f), vecZero));
			vec0f = _mm_cvtepi32_ps(_mm_castps_si128(vec0f));
			vec1f = _mm_cvtepi32_ps(_mm_castps_si128(vec1f));
			vec2f = _mm_cvtepi32_ps(_mm_castps_si128(vec2f));
			vec3f = _mm_cvtepi32_ps(_mm_castps_si128(vec3f));

			// compute scale = maxVal ? (1.f / maxVal) : 0.f
			vec0f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec0f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec0f))));
			vec1f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec1f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec1f))));
			vec2f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec2f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec2f))));
			vec3f = _mm_castsi128_ps(_mm_andnot_si128(_mm_cmpeq_epi32(_mm_castps_si128(vec3f), vecZero), _mm_castps_si128(_mm_rcp_ps(vec3f))));

			// compute scales255 = (255 * scale)
			vec0f = _mm_mul_ps(vec0f, vec255f);
			vec1f = _mm_mul_ps(vec1f, vec255f);
			vec2f = _mm_mul_ps(vec2f, vec255f);
			vec3f = _mm_mul_ps(vec3f, vec255f);

			// hsv[1].float = static_cast<uint8_t>(round(scales255 * minus))
			vec0f = _mm_mul_ps(vec0f, _mm_castsi128_ps(vec0));
			vec1f = _mm_mul_ps(vec1f, _mm_castsi128_ps(vec1));
			vec2f = _mm_mul_ps(vec2f, _mm_castsi128_ps(vec2));
			vec3f = _mm_mul_ps(vec3f, _mm_castsi128_ps(vec3));
			vec0f = _mm_castsi128_ps(_mm_cvtps_epi32(vec0f));
			vec1f = _mm_castsi128_ps(_mm_cvtps_epi32(vec1f));
			vec2f = _mm_castsi128_ps(_mm_cvtps_epi32(vec2f));
			vec3f = _mm_castsi128_ps(_mm_cvtps_epi32(vec3f));
			vec0f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec0f), _mm_castps_si128(vec1f)));
			vec2f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec2f), _mm_castps_si128(vec3f)));
			vec8 = _mm_packus_epi16(_mm_castps_si128(vec0f), _mm_castps_si128(vec2f)); // vec8 = hsv[1].u8

			// compute scale = minus ? (1.f / minus) : 0.f
			vec0 = _mm_andnot_si128(_mm_cmpeq_epi32(vec0, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec0))));
			vec1 = _mm_andnot_si128(_mm_cmpeq_epi32(vec1, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec1))));
			vec2 = _mm_andnot_si128(_mm_cmpeq_epi32(vec2, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec2))));
			vec3 = _mm_andnot_si128(_mm_cmpeq_epi32(vec3, vecZero), _mm_castps_si128(_mm_rcp_ps(_mm_castsi128_ps(vec3))));

			// compute scales43 = (43 * scale)
			vec0 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec0), vec43f));
			vec1 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec1), vec43f));
			vec2 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec2), vec43f));
			vec3 = _mm_castps_si128(_mm_mul_ps(_mm_castsi128_ps(vec3), vec43f));

			// convert diff to epi32 then to float32 (signed values -> cannot unpack/zero)
			vec9 = _mm_unpacklo_epi8(vec5, vec5);
			vec5 = _mm_unpackhi_epi8(vec5, vec5);
			vec0f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpacklo_epi16(vec9, vec9), 24));
			vec1f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpackhi_epi16(vec9, vec9), 24));
			vec2f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpacklo_epi16(vec5, vec5), 24));
			vec3f = _mm_castsi128_ps(_mm_srai_epi32(_mm_unpackhi_epi16(vec5, vec5), 24));
			vec0f = _mm_cvtepi32_ps(_mm_castps_si128(vec0f));
			vec1f = _mm_cvtepi32_ps(_mm_castps_si128(vec1f));
			vec2f = _mm_cvtepi32_ps(_mm_castps_si128(vec2f));
			vec3f = _mm_cvtepi32_ps(_mm_castps_si128(vec3f));

			// compute static_cast<uint8_t>(round(diff * scales43)) + ((85 & m1) | (171 & m2))
			vec0f = _mm_mul_ps(vec0f, _mm_castsi128_ps(vec0));
			vec1f = _mm_mul_ps(vec1f, _mm_castsi128_ps(vec1));
			vec2f = _mm_mul_ps(vec2f, _mm_castsi128_ps(vec2));
			vec3f = _mm_mul_ps(vec3f, _mm_castsi128_ps(vec3));
			vec0f = _mm_castsi128_ps(_mm_cvtps_epi32(vec0f));
			vec1f = _mm_castsi128_ps(_mm_cvtps_epi32(vec1f));
			vec2f = _mm_castsi128_ps(_mm_cvtps_epi32(vec2f));
			vec3f = _mm_castsi128_ps(_mm_cvtps_epi32(vec3f));
			vec0f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec0f), _mm_castps_si128(vec1f)));
			vec2f = _mm_castsi128_ps(_mm_packs_epi32(_mm_castps_si128(vec2f), _mm_castps_si128(vec3f)));
			vec9 = _mm_packs_epi16(_mm_castps_si128(vec0f), _mm_castps_si128(vec2f));
			vec6 = _mm_and_si128(vec6, vec85); // (85 & m1)
			vec7 = _mm_and_si128(vec7, vec171); // (171 & m2)
			vec6 = _mm_or_si128(vec6, vec7); // (85 & m1) | (171 & m2)
			vec9 = _mm_adds_epi8(vec9, vec6); // // vec9 = hsv[0].u8
			
			// Store the result
			COMPV_VST3_U8_SSSE3(&hsvPtr[i], vec9, vec8, vec4, vec0, vec1);

		} // End_Of for (i = 0; i < width; i += 48)
		rgb24Ptr += stride;
		hsvPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
