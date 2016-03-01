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
#include "compv/image/conv/compv_imageconv_rgba_i420.h"
#include "compv/image/conv/compv_imageconv_common.h"
#include "compv/compv_engine.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_mathutils.h"

#include "compv/intrinsics/x86/image/conv/compv_imageconv_rgba_i420_intrin_sse.h"
#include "compv/intrinsics/x86/image/conv/compv_imageconv_rgba_i420_intrin_avx2.h"

COMPV_NAMESPACE_BEGIN()

#if defined(COMPV_ARCH_X86) && defined(COMPV_ASM)
// SSSE3
extern "C" void rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel11_CompY_Asm_X86_Aligned0x_SSSE3(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_SSSE3(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_SSSE3(const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);

extern "C" void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_SSSE3(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned1xx_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned0xx_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);

extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned00_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned01_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned10_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned11_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8);

extern "C" void rgbToI420Kernel31_CompUV_Asm_X86_Aligned0xx_SSSE3(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbToI420Kernel31_CompUV_Asm_X86_Aligned1xx_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8);

extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned00_SSSE3(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned01_SSSE3(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned10_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned11_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);

// AVX2-X86
extern "C" void rgbaToI420Kernel11_CompY_Asm_X86_Aligned0_AVX2(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel11_CompY_Asm_X86_Aligned1_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_AVX2(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_AVX2(const uint8_t* rgbaPtr, COMPV_ALIGNED(AVX2) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMPV_ALIGNED(AVX2) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);

extern "C" void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned000_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned100_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned110_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned111_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);

extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned00_AVX2(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned01_AVX2(const uint8_t* rgbaPtr, COMPV_ALIGNED(AVX2) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned10_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);
extern "C" void rgbToI420Kernel31_CompY_Asm_X86_Aligned11_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMPV_ALIGNED(AVX2) uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs8);

extern "C" void rgbToI420Kernel31_CompUV_Asm_X86_Aligned000_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbToI420Kernel31_CompUV_Asm_X86_Aligned100_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbToI420Kernel31_CompUV_Asm_X86_Aligned110_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);
extern "C" void rgbToI420Kernel31_CompUV_Asm_X86_Aligned111_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8);

extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned00_AVX2(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned01_AVX2(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMPV_ALIGNED(AVX2) uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned10_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
extern "C" void i420ToRGBAKernel11_Asm_X86_Aligned11_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMPV_ALIGNED(AVX2) uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);

#endif

static void rgbaToI420Kernel11_CompY_C(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXXToYUV_YCoeffs8)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t padSample = (stride - width);
    compv_scalar_t padRGBA = padSample << 2;
    compv_scalar_t padY = padSample;
    const int16_t c0 = kXXXXToYUV_YCoeffs8[0];
    const int16_t c1 = kXXXXToYUV_YCoeffs8[1];
    const int16_t c2 = kXXXXToYUV_YCoeffs8[2];
    const int16_t c3 = kXXXXToYUV_YCoeffs8[3];
    // Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            *outYPtr++ = CompVMathUtils::clampPixel8((((c0 * rgbaPtr[0]) + (c1 * rgbaPtr[1]) + (c2 * rgbaPtr[2]) + (c3 * rgbaPtr[3])) >> 7) + 16);
            rgbaPtr += 4;
        }
        rgbaPtr += padRGBA;
        outYPtr += padY;
    }
}

static void rgbToI420Kernel11_CompY_C(const uint8_t* rgbaPtr, uint8_t* outYPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXToYUV_YCoeffs8)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t padSample = (stride - width);
    compv_scalar_t padRGBA = padSample * 3;
    compv_scalar_t padY = padSample;
    const int16_t c0 = kXXXToYUV_YCoeffs8[0];
    const int16_t c1 = kXXXToYUV_YCoeffs8[1];
    const int16_t c2 = kXXXToYUV_YCoeffs8[2];
    // Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            *outYPtr++ = CompVMathUtils::clampPixel8((((c0 * rgbaPtr[0]) + (c1 * rgbaPtr[1]) + (c2 * rgbaPtr[2])) >> 7) + 16);
            rgbaPtr += 3;
        }
        rgbaPtr += padRGBA;
        outYPtr += padY;
    }
}


static void rgbaToI420Kernel11_CompUV_C(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXXToYUV_VCoeffs8)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t i, j, maxI = ((width + 0) & -1), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2;
    const int16_t c0u = kXXXXToYUV_UCoeffs8[0], c0v = kXXXXToYUV_VCoeffs8[0];
    const int16_t c1u = kXXXXToYUV_UCoeffs8[1], c1v = kXXXXToYUV_VCoeffs8[1];
    const int16_t c2u = kXXXXToYUV_UCoeffs8[2], c2v = kXXXXToYUV_VCoeffs8[2];
    const int16_t c3u = kXXXXToYUV_UCoeffs8[3], c3v = kXXXXToYUV_VCoeffs8[3];
    // U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
    // V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
    for (j = 0; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            *outUPtr++ = CompVMathUtils::clampPixel8((((c0u* rgbaPtr[0]) + (c1u * rgbaPtr[1]) + (c2u * rgbaPtr[2]) + (c3u * rgbaPtr[3])) >> 8) + 128);
            *outVPtr++ = CompVMathUtils::clampPixel8(((((c0v * rgbaPtr[0]) + (c1v* rgbaPtr[1]) + (c2v * rgbaPtr[2]) + (c3v * rgbaPtr[3]))) >> 8) + 128);
            rgbaPtr += 8; // compSize*2 = 4*2 = 8
        }
        rgbaPtr += padRGBA;
        outUPtr += padUV;
        outVPtr += padUV;
    }
}

static void rgbToI420Kernel11_CompUV_C(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXToYUV_VCoeffs8)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t i, j, maxI = ((width + 0) & -1), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) * 3;
    const int16_t c0u = kXXXToYUV_UCoeffs8[0], c0v = kXXXToYUV_VCoeffs8[0];
    const int16_t c1u = kXXXToYUV_UCoeffs8[1], c1v = kXXXToYUV_VCoeffs8[1];
    const int16_t c2u = kXXXToYUV_UCoeffs8[2], c2v = kXXXToYUV_VCoeffs8[2];
    // U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
    // V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
    for (j = 0; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            *outUPtr++ = CompVMathUtils::clampPixel8((((c0u* rgbaPtr[0]) + (c1u * rgbaPtr[1]) + (c2u * rgbaPtr[2])) >> 8) + 128);
            *outVPtr++ = CompVMathUtils::clampPixel8(((((c0v * rgbaPtr[0]) + (c1v* rgbaPtr[1]) + (c2v * rgbaPtr[2]))) >> 8) + 128);
            rgbaPtr += 6; // compSize*2 = 3*2 = 6
        }
        rgbaPtr += padRGBA;
        outUPtr += padUV;
        outVPtr += padUV;
    }
}

static void i420ToRGBAKernel11_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t padSample = (stride - width);
    compv_scalar_t padRGBA = padSample << 2;
    compv_scalar_t padY = padSample;
    compv_scalar_t padUV = ((padY + 1) >> 1);
    int16_t Yp, Up, Vp;
    // R = (37Y' + 0U' + 51V') >> 5
    // G = (37Y' - 13U' - 26V') >> 5
    // B = (37Y' + 65U' + 0V') >> 5
    // where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
    // For ASM code _mm_subs_epu8(U, 128) produce overflow -> use I16
    // R!i16 = (37Y + 0U + 51V - 7120) >> 5
    // G!i16 = (37Y - 13U - 26V + 4400) >> 5
    // B!i16 = (37Y + 65U + 0V - 8912) >> 5
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            Yp = (*yPtr - 16);
            Up = (*uPtr - 127);
            Vp = (*vPtr - 127);
            outRgbaPtr[0] = CompVMathUtils::clampPixel8((37 * Yp + 51 * Vp) >> 5);
            outRgbaPtr[1] = CompVMathUtils::clampPixel8((37 * Yp - 13 * Up - 26 * Vp) >> 5);
            outRgbaPtr[2] = CompVMathUtils::clampPixel8((37 * Yp + 65 * Up) >> 5);
            outRgbaPtr[3] = 0xFF;

            outRgbaPtr += 4;
            yPtr += 1;
            uPtr += i & 1;
            vPtr += i & 1;
        }
        outRgbaPtr += padRGBA;
        yPtr += padY;
        uPtr += (j & 1) ? padUV : -(width >> 1);
        vPtr += (j & 1) ? padUV : -(width >> 1);
    }
}

// RGB/BGR -> I420
static COMPV_ERROR_CODE __xxxToI420(const CompVObjWrapper<CompVImage* >& rgb, CompVObjWrapper<CompVImage* >& i420, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXToYUV_YCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXToYUV_VCoeffs8)
{
    // This is a private function, up to the caller to check the input parameters
    // The output format must be I420 or Graysacle
    rgbToI420Kernel_CompY CompY = rgbToI420Kernel11_CompY_C;
    rgbToI420Kernel_CompUV CompUV = rgbToI420Kernel11_CompUV_C;
    const uint8_t* rgbPtr = (const uint8_t*)rgb->getDataPtr();
    int height = rgb->getHeight();
    int width = rgb->getWidth();
    int stride = rgb->getStride();
    uint8_t* outYPtr = (uint8_t*)(*i420)->getDataPtr();
    uint8_t* outUPtr = i420->getPixelFormat() == COMPV_PIXEL_FORMAT_GRAYSCALE ? NULL : outYPtr + (height * stride);
	uint8_t* outVPtr = i420->getPixelFormat() == COMPV_PIXEL_FORMAT_GRAYSCALE ? NULL : outUPtr + ((height * stride) >> 2);
    CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();
    int threadsCount = 1;

    // IS_ALIGNED(strideRgbaBytes, ALIGNV * 3) = IS_ALIGNED(stride * 3, ALIGNV * 3) = IS_ALIGNED(stride, ALIGNV)

    if (COMPV_IS_ALIGNED_SSE(stride)) {
        if (CompVCpu::isEnabled(kCpuFlagSSSE3)) {
            COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbToI420Kernel31_CompY_Asm_X86_Aligned00_SSSE3);
            COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbToI420Kernel31_CompUV_Asm_X86_Aligned0xx_SSSE3);
            if (COMPV_IS_ALIGNED_SSE(rgbPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86(CompUV = rgbToI420Kernel31_CompUV_Intrin_Aligned_SSSE3);
                COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbToI420Kernel31_CompY_Asm_X86_Aligned10_SSSE3);
                COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbToI420Kernel31_CompUV_Asm_X86_Aligned1xx_SSSE3);
                if (COMPV_IS_ALIGNED_SSE(outYPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(CompY = rgbToI420Kernel31_CompY_Intrin_Aligned_SSSE3);
                    COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbToI420Kernel31_CompY_Asm_X86_Aligned11_SSSE3);
                }
            }
            else if (COMPV_IS_ALIGNED_SSE(outYPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbToI420Kernel31_CompY_Asm_X86_Aligned01_SSSE3);
            }
        } // end-of-SSSE3
    } // end-of-SSE

    if (COMPV_IS_ALIGNED_AVX(stride)) {
        if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
            COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbToI420Kernel31_CompY_Asm_X86_Aligned00_AVX2);
            COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbToI420Kernel31_CompUV_Asm_X86_Aligned000_AVX2);
            if (COMPV_IS_ALIGNED_AVX2(rgbPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbToI420Kernel31_CompY_Asm_X86_Aligned10_AVX2);
                COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbToI420Kernel31_CompUV_Asm_X86_Aligned100_AVX2);
                if (COMPV_IS_ALIGNED_SSE(outYPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(CompY = rgbToI420Kernel31_CompY_Intrin_Aligned_AVX2);
                    COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbToI420Kernel31_CompY_Asm_X86_Aligned11_AVX2);
                }
                if (COMPV_IS_ALIGNED_SSE(outUPtr)) {  // vextractf128 -> SSE align
                    COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbToI420Kernel31_CompUV_Asm_X86_Aligned110_AVX2);
                    if (COMPV_IS_ALIGNED_SSE(outVPtr)) {  // vextractf128 -> SSE align
                        COMPV_EXEC_IFDEF_INTRIN_X86(CompUV = rgbToI420Kernel31_CompUV_Intrin_Aligned_AVX2);
                        COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbToI420Kernel31_CompUV_Asm_X86_Aligned111_AVX2);
                    }
                }
            }
        } // end-of-AVX2
    } // end-of-AVX

    // Compute number of threads
    if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
        threadsCount = threadDip->guessNumThreadsDividingAcrossY(stride, height, COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD);
    }

    // Process Y and UV lines
    if (threadsCount > 1) {
        int32_t rgbIdx = 0, YIdx = 0, UVIdx = 0, threadHeight, totalHeight = 0;
        uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
        for (int32_t i = 0; i < threadsCount; ++i) {
            threadHeight = ((height - totalHeight) / (threadsCount - i)) & -2; // the & -2 is to make sure we'll deal with even heights
            // YUV-rows
            COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX0, ImageConvKernelxx_AsynExec,
                                    COMPV_ASYNCTASK_SET_PARAM_ASISS(
                                        COMPV_IMAGECONV_FUNCID_RGBToI420_YUV,
                                        CompY,
                                        CompUV,
                                        (rgbPtr + rgbIdx),
                                        (outYPtr + YIdx),
                                        outUPtr ? (outUPtr + UVIdx) : NULL,
                                        outVPtr ? (outVPtr + UVIdx) : NULL,
                                        threadHeight,
                                        width,
                                        stride,
                                        kXXXToYUV_YCoeffs8,
                                        kXXXToYUV_UCoeffs8,
                                        kXXXToYUV_VCoeffs8),
                                    COMPV_ASYNCTASK_SET_PARAM_NULL()));
            YIdx += (threadHeight * stride);
            UVIdx += ((threadHeight * stride) >> 2);
            rgbIdx += (threadHeight * stride) * 3;
            totalHeight += threadHeight;
        }
        for (int32_t i = 0; i < threadsCount; ++i) {
            COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX0));
        }
    }
    else {
        CompY(rgbPtr, outYPtr, height, width, stride, kXXXToYUV_YCoeffs8);
        if (outUPtr && outVPtr) {
            CompUV(rgbPtr, outUPtr, outVPtr, height, width, stride, kXXXToYUV_UCoeffs8, kXXXToYUV_VCoeffs8);
        }
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConvRgbaI420::rgbToI420(const CompVObjWrapper<CompVImage* >& rgb, CompVObjWrapper<CompVImage* >& i420)
{
    return __xxxToI420(rgb, i420, kRGBAToYUV_YCoeffs8, kRGBAToYUV_UCoeffs8, kRGBAToYUV_VCoeffs8); // use RGBA coeffs because the smples are converted to RGBA first
}

COMPV_ERROR_CODE CompVImageConvRgbaI420::bgrToI420(const CompVObjWrapper<CompVImage* >& bgr, CompVObjWrapper<CompVImage* >& i420)
{
    return __xxxToI420(bgr, i420, kBGRAToYUV_YCoeffs8, kBGRAToYUV_UCoeffs8, kBGRAToYUV_VCoeffs8); // use BGRA coeffs because the smples are converted to BGRA first
}

// RGBA/ARGB/BGRA/ABGR -> I420
static COMPV_ERROR_CODE __xxxxToI420(const CompVObjWrapper<CompVImage* >& rgba, CompVObjWrapper<CompVImage* >& i420, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXXToYUV_YCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXXToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kXXXXToYUV_VCoeffs8)
{
    // This is a private function, up to the caller to check the input parameters
    // The output format must be I420 or Graysacle
    rgbaToI420Kernel_CompY CompY = rgbaToI420Kernel11_CompY_C;
    rgbaToI420Kernel_CompUV CompUV = rgbaToI420Kernel11_CompUV_C;
    const uint8_t* rgbaPtr = (const uint8_t*)rgba->getDataPtr();
    int height = rgba->getHeight();
    int width = rgba->getWidth();
    int stride = rgba->getStride();
    uint8_t* outYPtr = (uint8_t*)(*i420)->getDataPtr();
	uint8_t* outUPtr = i420->getPixelFormat() == COMPV_PIXEL_FORMAT_GRAYSCALE ? NULL : outYPtr + (height * stride);
	uint8_t* outVPtr = i420->getPixelFormat() == COMPV_PIXEL_FORMAT_GRAYSCALE ? NULL : outUPtr + ((height * stride) >> 2);
    int strideRgbaBytes = (stride << 2); // #20 not SSE aligned but (20*4)=#80 is aligned
    int widthRgbaBytes = (width << 2);
    CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();
    int threadsCount = 1;

    // IS_ALIGNED(strideRgbaBytes, ALIGNV * 4) = IS_ALIGNED(stride * 4, ALIGNV * 4) = IS_ALIGNED(stride, ALIGNV)

    if (COMPV_IS_ALIGNED_SSE(strideRgbaBytes)) {
        if (CompVCpu::isEnabled(kCpuFlagSSSE3)) {
            COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel11_CompY_Asm_X86_Aligned0x_SSSE3);
            COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_SSSE3);
            if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86(CompY = rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3);
                COMPV_EXEC_IFDEF_INTRIN_X86(CompUV = rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3);
                COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3);
                COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_SSSE3);
            }
            if (COMPV_IS_ALIGNED(strideRgbaBytes, 4 * COMPV_SIMD_ALIGNV_SSE)) {
                COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_SSSE3);
                COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned0xx_SSSE3);
                if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(CompY = rgbaToI420Kernel41_CompY_Intrin_Aligned_SSSE3);
                    COMPV_EXEC_IFDEF_INTRIN_X86(CompUV = rgbaToI420Kernel41_CompUV_Intrin_Aligned_SSSE3);
                    COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_SSSE3);
                    COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned1xx_SSSE3);
                    if (COMPV_IS_ALIGNED_SSE(outYPtr)) {
                        COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_SSSE3);
                    }
                }
                else if (COMPV_IS_ALIGNED_SSE(outYPtr)) {
                    COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_SSSE3);
                }
            }
        } // end-of-SSSE3
    } // end-of-SSE

    if (COMPV_IS_ALIGNED_AVX2(strideRgbaBytes)) {
        if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
            COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel11_CompY_Asm_X86_Aligned0_AVX2);
            COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_AVX2);
            if (COMPV_IS_ALIGNED_AVX2(rgbaPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86(CompY = rgbaToI420Kernel11_CompY_Intrin_Aligned_AVX2);
                COMPV_EXEC_IFDEF_INTRIN_X86(CompUV = rgbaToI420Kernel11_CompUV_Intrin_Aligned_AVX2);
                COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel11_CompY_Asm_X86_Aligned1_AVX2);
                COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_AVX2);
            }
            if (COMPV_IS_ALIGNED(strideRgbaBytes, 4 * COMPV_SIMD_ALIGNV_AVX2)) {
                COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_AVX2);
                COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned000_AVX2);
                if (COMPV_IS_ALIGNED_AVX2(rgbaPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(CompY = rgbaToI420Kernel41_CompY_Intrin_Aligned_AVX2);
                    COMPV_EXEC_IFDEF_INTRIN_X86(CompUV = rgbaToI420Kernel41_CompUV_Intrin_Aligned_AVX2);
                    COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_AVX2);
                    COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned100_AVX2);
                    if (COMPV_IS_ALIGNED_AVX2(outYPtr)) {
                        COMPV_EXEC_IFDEF_ASM_X86(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_AVX2);
                    }
                    if (COMPV_IS_ALIGNED_SSE(outUPtr)) {  // vextractf128 -> SSE align
                        COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned110_AVX2);
                        if (COMPV_IS_ALIGNED_SSE(outVPtr)) {  // vextractf128 -> SSE align
                            COMPV_EXEC_IFDEF_ASM_X86(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned111_AVX2);
                        }
                    }
                }
            }
        }
    }

    // Compute number of threads
    if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
        threadsCount = threadDip->guessNumThreadsDividingAcrossY(stride, height, COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD);
    }

    // Process Y and UV lines
    if (threadsCount > 1) {
        int32_t rgbaIdx = 0, YIdx = 0, UVIdx = 0, threadHeight, totalHeight = 0;
        uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
        for (int32_t i = 0; i < threadsCount; ++i) {
            threadHeight = ((height - totalHeight) / (threadsCount - i)) & -2; // the & -2 is to make sure we'll deal with odd heights
            // YUV-rows
            COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX0, ImageConvKernelxx_AsynExec,
                                    COMPV_ASYNCTASK_SET_PARAM_ASISS(
                                        COMPV_IMAGECONV_FUNCID_RGBAToI420_YUV,
                                        CompY,
                                        CompUV,
                                        (rgbaPtr + rgbaIdx),
                                        (outYPtr + YIdx),
                                        outUPtr ? (outUPtr + UVIdx) : NULL,
                                        outVPtr ? (outVPtr + UVIdx) : NULL,
                                        threadHeight,
                                        width,
                                        stride,
                                        kXXXXToYUV_YCoeffs8,
                                        kXXXXToYUV_UCoeffs8,
                                        kXXXXToYUV_VCoeffs8),
                                    COMPV_ASYNCTASK_SET_PARAM_NULL()));
            YIdx += (threadHeight * stride);
            UVIdx += ((threadHeight * stride) >> 2);
            rgbaIdx += (threadHeight * stride) << 2;
            totalHeight += threadHeight;
        }
        for (int32_t i = 0; i < threadsCount; ++i) {
            COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX0));
        }
    }
    else {
        CompY(rgbaPtr, outYPtr, height, width, stride, kXXXXToYUV_YCoeffs8);
        if (outUPtr && outVPtr) {
            CompUV(rgbaPtr, outUPtr, outVPtr, height, width, stride, kXXXXToYUV_UCoeffs8, kXXXXToYUV_VCoeffs8);
        }
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConvRgbaI420::rgbaToI420(const CompVObjWrapper<CompVImage* >& rgba, CompVObjWrapper<CompVImage* >& i420)
{
    return __xxxxToI420(rgba, i420, kRGBAToYUV_YCoeffs8, kRGBAToYUV_UCoeffs8, kRGBAToYUV_VCoeffs8);
}
COMPV_ERROR_CODE CompVImageConvRgbaI420::argbToI420(const CompVObjWrapper<CompVImage* >& argb, CompVObjWrapper<CompVImage* >& i420)
{
    return __xxxxToI420(argb, i420, kARGBToYUV_YCoeffs8, kARGBToYUV_UCoeffs8, kARGBToYUV_VCoeffs8);
}
COMPV_ERROR_CODE CompVImageConvRgbaI420::bgraToI420(const CompVObjWrapper<CompVImage* >& bgra, CompVObjWrapper<CompVImage* >& i420)
{
    return __xxxxToI420(bgra, i420, kBGRAToYUV_YCoeffs8, kBGRAToYUV_UCoeffs8, kBGRAToYUV_VCoeffs8);
}
COMPV_ERROR_CODE CompVImageConvRgbaI420::abgrToI420(const CompVObjWrapper<CompVImage* >& abgr, CompVObjWrapper<CompVImage* >& i420)
{
    return __xxxxToI420(abgr, i420, kABGRToYUV_YCoeffs8, kABGRToYUV_UCoeffs8, kABGRToYUV_VCoeffs8);
}

COMPV_ERROR_CODE CompVImageConvRgbaI420::i420ToRgba(const CompVObjWrapper<CompVImage* >& i420, CompVObjWrapper<CompVImage* >& rgba)
{
    // This is a private function, up to the caller to check the input parameters
    i420ToRGBAKernel toRGBA = i420ToRGBAKernel11_C;
    int32_t height = i420->getHeight();
    int32_t width = i420->getWidth();
    int32_t stride = i420->getStride();
    const uint8_t* yPtr = (const uint8_t*)i420->getDataPtr();
    const uint8_t* uPtr = yPtr + (height * stride);
    const uint8_t* vPtr = uPtr + ((height * stride) >> 2);
    uint8_t* outRgbaPtr = (uint8_t*)rgba->getDataPtr();

    int strideRgbaBytes = (stride << 2);
    int strideYBytes = stride;
    int widthRgbaBytes = (width << 2);
    CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();

    int threadsCount = 1;

    // IS_ALIGNED(strideRgbaBytes, ALIGNV * 4) = IS_ALIGNED(stride * 4, ALIGNV * 4) = IS_ALIGNED(stride, ALIGNV)

#if defined(COMPV_ARCH_X86)

    if (COMPV_IS_ALIGNED_SSE(stride)) {
        if (CompVCpu::isEnabled(kCpuFlagSSSE3)) {
            COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned00_SSSE3);
            if (COMPV_IS_ALIGNED_SSE(yPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned10_SSSE3);
                if (COMPV_IS_ALIGNED_SSE(outRgbaPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(toRGBA = i420ToRGBAKernel11_Intrin_Aligned_SSSE3);
                    COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned11_SSSE3);
                }
            }
            else if (COMPV_IS_ALIGNED_SSE(outRgbaPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned01_SSSE3);
            }
        }
    }
    if (COMPV_IS_ALIGNED_AVX2(stride)) {
        if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
            COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned00_AVX2);
            if (COMPV_IS_ALIGNED_AVX2(yPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned10_AVX2);
                if (COMPV_IS_ALIGNED_AVX2(outRgbaPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(toRGBA = i420ToRGBAKernel11_Intrin_Aligned_AVX2);
                    COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned11_AVX2);
                }
            }
            else if (COMPV_IS_ALIGNED_AVX2(outRgbaPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = i420ToRGBAKernel11_Asm_X86_Aligned01_AVX2);
            }
        }
    }
