/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_rgbfamily_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

// TODO(dmi): RGB -> RGBA conversion is done twice (Y plane then UV plane)

/*
Macro used to convert 32 RGB to 32 RGBA samples
32 RGB samples requires 96 Bytes(3 YMM registers), will be converted to 32 RGBA samples
requiring 128 Bytes (4 YMM registers)
The aplha channel will contain zeros instead of 0xff because this macro is used to fetch samples in place
*/
#define COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb24Ptr_, ymm0RGBA_, ymm1RGBA_, ymm2RGBA_, ymm3RGBA_, ymmABCDDEFG_, ymmMaskRgbToRgba_) \
	ymm0RGBA_ = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgb24Ptr_ + 0)), ymmABCDDEFG_), ymmMaskRgbToRgba_); \
	ymm1RGBA_ = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(rgb24Ptr_ + 24)), ymmABCDDEFG_), ymmMaskRgbToRgba_); \
	ymm2RGBA_ = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(rgb24Ptr_ + 48)), ymmABCDDEFG_), ymmMaskRgbToRgba_); \
	ymm3RGBA_ = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(rgb24Ptr_ + 72)), ymmABCDDEFG_), ymmMaskRgbToRgba_);
// Next version not optimized as we load the masks for each call, use above version and load masks once
#define COMPV_32xRGB_TO_32xRGBA_AVX2_SLOW(rgb32Ptr_, ymm0RGBA_, ymm1RGBA_, ymm2RGBA_, ymm3RGBA_) \
	COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb32Ptr_, ymm0RGBA_, ymm1RGBA_, ymm2RGBA_, ymm3RGBA_, \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32)), \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32)) \
	)

/*
Convert 32 RGBA to 32 Luma (Y)
*/
#define COMPV_32xRGBA_TO_32xLUMA_AVX2(ymm0RGBA_, ymm1RGBA_, ymm2RGBA_, ymm3RGBA_, ymmYCoeffs_, ymm16_, ymmAEBFCGDH_, outYPtr_) \
	ymm0RGBA_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymm0RGBA_, ymmYCoeffs_), _mm256_maddubs_epi16(ymm1RGBA_, ymmYCoeffs_)); /* hadd(ABCD) -> ACBD */ \
	ymm2RGBA_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymm2RGBA_, ymmYCoeffs_), _mm256_maddubs_epi16(ymm3RGBA_, ymmYCoeffs_)); /* hadd(EFGH) -> EGFH */ \
	ymm0RGBA_ = _mm256_srai_epi16(ymm0RGBA_, 7); /* >> 7 */ \
	ymm2RGBA_ = _mm256_srai_epi16(ymm2RGBA_, 7); /* >> 7 */ \
	ymm0RGBA_ = _mm256_add_epi16(ymm0RGBA_, ymm16_); /* + 16 */ \
	ymm2RGBA_ = _mm256_add_epi16(ymm2RGBA_, ymm16_); /* + 16 */ \
	ymm0RGBA_ = _mm256_packus_epi16(ymm0RGBA_, ymm2RGBA_); /* Saturate(I16 -> U8): packus(ACBD, EGFH) -> AEBFCGDH */ \
	ymm0RGBA_ = _mm256_permutevar8x32_epi32(ymm0RGBA_, ymmAEBFCGDH_); /* Final Permute */ \
	_mm256_store_si256(reinterpret_cast<__m256i*>(outYPtr_), ymm0RGBA_);

/*
Convert 32 RGBA samples to 32 chroma (U or V samples) samples. Chroma subsampled x1
*/
#define COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymm0RGBA_, ymm1RGBA_, ymm2RGBA_, ymm3RGBA_, ymm0C_, ymm1C_, ymmCCoeffs_, ymm128_, ymmAEBFCGDH_,outCPtr_) \
	ymm0C_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymm0RGBA_, ymmCCoeffs_), _mm256_maddubs_epi16(ymm1RGBA_, ymmCCoeffs_)); \
	ymm1C_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymm2RGBA_, ymmCCoeffs_), _mm256_maddubs_epi16(ymm3RGBA_, ymmCCoeffs_)); \
	ymm0C_ = _mm256_srai_epi16(ymm0C_, 8); /* >> 8 */ \
	ymm1C_ = _mm256_srai_epi16(ymm1C_, 8); /* >> 8 */ \
	ymm0C_ = _mm256_add_epi16(ymm0C_, ymm128_); /* + 128 */ \
	ymm1C_ = _mm256_add_epi16(ymm1C_, ymm128_); /* + 128 */ \
	ymm0C_ = _mm256_packus_epi16(ymm0C_, ymm1C_); /* packus(ACBD, EGFH) -> AEBFCGDH */ \
	ymm0C_ = _mm256_permutevar8x32_epi32(ymm0C_, ymmAEBFCGDH_); /* Final Permute */ \
	_mm256_store_si256(reinterpret_cast<__m256i*>(outCPtr_), ymm0C_);

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVImageConvRgb24family_to_y_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgb24Ptr, COMPV_ALIGNED(AVX) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	__m256i ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymmYCoeffs, ymm16, ymmAEBFCGDH, ymmABCDDEFG, ymmMaskRgbToRgba;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGB = padY * 3;

	ymmMaskRgbToRgba = _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32));
	ymmYCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_YCoeffs8));
	ymm16 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k16_i16));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));
	ymmABCDDEFG = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32));

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			//  convert from RGB to RGBA, alpha channel contains zeros
			COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb24Ptr, ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymmABCDDEFG, ymmMaskRgbToRgba);
			// convert from RGBA to Luma(Y)
			COMPV_32xRGBA_TO_32xLUMA_AVX2(ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymmYCoeffs, ymm16, ymmAEBFCGDH, outYPtr);
			outYPtr += 32;
			rgb24Ptr += 96;
		}
		outYPtr += padY;
		rgb24Ptr += padRGB;
	}
	_mm256_zeroupper();
}

void CompVImageConvRgb32family_to_y_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgb32Ptr, COMPV_ALIGNED(AVX) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	__m256i ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymmYCoeffs, ymm16, ymmAEBFCGDH;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGBA = padY << 2;

	ymmYCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBAfamilyToYUV_YCoeffs8));
	ymm16 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k16_i16));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			ymm0RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 0));
			ymm1RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 32));
			ymm2RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 64));
			ymm3RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 96));
			COMPV_32xRGBA_TO_32xLUMA_AVX2(ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymmYCoeffs, ymm16, ymmAEBFCGDH, outYPtr);
			outYPtr += 32;
			rgb32Ptr += 128;
		}
		outYPtr += padY;
		rgb32Ptr += padRGBA;
	}
	_mm256_zeroupper();
}

