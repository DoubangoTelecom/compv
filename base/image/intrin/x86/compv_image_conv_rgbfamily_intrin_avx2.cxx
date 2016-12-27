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
The aplha channel will contain garbage instead of 0xff because this macro is used to fetch samples in place
*/
#define COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb32Ptr_, ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_, ymmABCDDEFG_, ymmCDEFFGHX_, ymmXXABBCDE_, ymmMaskRgbToRgba_) \
	ymmRGBA3_ = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr_ + 32)); \
	ymmRGBA1_ = _mm256_permute2x128_si256(ymmRGBA3_, ymmRGBA3_, 0x11); \
	ymmRGBA3_ = _mm256_permutevar8x32_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr_ + 64)), ymmCDEFFGHX_); \
	ymmRGBA1_ = _mm256_permute2x128_si256(ymmRGBA1_, _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr_ + 64)), 0x20); \
	ymmRGBA2_ = _mm256_permutevar8x32_epi32(ymmRGBA1_, ymmABCDDEFG_); \
	ymmRGBA2_ = _mm256_shuffle_epi8(ymmRGBA2_, ymmMaskRgbToRgba_); \
	ymmRGBA3_ = _mm256_shuffle_epi8(ymmRGBA3_, ymmMaskRgbToRgba_); \
	ymmRGBA0_ = _mm256_permute4x64_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr_ + 0)), 0xff); \
	ymmRGBA1_ = _mm256_permutevar8x32_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr_ + 32)), ymmXXABBCDE_); \
	ymmRGBA1_ = _mm256_blend_epi32(ymmRGBA1_, ymmRGBA0_, 0x03); \
	ymmRGBA1_ = _mm256_shuffle_epi8(ymmRGBA1_, ymmMaskRgbToRgba_); \
	ymmRGBA0_ = _mm256_permutevar8x32_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr_ + 0)), ymmABCDDEFG_); \
	ymmRGBA0_ = _mm256_shuffle_epi8(ymmRGBA0_, ymmMaskRgbToRgba_);
// Next version not optimized as we load the masks for each call, use above version and load masks once
#define COMPV_32xRGB_TO_32xRGBA_AVX2_SLOW(rgb32Ptr_, ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_) \
	COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb32Ptr_, ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_, \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32)), \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_CDEFFGHX_i32)), \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_XXABBCDE_i32)), \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32)) \
	)

/*
Convert 32 RGBA to 32 Luma (Y)
*/
#define COMPV_32xRGBA_TO_32xLUMA_AVX2(ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_, ymmYCoeffs_, ymm16_, ymmAEBFCGDH_, outYPtr_) \
	ymmRGBA0_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymmRGBA0_, ymmYCoeffs_), _mm256_maddubs_epi16(ymmRGBA1_, ymmYCoeffs_)); /* hadd(ABCD) -> ACBD */ \
	ymmRGBA2_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymmRGBA2_, ymmYCoeffs_), _mm256_maddubs_epi16(ymmRGBA3_, ymmYCoeffs_)); /* hadd(EFGH) -> EGFH */ \
	ymmRGBA0_ = _mm256_srai_epi16(ymmRGBA0_, 7); /* >> 7 */ \
	ymmRGBA2_ = _mm256_srai_epi16(ymmRGBA2_, 7); /* >> 7 */ \
	ymmRGBA0_ = _mm256_add_epi16(ymmRGBA0_, ymm16_); /* + 16 */ \
	ymmRGBA2_ = _mm256_add_epi16(ymmRGBA2_, ymm16_); /* + 16 */ \
	ymmRGBA0_ = _mm256_packus_epi16(ymmRGBA0_, ymmRGBA2_); /* Saturate(I16 -> U8): packus(ACBD, EGFH) -> AEBFCGDH */ \
	ymmRGBA0_ = _mm256_permutevar8x32_epi32(ymmRGBA0_, ymmAEBFCGDH_); /* Final Permute */ \
	_mm256_store_si256(reinterpret_cast<__m256i*>(outYPtr_), ymmRGBA0_);

