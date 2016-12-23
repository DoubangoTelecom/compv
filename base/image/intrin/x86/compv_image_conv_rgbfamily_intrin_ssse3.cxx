/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_rgbfamily_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVImageConvRgb24family_to_y_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(SSE) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	__m128i rgba[4], xmm0, xmm1, xmmYCoeffs, xmmMaskRgbToRgba, xmm16;
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGB = padY * 3;

	_mm_store_si128(&xmmMaskRgbToRgba, _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_RgbToRgba_i32)));
	_mm_store_si128(&xmm16, _mm_load_si128(reinterpret_cast<const __m128i*>(k16_i16)));
	_mm_store_si128(&xmmYCoeffs, _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBfamilyToYUV_YCoeffs8))); // RGBA coeffs

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			/**  convert from RGB to RGBA **/
			COMPV_3RGB_TO_4RGBA_SSSE3(&rgba, rgbPtr, xmm0, xmm1, xmmMaskRgbToRgba);

			// starting here we're using the same code as rgba -> y

			_mm_store_si128(&rgba[0], _mm_maddubs_epi16(rgba[0], xmmYCoeffs));
			_mm_store_si128(&rgba[1], _mm_maddubs_epi16(rgba[1], xmmYCoeffs));
			_mm_store_si128(&rgba[2], _mm_maddubs_epi16(rgba[2], xmmYCoeffs));
			_mm_store_si128(&rgba[3], _mm_maddubs_epi16(rgba[3], xmmYCoeffs));

			_mm_store_si128(&rgba[0], _mm_hadd_epi16(rgba[0], rgba[1]));
			_mm_store_si128(&rgba[2], _mm_hadd_epi16(rgba[2], rgba[3]));

			_mm_store_si128(&rgba[0], _mm_srai_epi16(rgba[0], 7)); // >> 7
			_mm_store_si128(&rgba[2], _mm_srai_epi16(rgba[2], 7)); // >> 7

			_mm_store_si128(&rgba[0], _mm_add_epi16(rgba[0], xmm16)); // + 16
			_mm_store_si128(&rgba[2], _mm_add_epi16(rgba[2], xmm16)); // + 16

			_mm_store_si128((__m128i*)outYPtr, _mm_packus_epi16(rgba[0], rgba[2])); // Saturate(I16 -> U8)

			outYPtr += 16;
			rgbPtr += 48;
		}
		outYPtr += padY;
		rgbPtr += padRGB;
	}
}

void CompVImageConvRgb24family_to_uv_planar_11_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbPtr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	__m128i rgba[4], xmm0, xmm1, xmm2, xmm3, xmmUCoeffs, xmmVCoeffs, xmm128, xmmMaskRgbToRgba;
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padUV = (stride - maxI), padRGB = padUV * 3;

	_mm_store_si128(&xmmMaskRgbToRgba, _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_RgbToRgba_i32)));
	_mm_store_si128(&xmmUCoeffs, _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBfamilyToYUV_UCoeffs8)));
	_mm_store_si128(&xmmVCoeffs, _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBfamilyToYUV_VCoeffs8)));
	_mm_store_si128(&xmm128, _mm_load_si128(reinterpret_cast<const __m128i*>(k128_i16)));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			/**  convert from RGB to RGBA **/
			COMPV_3RGB_TO_4RGBA_SSSE3(&rgba, rgbPtr, xmm0, xmm1, xmmMaskRgbToRgba);

			// starting here we're using the same code as rgba -> uv

			_mm_store_si128(&xmm0, _mm_maddubs_epi16(rgba[0], xmmUCoeffs));
			_mm_store_si128(&xmm1, _mm_maddubs_epi16(rgba[1], xmmUCoeffs));
			_mm_store_si128(&xmm2, _mm_maddubs_epi16(rgba[2], xmmUCoeffs));
			_mm_store_si128(&xmm3, _mm_maddubs_epi16(rgba[3], xmmUCoeffs));
			_mm_store_si128(&rgba[0], _mm_maddubs_epi16(rgba[0], xmmVCoeffs));
			_mm_store_si128(&rgba[1], _mm_maddubs_epi16(rgba[1], xmmVCoeffs));
			_mm_store_si128(&rgba[2], _mm_maddubs_epi16(rgba[2], xmmVCoeffs));
			_mm_store_si128(&rgba[3], _mm_maddubs_epi16(rgba[3], xmmVCoeffs));

			_mm_store_si128(&xmm0, _mm_hadd_epi16(xmm0, xmm1));
			_mm_store_si128(&xmm2, _mm_hadd_epi16(xmm2, xmm3));
			_mm_store_si128(&rgba[0], _mm_hadd_epi16(rgba[0], rgba[1]));
			_mm_store_si128(&rgba[2], _mm_hadd_epi16(rgba[2], rgba[3]));

			_mm_store_si128(&xmm0, _mm_srai_epi16(xmm0, 8)); // >> 8
			_mm_store_si128(&xmm2, _mm_srai_epi16(xmm2, 8)); // >> 8
			_mm_store_si128(&rgba[0], _mm_srai_epi16(rgba[0], 8)); // >> 8
			_mm_store_si128(&rgba[2], _mm_srai_epi16(rgba[2], 8)); // >> 8

			_mm_store_si128(&xmm0, _mm_add_epi16(xmm0, xmm128)); // + 128
			_mm_store_si128(&xmm2, _mm_add_epi16(xmm2, xmm128)); // + 128
			_mm_store_si128(&rgba[0], _mm_add_epi16(rgba[0], xmm128)); // + 128
			_mm_store_si128(&rgba[2], _mm_add_epi16(rgba[2], xmm128)); // + 128

			_mm_store_si128(reinterpret_cast<__m128i*>(outUPtr), _mm_packus_epi16(xmm0, xmm2)); // Saturate(I16 -> U8)
			_mm_store_si128(reinterpret_cast<__m128i*>(outVPtr), _mm_packus_epi16(rgba[0], rgba[2])); // Saturate(I16 -> U8)

			outUPtr += 16;
			outVPtr += 16;
			rgbPtr += 48;
		}
		rgbPtr += padRGB;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