#endif

    // Compute number of threads
    if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
        threadsCount = threadDip->guessNumThreadsDividingAcrossY(stride, height, COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD);
    }

    if (threadsCount > 1) {
        int32_t rgbaIdx = 0, YIdx = 0, UVIdx = 0, threadHeight, totalHeight = 0;
        uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
        for (int32_t i = 0; i < threadsCount; ++i) {
            threadHeight = ((height - totalHeight) / (threadsCount - i)) & -2; // the & -2 is to make sure we'll deal with odd heights
            COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX0, ImageConvKernelxx_AsynExec,
                                    COMPV_ASYNCTASK_SET_PARAM_ASISS(COMPV_IMAGECONV_FUNCID_I420ToRGBA, toRGBA, (yPtr + YIdx), (uPtr + UVIdx), (vPtr + UVIdx), (outRgbaPtr + rgbaIdx), threadHeight, width, stride),
                                    COMPV_ASYNCTASK_SET_PARAM_NULL()));
            rgbaIdx += (threadHeight * stride) << 2;
            YIdx += (threadHeight * stride);
            UVIdx += ((threadHeight * stride) >> 2);
            totalHeight += threadHeight;
        }
        for (int32_t i = 0; i < threadsCount; ++i) {
            COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX0));
        }
    }
    else {
        toRGBA(yPtr, uPtr, vPtr, outRgbaPtr, height, width, stride);
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