void CompVImageConvRgb565family_to_y_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgb565lePtr, COMPV_ALIGNED(AVX) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	__m256i ymmR0, ymmG0, ymmB0, ymmR1, ymmG1, ymmB1, ymm0, ymm1;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGB = padY << 1;

	const __m256i ymm16 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k16_i16));
	const __m256i ymmMaskR = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGB565ToYUV_RMask_u16));
	const __m256i ymmMaskG = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGB565ToYUV_GMask_u16));
	const __m256i ymmMaskB = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGB565ToYUV_BMask_u16));
	const __m256i ymmCoeffR = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_YCoeffs8[0]));
	const __m256i ymmCoeffG = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_YCoeffs8[1]));
	const __m256i ymmCoeffB = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_YCoeffs8[2]));

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			ymm0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb565lePtr + 0));
			ymm1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb565lePtr + 32));
			ymmR0 = _mm256_srli_epi16(_mm256_and_si256(ymm0, ymmMaskR), 8);
			ymmR1 = _mm256_srli_epi16(_mm256_and_si256(ymm1, ymmMaskR), 8);
			ymmG0 = _mm256_srli_epi16(_mm256_and_si256(ymm0, ymmMaskG), 3);
			ymmG1 = _mm256_srli_epi16(_mm256_and_si256(ymm1, ymmMaskG), 3);
			ymmB0 = _mm256_slli_epi16(_mm256_and_si256(ymm0, ymmMaskB), 3);
			ymmB1 = _mm256_slli_epi16(_mm256_and_si256(ymm1, ymmMaskB), 3);
			ymmR0 = _mm256_or_si256(ymmR0, _mm256_srli_epi16(ymmR0, 5));
			ymmR1 = _mm256_or_si256(ymmR1, _mm256_srli_epi16(ymmR1, 5));
			ymmG0 = _mm256_or_si256(ymmG0, _mm256_srli_epi16(ymmG0, 6));
			ymmG1 = _mm256_or_si256(ymmG1, _mm256_srli_epi16(ymmG1, 6));
			ymmB0 = _mm256_or_si256(ymmB0, _mm256_srli_epi16(ymmB0, 5));
			ymmB1 = _mm256_or_si256(ymmB1, _mm256_srli_epi16(ymmB1, 5));
			ymmR0 = _mm256_mullo_epi16(ymmR0, ymmCoeffR);
			ymmR1 = _mm256_mullo_epi16(ymmR1, ymmCoeffR);
			ymmG0 = _mm256_mullo_epi16(ymmG0, ymmCoeffG);
			ymmG1 = _mm256_mullo_epi16(ymmG1, ymmCoeffG);
			ymmB0 = _mm256_mullo_epi16(ymmB0, ymmCoeffB);
			ymmB1 = _mm256_mullo_epi16(ymmB1, ymmCoeffB);
			ymm0 = _mm256_add_epi16(_mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ymmR0, ymmG0), ymmB0), 7), ymm16);
			ymm1 = _mm256_add_epi16(_mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ymmR1, ymmG1), ymmB1), 7), ymm16);

			_mm256_store_si256(reinterpret_cast<__m256i*>(outYPtr), compv_avx2_packus_epi16(ymm0, ymm1));
			outYPtr += 32;
			rgb565lePtr += 64;
		}
		outYPtr += padY;
		rgb565lePtr += padRGB;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVImageConvRgb24family_to_uv_planar_11_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgb24Ptr, COMPV_ALIGNED(AVX) uint8_t* outUPtr, COMPV_ALIGNED(AVX) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();

	_mm256_zeroupper();
	__m256i ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymm0C, ymm1C, ymmUCoeffs, ymmVCoeffs, ymm128, ymmAEBFCGDH, ymmABCDDEFG, ymmMaskRgbToRgba;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI), padRGB = padUV * 3;

	ymmUCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_UCoeffs8));
	ymmVCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_VCoeffs8));
	ymm128 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k128_i16));
	ymmMaskRgbToRgba = _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));
	ymmABCDDEFG = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			//  convert from RGB to RGBA, alpha channel contains zeros
			COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb24Ptr, ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymmABCDDEFG, ymmMaskRgbToRgba);
			// Convert from RGBA to Chroma (U and V)
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymm0C, ymm1C, ymmUCoeffs, ymm128, ymmAEBFCGDH, outUPtr);
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymm0C, ymm1C, ymmVCoeffs, ymm128, ymmAEBFCGDH, outVPtr);
			outUPtr += 32;
			outVPtr += 32;
			rgb24Ptr += 96;
		}
		rgb24Ptr += padRGB;
		outUPtr += padUV;
		outVPtr += padUV;
	}
	_mm256_zeroupper();
}

void CompVImageConvRgb32family_to_uv_planar_11_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgb32Ptr, COMPV_ALIGNED(AVX) uint8_t* outUPtr, COMPV_ALIGNED(AVX) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	__m256i ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymm0C, ymm1C, ymmUCoeffs, ymmVCoeffs, ymm128, ymmAEBFCGDH;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI), padRGBA = padUV << 2;

	ymmUCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBAfamilyToYUV_UCoeffs8));
	ymmVCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBAfamilyToYUV_VCoeffs8));
	ymm128 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k128_i16));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			ymm0RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 0));
			ymm1RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 32));
			ymm2RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 64));
			ymm3RGBA = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 96));
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymm0C, ymm1C, ymmUCoeffs, ymm128, ymmAEBFCGDH, outUPtr);
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymm0RGBA, ymm1RGBA, ymm2RGBA, ymm3RGBA, ymm0C, ymm1C, ymmVCoeffs, ymm128, ymmAEBFCGDH, outVPtr);
			outUPtr += 32;
			outVPtr += 32;
			rgb32Ptr += 128;
		}
		rgb32Ptr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
	_mm256_zeroupper();
}

