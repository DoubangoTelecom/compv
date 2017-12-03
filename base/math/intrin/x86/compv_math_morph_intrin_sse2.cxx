/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_morph_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathMorphProcessErode_8u_Intrin_SSE2(const compv_uscalar_t* strelInputPtrsPtr, const compv_uscalar_t strelInputPtrsCount, uint8_t* outPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t i, j, k, v;
	const compv_uscalar_t width64 = width & -64;
	const compv_uscalar_t width16 = width & -16;
	const compv_uscalar_t strelInputPtrsPad = (stride - ((width + 15) & -16));
	__m128i vec0, vec1, vec2, vec3;
	COMPV_ALIGN_SSE() uint8_t mem[16];

	for (j = 0, k = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64, k += 64) {
			vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[0]));
			vec1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[0]) + 1);
			vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[0]) + 2);
			vec3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[0]) + 3);
			for (v = 1; v < strelInputPtrsCount; ++v) {
				vec0 = _mm_min_epu8(vec0, _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[v])));
				vec1 = _mm_min_epu8(vec1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[v]) + 1));
				vec2 = _mm_min_epu8(vec2, _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[v]) + 2));
				vec3 = _mm_min_epu8(vec3, _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[v]) + 3));
			}
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vec0);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]) + 1, vec1);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]) + 2, vec2);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]) + 3, vec3);
		}
		for (; i < width; i += 16, k += 16) {
			vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[0]));
			for (v = 1; v < strelInputPtrsCount; ++v) {
				vec0 = _mm_min_epu8(vec0, _mm_loadu_si128(reinterpret_cast<const __m128i*>(k + strelInputPtrsPtr[v])));
			}
			if (i < width16) {
				_mm_storeu_si128(reinterpret_cast<__m128i*>(&outPtr[i]), vec0);
			}
			else {
				_mm_store_si128(reinterpret_cast<__m128i*>(mem), vec0);
				for (v = 0; i < width; ++i, ++v) {
					outPtr[i] = mem[v];
				}
			}
		}
		outPtr += stride;
		k += strelInputPtrsPad;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
