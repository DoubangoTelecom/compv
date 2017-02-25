/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/x86/compv_patch_intrin_sse2.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// top and bottom are padded with zeros and aligned to the max SIMD alignv which means we can read beyond count
// requires radius <= 64 to make sure (x * (top +- bottom)) is within [-0x7fff, +0x7fff]
void CompVPatchRadiusLte64Moments0110_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* top, COMPV_ALIGNED(SSE) const uint8_t* bottom, COMPV_ALIGNED(SSE) const int16_t* x, COMPV_ALIGNED(SSE) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	__m128i vec0, vec1, vec2, vec3, vec4, vec5, vecS10[4], vecS01[4];
	const __m128i vecZero = _mm_setzero_si128();
	vecS10[0] = vecS10[1] = vecS10[2] = vecS10[3] = _mm_setzero_si128();
	vecS01[0] = vecS01[1] = vecS01[2] = vecS01[3] = _mm_setzero_si128();
	// s10 = sum[(x * top) + (x * bottom)] = sum[x * (top + bottom)]
	// s01 = sum[(y * top) - (y * bottom)] = sum[y * (top - bottom)]
	for (compv_uscalar_t i = 0; i < count; i += 16) {
		vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&top[i]));
		vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&bottom[i]));

		vec2 = _mm_unpacklo_epi8(vec0, vecZero); // epu8 -> epi16 [top]
		vec3 = _mm_unpacklo_epi8(vec1, vecZero); // epu8 -> epi16 [bottom]
		vec4 = _mm_add_epi16(vec2, vec3); // top + bottom
		vec5 = _mm_sub_epi16(vec2, vec3); // top - bottom
		// No overflow when doing mullo_epi16 because x and y are within [0-64], top and bottom within [0-255]
		vec4 = _mm_mullo_epi16(vec4, _mm_load_si128(reinterpret_cast<const __m128i*>(&x[i])));
		vec5 = _mm_mullo_epi16(vec5, _mm_load_si128(reinterpret_cast<const __m128i*>(&y[i])));
		vecS10[0] = _mm_add_epi32(vecS10[0], _mm_cvtepi16_epi32_low_SSE2(vec4)); // epi16 -> epi32
		vecS10[1] = _mm_add_epi32(vecS10[1], _mm_cvtepi16_epi32_hi_SSE2(vec4)); // epi16 -> epi32
		vecS01[0] = _mm_add_epi32(vecS01[0], _mm_cvtepi16_epi32_low_SSE2(vec5)); // epi16 -> epi32
		vecS01[1] = _mm_add_epi32(vecS01[1], _mm_cvtepi16_epi32_hi_SSE2(vec5)); // epi16 -> epi32

		vec2 = _mm_unpackhi_epi8(vec0, vecZero); // epu8 -> epi16 [top]
		vec3 = _mm_unpackhi_epi8(vec1, vecZero); // epu8 -> epi16 [bottom]
		vec4 = _mm_add_epi16(vec2, vec3); // top + bottom
		vec5 = _mm_sub_epi16(vec2, vec3); // top - bottom
		// No overflow when doing mullo_epi16 because x and y are within [0-64], top and bottom within [0-255]
		vec4 = _mm_mullo_epi16(vec4, _mm_load_si128(reinterpret_cast<const __m128i*>(&x[i + 8])));
		vec5 = _mm_mullo_epi16(vec5, _mm_load_si128(reinterpret_cast<const __m128i*>(&y[i + 8])));
		vecS10[2] = _mm_add_epi32(vecS10[2], _mm_cvtepi16_epi32_low_SSE2(vec4)); // epi16 -> epi32
		vecS10[3] = _mm_add_epi32(vecS10[3], _mm_cvtepi16_epi32_hi_SSE2(vec4)); // epi16 -> epi32
		vecS01[2] = _mm_add_epi32(vecS01[2], _mm_cvtepi16_epi32_low_SSE2(vec5)); // epi16 -> epi32
		vecS01[3] = _mm_add_epi32(vecS01[3], _mm_cvtepi16_epi32_hi_SSE2(vec5)); // epi16 -> epi32
	}

	vec0 = _mm_add_epi32(vecS10[0], vecS10[1]);
	vec1 = _mm_add_epi32(vecS10[2], vecS10[3]);
	vec2 = _mm_add_epi32(vecS01[0], vecS01[1]);
	vec3 = _mm_add_epi32(vecS01[2], vecS01[3]);
	
	vec0 = _mm_add_epi32(vec0, vec1);
	vec2 = _mm_add_epi32(vec2, vec3);

	// SSSE3 _mm_hadd_epi32
	vec0 = _mm_add_epi32(vec0, _mm_srli_si128(vec0, 8));
	vec2 = _mm_add_epi32(vec2, _mm_srli_si128(vec2, 8));
	vec0 = _mm_add_epi32(vec0, _mm_srli_si128(vec0, 4));
	vec2 = _mm_add_epi32(vec2, _mm_srli_si128(vec2, 4));

	*s10 += static_cast<compv_scalar_t>(_mm_cvtsi128_si32(vec0));
	*s01 += static_cast<compv_scalar_t>(_mm_cvtsi128_si32(vec2));
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

