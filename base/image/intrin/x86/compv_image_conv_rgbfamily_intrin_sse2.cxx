/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_rgbfamily_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

// TODO(dmi): RGB -> RGBA conversion is done twice (Y plane then UV plane)

#define RGB565_BIG_TO_LITTLE_SSE2(xmm0, xmm1) \
		xmm0 = _mm_or_si128(_mm_slli_epi16(xmm0, 8), _mm_srli_epi16(xmm0, 8)); \
		xmm1 = _mm_or_si128(_mm_slli_epi16(xmm1, 8), _mm_srli_epi16(xmm1, 8))
#define RGB565_LITTLE_TO_LITTLE_SSE2(xmm0, xmm1) 

#define RGB565_TO_Y_SSE2(ENDIANNESS) \
	COMPV_DEBUG_INFO_CHECK_SSE2(); \
	__m128i xmmR0, xmmG0, xmmB0, xmmR1, xmmG1, xmmB1, xmm0, xmm1; \
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGB = padY << 1; \
	const __m128i xmm16 = _mm_load_si128(reinterpret_cast<const __m128i*>(k16_16s)); \
	const __m128i xmmMaskR = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGB565ToYUV_RMask_u16)); \
	const __m128i xmmMaskG = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGB565ToYUV_GMask_u16)); \
	const __m128i xmmMaskB = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGB565ToYUV_BMask_u16)); \
	const __m128i xmmCoeffR = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_YCoeffs8[0])); \
	const __m128i xmmCoeffG = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_YCoeffs8[1])); \
	const __m128i xmmCoeffB = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_YCoeffs8[2])); \
	/* Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16 */ \
	for (j = 0; j < height; ++j) { \
		for (i = 0; i < width; i += 16) { \
			xmm0 = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb565Ptr + 0)); \
			xmm1 = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb565Ptr + 16)); \
			RGB565_##ENDIANNESS##_TO_LITTLE_SSE2(xmm0, xmm1); \
			xmmR0 = _mm_srli_epi16(_mm_and_si128(xmm0, xmmMaskR), 8); \
			xmmR1 = _mm_srli_epi16(_mm_and_si128(xmm1, xmmMaskR), 8); \
			xmmG0 = _mm_srli_epi16(_mm_and_si128(xmm0, xmmMaskG), 3); \
			xmmG1 = _mm_srli_epi16(_mm_and_si128(xmm1, xmmMaskG), 3); \
			xmmB0 = _mm_slli_epi16(_mm_and_si128(xmm0, xmmMaskB), 3); \
			xmmB1 = _mm_slli_epi16(_mm_and_si128(xmm1, xmmMaskB), 3); \
			xmmR0 = _mm_or_si128(xmmR0, _mm_srli_epi16(xmmR0, 5)); \
			xmmR1 = _mm_or_si128(xmmR1, _mm_srli_epi16(xmmR1, 5)); \
			xmmG0 = _mm_or_si128(xmmG0, _mm_srli_epi16(xmmG0, 6)); \
			xmmG1 = _mm_or_si128(xmmG1, _mm_srli_epi16(xmmG1, 6)); \
			xmmB0 = _mm_or_si128(xmmB0, _mm_srli_epi16(xmmB0, 5)); \
			xmmB1 = _mm_or_si128(xmmB1, _mm_srli_epi16(xmmB1, 5)); \
			xmmR0 = _mm_mullo_epi16(xmmR0, xmmCoeffR); \
			xmmR1 = _mm_mullo_epi16(xmmR1, xmmCoeffR); \
			xmmG0 = _mm_mullo_epi16(xmmG0, xmmCoeffG); \
			xmmG1 = _mm_mullo_epi16(xmmG1, xmmCoeffG); \
			xmmB0 = _mm_mullo_epi16(xmmB0, xmmCoeffB); \
			xmmB1 = _mm_mullo_epi16(xmmB1, xmmCoeffB); \
			xmm0 = _mm_add_epi16(_mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(xmmR0, xmmG0), xmmB0), 7), xmm16); \
			xmm1 = _mm_add_epi16(_mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(xmmR1, xmmG1), xmmB1), 7), xmm16); \
			_mm_store_si128(reinterpret_cast<__m128i*>(outYPtr), _mm_packus_epi16(xmm0, xmm1)); \
			outYPtr += 16; \
			rgb565Ptr += 32; \
		} \
		outYPtr += padY; \
		rgb565Ptr += padRGB; \
	}

