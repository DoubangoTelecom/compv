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
#include "compv/intrinsics/x86/compv_imageconv_rgba_i420_intrin_sse.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	__m128i xmmRgba;
	__m128i xmmYCoeffs = _mm_load_si128((__m128i*)kRGBAToYUV_YCoeffs8);
	__m128i y16 = _mm_load_si128((__m128i*)k16_i16);
	vcomp_scalar_t i, j, maxI = ((width + 3) & -4), padY = (stride - maxI), padRGBA = padY << 2;

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 4) {
			_mm_store_si128(&xmmRgba, _mm_load_si128((__m128i*)rgbaPtr)); // 4 RGBA samples = 16bytes
			_mm_store_si128(&xmmRgba, _mm_maddubs_epi16(xmmRgba, xmmYCoeffs)); // 
			_mm_store_si128(&xmmRgba, _mm_hadd_epi16(xmmRgba, xmmRgba));
			_mm_store_si128(&xmmRgba, _mm_srai_epi16(xmmRgba, 7)); // >> 7
			_mm_store_si128(&xmmRgba, _mm_add_epi16(xmmRgba, y16)); // + 16
			_mm_store_si128(&xmmRgba, _mm_packus_epi16(xmmRgba, xmmRgba)); // Saturate(I16 -> U8)
			*((int*)outYPtr) = _mm_cvtsi128_si32(xmmRgba);

			outYPtr += 4;
			rgbaPtr += 16;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
}

void rgbaToI420Kernel41_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	__m128i xmmRgba0, xmmRgba1, xmmRgba2, xmmRgba3;
	__m128i xmmYCoeffs = _mm_load_si128((__m128i*)kRGBAToYUV_YCoeffs8);
	__m128i y16 = _mm_load_si128((__m128i*)k16_i16);
	vcomp_scalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGBA = padY << 2;

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			// load 16 RGBA samples
			_mm_store_si128(&xmmRgba0, _mm_load_si128((__m128i*)rgbaPtr)); // 4 RGBA samples = 16bytes
			_mm_store_si128(&xmmRgba1, _mm_load_si128((__m128i*)(rgbaPtr + 16))); // 4 RGBA samples = 16bytes
			_mm_store_si128(&xmmRgba2, _mm_load_si128((__m128i*)(rgbaPtr + 32))); // 4 RGBA samples = 16bytes
			_mm_store_si128(&xmmRgba3, _mm_load_si128((__m128i*)(rgbaPtr + 48))); // 4 RGBA samples = 16bytes

			_mm_store_si128(&xmmRgba0, _mm_maddubs_epi16(xmmRgba0, xmmYCoeffs));
			_mm_store_si128(&xmmRgba1, _mm_maddubs_epi16(xmmRgba1, xmmYCoeffs));
			_mm_store_si128(&xmmRgba2, _mm_maddubs_epi16(xmmRgba2, xmmYCoeffs));
			_mm_store_si128(&xmmRgba3, _mm_maddubs_epi16(xmmRgba3, xmmYCoeffs));

			_mm_store_si128(&xmmRgba0, _mm_hadd_epi16(xmmRgba0, xmmRgba1));
			_mm_store_si128(&xmmRgba2, _mm_hadd_epi16(xmmRgba2, xmmRgba3));

			_mm_store_si128(&xmmRgba0, _mm_srai_epi16(xmmRgba0, 7)); // >> 7
			_mm_store_si128(&xmmRgba2, _mm_srai_epi16(xmmRgba2, 7)); // >> 7

			_mm_store_si128(&xmmRgba0, _mm_add_epi16(xmmRgba0, y16)); // + 16
			_mm_store_si128(&xmmRgba2, _mm_add_epi16(xmmRgba2, y16)); // + 16

			_mm_store_si128(&xmmRgba0, _mm_packus_epi16(xmmRgba0, xmmRgba2)); // Saturate(I16 -> U8)

			_mm_storeu_si128((__m128i*)outYPtr, xmmRgba0);

			outYPtr += 16;
			rgbaPtr += 64;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
}

void rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	__m128i xmmRgba, xmm0, xmm1;
	__m128i xmmUV2Coeffs = _mm_load_si128((__m128i*)kRGBAToYUV_U2V2Coeffs8); // UV coeffs interleaved: each appear #2 times
	__m128i y128 = _mm_load_si128((__m128i*)k128_i16);
	vcomp_scalar_t i, j, maxI = ((width + 3) & -4), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines
	int32_t UUVV;

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 4) {
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

void rgbaToI420Kernel41_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	__m128i xmmRgba0, xmmRgba1, xmmRgba2, xmmRgba3, xmm0, xmm1;
	__m128i xmmUCoeffs = _mm_load_si128((__m128i*)kRGBAToYUV_UCoeffs8);
	__m128i xmmVCoeffs = _mm_load_si128((__m128i*)kRGBAToYUV_VCoeffs8);
	__m128i xmm128 = _mm_load_si128((__m128i*)k128_i16);
	vcomp_scalar_t i, j, maxI = ((width + 15) & -16), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 16) {
			// load 16 RGBA samples
			_mm_store_si128(&xmmRgba0, _mm_load_si128((__m128i*)rgbaPtr)); // 4 RGBA samples = 16bytes (2 are useless, we want 1 out of 2): 0x1x
			_mm_store_si128(&xmmRgba1, _mm_load_si128((__m128i*)(rgbaPtr + 16))); // 4 RGBA samples = 16bytes :2x3x
			_mm_store_si128(&xmmRgba2, _mm_load_si128((__m128i*)(rgbaPtr + 32))); // 4 RGBA samples = 16bytes : 4x5x
			_mm_store_si128(&xmmRgba3, _mm_load_si128((__m128i*)(rgbaPtr + 48))); // 4 RGBA samples = 16bytes : 6x7x

			_mm_store_si128(&xmm0, _mm_unpacklo_epi32(xmmRgba0, xmmRgba1)); // 02xx
			_mm_store_si128(&xmm1, _mm_unpackhi_epi32(xmmRgba0, xmmRgba1)); // 13xx
			_mm_store_si128(&xmmRgba0, _mm_unpacklo_epi32(xmm0, xmm1)); // 0123
			_mm_store_si128(&xmmRgba1, xmmRgba0);

			_mm_store_si128(&xmm0, _mm_unpacklo_epi32(xmmRgba2, xmmRgba3)); // 46xx
			_mm_store_si128(&xmm1, _mm_unpackhi_epi32(xmmRgba2, xmmRgba3)); // 57xx
			_mm_store_si128(&xmmRgba2, _mm_unpacklo_epi32(xmm0, xmm1)); // 4567
			_mm_store_si128(&xmmRgba3, xmmRgba2);

			// U = (xmmRgba0, xmmRgba2)
			// V = (xmmRgba1, xmmRgba3)
			_mm_store_si128(&xmmRgba0, _mm_maddubs_epi16(xmmRgba0, xmmUCoeffs));
			_mm_store_si128(&xmmRgba2, _mm_maddubs_epi16(xmmRgba2, xmmUCoeffs));
			_mm_store_si128(&xmmRgba1, _mm_maddubs_epi16(xmmRgba1, xmmVCoeffs));
			_mm_store_si128(&xmmRgba3, _mm_maddubs_epi16(xmmRgba3, xmmVCoeffs));

			// U = xmmRgba0
			// V = xmmRgba1
			_mm_store_si128(&xmmRgba0, _mm_hadd_epi16(xmmRgba0, xmmRgba2));
			_mm_store_si128(&xmmRgba1, _mm_hadd_epi16(xmmRgba1, xmmRgba3));

			_mm_store_si128(&xmmRgba0, _mm_srai_epi16(xmmRgba0, 8)); // >> 8
			_mm_store_si128(&xmmRgba1, _mm_srai_epi16(xmmRgba1, 8)); // >> 8

			_mm_store_si128(&xmmRgba0, _mm_add_epi16(xmmRgba0, xmm128)); // + 128 -> UUVV----
			_mm_store_si128(&xmmRgba1, _mm_add_epi16(xmmRgba1, xmm128)); // + 128 -> UUVV----

			// UV = xmmRgba0
			_mm_store_si128(&xmmRgba0, _mm_packus_epi16(xmmRgba0, xmmRgba1)); // Saturate(I16 -> U8)

#if defined(COMPV_ARCH_X64)
			*((uint64_t*)outUPtr) = _mm_cvtsi128_si64(xmmRgba0);
			_mm_store_si128(&xmmRgba0, _mm_srli_si128(xmmRgba0, 8)); // >> 8
			*((uint64_t*)outVPtr) = _mm_cvtsi128_si64(xmmRgba0);
#else
			*((uint64_t*)outUPtr) = ((uint64_t*)&xmmRgba0)[0];
			*((uint64_t*)outVPtr) = ((uint64_t*)&xmmRgba0)[1];
#endif

			outUPtr += 8;
			outVPtr += 8;
			rgbaPtr += 64;
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */