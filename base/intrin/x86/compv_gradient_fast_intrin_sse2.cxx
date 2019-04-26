/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/x86/compv_gradient_fast_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"
#include "compv/base/intrin/x86/compv_intrin_sse.h"

COMPV_NAMESPACE_BEGIN()

#define COMPVGRADIENTFASTGRADX_8UXX_INTRIN_SSE2(vec2, vec0) \
	/* TODO(dmi): for ARM NEON no need to convert to epi16, use 'vsubl_u8' */ \
	__m128i vec3 = _mm_unpackhi_epi8(vec2, vecZero); \
	vec2 = _mm_unpacklo_epi8(vec2, vecZero); \
	__m128i vec1 = _mm_unpackhi_epi8(vec0, vecZero); \
	vec0 = _mm_unpacklo_epi8(vec0, vecZero); \
	vec0 = _mm_sub_epi16(vec0, vec2); \
	vec1 = _mm_sub_epi16(vec1, vec3);


void CompVGradientFastGradX_8u16s_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecZero = _mm_setzero_si128();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			__m128i vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i - 1]));
			__m128i vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i + 1]));
			COMPVGRADIENTFASTGRADX_8UXX_INTRIN_SSE2(vec2, vec0);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dx[i]), vec0);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dx[i + 8]), vec1);
		}
		input += stride;
		dx += stride;
	}
}

void CompVGradientFastGradX_8u32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecZero = _mm_setzero_si128();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			__m128i vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i - 1]));
			__m128i vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i + 1]));
			COMPVGRADIENTFASTGRADX_8UXX_INTRIN_SSE2(vec2, vec0);
			vec2 = _mm_cvtepi16_epi32_low_SSE2(vec0);
			vec0 = _mm_cvtepi16_epi32_hi_SSE2(vec0);
			vec3 = _mm_cvtepi16_epi32_low_SSE2(vec1);
			vec1 = _mm_cvtepi16_epi32_hi_SSE2(vec1);
			_mm_store_ps(&dx[i], _mm_cvtepi32_ps(vec2));
			_mm_store_ps(&dx[i + 4], _mm_cvtepi32_ps(vec0));
			_mm_store_ps(&dx[i + 8], _mm_cvtepi32_ps(vec3));
			_mm_store_ps(&dx[i + 12], _mm_cvtepi32_ps(vec1));
		}
		input += stride;
		dx += stride;
	}
}

void CompVGradientFastGradX_32f32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_TODO("No ASM code");
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_uscalar_t width16 = width & -16;
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			_mm_store_ps(&dx[i], _mm_sub_ps(_mm_loadu_ps(&input[i + 1]), _mm_loadu_ps(&input[i - 1])));
			_mm_store_ps(&dx[i + 4], _mm_sub_ps(_mm_loadu_ps(&input[i + 5]), _mm_loadu_ps(&input[i + 3])));
			_mm_store_ps(&dx[i + 8], _mm_sub_ps(_mm_loadu_ps(&input[i + 9]), _mm_loadu_ps(&input[i + 7])));
			_mm_store_ps(&dx[i + 12], _mm_sub_ps(_mm_loadu_ps(&input[i + 13]), _mm_loadu_ps(&input[i + 11])));
		}
		for (; i < width; i += 4) {
			_mm_store_ps(&dx[i], _mm_sub_ps(_mm_loadu_ps(&input[i + 1]), _mm_loadu_ps(&input[i - 1])));
		}
		input += stride;
		dx += stride;
	}
}

void CompVGradientFastGradY_8u16s_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecZero = _mm_setzero_si128();
	const uint8_t* inputMinus1 = input - stride;
	const uint8_t* inputPlus1 = input + stride;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			__m128i vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inputMinus1[i]));
			__m128i vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inputPlus1[i]));
			COMPVGRADIENTFASTGRADX_8UXX_INTRIN_SSE2(vec2, vec0);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dy[i]), vec0);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dy[i + 8]), vec1);
		}
		inputMinus1 += stride;
		inputPlus1 += stride;
		dy += stride;
	}
}

void CompVGradientFastGradY_8u32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const __m128i vecZero = _mm_setzero_si128();
	const uint8_t* inputMinus1 = input - stride;
	const uint8_t* inputPlus1 = input + stride;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			__m128i vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inputMinus1[i]));
			__m128i vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&inputPlus1[i]));
			COMPVGRADIENTFASTGRADX_8UXX_INTRIN_SSE2(vec2, vec0);
			vec2 = _mm_cvtepi16_epi32_low_SSE2(vec0);
			vec0 = _mm_cvtepi16_epi32_hi_SSE2(vec0);
			vec3 = _mm_cvtepi16_epi32_low_SSE2(vec1);
			vec1 = _mm_cvtepi16_epi32_hi_SSE2(vec1);
			_mm_store_ps(&dy[i], _mm_cvtepi32_ps(vec2));
			_mm_store_ps(&dy[i + 4], _mm_cvtepi32_ps(vec0));
			_mm_store_ps(&dy[i + 8], _mm_cvtepi32_ps(vec3));
			_mm_store_ps(&dy[i + 12], _mm_cvtepi32_ps(vec1));
		}
		inputMinus1 += stride;
		inputPlus1 += stride;
		dy += stride;
	}
}

void CompVGradientFastGradY_32f32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_TODO("No ASM code");
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_float32_t* inputMinus1 = input - stride;
	const compv_float32_t* inputPlus1 = input + stride;
	const compv_uscalar_t width16 = width & -16;
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			_mm_store_ps(&dy[i], _mm_sub_ps(_mm_load_ps(&inputPlus1[i]), _mm_load_ps(&inputMinus1[i])));
			_mm_store_ps(&dy[i + 4], _mm_sub_ps(_mm_load_ps(&inputPlus1[i + 4]), _mm_load_ps(&inputMinus1[i + 4])));
			_mm_store_ps(&dy[i + 8], _mm_sub_ps(_mm_load_ps(&inputPlus1[i + 8]), _mm_load_ps(&inputMinus1[i + 8])));
			_mm_store_ps(&dy[i + 12], _mm_sub_ps(_mm_load_ps(&inputPlus1[i + 12]), _mm_load_ps(&inputMinus1[i + 12])));
		}
		for (; i < width; i += 4) {
			_mm_store_ps(&dy[i], _mm_sub_ps(_mm_load_ps(&inputPlus1[i]), _mm_load_ps(&inputMinus1[i])));
		}
		inputMinus1 += stride;
		inputPlus1 += stride;
		dy += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