void CompVImageConvRgb565family_to_uv_planar_11_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgblePtr, COMPV_ALIGNED(AVX) uint8_t* outUPtr, COMPV_ALIGNED(AVX) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	__m256i ymmR0, ymmG0, ymmB0, ymmR1, ymmG1, ymmB1, ymm0, ymm1, ymm2, ymm3, ymm4, ymm5;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI), padRGB = padUV << 1;

	const __m256i ymm128 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k128_i16));
	const __m256i ymmMaskR = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGB565ToYUV_RMask_u16));
	const __m256i ymmMaskG = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGB565ToYUV_GMask_u16));
	const __m256i ymmMaskB = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGB565ToYUV_BMask_u16));
	const __m256i ymmCoeffRU = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_UCoeffs8[0]));
	const __m256i ymmCoeffGU = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_UCoeffs8[1]));
	const __m256i ymmCoeffBU = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_UCoeffs8[2]));
	const __m256i ymmCoeffRV = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_VCoeffs8[0]));
	const __m256i ymmCoeffGV = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_VCoeffs8[1]));
	const __m256i ymmCoeffBV = _mm256_set1_epi16(static_cast<int16_t>(kRGBAfamilyToYUV_VCoeffs8[2]));

	/* U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128 */
	/* V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128 */
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			ymm0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgblePtr + 0));
			ymm1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgblePtr + 32));
			ymmR0 = _mm256_srli_epi16(_mm256_and_si256(ymm0, ymmMaskR), 8);
			ymmR1 = _mm256_srli_epi16(_mm256_and_si256(ymm1, ymmMaskR), 8);
			ymmG0 = _mm256_srli_epi16(_mm256_and_si256(ymm0, ymmMaskG), 3);
			ymmG1 = _mm256_srli_epi16(_mm256_and_si256(ymm1, ymmMaskG), 3);
			ymmB0 = _mm256_slli_epi16(_mm256_and_si256(ymm0, ymmMaskB), 3);
			ymmB1 = _mm256_slli_epi16(_mm256_and_si256(ymm1, ymmMaskB), 3);
			ymmR0 = _mm256_or_si256(ymmR0, _mm256_srli_epi16(ymmR0, 5));
			ymmR1 = _mm256_or_si256(ymmR1, _mm256_srli_epi16(ymmR1, 5));
			ymmG0 = _mm256_or_si256(ymmG0, _mm256_srli_epi16(ymmG0, 6));
			ymmG1 = _mm256_or_si256(ymmG1, _mm256_srli_epi16(ymmG1, 6));
			ymmB0 = _mm256_or_si256(ymmB0, _mm256_srli_epi16(ymmB0, 5));
			ymmB1 = _mm256_or_si256(ymmB1, _mm256_srli_epi16(ymmB1, 5));
			ymm0 = _mm256_mullo_epi16(ymmR0, ymmCoeffRU);
			ymm1 = _mm256_mullo_epi16(ymmR1, ymmCoeffRU);
			ymm2 = _mm256_mullo_epi16(ymmG0, ymmCoeffGU);
			ymm3 = _mm256_mullo_epi16(ymmG1, ymmCoeffGU);
			ymm4 = _mm256_mullo_epi16(ymmB0, ymmCoeffBU);
			ymm5 = _mm256_mullo_epi16(ymmB1, ymmCoeffBU);
			ymmR0 = _mm256_mullo_epi16(ymmR0, ymmCoeffRV);
			ymmR1 = _mm256_mullo_epi16(ymmR1, ymmCoeffRV);
			ymmG0 = _mm256_mullo_epi16(ymmG0, ymmCoeffGV);
			ymmG1 = _mm256_mullo_epi16(ymmG1, ymmCoeffGV);
			ymmB0 = _mm256_mullo_epi16(ymmB0, ymmCoeffBV);
			ymmB1 = _mm256_mullo_epi16(ymmB1, ymmCoeffBV);

			ymm0 = _mm256_add_epi16(_mm256_srai_epi16(_mm256_add_epi16(_mm256_add_epi16(ymm0, ymm2), ymm4), 8), ymm128);
			ymm1 = _mm256_add_epi16(_mm256_srai_epi16(_mm256_add_epi16(_mm256_add_epi16(ymm1, ymm3), ymm5), 8), ymm128);
			ymmR0 = _mm256_add_epi16(_mm256_srai_epi16(_mm256_add_epi16(_mm256_add_epi16(ymmR0, ymmG0), ymmB0), 8), ymm128);
			ymmR1 = _mm256_add_epi16(_mm256_srai_epi16(_mm256_add_epi16(_mm256_add_epi16(ymmR1, ymmG1), ymmB1), 8), ymm128);

			_mm256_store_si256(reinterpret_cast<__m256i*>(outUPtr), compv_avx2_packus_epi16(ymm0, ymm1));
			_mm256_store_si256(reinterpret_cast<__m256i*>(outVPtr), compv_avx2_packus_epi16(ymmR0, ymmR1));
			outUPtr += 32;
			outVPtr += 32;
			rgblePtr += 64;
		}
		outUPtr += padUV;
		outVPtr += padUV;
		rgblePtr += padRGB;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
