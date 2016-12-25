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

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVImageConvRgb24family_to_y_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgbPtr, COMPV_ALIGNED(AVX) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
	__m256i ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmYCoeffs, ymm16, ymmAEBFCGDH, ymmABCDDEFG, ymmCDEFFGHX, ymmMaskRgbToRgba, ymmXXABBCDE;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGB = padY * 3;

	_mm256_store_si256(&ymmMaskRgbToRgba, _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32)));
	_mm256_store_si256(&ymmYCoeffs, _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_YCoeffs8)));
	_mm256_store_si256(&ymm16, _mm256_load_si256(reinterpret_cast<const __m256i*>(k16_i16)));
	_mm256_store_si256(&ymmAEBFCGDH, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32)));
	_mm256_store_si256(&ymmABCDDEFG, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32)));
	_mm256_store_si256(&ymmCDEFFGHX, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_CDEFFGHX_i32)));
	_mm256_store_si256(&ymmXXABBCDE, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_XXABBCDE_i32)));

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			/**  convert from RGB to RGBA, alpha channel contains garbage (later multiplied with zero coeff) **/
			COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgbPtr, ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmABCDDEFG, ymmCDEFFGHX, ymmXXABBCDE, ymmMaskRgbToRgba);
			
			// starting here we're using the same code as rgba -> y

			_mm256_store_si256(&ymmRGBA0, _mm256_maddubs_epi16(ymmRGBA0, ymmYCoeffs));
			_mm256_store_si256(&ymmRGBA1, _mm256_maddubs_epi16(ymmRGBA1, ymmYCoeffs));
			_mm256_store_si256(&ymmRGBA2, _mm256_maddubs_epi16(ymmRGBA2, ymmYCoeffs));
			_mm256_store_si256(&ymmRGBA3, _mm256_maddubs_epi16(ymmRGBA3, ymmYCoeffs));

			_mm256_store_si256(&ymmRGBA0, _mm256_hadd_epi16(ymmRGBA0, ymmRGBA1)); // hadd(ABCD) -> ACBD
			_mm256_store_si256(&ymmRGBA2, _mm256_hadd_epi16(ymmRGBA2, ymmRGBA3)); // hadd(EFGH) -> EGFH

			_mm256_store_si256(&ymmRGBA0, _mm256_srai_epi16(ymmRGBA0, 7)); // >> 7
			_mm256_store_si256(&ymmRGBA2, _mm256_srai_epi16(ymmRGBA2, 7)); // >> 7

			_mm256_store_si256(&ymmRGBA0, _mm256_add_epi16(ymmRGBA0, ymm16)); // + 16
			_mm256_store_si256(&ymmRGBA2, _mm256_add_epi16(ymmRGBA2, ymm16)); // + 16

			// Saturate(I16 -> U8)
			_mm256_store_si256(&ymmRGBA0, _mm256_packus_epi16(ymmRGBA0, ymmRGBA2)); // packus(ACBD, EGFH) -> AEBFCGDH

			// Final Permute
			_mm256_store_si256(&ymmRGBA0, _mm256_permutevar8x32_epi32(ymmRGBA0, ymmAEBFCGDH));

			_mm256_store_si256(reinterpret_cast<__m256i*>(outYPtr), ymmRGBA0);

			outYPtr += 32;
			rgbPtr += 96;
		}
		outYPtr += padY;
		rgbPtr += padRGB;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVImageConvRgb24family_to_uv_planar_11_Intrin_AVX2(COMPV_ALIGNED(AVX) const uint8_t* rgbPtr, COMPV_ALIGNED(AVX) uint8_t* outUPtr, COMPV_ALIGNED(AVX) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();

	_mm256_zeroupper();
	__m256i ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmU0, ymmU1, ymmU2, ymmU3, ymmUCoeffs, ymmVCoeffs, ymm128, ymmAEBFCGDH, ymmABCDDEFG, ymmCDEFFGHX, ymmMaskRgbToRgba, ymmXXABBCDE;
	compv_uscalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI), padRGB = padUV * 3;

	_mm256_store_si256(&ymmUCoeffs, _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_UCoeffs8)));
	_mm256_store_si256(&ymmVCoeffs, _mm256_load_si256(reinterpret_cast<const __m256i*>(kRGBfamilyToYUV_VCoeffs8)));
	_mm256_store_si256(&ymm128, _mm256_load_si256(reinterpret_cast<const __m256i*>(k128_i16)));
	_mm256_store_si256(&ymmMaskRgbToRgba, _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32)));
	_mm256_store_si256(&ymmAEBFCGDH, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_AEBFCGDH_i32)));
	_mm256_store_si256(&ymmABCDDEFG, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32)));
	_mm256_store_si256(&ymmCDEFFGHX, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_CDEFFGHX_i32)));
	_mm256_store_si256(&ymmXXABBCDE, _mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_XXABBCDE_i32)));

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			/**  convert from RGB to RGBA, alpha channel contains garbage (later multiplied with zero coeff) **/
			COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgbPtr, ymmRGBA0, ymmRGBA1, ymmRGBA2, ymmRGBA3, ymmABCDDEFG, ymmCDEFFGHX, ymmXXABBCDE, ymmMaskRgbToRgba);

			// starting here we're using the same code as rgba -> uv

			_mm256_store_si256(&ymmU0, _mm256_maddubs_epi16(ymmRGBA0, ymmUCoeffs));
			_mm256_store_si256(&ymmU1, _mm256_maddubs_epi16(ymmRGBA1, ymmUCoeffs));
			_mm256_store_si256(&ymmU2, _mm256_maddubs_epi16(ymmRGBA2, ymmUCoeffs));
			_mm256_store_si256(&ymmU3, _mm256_maddubs_epi16(ymmRGBA3, ymmUCoeffs));
			_mm256_store_si256(&ymmRGBA0, _mm256_maddubs_epi16(ymmRGBA0, ymmVCoeffs));
			_mm256_store_si256(&ymmRGBA1, _mm256_maddubs_epi16(ymmRGBA1, ymmVCoeffs));
			_mm256_store_si256(&ymmRGBA2, _mm256_maddubs_epi16(ymmRGBA2, ymmVCoeffs));
			_mm256_store_si256(&ymmRGBA3, _mm256_maddubs_epi16(ymmRGBA3, ymmVCoeffs));

			_mm256_store_si256(&ymmU0, _mm256_hadd_epi16(ymmU0, ymmU1));
			_mm256_store_si256(&ymmU2, _mm256_hadd_epi16(ymmU2, ymmU3));
			_mm256_store_si256(&ymmRGBA0, _mm256_hadd_epi16(ymmRGBA0, ymmRGBA1));
			_mm256_store_si256(&ymmRGBA2, _mm256_hadd_epi16(ymmRGBA2, ymmRGBA3));

			_mm256_store_si256(&ymmU0, _mm256_srai_epi16(ymmU0, 8)); // >> 8
			_mm256_store_si256(&ymmU2, _mm256_srai_epi16(ymmU2, 8)); // >> 8
			_mm256_store_si256(&ymmRGBA0, _mm256_srai_epi16(ymmRGBA0, 8)); // >> 8
			_mm256_store_si256(&ymmRGBA2, _mm256_srai_epi16(ymmRGBA2, 8)); // >> 8

			_mm256_store_si256(&ymmU0, _mm256_add_epi16(ymmU0, ymm128)); // + 128
			_mm256_store_si256(&ymmU2, _mm256_add_epi16(ymmU2, ymm128)); // + 128
			_mm256_store_si256(&ymmRGBA0, _mm256_add_epi16(ymmRGBA0, ymm128)); // + 128
			_mm256_store_si256(&ymmRGBA2, _mm256_add_epi16(ymmRGBA2, ymm128)); // + 128

			// packus(ACBD, EGFH) -> AEBFCGDH
			_mm256_store_si256(&ymmU0, _mm256_packus_epi16(ymmU0, ymmU2));
			_mm256_store_si256(&ymmRGBA0, _mm256_packus_epi16(ymmRGBA0, ymmRGBA2)); 

			// Final Permute
			_mm256_store_si256(&ymmU0, _mm256_permutevar8x32_epi32(ymmU0, ymmAEBFCGDH));
			_mm256_store_si256(&ymmRGBA0, _mm256_permutevar8x32_epi32(ymmRGBA0, ymmAEBFCGDH));

			_mm256_store_si256(reinterpret_cast<__m256i*>(outUPtr), ymmU0);
			_mm256_store_si256(reinterpret_cast<__m256i*>(outVPtr), ymmRGBA0);

			outUPtr += 32;
			outVPtr += 32;
			rgbPtr += 96;
		}
		rgbPtr += padRGB;
		outUPtr += padUV;
		outVPtr += padUV;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