/*
Convert 32 RGBA samples to 32 chroma (U or V samples) samples. Chroma subsampled x1
*/
#define COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_, ymm0C_, ymm1C_, ymmCCoeffs_, ymm128_, ymmAEBFCGDH_,outCPtr_) \
	ymm0C_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymmRGBA0_, ymmCCoeffs_), _mm256_maddubs_epi16(ymmRGBA1_, ymmCCoeffs_)); \
	ymm1C_ = _mm256_hadd_epi16(_mm256_maddubs_epi16(ymmRGBA2_, ymmCCoeffs_), _mm256_maddubs_epi16(ymmRGBA3_, ymmCCoeffs_)); \
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
	__m256i ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmYCoeffs, ymm16, ymmAEBFCGDH, ymmABCDDEFG, ymmCDEFFGHX, ymmMaskRgbToRgba, ymmXXABBCDE;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGB = padY * 3;

	ymmMaskRgbToRgba = _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32));
	ymmYCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_YCoeffs8));
	ymm16 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k16_i16));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));
	ymmABCDDEFG = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32));
	ymmCDEFFGHX = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_CDEFFGHX_i32));
	ymmXXABBCDE = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_XXABBCDE_i32));

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			//  convert from RGB to RGBA, alpha channel contains garbage (later multiplied with zero coeff)
			COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb24Ptr, ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmABCDDEFG, ymmCDEFFGHX, ymmXXABBCDE, ymmMaskRgbToRgba);
			// convert from RGBA to Luma(Y)
			COMPV_32xRGBA_TO_32xLUMA_AVX2(ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmYCoeffs, ymm16, ymmAEBFCGDH, outYPtr);
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
	__m256i ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmYCoeffs, ymm16, ymmAEBFCGDH;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGBA = padY << 2;

	ymmYCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBAfamilyToYUV_YCoeffs8));
	ymm16 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k16_i16));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			ymmRGBA0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 0));
			ymmRGBA1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 32));
			ymmRGBA2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 64));
			ymmRGBA3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 96));
			COMPV_32xRGBA_TO_32xLUMA_AVX2(ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmYCoeffs, ymm16, ymmAEBFCGDH, outYPtr);
			outYPtr += 32;
			rgb32Ptr += 128;
		}
		outYPtr += padY;
		rgb32Ptr += padRGBA;
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
	__m256i ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymm0C, ymm1C, ymmUCoeffs, ymmVCoeffs, ymm128, ymmAEBFCGDH, ymmABCDDEFG, ymmCDEFFGHX, ymmMaskRgbToRgba, ymmXXABBCDE;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI), padRGB = padUV * 3;

	ymmUCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_UCoeffs8));
	ymmVCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_VCoeffs8));
	ymm128 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k128_i16));
	ymmMaskRgbToRgba = _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));
	ymmABCDDEFG = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32));
	ymmCDEFFGHX = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_CDEFFGHX_i32));
	ymmXXABBCDE = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_XXABBCDE_i32));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			//  convert from RGB to RGBA, alpha channel contains garbage (later multiplied with zero coeff)
			COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgb24Ptr, ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmABCDDEFG, ymmCDEFFGHX, ymmXXABBCDE, ymmMaskRgbToRgba);
			// Convert from RGBA to Chroma (U and V)
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymm0C, ymm1C, ymmUCoeffs, ymm128, ymmAEBFCGDH, outUPtr);
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymm0C, ymm1C, ymmVCoeffs, ymm128, ymmAEBFCGDH, outVPtr);
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
	__m256i ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymm0C, ymm1C, ymmUCoeffs, ymmVCoeffs, ymm128, ymmAEBFCGDH;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI), padRGBA = padUV << 2;

	ymmUCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBAfamilyToYUV_UCoeffs8));
	ymmVCoeffs = _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBAfamilyToYUV_VCoeffs8));
	ymm128 = _mm256_load_si256(reinterpret_cast<const __m256i*>(k128_i16));
	ymmAEBFCGDH = _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			ymmRGBA0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 0));
			ymmRGBA1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 32));
			ymmRGBA2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 64));
			ymmRGBA3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(rgb32Ptr + 96));
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymm0C, ymm1C, ymmUCoeffs, ymm128, ymmAEBFCGDH, outUPtr);
			COMPV_32xRGBA_TO_32xCHROMA1_AVX2(ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymm0C, ymm1C, ymmVCoeffs, ymm128, ymmAEBFCGDH, outVPtr);
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

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
