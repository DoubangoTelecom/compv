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

// TODO(dmi): RGB -> RGBA conversion is done twice (Y plane then UV plane)

/*
; Macro used to convert 16 RGB to 16 RGBA samples
; 16 RGB samples requires 48 Bytes(3 XMM registers), will be converted to 16 RGBA samples
; requiring 64 Bytes (4 XMM registers)
; The aplha channel will contain zeros instead of 0xff because this macro is used to fetch samples in place
*/
#define COMPV_16xRGB_TO_16xRGBA_SSSE3_FAST(rgb24Ptr_, xmm0RGBA_, xmm1RGBA_, xmm2RGBA_, xmm3RGBA_, xmmMaskRgbToRgba_) \
	xmm0RGBA_ = _mm_shuffle_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(rgb24Ptr_ + 0)), xmmMaskRgbToRgba_); \
	xmm1RGBA_ = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb24Ptr_ + 12)), xmmMaskRgbToRgba_); \
	xmm2RGBA_ = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb24Ptr_ + 24)), xmmMaskRgbToRgba_); \
	xmm3RGBA_ = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(rgb24Ptr_ + 36)), xmmMaskRgbToRgba_); 
// Next version not optimized as we load the masks for each call, use above version and load masks once
#define COMPV_16xRGB_TO_16xRGBA_SSSE3_SLOW(rgb24Ptr_, xmmRGBA0_, xmmRGBA1_, xmmRGBA2_, xmmRGBA3_) \
	COMPV_16xRGB_TO_16xRGBA_SSSE3_FAST(rgb24Ptr_, xmmRGBA0_, xmmRGBA1_, xmmRGBA2_, xmmRGBA3_, _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_RgbToRgba_i32)))

/*
Convert 16 RGBA samples to 16 Y samples
*/
#define COMPV_16xRGBA_TO_16xLUMA_SSSE3(xmm0RGBA_, xmm1RGBA_, xmm2RGBA_, xmm3RGBA_, xmmYCoeffs_, xmm16_, outYPtr_) \
	xmm0RGBA_ = _mm_hadd_epi16(_mm_maddubs_epi16(xmm0RGBA_, xmmYCoeffs_), _mm_maddubs_epi16(xmm1RGBA_, xmmYCoeffs_)); \
	xmm2RGBA_ = _mm_hadd_epi16(_mm_maddubs_epi16(xmm2RGBA_, xmmYCoeffs_), _mm_maddubs_epi16(xmm3RGBA_, xmmYCoeffs_)); \
	xmm0RGBA_ = _mm_srai_epi16(xmm0RGBA_, 7); /* >> 7 */ \
	xmm2RGBA_ = _mm_srai_epi16(xmm2RGBA_, 7); /* >> 7 */  \
	xmm0RGBA_ = _mm_add_epi16(xmm0RGBA_, xmm16_); /* + 16 */  \
	xmm2RGBA_ = _mm_add_epi16(xmm2RGBA_, xmm16_); /* + 16 */  \
	_mm_store_si128(reinterpret_cast<__m128i*>(outYPtr_), _mm_packus_epi16(xmm0RGBA_, xmm2RGBA_)); /* Saturate(I16 -> U8) */

/*
Convert 16 RGBA samples to 16 chroma (U or V samples) samples. Chroma subsampled x1
*/
#define COMPV_16xRGBA_TO_16xCHROMA1_SSSE3(xmm0RGBA_, xmm1RGBA_, xmm2RGBA_, xmm3RGBA_, xmm0C_, xmm1C_, xmmCCoeffs_, xmm128_, outCPtr_) \
	xmm0C_ = _mm_hadd_epi16(_mm_maddubs_epi16(xmm0RGBA_, xmmCCoeffs_), _mm_maddubs_epi16(xmm1RGBA_, xmmCCoeffs_)); \
	xmm1C_ = _mm_hadd_epi16(_mm_maddubs_epi16(xmm2RGBA_, xmmCCoeffs_), _mm_maddubs_epi16(xmm3RGBA_, xmmCCoeffs_)); \
	xmm0C_ = _mm_srai_epi16(xmm0C_, 8); /* >> 8 */ \
	xmm1C_ = _mm_srai_epi16(xmm1C_, 8); /* >> 8 */ \
	xmm0C_ = _mm_add_epi16(xmm0C_, xmm128_); /* + 128 */ \
	xmm1C_ = _mm_add_epi16(xmm1C_, xmm128_); /* + 128 */ \
	_mm_store_si128(reinterpret_cast<__m128i*>(outCPtr_), _mm_packus_epi16(xmm0C_, xmm1C_)); /* Saturate(I16 -> U8) */

