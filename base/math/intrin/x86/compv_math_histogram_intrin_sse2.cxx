/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_histogram_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// width must be >= 16
void CompVMathHistogramBuildProjectionX_8u32s_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* ptrIn, COMPV_ALIGNED(SSE) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_ASSERT(width >= 16);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM/AVX2 faster");
	const compv_uscalar_t width16 = width & -16;
	compv_uscalar_t i;
	const __m128i vecZero = _mm_setzero_si128();
	__m128i vec0, vec1;

	/* Copy first row (to avoid using memset(0)) */
	for (i = 0; i < width16; i += 16) {
		// int32_t <- uint8_t
		vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptrIn[i]));
		vec0 = _mm_unpacklo_epi8(vec1, vecZero);
		vec1 = _mm_unpackhi_epi8(vec1, vecZero);
		_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i]), _mm_unpacklo_epi16(vec0, vecZero));
		_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i + 4]), _mm_unpackhi_epi16(vec0, vecZero));
		_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i + 8]), _mm_unpacklo_epi16(vec1, vecZero));
		_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i + 12]), _mm_unpackhi_epi16(vec1, vecZero));
	}
	for (; i < width; ++i) {
		ptrOut[i] = ptrIn[i]; // int32_t <- uint8_t
	}
	ptrIn += stride;
	/* Other rows */
	for (compv_uscalar_t j = 1; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptrIn[i]));
			vec0 = _mm_unpacklo_epi8(vec1, vecZero);
			vec1 = _mm_unpackhi_epi8(vec1, vecZero);
			// TODO(dmi): for ASM/AVX no need to load "ptrOut" for the sum, use it as 3rd operator
			_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i]),
				_mm_add_epi32(
					_mm_load_si128(reinterpret_cast<const __m128i*>(&ptrOut[i])), // Perf issue, do not load ptrOut -> see ASM/AVX2 code
					_mm_unpacklo_epi16(vec0, vecZero))
			);
			_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i + 4]),
				_mm_add_epi32(
					_mm_load_si128(reinterpret_cast<const __m128i*>(&ptrOut[i + 4])),
					_mm_unpackhi_epi16(vec0, vecZero))
			);
			_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i + 8]),
				_mm_add_epi32(
					_mm_load_si128(reinterpret_cast<const __m128i*>(&ptrOut[i + 8])),
					_mm_unpacklo_epi16(vec1, vecZero))
			);
			_mm_store_si128(reinterpret_cast<__m128i*>(&ptrOut[i + 12]),
				_mm_add_epi32(
					_mm_load_si128(reinterpret_cast<const __m128i*>(&ptrOut[i + 12])),
					_mm_unpackhi_epi16(vec1, vecZero))
			);
		}
		for (; i < width; ++i) {
			ptrOut[i] += ptrIn[i]; // int32_t <- uint8_t
		}
		ptrIn += stride;
	}
}

// width must be >= 16
void CompVMathHistogramBuildProjectionY_8u32s_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* ptrIn, COMPV_ALIGNED(SSE) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_ASSERT(width >= 16);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM/AVX2 faster");
	int32_t sum;
	compv_uscalar_t i;
	const compv_uscalar_t width16 = width & -16;
	const __m128i vecZero = _mm_setzero_si128();
	__m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		// int32_t <- uint8_t
		vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptrIn[0]));
		vec3 = _mm_unpackhi_epi8(vec1, vecZero);
		vec1 = _mm_unpacklo_epi8(vec1, vecZero);
		vec0 = _mm_unpacklo_epi16(vec1, vecZero);
		vec1 = _mm_unpackhi_epi16(vec1, vecZero);
		vec2 = _mm_unpacklo_epi16(vec3, vecZero);
		vec3 = _mm_unpackhi_epi16(vec3, vecZero);
		for (i = 16; i < width16; i += 16) {
			vec5 = _mm_load_si128(reinterpret_cast<const __m128i*>(&ptrIn[i]));
			vec7 = _mm_unpackhi_epi8(vec5, vecZero);
			vec5 = _mm_unpacklo_epi8(vec5, vecZero);
			vec4 = _mm_unpacklo_epi16(vec5, vecZero);
			vec5 = _mm_unpackhi_epi16(vec5, vecZero);
			vec6 = _mm_unpacklo_epi16(vec7, vecZero);
			vec7 = _mm_unpackhi_epi16(vec7, vecZero);
			vec0 = _mm_add_epi32(vec0, vec4);
			vec1 = _mm_add_epi32(vec1, vec5);
			vec2 = _mm_add_epi32(vec2, vec6);
			vec3 = _mm_add_epi32(vec3, vec7);
		}
		vec0 = _mm_add_epi32(vec0, vec2);
		vec1 = _mm_add_epi32(vec1, vec3);
		vec0 = _mm_add_epi32(vec0, vec1);
		// _mm_hadd_epi32 is SSSE3
		vec1 = _mm_shuffle_epi32(vec0, 0xE);
		vec0 = _mm_add_epi32(vec0, vec1);
		vec1 = _mm_shuffle_epi32(vec0, 0x1);
		vec0 = _mm_add_epi32(vec0, vec1);
		sum = _mm_cvtsi128_si32(vec0);

		for (; i < width; ++i) {
			sum += ptrIn[i]; // int32_t <- uint8_t
		}

		ptrIn += stride;
		ptrOut[j] = sum;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
