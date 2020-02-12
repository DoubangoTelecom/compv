/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_cast_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathCastProcess_static_64f32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float64_t* src,
	COMPV_ALIGNED(SSE) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(SSE) const compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_uscalar_t width8 = width & -8;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		compv_uscalar_t i = 0;
		for (; i < width8; i += 8) {
			const __m128d vec0 = _mm_load_pd(&src[i]);
			const __m128d vec1 = _mm_load_pd(&src[i + 2]);
			const __m128d vec2 = _mm_load_pd(&src[i + 4]);
			const __m128d vec3 = _mm_load_pd(&src[i + 6]);
			_mm_store_ps(&dst[i], _mm_shuffle_ps(_mm_cvtpd_ps(vec0), _mm_cvtpd_ps(vec1), 0x44));
			_mm_store_ps(&dst[i + 4], _mm_shuffle_ps(_mm_cvtpd_ps(vec2), _mm_cvtpd_ps(vec3), 0x44));
		}
		for (; i < width; i += 4) {
			const __m128d vec0 = _mm_load_pd(&src[i]);
			const __m128d vec1 = _mm_load_pd(&src[i + 2]);
			_mm_store_ps(&dst[i], _mm_shuffle_ps(_mm_cvtpd_ps(vec0), _mm_cvtpd_ps(vec1), 0x44)); // SSE-aligned -> can write beyond width and up to stride
		}
		src += stride;
		dst += stride;
	}
}

void CompVMathCastProcess_static_8u32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const uint8_t* src,
	COMPV_ALIGNED(SSE) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(SSE) const compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecZero = _mm_setzero_si128();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			__m128i vec1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&src[i]));
			__m128i vec3 = _mm_unpackhi_epi8(vec1, vecZero);
			vec1 = _mm_unpacklo_epi8(vec1, vecZero);
			__m128i vec0 = _mm_unpacklo_epi16(vec1, vecZero);
			vec1 = _mm_unpackhi_epi16(vec1, vecZero);
			__m128i vec2 = _mm_unpacklo_epi16(vec3, vecZero);
			vec3 = _mm_unpackhi_epi16(vec3, vecZero);
			_mm_store_ps(&dst[i], _mm_cvtepi32_ps(vec0));
			_mm_store_ps(&dst[i + 4], _mm_cvtepi32_ps(vec1));
			_mm_store_ps(&dst[i + 8], _mm_cvtepi32_ps(vec2));
			_mm_store_ps(&dst[i + 12], _mm_cvtepi32_ps(vec3));
		}
		src += stride;
		dst += stride;
	}
}

void CompVMathCastProcess_static_pixel8_32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float32_t* src,
	COMPV_ALIGNED(SSE) uint8_t* dst,
	compv_uscalar_t width,
	compv_uscalar_t height,
	COMPV_ALIGNED(SSE) compv_uscalar_t stride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			__m128i vec0 = _mm_cvttps_epi32(_mm_load_ps(&src[i]));
			__m128i vec1 = _mm_cvttps_epi32(_mm_load_ps(&src[i + 4]));
			__m128i vec2 = _mm_cvttps_epi32(_mm_load_ps(&src[i + 8]));
			__m128i vec3 = _mm_cvttps_epi32(_mm_load_ps(&src[i + 12]));
			vec0 = _mm_packs_epi32(vec0, vec1);
			vec2 = _mm_packs_epi32(vec2, vec3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dst[i]), _mm_packus_epi16(vec0, vec2));
		}
		src += stride;
		dst += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
