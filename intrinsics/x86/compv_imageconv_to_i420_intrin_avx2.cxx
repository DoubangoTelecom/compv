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
#include "compv/intrinsics/x86/compv_imageconv_to_i420_intrin_avx2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba;
	__m256i ymmYCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_YCoeffs8);
	__m256i ymm16 = _mm256_load_si256((__m256i*)k16_i16);
	__m256i ymmMaskToExtractFirst64Bits = _mm256_load_si256((__m256i*)kMaskstore_0_i64);
	vcomp_scalar_t i, j, maxI = ((width + 7) & -8), padY = (stride - maxI), padRGBA = padY << 2;

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 8) {
			_mm256_store_si256(&ymmRgba, _mm256_load_si256((__m256i*)rgbaPtr)); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba, _mm256_maddubs_epi16(ymmRgba, ymmYCoeffs)); // 
			_mm256_store_si256(&ymmRgba, _mm256_hadd_epi16(ymmRgba, ymmRgba)); // aaaabbbbaaaabbbb
			_mm256_store_si256(&ymmRgba, _mm256_permute4x64_epi64(ymmRgba, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // aaaaaaaabbbbbbbb
			_mm256_store_si256(&ymmRgba, _mm256_srai_epi16(ymmRgba, 7)); // >> 7
			_mm256_store_si256(&ymmRgba, _mm256_add_epi16(ymmRgba, ymm16)); // + 16
			_mm256_store_si256(&ymmRgba, _mm256_packus_epi16(ymmRgba, ymmRgba)); // Saturate(I16 -> U8)
#if 1		// best way to use AVX code *only* and avoid AVX/SSE mixing penalities
			_mm256_maskstore_epi64((int64_t*)outYPtr, ymmMaskToExtractFirst64Bits, ymmRgba);
#else
			*((int64_t*)outYPtr) = _mm_cvtsi128_si64(_mm256_castsi256_si128(ymmRgba));
#endif

			outYPtr += 8;
			rgbaPtr += 32;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
	_mm256_zeroupper();
}

void rgbaToI420Kernel41_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba0, ymmRgba1, ymmRgba2, ymmRgba3;
	__m256i ymmYCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_YCoeffs8);
	__m256i ymm16 = _mm256_load_si256((__m256i*)k16_i16);
	vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGBA = padY << 2;

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 32) {
			_mm256_store_si256(&ymmRgba0, _mm256_load_si256((__m256i*)(rgbaPtr))); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba1, _mm256_load_si256((__m256i*)(rgbaPtr + 32))); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba2, _mm256_load_si256((__m256i*)(rgbaPtr + 64))); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba3, _mm256_load_si256((__m256i*)(rgbaPtr + 96))); // 8 RGBA samples
			
			_mm256_store_si256(&ymmRgba0, _mm256_maddubs_epi16(ymmRgba0, ymmYCoeffs));
			_mm256_store_si256(&ymmRgba1, _mm256_maddubs_epi16(ymmRgba1, ymmYCoeffs));
			_mm256_store_si256(&ymmRgba2, _mm256_maddubs_epi16(ymmRgba2, ymmYCoeffs));
			_mm256_store_si256(&ymmRgba3, _mm256_maddubs_epi16(ymmRgba3, ymmYCoeffs));

			_mm256_store_si256(&ymmRgba0, _mm256_hadd_epi16(ymmRgba0, ymmRgba1)); // 0000111100001111
			_mm256_store_si256(&ymmRgba2, _mm256_hadd_epi16(ymmRgba2, ymmRgba3)); // 2222333322223333

			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // 0000000011111111
			_mm256_store_si256(&ymmRgba2, _mm256_permute4x64_epi64(ymmRgba2, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // 2222222233333333
			
			_mm256_store_si256(&ymmRgba0, _mm256_srai_epi16(ymmRgba0, 7)); // >> 7
			_mm256_store_si256(&ymmRgba2, _mm256_srai_epi16(ymmRgba2, 7)); // >> 7

			_mm256_store_si256(&ymmRgba0, _mm256_add_epi16(ymmRgba0, ymm16)); // + 16
			_mm256_store_si256(&ymmRgba2, _mm256_add_epi16(ymmRgba2, ymm16)); // + 16

			_mm256_store_si256(&ymmRgba0, _mm256_packus_epi16(ymmRgba0, ymmRgba2)); // Saturate(I16 -> U8): 002200220022...
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0))); //000000022222.....
			
			_mm256_storeu_si256((__m256i*)outYPtr, ymmRgba0);

			outYPtr += 32;
			rgbaPtr += 128;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
	_mm256_zeroupper();
}

void rgbaToI420Kernel11_CompUV_Intrin_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba;
#if 0
	__m128i xmmUV;
#endif
	__m256i ymmUV4Coeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_U4V4Coeffs8); // UV coeffs interleaved: each appear #4 times
	__m256i ymm128 = _mm256_load_si256((__m256i*)k128_i16);
	__m256i ymmMaskToExtractFirst32Bits = _mm256_load_si256((__m256i*)kMaskstore_0_i32);
	vcomp_scalar_t i, j, maxI = ((width + 7) & -8), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 8) {
			_mm256_store_si256(&ymmRgba, _mm256_load_si256((__m256i*)rgbaPtr)); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			_mm256_store_si256(&ymmRgba, _mm256_maddubs_epi16(ymmRgba, ymmUV4Coeffs)); // Ua Ub Uc Ud Va Vb Vc Vd
			_mm256_store_si256(&ymmRgba, _mm256_hadd_epi16(ymmRgba, ymmRgba));
			_mm256_store_si256(&ymmRgba, _mm256_permute4x64_epi64(ymmRgba, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
			_mm256_store_si256(&ymmRgba, _mm256_srai_epi16(ymmRgba, 8)); // >> 8
			_mm256_store_si256(&ymmRgba, _mm256_add_epi16(ymmRgba, ymm128)); // + 128 -> UUVV----
			_mm256_store_si256(&ymmRgba, _mm256_packus_epi16(ymmRgba, ymmRgba)); // Saturate(I16 -> U8)
			
#if 1 // best way to use AVX code *only* and avoid AVX/SSE mixing penalities
			_mm256_maskstore_epi32((int*)outUPtr, ymmMaskToExtractFirst32Bits, ymmRgba);
			_mm256_store_si256(&ymmRgba, _mm256_srli_si256(ymmRgba, 4)); // >> 4
			_mm256_maskstore_epi32((int*)outVPtr, ymmMaskToExtractFirst32Bits, ymmRgba);
#else
			_mm_store_si128(&xmmUV, _mm256_castsi256_si128(ymmRgba)); // UV
			*((uint*)outUPtr) = _mm_cvtsi128_si32(xmmUV);
			_mm_store_si128(&xmmUV, _mm_srli_si128(xmmUV, 4)); // >> 4
			*((uint*)outVPtr) = _mm_cvtsi128_si32(xmmUV);
#endif

			outUPtr += 4;
			outVPtr += 4;
			rgbaPtr += 32; // 4 * 8
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
	_mm256_zeroupper();
}

void rgbaToI420Kernel41_CompUV_Intrin_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	_mm256_zeroupper();
	__m256i ymmRgba0, ymmRgba1, ymmRgba2, ymmRgba3, ymm0, ymm1;
	__m256i ymmUCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_UCoeffs8);
	__m256i ymmVCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_VCoeffs8);
	__m256i ymm128 = _mm256_load_si256((__m256i*)k128_i16);
	__m256i ymmMaskToExtract128bits = _mm256_load_si256((__m256i*)kMaskstore_0_1_i64);
	vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines
	
	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 32) {			
			// Read 32 RGBA samples
			_mm256_store_si256(&ymmRgba0, _mm256_load_si256((__m256i*)(rgbaPtr))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			_mm256_store_si256(&ymmRgba1, _mm256_load_si256((__m256i*)(rgbaPtr + 32))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): exfxgxhx
			_mm256_store_si256(&ymmRgba2, _mm256_load_si256((__m256i*)(rgbaPtr + 64))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): ixjxkxlx
			_mm256_store_si256(&ymmRgba3, _mm256_load_si256((__m256i*)(rgbaPtr + 96))); // 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): mxnxoxpx

			_mm256_store_si256(&ymm0, _mm256_unpacklo_epi32(ymmRgba0, ymmRgba1)); // aexxcgxx
			_mm256_store_si256(&ymm1, _mm256_unpackhi_epi32(ymmRgba0, ymmRgba1)); // bfxxdhxx
			_mm256_store_si256(&ymmRgba0, _mm256_unpacklo_epi32(ymm0, ymm1)); // abefcdgh
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // abcdefgh
			_mm256_store_si256(&ymmRgba1, ymmRgba0);

			_mm256_store_si256(&ymm0, _mm256_unpacklo_epi32(ymmRgba2, ymmRgba3)); // imxxkoxx
			_mm256_store_si256(&ymm1, _mm256_unpackhi_epi32(ymmRgba2, ymmRgba3)); // jnxxlpxx
			_mm256_store_si256(&ymmRgba2, _mm256_unpacklo_epi32(ymm0, ymm1)); // ijmnklop
			_mm256_store_si256(&ymmRgba2, _mm256_permute4x64_epi64(ymmRgba2, COMPV_MM_SHUFFLE(3, 1, 2, 0))); // ijklmnop
			_mm256_store_si256(&ymmRgba3, ymmRgba2);

			// U = (ymmRgba0, ymmRgba2)
			// V = (ymmRgba1, ymmRgba3)
			_mm256_store_si256(&ymmRgba0, _mm256_maddubs_epi16(ymmRgba0, ymmUCoeffs));
			_mm256_store_si256(&ymmRgba2, _mm256_maddubs_epi16(ymmRgba2, ymmUCoeffs));
			_mm256_store_si256(&ymmRgba1, _mm256_maddubs_epi16(ymmRgba1, ymmVCoeffs));
			_mm256_store_si256(&ymmRgba3, _mm256_maddubs_epi16(ymmRgba3, ymmVCoeffs));

			// U = ymmRgba0
			// V = ymmRgba1
			_mm256_store_si256(&ymmRgba0, _mm256_hadd_epi16(ymmRgba0, ymmRgba2));
			_mm256_store_si256(&ymmRgba1, _mm256_hadd_epi16(ymmRgba1, ymmRgba3));
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
			_mm256_store_si256(&ymmRgba1, _mm256_permute4x64_epi64(ymmRgba1, COMPV_MM_SHUFFLE(3, 1, 2, 0)));

			_mm256_store_si256(&ymmRgba0, _mm256_srai_epi16(ymmRgba0, 8)); // >> 8
			_mm256_store_si256(&ymmRgba1, _mm256_srai_epi16(ymmRgba1, 8)); // >> 8

			_mm256_store_si256(&ymmRgba0, _mm256_add_epi16(ymmRgba0, ymm128)); // + 128 -> UUVV----
			_mm256_store_si256(&ymmRgba1, _mm256_add_epi16(ymmRgba1, ymm128)); // + 128 -> UUVV----
			
			// UV = ymmRgba0
			_mm256_store_si256(&ymmRgba0, _mm256_packus_epi16(ymmRgba0, ymmRgba1)); // Saturate(I16 -> U8)
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(3, 1, 2, 0)));			

#if 1		// Best way to have AVX code *only* and avoid SSE/AVX mixing penalities
			_mm256_maskstore_epi64((int64_t*)outUPtr, ymmMaskToExtract128bits, ymmRgba0);
			_mm256_store_si256(&ymmRgba0, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(0, 0, 3, 2)));
			_mm256_maskstore_epi64((int64_t*)outVPtr, ymmMaskToExtract128bits, ymmRgba0);
			
#elif 0
			_mm256_store_si256(&ymmRgba1, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(1, 0, 3, 2)));
			_mm_storeu_si128((__m128i*)outUPtr, _mm256_castsi256_si128(ymmRgba0));
			_mm_storeu_si128((__m128i*)outVPtr, _mm256_castsi256_si128(ymmRgba1));
#elif 0
			// ASM code
			vmovups xmm0,xmm2
			vextractf128 xmm1,ymm2,1
#elif 0
			// SSE / AVX mix, no way to invoke _mm256_zeroupper()
			_mm_storeu_si128((__m128i*)outUPtr, _mm256_castsi256_si128(ymmRgba0));
			_mm_storeu_si128((__m128i*)outVPtr, _mm256_extractf128_si256(ymmRgba0, 0x1));
#elif 0
			// SSE code is inserted 
			_mm256_storeu2_m128i((__m128i*)outVPtr, (__m128i*)outUPtr, ymmRgba0);
#endif

			outUPtr += 16;
			outVPtr += 16;
			rgbaPtr += 128;
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */