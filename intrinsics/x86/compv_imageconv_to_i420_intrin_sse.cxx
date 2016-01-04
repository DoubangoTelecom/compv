/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/intrinsics/x86/compv_imageconv_to_i420_intrin_sse.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
{
	__m128i xmmRgba;
	__m128i xmmYCoeffs = _mm_load_si128((__m128i*)kRGBAToYUV_YCoeffs8);
	__m128i y16 = _mm_load_si128((__m128i*)k16_i16);
	size_t padRGBA = (stride - width) << 2;
	size_t padY = (stride - width);

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; i += 4) {
			_mm_store_si128(&xmmRgba, _mm_load_si128((__m128i*)rgbaPtr)); // 4 RGBA samples = 16bytes
			_mm_store_si128(&xmmRgba, _mm_maddubs_epi16(xmmRgba, xmmYCoeffs)); // 
			_mm_store_si128(&xmmRgba, _mm_hadd_epi16(xmmRgba, xmmRgba));
			_mm_store_si128(&xmmRgba, _mm_srai_epi16(xmmRgba, 7)); // >> 7
			_mm_store_si128(&xmmRgba, _mm_add_epi16(xmmRgba, y16)); // + 16
			_mm_store_si128(&xmmRgba, _mm_packus_epi16(xmmRgba, xmmRgba)); // Saturate(I16 -> U8)
			*((int32_t*)outYPtr) = _mm_cvtsi128_si32(xmmRgba);

			outYPtr += 4;
			rgbaPtr += 16;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
}

void rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t height, size_t width, size_t stride)
{
	__m128i xmmRgba, xmm0, xmm1;
	__m128i xmmUV2Coeffs = _mm_load_si128((__m128i*)kRGBAToYUV_U2V2Coeffs8); // UV coeffs interleaved: each appear #2 times
	__m128i y128 = _mm_load_si128((__m128i*)k128_i16);
	size_t padRGBA = ((stride - width) << 2) + (stride << 2); // "+ (stride << 2)" -> because one line out of two
	size_t padUV = (stride - width) >> 1;
	int32_t UUVV;

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (size_t j = 0; j < height; j += 2) {
		for (size_t i = 0; i < width; i += 4) {
			_mm_store_si128(&xmmRgba, _mm_load_si128((__m128i*)rgbaPtr)); // 4 RGBA samples = 16bytes (2 are useless, we want 1 out of 2): axbx
			_mm_store_si128(&xmm0, _mm_unpacklo_epi32(xmmRgba, xmmRgba)); // aaxx
			_mm_store_si128(&xmm1, _mm_unpackhi_epi32(xmmRgba, xmmRgba)); // bbxx
			_mm_store_si128(&xmmRgba, _mm_unpacklo_epi32(xmm0, xmm1)); // abab
			_mm_store_si128(&xmmRgba, _mm_maddubs_epi16(xmmRgba, xmmUV2Coeffs)); // Ua Ub Va Vb
			_mm_store_si128(&xmmRgba, _mm_hadd_epi16(xmmRgba, xmmRgba));
			_mm_store_si128(&xmmRgba, _mm_srai_epi16(xmmRgba, 8)); // >> 8
			_mm_store_si128(&xmmRgba, _mm_add_epi16(xmmRgba, y128)); // + 128 -> UUVV----
			_mm_store_si128(&xmmRgba, _mm_packus_epi16(xmmRgba, xmmRgba)); // Saturate(I16 -> U8)
			UUVV = _mm_cvtsi128_si32(xmmRgba);
			*outUPtr++ = (UUVV & 0xFF);
			*outUPtr++ = (UUVV >> 8) & 0xFF;
			*outVPtr++ = (UUVV >> 16) & 0xFF;
			*outVPtr++ = (UUVV >> 24);		
			
			rgbaPtr += 16; // 4 * 4
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

void rgbaToI420Kernel11_CompY_Intrin_Unaligned_SSSE3(const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
{
	__m128i xmmRgba;
	__m128i xmmYCoeffs = _mm_load_si128((__m128i*)kRGBAToYUV_YCoeffs8);
	__m128i y16 = _mm_load_si128((__m128i*)k16_i16);
	size_t padRGBA = (stride - width) << 2;
	size_t padY = (stride - width);

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; i += 4) {
			_mm_store_si128(&xmmRgba, _mm_loadu_si128((__m128i*)rgbaPtr)); // 4 RGBA samples
			_mm_store_si128(&xmmRgba, _mm_maddubs_epi16(xmmRgba, xmmYCoeffs)); // 
			_mm_store_si128(&xmmRgba, _mm_hadd_epi16(xmmRgba, xmmRgba));
			_mm_store_si128(&xmmRgba, _mm_srai_epi16(xmmRgba, 7)); // >> 7
			_mm_store_si128(&xmmRgba, _mm_add_epi16(xmmRgba, y16)); // + 16
			_mm_store_si128(&xmmRgba, _mm_packus_epi16(xmmRgba, xmmRgba)); // Saturate(I16 -> U8)
			*((int32_t*)outYPtr) = _mm_cvtsi128_si32(xmmRgba);

			outYPtr += 4 + padY;
			rgbaPtr += 16 + padRGBA;
		}
	}
}


COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */