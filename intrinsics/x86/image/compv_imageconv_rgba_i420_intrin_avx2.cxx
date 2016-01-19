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
#include "compv/intrinsics/x86/image/compv_imageconv_rgba_i420_intrin_avx2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_YCoeffs8)
{
    _mm256_zeroupper();
    __m256i ymmRgba, ymmYCoeffs, ymm16, ymmMaskToExtractFirst64Bits;
    vcomp_scalar_t i, j, maxI = ((width + 7) & -8), padY = (stride - maxI), padRGBA = padY << 2;

    _mm256_store_si256(&ymmYCoeffs, _mm256_load_si256((__m256i*)kXXXXToYUV_YCoeffs8));
    _mm256_store_si256(&ymm16, _mm256_load_si256((__m256i*)k16_i16));
    _mm256_store_si256(&ymmMaskToExtractFirst64Bits, _mm256_load_si256((__m256i*)kAVXMaskstore_0_i64));

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

void rgbaToI420Kernel41_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_YCoeffs8)
{
    _mm256_zeroupper();
    __m256i ymmRgba0, ymmRgba1, ymmRgba2, ymmRgba3, ymmYCoeffs, ymm16, ymmAEBFCGDH;
    vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGBA = padY << 2;

    _mm256_store_si256(&ymmYCoeffs, _mm256_load_si256((__m256i*)kXXXXToYUV_YCoeffs8));
    _mm256_store_si256(&ymm16, _mm256_load_si256((__m256i*)k16_i16));
    _mm256_store_si256(&ymmAEBFCGDH, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_AEBFCGDH_i32));

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

            _mm256_store_si256(&ymmRgba0, _mm256_hadd_epi16(ymmRgba0, ymmRgba1)); // hadd(ABCD) -> ACBD
            _mm256_store_si256(&ymmRgba2, _mm256_hadd_epi16(ymmRgba2, ymmRgba3)); // hadd(EFGH) -> EGFH

            _mm256_store_si256(&ymmRgba0, _mm256_srai_epi16(ymmRgba0, 7)); // >> 7
            _mm256_store_si256(&ymmRgba2, _mm256_srai_epi16(ymmRgba2, 7)); // >> 7

            _mm256_store_si256(&ymmRgba0, _mm256_add_epi16(ymmRgba0, ymm16)); // + 16
            _mm256_store_si256(&ymmRgba2, _mm256_add_epi16(ymmRgba2, ymm16)); // + 16

            // Saturate(I16 -> U8)
            _mm256_store_si256(&ymmRgba0, _mm256_packus_epi16(ymmRgba0, ymmRgba2)); // packus(ACBD, EGFH) -> AEBFCGDH

            // Final Permute
            _mm256_store_si256(&ymmRgba0, _mm256_permutevar8x32_epi32(ymmRgba0, ymmAEBFCGDH));

            _mm256_store_si256((__m256i*)outYPtr, ymmRgba0);

            outYPtr += 32;
            rgbaPtr += 128;
        }
        outYPtr += padY;
        rgbaPtr += padRGBA;
    }
    _mm256_zeroupper();
}

