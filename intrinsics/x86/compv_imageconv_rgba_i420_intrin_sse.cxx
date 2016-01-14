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

void rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
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

void rgbaToI420Kernel41_CompY_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
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

void rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
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

void rgbaToI420Kernel41_CompUV_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
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

void i420ToRGBAKernel11_Intrin_Aligned_SSSE3(COMV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(SSE) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
    vcomp_scalar_t i, j, maxI = ((width + 15) & -16), rollbackUV = -((maxI + 1) >> 1), padY = (stride - maxI), padUV = ((padY + 1) >> 1), padRGBA = (padY << 2);
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmmY, xmmU, xmmV, xmm16, xmmRCoeffs, xmmGCoeffs, xmmBCoeffs, xmmZeroCoeffs, xmmAlpha, xmm7120, xmm8912, xmm4400;

    _mm_store_si128(&xmmRCoeffs, _mm_load_si128((__m128i*)kYUVToRGBA_RCoeffs8));
    _mm_store_si128(&xmmGCoeffs, _mm_load_si128((__m128i*)kYUVToRGBA_GCoeffs8));
    _mm_store_si128(&xmmBCoeffs, _mm_load_si128((__m128i*)kYUVToRGBA_BCoeffs8));
    _mm_store_si128(&xmmZeroCoeffs, _mm_setzero_si128());
    _mm_store_si128(&xmm5, _mm_load_si128((__m128i*)k5_i8));
    _mm_store_si128(&xmm16, _mm_load_si128((__m128i*)k16_i8));
    _mm_store_si128(&xmmAlpha, _mm_load_si128((__m128i*)k255_i16));
    _mm_store_si128(&xmm7120, _mm_load_si128((__m128i*)k7120_i16));
    _mm_store_si128(&xmm8912, _mm_load_si128((__m128i*)k8912_i16));
    _mm_store_si128(&xmm4400, _mm_load_si128((__m128i*)k4400_i16));

    // R!u8 = (37Y' + 0U' + 51V') >> 5
    // G!u8 = (37Y' - 13U' - 26V') >> 5
    // B!u8 = (37Y' + 65U' + 0V') >> 5
    // where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
    // _mm_subs_epu8(U, 128) produce overflow -> use I16
    // R!i16 = (37Y + 0U + 51V - 7120) >> 5
    // G!i16 = (37Y - 13U - 26V + 4400) >> 5
    // B!i16 = (37Y + 65U + 0V - 8912) >> 5

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 16) {
            _mm_store_si128(&xmmY, _mm_load_si128((__m128i*)yPtr)); // 16 Y samples = 16bytes
            _mm_store_si128(&xmmU, _mm_loadl_epi64((__m128i const*)uPtr)); // 8 U samples, low mem
            _mm_store_si128(&xmmV, _mm_loadl_epi64((__m128i const*)vPtr)); // 8 V samples, low mem

            _mm_store_si128(&xmmU, _mm_unpacklo_epi8(xmmU, xmmU)); // duplicate -> 16 U samples
            _mm_store_si128(&xmmV, _mm_unpacklo_epi8(xmmV, xmmV)); // duplicate -> 16 V samples

            /////// 8Y - LOW ///////

            // YUV0 = (xmm2 || xmm3)
            _mm_store_si128(&xmm0, _mm_unpacklo_epi8(xmmY, xmmV)); // YVYVYVYVYVYVYV....
            _mm_store_si128(&xmm1, _mm_unpacklo_epi8(xmmU, xmmZeroCoeffs)); //U0U0U0U0U0U0U0U0....
            _mm_store_si128(&xmm2, _mm_unpacklo_epi8(xmm0, xmm1)); // YUV0YUV0YUV0YUV0YUV0YUV0
            _mm_store_si128(&xmm3, _mm_unpackhi_epi8(xmm0, xmm1)); // YUV0YUV0YUV0YUV0YUV0YUV0

            // xmm0 = R
            _mm_store_si128(&xmm0, _mm_maddubs_epi16(xmm2, xmmRCoeffs));
            _mm_store_si128(&xmm1, _mm_maddubs_epi16(xmm3, xmmRCoeffs));
            _mm_store_si128(&xmm0, _mm_hadd_epi16(xmm0, xmm1));
            _mm_store_si128(&xmm0, _mm_sub_epi16(xmm0, xmm7120));
            _mm_store_si128(&xmm0, _mm_srai_epi16(xmm0, 5)); // >> 5
            // xmm1 = B
            _mm_store_si128(&xmm1, _mm_maddubs_epi16(xmm2, xmmBCoeffs));
            _mm_store_si128(&xmm4, _mm_maddubs_epi16(xmm3, xmmBCoeffs));
            _mm_store_si128(&xmm1, _mm_hadd_epi16(xmm1, xmm4));
            _mm_store_si128(&xmm1, _mm_sub_epi16(xmm1, xmm8912));
            _mm_store_si128(&xmm1, _mm_srai_epi16(xmm1, 5)); // >> 5
            // xmm4 = RBRBRBRBRBRB
            _mm_store_si128(&xmm4, _mm_unpacklo_epi16(xmm0, xmm1)); // low16(RBRBRBRBRBRB)
            _mm_store_si128(&xmm5, _mm_unpackhi_epi16(xmm0, xmm1)); // high16(RBRBRBRBRBRB)
            _mm_store_si128(&xmm4, _mm_packus_epi16(xmm4, xmm5)); // u8(RBRBRBRBRBRB)

            // xmm2 = G
            _mm_store_si128(&xmm2, _mm_maddubs_epi16(xmm2, xmmGCoeffs));
            _mm_store_si128(&xmm3, _mm_maddubs_epi16(xmm3, xmmGCoeffs));
            _mm_store_si128(&xmm2, _mm_hadd_epi16(xmm2, xmm3));
            _mm_store_si128(&xmm2, _mm_add_epi16(xmm2, xmm4400));
            _mm_store_si128(&xmm2, _mm_srai_epi16(xmm2, 5)); // >> 5
            // xmm3 = GAGAGAGAGAGAGA
            _mm_store_si128(&xmm3, _mm_unpacklo_epi16(xmm2, xmmAlpha)); // low16(GAGAGAGAGAGAGA)
            _mm_store_si128(&xmm2, _mm_unpackhi_epi16(xmm2, xmmAlpha)); // high16(GAGAGAGAGAGAGA)
            _mm_store_si128(&xmm3, _mm_packus_epi16(xmm3, xmm2)); // u8(GAGAGAGAGAGAGA)

            // outRgbaPtr[0-32] = RGBARGBARGBARGBA
            _mm_store_si128((__m128i*)(outRgbaPtr + 0), _mm_unpacklo_epi8(xmm4, xmm3)); // low8(RGBARGBARGBARGBA)
            _mm_store_si128((__m128i*)(outRgbaPtr + 16), _mm_unpackhi_epi8(xmm4, xmm3)); // high8(RGBARGBARGBARGBA)

            /////// 8Y - HIGH ///////

            // YUV0 = (xmm2 || xmm3)
            _mm_store_si128(&xmm0, _mm_unpackhi_epi8(xmmY, xmmV)); // YVYVYVYVYVYVYV....
            _mm_store_si128(&xmm1, _mm_unpackhi_epi8(xmmU, xmmZeroCoeffs)); //U0U0U0U0U0U0U0U0....
            _mm_store_si128(&xmm2, _mm_unpacklo_epi8(xmm0, xmm1)); // YUV0YUV0YUV0YUV0YUV0YUV0
            _mm_store_si128(&xmm3, _mm_unpackhi_epi8(xmm0, xmm1)); // YUV0YUV0YUV0YUV0YUV0YUV0

            // xmm0 = R
            _mm_store_si128(&xmm0, _mm_maddubs_epi16(xmm2, xmmRCoeffs));
            _mm_store_si128(&xmm1, _mm_maddubs_epi16(xmm3, xmmRCoeffs));
            _mm_store_si128(&xmm0, _mm_hadd_epi16(xmm0, xmm1));
            _mm_store_si128(&xmm0, _mm_sub_epi16(xmm0, xmm7120));
            _mm_store_si128(&xmm0, _mm_srai_epi16(xmm0, 5)); // >> 5
            // xmm1 = B
            _mm_store_si128(&xmm1, _mm_maddubs_epi16(xmm2, xmmBCoeffs));
            _mm_store_si128(&xmm4, _mm_maddubs_epi16(xmm3, xmmBCoeffs));
            _mm_store_si128(&xmm1, _mm_hadd_epi16(xmm1, xmm4));
            _mm_store_si128(&xmm1, _mm_sub_epi16(xmm1, xmm8912));
            _mm_store_si128(&xmm1, _mm_srai_epi16(xmm1, 5)); // >> 5
            // xmm4 = RBRBRBRBRBRB
            _mm_store_si128(&xmm4, _mm_unpacklo_epi16(xmm0, xmm1)); // low16(RBRBRBRBRBRB)
            _mm_store_si128(&xmm5, _mm_unpackhi_epi16(xmm0, xmm1)); // high16(RBRBRBRBRBRB)
            _mm_store_si128(&xmm4, _mm_packus_epi16(xmm4, xmm5)); // u8(RBRBRBRBRBRB)

            // xmm2 = G
            _mm_store_si128(&xmm2, _mm_maddubs_epi16(xmm2, xmmGCoeffs));
            _mm_store_si128(&xmm3, _mm_maddubs_epi16(xmm3, xmmGCoeffs));
            _mm_store_si128(&xmm2, _mm_hadd_epi16(xmm2, xmm3));
            _mm_store_si128(&xmm2, _mm_add_epi16(xmm2, xmm4400));
            _mm_store_si128(&xmm2, _mm_srai_epi16(xmm2, 5)); // >> 5
            // xmm3 = GAGAGAGAGAGAGA
            _mm_store_si128(&xmm3, _mm_unpacklo_epi16(xmm2, xmmAlpha)); // low16(GAGAGAGAGAGAGA)
            _mm_store_si128(&xmm2, _mm_unpackhi_epi16(xmm2, xmmAlpha)); // high16(GAGAGAGAGAGAGA)
            _mm_store_si128(&xmm3, _mm_packus_epi16(xmm3, xmm2)); // u8(GAGAGAGAGAGAGA)

            // outRgbaPtr[32-64] = RGBARGBARGBARGBA
            _mm_store_si128((__m128i*)(outRgbaPtr + 32), _mm_unpacklo_epi8(xmm4, xmm3)); // low8(RGBARGBARGBARGBA)
            _mm_store_si128((__m128i*)(outRgbaPtr + 48), _mm_unpackhi_epi8(xmm4, xmm3)); // high8(RGBARGBARGBARGBA)

            yPtr += 16;
            uPtr += 8;
            vPtr += 8;
            outRgbaPtr += 64;
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
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */