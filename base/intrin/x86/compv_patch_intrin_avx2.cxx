/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/x86/compv_patch_intrin_avx2.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// top and bottom are padded with zeros and aligned to the max SIMD alignv which means we can read beyond count
// requires radius <= 64 to make sure (x * (top +- bottom)) is within [-0x7fff, +0x7fff]
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVPatchRadiusLte64Moments0110_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* top, COMPV_ALIGNED(AVX) const uint8_t* bottom, COMPV_ALIGNED(AVX) const int16_t* x, COMPV_ALIGNED(AVX) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	__m256i vec0, vec1, vec2, vec3, vec4, vec5, vecS10[4], vecS01[4];
	vecS10[0] = vecS10[1] = vecS10[2] = vecS10[3] = _mm256_setzero_si256();
	vecS01[0] = vecS01[1] = vecS01[2] = vecS01[3] = _mm256_setzero_si256();
	// s10 = sum[(x * top) + (x * bottom)] = sum[x * (top + bottom)]
	// s01 = sum[(y * top) - (y * bottom)] = sum[y * (top - bottom)]
	for (compv_uscalar_t i = 0; i < count; i += 32) {
		vec0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&top[i]));
		vec1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(&bottom[i]));

		vec2 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vec0)); // epu8 -> epi16 [top]
		vec3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vec1)); // epu8 -> epi16 [bottom]
		vec4 = _mm256_add_epi16(vec2, vec3); // top + bottom
		vec5 = _mm256_sub_epi16(vec2, vec3); // top - bottom
		// No overflow when doing mullo_epi16 because x and y are within [0-64], top and bottom within [0-255]
		vec4 = _mm256_mullo_epi16(vec4, _mm256_load_si256(reinterpret_cast<const __m256i*>(&x[i])));
		vec5 = _mm256_mullo_epi16(vec5, _mm256_load_si256(reinterpret_cast<const __m256i*>(&y[i])));
		vecS10[0] = _mm256_add_epi32(vecS10[0], _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vec4))); // epi16 -> epi32
		vecS10[1] = _mm256_add_epi32(vecS10[1], _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vec4, 0x1))); // epi16 -> epi32
		vecS01[0] = _mm256_add_epi32(vecS01[0], _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vec5))); // epi16 -> epi32
		vecS01[1] = _mm256_add_epi32(vecS01[1], _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vec5, 0x1))); // epi16 -> epi32

		vec2 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(vec0, 0x1)); // epu8 -> epi16 [top]
		vec3 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(vec1, 0x1)); // epu8 -> epi16 [bottom]
		vec4 = _mm256_add_epi16(vec2, vec3); // top + bottom
		vec5 = _mm256_sub_epi16(vec2, vec3); // top - bottom
		// No overflow when doing mullo_epi16 because x and y are within [0-64], top and bottom within [0-255]
		vec4 = _mm256_mullo_epi16(vec4, _mm256_load_si256(reinterpret_cast<const __m256i*>(&x[i + 16])));
		vec5 = _mm256_mullo_epi16(vec5, _mm256_load_si256(reinterpret_cast<const __m256i*>(&y[i + 16])));
		vecS10[2] = _mm256_add_epi32(vecS10[2], _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vec4))); // epi16 -> epi32
		vecS10[3] = _mm256_add_epi32(vecS10[3], _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vec4, 0x1))); // epi16 -> epi32
		vecS01[2] = _mm256_add_epi32(vecS01[2], _mm256_cvtepi16_epi32(_mm256_castsi256_si128(vec5))); // epi16 -> epi32
		vecS01[3] = _mm256_add_epi32(vecS01[3], _mm256_cvtepi16_epi32(_mm256_extracti128_si256(vec5, 0x1))); // epi16 -> epi32
	}

	vec0 = _mm256_add_epi32(vecS10[0], vecS10[1]);
	vec1 = _mm256_add_epi32(vecS10[2], vecS10[3]);
	vec2 = _mm256_add_epi32(vecS01[0], vecS01[1]);
	vec3 = _mm256_add_epi32(vecS01[2], vecS01[3]);
	
	vec0 = _mm256_add_epi32(vec0, vec1);
	vec2 = _mm256_add_epi32(vec2, vec3);

	vec0 = _mm256_hadd_epi32(vec0, vec2);
	vec0 = _mm256_hadd_epi32(vec0, vec0);

	// SSE/AVX transition issue if the code not compiled with /AVX flag
	const __m128i vecSSE = _mm_add_epi32(_mm256_castsi256_si128(vec0), _mm256_extracti128_si256(vec0, 0x1));
	*s10 += static_cast<compv_scalar_t>(_mm_cvtsi128_si32(vecSSE));
	*s01 += static_cast<compv_scalar_t>(_mm_extract_epi32(vecSSE, 0x1));

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