#define RGB565_TO_UV_SSE2(ENDIANNESS) \
	COMPV_DEBUG_INFO_CHECK_SSE2(); \
	__m128i xmmR0, xmmG0, xmmB0, xmmR1, xmmG1, xmmB1, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5; \
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padUV = (stride - maxI), padRGB = padUV << 1; \
	const __m128i xmm128 = _mm_load_si128(reinterpret_cast<const __m128i*>(k128_16s)); \
	const __m128i xmmMaskR = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGB565ToYUV_RMask_u16)); \
	const __m128i xmmMaskG = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGB565ToYUV_GMask_u16)); \
	const __m128i xmmMaskB = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGB565ToYUV_BMask_u16)); \
	const __m128i xmmCoeffRU = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_UCoeffs8[0])); \
	const __m128i xmmCoeffGU = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_UCoeffs8[1])); \
	const __m128i xmmCoeffBU = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_UCoeffs8[2])); \
	const __m128i xmmCoeffRV = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_VCoeffs8[0])); \
	const __m128i xmmCoeffGV = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_VCoeffs8[1])); \
	const __m128i xmmCoeffBV = _mm_set1_epi16(static_cast<int16_t>(kRGBfamilyToYUV_VCoeffs8[2])); \
	/* U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128 */ \
	/* V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128 */ \
	for (j = 0; j < height; ++j) { \
		for (i = 0; i < width; i += 16) { \
			xmm0 = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb565Ptr + 0)); \
			xmm1 = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb565Ptr + 16)); \
			RGB565_##ENDIANNESS##_TO_LITTLE_SSE2(xmm0, xmm1); \
			xmmR0 = _mm_srli_epi16(_mm_and_si128(xmm0, xmmMaskR), 8); \
			xmmR1 = _mm_srli_epi16(_mm_and_si128(xmm1, xmmMaskR), 8); \
			xmmG0 = _mm_srli_epi16(_mm_and_si128(xmm0, xmmMaskG), 3); \
			xmmG1 = _mm_srli_epi16(_mm_and_si128(xmm1, xmmMaskG), 3); \
			xmmB0 = _mm_slli_epi16(_mm_and_si128(xmm0, xmmMaskB), 3); \
			xmmB1 = _mm_slli_epi16(_mm_and_si128(xmm1, xmmMaskB), 3); \
			xmmR0 = _mm_or_si128(xmmR0, _mm_srli_epi16(xmmR0, 5)); \
			xmmR1 = _mm_or_si128(xmmR1, _mm_srli_epi16(xmmR1, 5)); \
			xmmG0 = _mm_or_si128(xmmG0, _mm_srli_epi16(xmmG0, 6)); \
			xmmG1 = _mm_or_si128(xmmG1, _mm_srli_epi16(xmmG1, 6)); \
			xmmB0 = _mm_or_si128(xmmB0, _mm_srli_epi16(xmmB0, 5)); \
			xmmB1 = _mm_or_si128(xmmB1, _mm_srli_epi16(xmmB1, 5)); \
			xmm0 = _mm_mullo_epi16(xmmR0, xmmCoeffRU); \
			xmm1 = _mm_mullo_epi16(xmmR1, xmmCoeffRU); \
			xmm2 = _mm_mullo_epi16(xmmG0, xmmCoeffGU); \
			xmm3 = _mm_mullo_epi16(xmmG1, xmmCoeffGU); \
			xmm4 = _mm_mullo_epi16(xmmB0, xmmCoeffBU); \
			xmm5 = _mm_mullo_epi16(xmmB1, xmmCoeffBU); \
			xmmR0 = _mm_mullo_epi16(xmmR0, xmmCoeffRV); \
			xmmR1 = _mm_mullo_epi16(xmmR1, xmmCoeffRV); \
			xmmG0 = _mm_mullo_epi16(xmmG0, xmmCoeffGV); \
			xmmG1 = _mm_mullo_epi16(xmmG1, xmmCoeffGV); \
			xmmB0 = _mm_mullo_epi16(xmmB0, xmmCoeffBV); \
			xmmB1 = _mm_mullo_epi16(xmmB1, xmmCoeffBV); \
			xmm0 = _mm_add_epi16(_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(xmm0, xmm2), xmm4), 8), xmm128); \
			xmm1 = _mm_add_epi16(_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(xmm1, xmm3), xmm5), 8), xmm128); \
			xmmR0 = _mm_add_epi16(_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(xmmR0, xmmG0), xmmB0), 8), xmm128); \
			xmmR1 = _mm_add_epi16(_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(xmmR1, xmmG1), xmmB1), 8), xmm128); \
			_mm_store_si128(reinterpret_cast<__m128i*>(outUPtr), _mm_packus_epi16(xmm0, xmm1)); \
			_mm_store_si128(reinterpret_cast<__m128i*>(outVPtr), _mm_packus_epi16(xmmR0, xmmR1)); \
			outUPtr += 16; \
			outVPtr += 16; \
			rgb565Ptr += 32; \
		} \
		outUPtr += padUV; \
		outVPtr += padUV; \
		rgb565Ptr += padRGB; \
	}

COMPV_NAMESPACE_BEGIN()

void CompVImageConvRgb565lefamily_to_y_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	RGB565_TO_Y_SSE2(LITTLE);
}

void CompVImageConvRgb565befamily_to_y_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	RGB565_TO_Y_SSE2(BIG);
}

void CompVImageConvRgb565lefamily_to_uv_planar_11_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565Ptr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	RGB565_TO_UV_SSE2(LITTLE);
}

void CompVImageConvRgb565befamily_to_uv_planar_11_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565Ptr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	RGB565_TO_UV_SSE2(BIG);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