void rgbaToI420Kernel11_CompUV_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8)
{
    _mm256_zeroupper();
    __m256i ymmRgba, ymm128, ymmUV4Coeffs, ymmMaskToExtractFirst32Bits;
#if 0
    __m128i xmmUV;
#endif
    vcomp_scalar_t i, j, maxI = ((width + 7) & -8), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines

    // load UV coeffs interleaved: each appear #4 times (kRGBAToYUV_U4V4Coeffs8) - #4times U(or V) = #4 times 32bits = 128bits
    // ASM, use vinsertf128
    _mm256_store_si256(&ymmUV4Coeffs, _mm256_insertf128_si256(_mm256_load_si256((__m256i*)kXXXXToYUV_UCoeffs8), _mm256_castsi256_si128(_mm256_load_si256((__m256i*)kXXXXToYUV_VCoeffs8)), 0x1));
    // load 128's16
    _mm256_store_si256(&ymm128, _mm256_load_si256((__m256i*)k128_i16));
    // load mask used to extract first 32bits
    _mm256_store_si256(&ymmMaskToExtractFirst32Bits, _mm256_load_si256((__m256i*)kAVXMaskstore_0_i32));

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

void rgbaToI420Kernel41_CompUV_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8)
{
    _mm256_zeroupper();
    __m256i ymmRgba0, ymmRgba1, ymmRgba2, ymmRgba3, ymm0, ymm1, ymmUCoeffs, ymmVCoeffs, ymm128, ymmAEBFCGDH, ymmMaskToExtract128bits;
    vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2; // +stride to skip even lines

    _mm256_store_si256(&ymmUCoeffs, _mm256_load_si256((__m256i*)kXXXXToYUV_UCoeffs8));
    _mm256_store_si256(&ymmVCoeffs, _mm256_load_si256((__m256i*)kXXXXToYUV_VCoeffs8));
    _mm256_store_si256(&ymm128, _mm256_load_si256((__m256i*)k128_i16));
    _mm256_store_si256(&ymmAEBFCGDH, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_AEBFCGDH_i32));
    _mm256_store_si256(&ymmMaskToExtract128bits, _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_i64));

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
            _mm256_store_si256(&ymmRgba0, _mm256_hadd_epi16(ymmRgba0, ymmRgba2)); // hadd -> A C B D
            _mm256_store_si256(&ymmRgba1, _mm256_hadd_epi16(ymmRgba1, ymmRgba3)); // hadd -> E G F H

            _mm256_store_si256(&ymmRgba0, _mm256_srai_epi16(ymmRgba0, 8)); // >> 8
            _mm256_store_si256(&ymmRgba1, _mm256_srai_epi16(ymmRgba1, 8)); // >> 8

            _mm256_store_si256(&ymmRgba0, _mm256_add_epi16(ymmRgba0, ymm128)); // + 128 -> UUVV----
            _mm256_store_si256(&ymmRgba1, _mm256_add_epi16(ymmRgba1, ymm128)); // + 128 -> UUVV----

            // UV = ymmRgba0
            _mm256_store_si256(&ymmRgba0, _mm256_packus_epi16(ymmRgba0, ymmRgba1)); // Saturate(I16 -> U8) packus(ACBD, EGFH) -> AEBFCGDH

            // Final Permute
            _mm256_store_si256(&ymmRgba0, _mm256_permutevar8x32_epi32(ymmRgba0, ymmAEBFCGDH));

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

// RGB -> I420 (Y)
void rgbToI420Kernel31_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbPtr, COMV_ALIGNED(AVX2) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_YCoeffs8)
{
    _mm256_zeroupper();
    __m256i rgba[4], ymmYCoeffs, ymm16, ymm0, ymm1, ymmAEBFCGDH, ymmABCDDEFG, ymmCDEFFGHX, ymmMaskRgbToRgba, ymmXXABBCDE, ymmLost;
    vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padY = (stride - maxI), padRGB = padY * 3;

    _mm256_store_si256(&ymmMaskRgbToRgba, _mm256_load_si256((__m256i*)kShuffleEpi8_RgbToRgba_i32));
    _mm256_store_si256(&ymmYCoeffs, _mm256_load_si256((__m256i*)kXXXXToYUV_YCoeffs8)); // RGBA coeffs
    _mm256_store_si256(&ymm16, _mm256_load_si256((__m256i*)k16_i16));
    _mm256_store_si256(&ymmAEBFCGDH, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_AEBFCGDH_i32));
    _mm256_store_si256(&ymmABCDDEFG, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_ABCDDEFG_i32));
    _mm256_store_si256(&ymmCDEFFGHX, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_CDEFFGHX_i32));
    _mm256_store_si256(&ymmXXABBCDE, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_XXABBCDE_i32));

    // Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 32) {
            // TODO(dmi): RGB -> RGBA conversion is common to Y and UV -> do it once
            COMPV_3RGB_TO_4RGBA_AVX2(&rgba, rgbPtr, ymm0, ymm1, ymmLost, ymmMaskRgbToRgba, ymmABCDDEFG, ymmXXABBCDE, ymmCDEFFGHX);

            // starting here we're using the same code as rgba -> i420 (Y)

            _mm256_store_si256(&rgba[0], _mm256_maddubs_epi16(rgba[0], ymmYCoeffs));
            _mm256_store_si256(&rgba[1], _mm256_maddubs_epi16(rgba[1], ymmYCoeffs));
            _mm256_store_si256(&rgba[2], _mm256_maddubs_epi16(rgba[2], ymmYCoeffs));
            _mm256_store_si256(&rgba[3], _mm256_maddubs_epi16(rgba[3], ymmYCoeffs));

            _mm256_store_si256(&rgba[0], _mm256_hadd_epi16(rgba[0], rgba[1])); // hadd(ABCD) -> ACBD
            _mm256_store_si256(&rgba[2], _mm256_hadd_epi16(rgba[2], rgba[3])); // hadd(EFGH) -> EGFH

            _mm256_store_si256(&rgba[0], _mm256_srai_epi16(rgba[0], 7)); // >> 7
            _mm256_store_si256(&rgba[2], _mm256_srai_epi16(rgba[2], 7)); // >> 7

            _mm256_store_si256(&rgba[0], _mm256_add_epi16(rgba[0], ymm16)); // + 16
            _mm256_store_si256(&rgba[2], _mm256_add_epi16(rgba[2], ymm16)); // + 16

            // Saturate(I16 -> U8)
            _mm256_store_si256(&rgba[0], _mm256_packus_epi16(rgba[0], rgba[2])); // packus(ACBD, EGFH) -> AEBFCGDH

            // Final Permute
            _mm256_store_si256(&rgba[0], _mm256_permutevar8x32_epi32(rgba[0], ymmAEBFCGDH));

            _mm256_store_si256((__m256i*)outYPtr, rgba[0]);

            outYPtr += 32;
            rgbPtr += 96;
        }
        outYPtr += padY;
        rgbPtr += padRGB;
    }
    _mm256_zeroupper();
}

// RGB -> I420 (UV)
void rgbToI420Kernel31_CompUV_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8)
{
    _mm256_zeroupper();
    __m256i rgba[4], ymm0, ymm1, ymmUCoeffs, ymmVCoeffs, ymm128, ymmAEBFCGDH, ymmMaskToExtract128bits, ymmXXABBCDE, ymmABCDDEFG, ymmMaskRgbToRgba, ymmCDEFFGHX, ymmLost;
    vcomp_scalar_t i, j, maxI = ((width + 31) & -32), padUV = (stride - maxI) >> 1, padRGB = ((stride - maxI) + stride) * 3; // +stride to skip even lines

    _mm256_store_si256(&ymmUCoeffs, _mm256_load_si256((__m256i*)kXXXXToYUV_UCoeffs8));
    _mm256_store_si256(&ymmVCoeffs, _mm256_load_si256((__m256i*)kXXXXToYUV_VCoeffs8));
    _mm256_store_si256(&ymm128, _mm256_load_si256((__m256i*)k128_i16));
    _mm256_store_si256(&ymmMaskToExtract128bits, _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_i64));
    _mm256_store_si256(&ymmMaskRgbToRgba, _mm256_load_si256((__m256i*)kShuffleEpi8_RgbToRgba_i32));
    _mm256_store_si256(&ymmAEBFCGDH, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_AEBFCGDH_i32));
    _mm256_store_si256(&ymmABCDDEFG, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_ABCDDEFG_i32));
    _mm256_store_si256(&ymmCDEFFGHX, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_CDEFFGHX_i32));
    _mm256_store_si256(&ymmXXABBCDE, _mm256_load_si256((__m256i*)kAVXPermutevar8x32_XXABBCDE_i32));

    // U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
    // V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
    for (j = 0; j < height; j += 2) {
        for (i = 0; i < width; i += 32) {
            // TODO(dmi): RGB -> RGBA conversion is common to Y and UV -> do it once
            COMPV_3RGB_TO_4RGBA_AVX2(&rgba, rgbPtr, ymm0, ymm1, ymmLost, ymmMaskRgbToRgba, ymmABCDDEFG, ymmXXABBCDE, ymmCDEFFGHX);

            // starting here we're using the same code as rgba -> i420 (UV)

            _mm256_store_si256(&ymm0, _mm256_unpacklo_epi32(rgba[0], rgba[1])); // aexxcgxx
            _mm256_store_si256(&ymm1, _mm256_unpackhi_epi32(rgba[0], rgba[1])); // bfxxdhxx
            _mm256_store_si256(&rgba[0], _mm256_unpacklo_epi32(ymm0, ymm1)); // abefcdgh
            _mm256_store_si256(&rgba[0], _mm256_permute4x64_epi64(rgba[0], COMPV_MM_SHUFFLE(3, 1, 2, 0))); // abcdefgh
            _mm256_store_si256(&rgba[1], rgba[0]);

            _mm256_store_si256(&ymm0, _mm256_unpacklo_epi32(rgba[2], rgba[3])); // imxxkoxx
            _mm256_store_si256(&ymm1, _mm256_unpackhi_epi32(rgba[2], rgba[3])); // jnxxlpxx
            _mm256_store_si256(&rgba[2], _mm256_unpacklo_epi32(ymm0, ymm1)); // ijmnklop
            _mm256_store_si256(&rgba[2], _mm256_permute4x64_epi64(rgba[2], COMPV_MM_SHUFFLE(3, 1, 2, 0))); // ijklmnop
            _mm256_store_si256(&rgba[3], rgba[2]);

            // U = (ymmRgba0, ymmRgba2)
            // V = (ymmRgba1, ymmRgba3)
            _mm256_store_si256(&rgba[0], _mm256_maddubs_epi16(rgba[0], ymmUCoeffs));
            _mm256_store_si256(&rgba[2], _mm256_maddubs_epi16(rgba[2], ymmUCoeffs));
            _mm256_store_si256(&rgba[1], _mm256_maddubs_epi16(rgba[1], ymmVCoeffs));
            _mm256_store_si256(&rgba[3], _mm256_maddubs_epi16(rgba[3], ymmVCoeffs));

            // U = ymmRgba0
            // V = ymmRgba1
            _mm256_store_si256(&rgba[0], _mm256_hadd_epi16(rgba[0], rgba[2])); // hadd -> A C B D
            _mm256_store_si256(&rgba[1], _mm256_hadd_epi16(rgba[1], rgba[3])); // hadd -> E G F H

            _mm256_store_si256(&rgba[0], _mm256_srai_epi16(rgba[0], 8)); // >> 8
            _mm256_store_si256(&rgba[1], _mm256_srai_epi16(rgba[1], 8)); // >> 8

            _mm256_store_si256(&rgba[0], _mm256_add_epi16(rgba[0], ymm128)); // + 128 -> UUVV----
            _mm256_store_si256(&rgba[1], _mm256_add_epi16(rgba[1], ymm128)); // + 128 -> UUVV----

            // UV = ymmRgba0
            _mm256_store_si256(&rgba[0], _mm256_packus_epi16(rgba[0], rgba[1])); // Saturate(I16 -> U8) packus(ACBD, EGFH) -> AEBFCGDH

            // Final Permute
            _mm256_store_si256(&rgba[0], _mm256_permutevar8x32_epi32(rgba[0], ymmAEBFCGDH));

#if 1		// Best way to have AVX code *only* and avoid SSE/AVX mixing penalities
            _mm256_maskstore_epi64((int64_t*)outUPtr, ymmMaskToExtract128bits, rgba[0]);
            _mm256_store_si256(&rgba[0], _mm256_permute4x64_epi64(rgba[0], COMPV_MM_SHUFFLE(0, 0, 3, 2)));
            _mm256_maskstore_epi64((int64_t*)outVPtr, ymmMaskToExtract128bits, rgba[0]);

#elif 0
            _mm256_store_si256(&ymmRgba1, _mm256_permute4x64_epi64(ymmRgba0, COMPV_MM_SHUFFLE(1, 0, 3, 2)));
            _mm_storeu_si128((__m128i*)outUPtr, _mm256_castsi256_si128(ymmRgba0));
            _mm_storeu_si128((__m128i*)outVPtr, _mm256_castsi256_si128(ymmRgba1));
#elif 0
            // ASM code
            vmovups xmm0, xmm2
            vextractf128 xmm1, ymm2, 1
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
            rgbPtr += 96;
        }
        rgbPtr += padRGB;
        outUPtr += padUV;
        outVPtr += padUV;
    }
    _mm256_zeroupper();
}

void i420ToRGBAKernel11_Intrin_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(AVX2) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
    _mm256_zeroupper();

    vcomp_scalar_t i, j, maxI = ((width + 31) & -32), rollbackUV = -((maxI + 1) >> 1), padY = (stride - maxI), padUV = ((padY + 1) >> 1), padRGBA = (padY << 2);
    __m256i ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymmY, ymmU, ymmV, ymm16, ymmRCoeffs, ymmGCoeffs, ymmBCoeffs, ymmZeroCoeffs, ymmAlpha, ymmMaskToExtract128bits, ymm7120, ymm8912, ymm4400;

    _mm256_store_si256(&ymmRCoeffs, _mm256_load_si256((__m256i*)kYUVToRGBA_RCoeffs8));
    _mm256_store_si256(&ymmGCoeffs, _mm256_load_si256((__m256i*)kYUVToRGBA_GCoeffs8));
    _mm256_store_si256(&ymmBCoeffs, _mm256_load_si256((__m256i*)kYUVToRGBA_BCoeffs8));
    _mm256_store_si256(&ymmZeroCoeffs, _mm256_setzero_si256());
    _mm256_store_si256(&ymm5, _mm256_load_si256((__m256i*)k5_i8));
    _mm256_store_si256(&ymm16, _mm256_load_si256((__m256i*)k16_i8));
    _mm256_store_si256(&ymmAlpha, _mm256_load_si256((__m256i*)k255_i16));
    _mm256_store_si256(&ymm7120, _mm256_load_si256((__m256i*)k7120_i16));
    _mm256_store_si256(&ymm8912, _mm256_load_si256((__m256i*)k8912_i16));
    _mm256_store_si256(&ymm4400, _mm256_load_si256((__m256i*)k4400_i16));
    _mm256_store_si256(&ymmMaskToExtract128bits, _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_i64));

    // R!u8 = (37Y' + 0U' + 51V') >> 5
    // G!u8 = (37Y' - 13U' - 26V') >> 5
    // B!u8 = (37Y' + 65U' + 0V') >> 5
    // where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
    // _mm_subs_epu8(U, 128) produce overflow -> use I16
    // R!i16 = (37Y + 0U + 51V - 7120) >> 5
    // G!i16 = (37Y - 13U - 26V + 4400) >> 5
    // B!i16 = (37Y + 65U + 0V - 8912) >> 5
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 32) {
            _mm256_store_si256(&ymmY, _mm256_load_si256((__m256i*)yPtr)); // 32 Y samples
            _mm256_store_si256(&ymmU, _mm256_maskload_epi64((int64_t const*)uPtr, ymmMaskToExtract128bits)); // 16 U samples, low mem
            _mm256_store_si256(&ymmV, _mm256_maskload_epi64((int64_t const*)vPtr, ymmMaskToExtract128bits)); // 16 V samples, low mem

            // Duplicate and interleave
            _mm256_store_si256(&ymmU, _mm256_permute4x64_epi64(ymmU, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
            _mm256_store_si256(&ymmU, _mm256_unpacklo_epi8(ymmU, ymmU));
            _mm256_store_si256(&ymmV, _mm256_permute4x64_epi64(ymmV, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
            _mm256_store_si256(&ymmV, _mm256_unpacklo_epi8(ymmV, ymmV));

            /////////////////////////
            /////// 16Y - LOW ///////
            /////////////////////////

            // YUV0 = (ymm2 || ymm3)
            _mm256_store_si256(&ymm0, _mm256_unpacklo_epi8(ymmY, ymmV)); // YVYVYVYVYVYVYV....
            _mm256_store_si256(&ymm1, _mm256_unpacklo_epi8(ymmU, ymmZeroCoeffs)); //U0U0U0U0U0U0U0U0....
            _mm256_store_si256(&ymm2, _mm256_unpacklo_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0
            _mm256_store_si256(&ymm3, _mm256_unpackhi_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0

            // ymm0 = R
            _mm256_store_si256(&ymm0, _mm256_maddubs_epi16(ymm2, ymmRCoeffs));
            _mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm3, ymmRCoeffs));
            _mm256_store_si256(&ymm0, _mm256_hadd_epi16(ymm0, ymm1));
            _mm256_store_si256(&ymm0, _mm256_sub_epi16(ymm0, ymm7120));
            _mm256_store_si256(&ymm0, _mm256_srai_epi16(ymm0, 5)); // >> 5
            // ymm1 = B
            _mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm2, ymmBCoeffs));
            _mm256_store_si256(&ymm4, _mm256_maddubs_epi16(ymm3, ymmBCoeffs));
            _mm256_store_si256(&ymm1, _mm256_hadd_epi16(ymm1, ymm4));
            _mm256_store_si256(&ymm1, _mm256_sub_epi16(ymm1, ymm8912));
            _mm256_store_si256(&ymm1, _mm256_srai_epi16(ymm1, 5)); // >> 5
            // ymm4 = RBRBRBRBRBRB
            _mm256_store_si256(&ymm4, _mm256_unpacklo_epi16(ymm0, ymm1)); // low16(RBRBRBRBRBRB)
            _mm256_store_si256(&ymm5, _mm256_unpackhi_epi16(ymm0, ymm1)); // high16(RBRBRBRBRBRB)
            _mm256_store_si256(&ymm4, _mm256_packus_epi16(ymm4, ymm5)); // u8(RBRBRBRBRBRB)

            // ymm2 = G
            _mm256_store_si256(&ymm2, _mm256_maddubs_epi16(ymm2, ymmGCoeffs));
            _mm256_store_si256(&ymm3, _mm256_maddubs_epi16(ymm3, ymmGCoeffs));
            _mm256_store_si256(&ymm2, _mm256_hadd_epi16(ymm2, ymm3));
            _mm256_store_si256(&ymm2, _mm256_add_epi16(ymm2, ymm4400));
            _mm256_store_si256(&ymm2, _mm256_srai_epi16(ymm2, 5)); // >> 5
            // ymm3 = GAGAGAGAGAGAGA
            _mm256_store_si256(&ymm3, _mm256_unpacklo_epi16(ymm2, ymmAlpha)); // low16(GAGAGAGAGAGAGA)
            _mm256_store_si256(&ymm2, _mm256_unpackhi_epi16(ymm2, ymmAlpha)); // high16(GAGAGAGAGAGAGA)
            _mm256_store_si256(&ymm3, _mm256_packus_epi16(ymm3, ymm2)); // u8(GAGAGAGAGAGAGA)

            // outRgbaPtr[x-y] = RGBARGBARGBARGBA
            // re-order the samples for the final unpacklo, unpackhi
            _mm256_store_si256(&ymm4, _mm256_permute4x64_epi64(ymm4, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
            _mm256_store_si256(&ymm3, _mm256_permute4x64_epi64(ymm3, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
            // because of AVX cross-lane issue final data = (0, 2, 1, 3)*32 = (0, 64, 32, 96)
            _mm256_store_si256((__m256i*)(outRgbaPtr + 0), _mm256_unpacklo_epi8(ymm4, ymm3)); // low8(RGBARGBARGBARGBA)
            _mm256_store_si256((__m256i*)(outRgbaPtr + 64), _mm256_unpackhi_epi8(ymm4, ymm3)); // high8(RGBARGBARGBARGBA)

            //////////////////////////
            /////// 16Y - HIGH ///////
            //////////////////////////

            // YUV0 = (ymm2 || ymm3)
            _mm256_store_si256(&ymm0, _mm256_unpackhi_epi8(ymmY, ymmV)); // YVYVYVYVYVYVYV....
            _mm256_store_si256(&ymm1, _mm256_unpackhi_epi8(ymmU, ymmZeroCoeffs)); //U0U0U0U0U0U0U0U0....
            _mm256_store_si256(&ymm2, _mm256_unpacklo_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0
            _mm256_store_si256(&ymm3, _mm256_unpackhi_epi8(ymm0, ymm1)); // YUV0YUV0YUV0YUV0YUV0YUV0

            // ymm0 = R
            _mm256_store_si256(&ymm0, _mm256_maddubs_epi16(ymm2, ymmRCoeffs));
            _mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm3, ymmRCoeffs));
            _mm256_store_si256(&ymm0, _mm256_hadd_epi16(ymm0, ymm1));
            _mm256_store_si256(&ymm0, _mm256_sub_epi16(ymm0, ymm7120));
            _mm256_store_si256(&ymm0, _mm256_srai_epi16(ymm0, 5)); // >> 5
            // ymm1 = B
            _mm256_store_si256(&ymm1, _mm256_maddubs_epi16(ymm2, ymmBCoeffs));
            _mm256_store_si256(&ymm4, _mm256_maddubs_epi16(ymm3, ymmBCoeffs));
            _mm256_store_si256(&ymm1, _mm256_hadd_epi16(ymm1, ymm4));
            _mm256_store_si256(&ymm1, _mm256_sub_epi16(ymm1, ymm8912));
            _mm256_store_si256(&ymm1, _mm256_srai_epi16(ymm1, 5)); // >> 5
            // ymm4 = RBRBRBRBRBRB
            _mm256_store_si256(&ymm4, _mm256_unpacklo_epi16(ymm0, ymm1)); // low16(RBRBRBRBRBRB)
            _mm256_store_si256(&ymm5, _mm256_unpackhi_epi16(ymm0, ymm1)); // high16(RBRBRBRBRBRB)
            _mm256_store_si256(&ymm4, _mm256_packus_epi16(ymm4, ymm5)); // u8(RBRBRBRBRBRB)

            // ymm2 = G
            _mm256_store_si256(&ymm2, _mm256_maddubs_epi16(ymm2, ymmGCoeffs));
            _mm256_store_si256(&ymm3, _mm256_maddubs_epi16(ymm3, ymmGCoeffs));
            _mm256_store_si256(&ymm2, _mm256_hadd_epi16(ymm2, ymm3));
            _mm256_store_si256(&ymm2, _mm256_add_epi16(ymm2, ymm4400));
            _mm256_store_si256(&ymm2, _mm256_srai_epi16(ymm2, 5)); // >> 5
            // ymm3 = GAGAGAGAGAGAGA
            _mm256_store_si256(&ymm3, _mm256_unpacklo_epi16(ymm2, ymmAlpha)); // low16(GAGAGAGAGAGAGA)
            _mm256_store_si256(&ymm2, _mm256_unpackhi_epi16(ymm2, ymmAlpha)); // high16(GAGAGAGAGAGAGA)
            _mm256_store_si256(&ymm3, _mm256_packus_epi16(ymm3, ymm2)); // u8(GAGAGAGAGAGAGA)

            // outRgbaPtr[x-y] = RGBARGBARGBARGBA
            // re-order the samples for the final unpacklo, unpackhi
            _mm256_store_si256(&ymm4, _mm256_permute4x64_epi64(ymm4, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
            _mm256_store_si256(&ymm3, _mm256_permute4x64_epi64(ymm3, COMPV_MM_SHUFFLE(3, 1, 2, 0)));
            // because of AVX cross-lane issue final data = (0, 2, 1, 3)*32 = (0, 64, 32, 96)
            _mm256_store_si256((__m256i*)(outRgbaPtr + 32), _mm256_unpacklo_epi8(ymm4, ymm3)); // low8(RGBARGBARGBARGBA)
            _mm256_store_si256((__m256i*)(outRgbaPtr + 96), _mm256_unpackhi_epi8(ymm4, ymm3)); // high8(RGBARGBARGBARGBA)


            yPtr += 32;
            uPtr += 16;
            vPtr += 16;
            outRgbaPtr += 128;
        }
        yPtr += padY;
#if 1
        uPtr += (j & 1) ? padUV : rollbackUV;
        vPtr += (j & 1) ? padUV : rollbackUV;
#else
        uPtr += ((j & 1) * padUV) + (((j + 1) & 1) * rollbackUV);
        vPtr += ((j & 1) * padUV) + (((j + 1) & 1) * rollbackUV);
#endif
        outRgbaPtr += padRGBA;
    }

    _mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */