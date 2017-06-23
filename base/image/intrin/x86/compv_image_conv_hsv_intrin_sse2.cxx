/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_hsv_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// width must be >= 16
void CompVImageConvRgba32ToHsv_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgba32Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	compv_uscalar_t i, j;
	const compv_uscalar_t strideInBytes = (stride << 2);
	__m128i vec0, vec1, vec2, vec3, vec4, vec5;
	static const __m128i vecZero = _mm_setzero_si128();

	width <<= 2; // from samples to bytes
	stride <<= 2; // from samples to bytes

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 64) { // 64 = (16 * 4)
			/* Load samples (TODO(dmi): for ARM neon use vld4.u8) */
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 0])); // RGBA RGBA RGBA RGBA
			vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 16])); // RGBA RGBA RGBA RGBA
			vec2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 32])); // RGBA RGBA RGBA RGBA
			vec3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&rgba32Ptr[i + 48])); // RGBA RGBA RGBA RGBA
			
			/* Transpose */
			COMPV_TRANSPOSE_I8_4X16_SSE2(vec0, vec1, vec2, vec3, vec4); // [RRRR GGGG BBBB AAAA] * 4 -> indexes: 0, 16, 32, 48

			/* De-interleave */
			vec4 = _mm_unpacklo_epi32(vec0, vec1); // RRRR RRRR GGGG GGGG
			vec5 = _mm_unpacklo_epi32(vec2, vec3); // RRRR RRRR GGGG GGGG
			vec0 = _mm_unpackhi_epi32(vec0, vec1); // BBBB BBBB AAAA AAAA
			vec2 = _mm_unpackhi_epi32(vec2, vec3); // BBBB BBBB AAAA AAAA
			vec1 = _mm_unpacklo_epi64(vec4, vec5); // [[RRRR RRRR RRRR RRRR]]
			vec4 = _mm_unpackhi_epi64(vec4, vec5); // [[GGGG GGGG GGGG GGGG]]
			vec0 = _mm_unpacklo_epi64(vec0, vec2); // [[BBBB BBBB BBBB BBBB]]

			// vec1 = R, vec4 = G, vec0 = B

			vec2 = _mm_min_epu8(vec1, _mm_min_epu8(vec4, vec0)); // vec2 = minVal
			vec3 = _mm_max_epu8(vec1, _mm_max_epu8(vec4, vec0)); // vec3 = maxVal

			vec5 = _mm_cmpeq_epi8(vec3, vecZero); // vec5 = mask(!(hsv[2] = maxVal))
			if (_mm_movemask_epi8(vec5) == 0xffff) {
				// hsv[0] = hsv[1] = 0;
			}
			
		} // End_Of for (i = 0; i < width; i += 64)
		rgba32Ptr += stride;
		hsvPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