COMPV_NAMESPACE_BEGIN()

void CompVImageConvRgb24family_to_y_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb24Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	__m128i xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmmYCoeffs, xmmMaskRgbToRgba, xmm16;
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGB = padY * 3;

	xmmMaskRgbToRgba = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_RgbToRgba_i32));
	xmm16 = _mm_load_si128(reinterpret_cast<const __m128i*>(k16_i16));
	xmmYCoeffs = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBfamilyToYUV_YCoeffs8)); // RGBA coeffs

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			//  convert from RGB to RGBA, alpha channel contains zeros
			COMPV_16xRGB_TO_16xRGBA_SSSE3_FAST(rgb24Ptr, xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmmMaskRgbToRgba);
			// convert from RGBA to Luma(Y)
			COMPV_16xRGBA_TO_16xLUMA_SSSE3(xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmmYCoeffs, xmm16, outYPtr);
			outYPtr += 16;
			rgb24Ptr += 48;
		}
		outYPtr += padY;
		rgb24Ptr += padRGB;
	}
}

void CompVImageConvRgb32family_to_y_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb32Ptr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	__m128i xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmmYCoeffs, xmm16;
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGBA = padY << 2;

	xmm16 = _mm_load_si128(reinterpret_cast<const __m128i*>(k16_i16));
	xmmYCoeffs = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBAfamilyToYUV_YCoeffs8)); // RGBA coeffs

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			xmm0RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 0));
			xmm1RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 16));
			xmm2RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 32));
			xmm3RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 48));
			COMPV_16xRGBA_TO_16xLUMA_SSSE3(xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmmYCoeffs, xmm16, outYPtr);
			outYPtr += 16;
			rgb32Ptr += 64;
		}
		outYPtr += padY;
		rgb32Ptr += padRGBA;
	}
}

void CompVImageConvRgb24family_to_uv_planar_11_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb24Ptr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	__m128i xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmm0C, xmm1C, xmmUCoeffs, xmmVCoeffs, xmm128, xmmMaskRgbToRgba;
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padUV = (stride - maxI), padRGB = padUV * 3;

	xmmMaskRgbToRgba = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_RgbToRgba_i32));
	xmmUCoeffs = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBfamilyToYUV_UCoeffs8));
	xmmVCoeffs = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBfamilyToYUV_VCoeffs8));
	xmm128 = _mm_load_si128(reinterpret_cast<const __m128i*>(k128_i16));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			//  convert from RGB to RGBA, alpha channel contains zeros
			COMPV_16xRGB_TO_16xRGBA_SSSE3_FAST(rgb24Ptr, xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmmMaskRgbToRgba);
			// convert from RGBA to chroma (U and V)
			COMPV_16xRGBA_TO_16xCHROMA1_SSSE3(xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmm0C, xmm1C, xmmUCoeffs, xmm128, outUPtr);
			COMPV_16xRGBA_TO_16xCHROMA1_SSSE3(xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmm0C, xmm1C, xmmVCoeffs, xmm128, outVPtr);
			outUPtr += 16;
			outVPtr += 16;
			rgb24Ptr += 48;
		}
		rgb24Ptr += padRGB;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

void CompVImageConvRgb32family_to_uv_planar_11_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb32Ptr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	__m128i xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmm0C, xmm1C, xmmUCoeffs, xmmVCoeffs, xmm128;
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padUV = (stride - maxI), padRGBA = padUV << 2;

	xmmUCoeffs = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBAfamilyToYUV_UCoeffs8));
	xmmVCoeffs = _mm_load_si128(reinterpret_cast<const __m128i*>(kRGBAfamilyToYUV_VCoeffs8));
	xmm128 = _mm_load_si128(reinterpret_cast<const __m128i*>(k128_i16));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			xmm0RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 0));
			xmm1RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 16));
			xmm2RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 32));
			xmm3RGBA = _mm_load_si128(reinterpret_cast<const __m128i*>(rgb32Ptr + 48));
			COMPV_16xRGBA_TO_16xCHROMA1_SSSE3(xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmm0C, xmm1C, xmmUCoeffs, xmm128, outUPtr);
			COMPV_16xRGBA_TO_16xCHROMA1_SSSE3(xmm0RGBA, xmm1RGBA, xmm2RGBA, xmm3RGBA, xmm0C, xmm1C, xmmVCoeffs, xmm128, outVPtr);
			outUPtr += 16;
			outVPtr += 16;
			rgb32Ptr += 64;
		}
		rgb32Ptr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
