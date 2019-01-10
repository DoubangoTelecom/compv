/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_threshold_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVImageThresholdGlobal_8u8u_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const uint8_t* inPtr,
	COMPV_ALIGNED(SSE) uint8_t* outPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	compv_uscalar_t threshold
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	__m128i vec0, vec1, vec2, vec3;
	const __m128i vecThreshold = _mm_set1_epi8((int8_t)(threshold ^ 0x80));
	const __m128i vecMask = _mm_set1_epi8((int8_t)(0x80));
	const compv_uscalar_t width1 = width & -64;

	compv_uscalar_t i, j;
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width1; i += 64) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&inPtr[i]));
			vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&inPtr[i + 16]));
			vec2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&inPtr[i + 32]));
			vec3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&inPtr[i + 48]));

			vec0 = _mm_xor_si128(vec0, vecMask);
			vec1 = _mm_xor_si128(vec1, vecMask);
			vec2 = _mm_xor_si128(vec2, vecMask);
			vec3 = _mm_xor_si128(vec3, vecMask);

			vec0 = _mm_cmpgt_epi8(vec0, vecThreshold);
			vec1 = _mm_cmpgt_epi8(vec1, vecThreshold);
			vec2 = _mm_cmpgt_epi8(vec2, vecThreshold);
			vec3 = _mm_cmpgt_epi8(vec3, vecThreshold);

			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vec0);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i + 16]), vec1);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i + 32]), vec2);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i + 48]), vec3);
		}
		for (; i < width; i += 16) {
			vec0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&inPtr[i]));
			vec0 = _mm_xor_si128(vec0, vecMask);
			vec0 = _mm_cmpgt_epi8(vec0, vecThreshold);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vec0);
		}
		inPtr += stride;
		outPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
