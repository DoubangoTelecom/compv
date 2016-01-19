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
#include "compv/image/compv_imageconv_rgba_rgb.h"
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_engine.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_mathutils.h"

#include "compv/intrinsics/x86/image/compv_imageconv_rgba_rgb_intrin_sse.h"
#include "compv/intrinsics/x86/image/compv_imageconv_rgba_rgb_intrin_avx2.h"

COMPV_NAMESPACE_BEGIN()

#if defined(COMPV_ARCH_X86) && defined(COMPV_ASM)
// SSSE3
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned00_SSSE3(const uint8_t* rgb, uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned01_SSSE3(const uint8_t* rgb, COMV_ALIGNED(SSE) uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned10_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgb, uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned11_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgb, COMV_ALIGNED(SSE) uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

// AVX2
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2(const uint8_t* rgb, uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2(const uint8_t* rgb, COMV_ALIGNED(SSE) uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2(COMV_ALIGNED(SSE) const uint8_t* rgb, uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2(COMV_ALIGNED(SSE) const uint8_t* rgb, COMV_ALIGNED(SSE) uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_ASM) */

static void __rgbToRgbaKernel11_C(const uint8_t* rgb, uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
    vcomp_scalar_t padRGBA = (stride - width) << 2;
    vcomp_scalar_t padRGB = (stride - width) * 3;
    for (vcomp_scalar_t j = 0; j < height; ++j) {
        for (vcomp_scalar_t i = 0; i < width; ++i) {
            rgba[0] = rgb[0];
            rgba[1] = rgb[1];
            rgba[2] = rgb[2];
            rgba[3] = 0xFF;

            rgba += 4;
            rgb += 3;
        }
        rgba += padRGBA;
        rgb += padRGB;
    }
}

COMPV_ERROR_CODE CompVImageConvRgbaRgb::rgbToRgba(const CompVObjWrapper<CompVImage* >& rgb, CompVObjWrapper<CompVImage* >& rgba)
{
    // This is a private function, up to the caller to check the input parameters
    rgbToRgbaKernel toRGBA = __rgbToRgbaKernel11_C;
    const uint8_t* rgbPtr = (const uint8_t*)rgb->getDataPtr();
    uint8_t* rgbaPtr = (uint8_t*)rgba->getDataPtr();
    int height = rgb->getHeight();
    int width = rgb->getWidth();
    int stride = rgb->getStride();
    CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();

    if (COMPV_IS_ALIGNED_SSE(stride)) {
        if (CompVCpu::isSupported(kCpuFlagSSSE3)) {
            COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned00_SSSE3);
            if (COMPV_IS_ALIGNED_SSE(rgbPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned10_SSSE3);
                if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(toRGBA = rgbToRgbaKernel31_Intrin_Aligned_SSSE3);
                    COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned11_SSSE3);
                }
            }
            else if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned01_SSSE3);
            }
        }
    }

    if (COMPV_IS_ALIGNED_AVX2(stride)) {
        if (CompVCpu::isSupported(kCpuFlagAVX2)) {
            COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2);
            if (COMPV_IS_ALIGNED_AVX2(rgbPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2);
                if (COMPV_IS_ALIGNED_AVX2(rgbaPtr)) {
                    COMPV_EXEC_IFDEF_INTRIN_X86(toRGBA = rgbToRgbaKernel31_Intrin_Aligned_AVX2);
                    COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2);
                }
            }
            else if (COMPV_IS_ALIGNED_AVX2(rgbaPtr)) {
                COMPV_EXEC_IFDEF_ASM_X86(toRGBA = rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2);
            }
        }
    }

    // Process
    if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
        int divCount, rgbIdx = 0, rgbaIdx = 0, threadHeight, totalHeight = 0;
        // RGB<->RGBA is a memcpy operation and not CPU demanding -> use more samples per thread
        static const int minSamplesPerThread = COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD << 2;
        divCount = threadDivideAcrossY(stride, height, minSamplesPerThread, threadDip->getThreadsCount());
        uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
        for (int i = 0; i < divCount; ++i) {
            threadHeight = ((height - totalHeight) / (divCount - i)) & -2; // the & -2 is to make sure we'll deal with even heights
            COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT0, ImageConvKernelxx_AsynExec,
                                    COMPV_ASYNCTASK_SET_PARAM_ASISS(COMPV_IMAGECONV_FUNCID_RGBToRGBA, toRGBA, (rgbPtr + rgbIdx), (rgbaPtr + rgbaIdx), threadHeight, width, stride),
                                    COMPV_ASYNCTASK_SET_PARAM_NULL()));
            rgbIdx += (threadHeight * stride) * 3;
            rgbaIdx += (threadHeight * stride) << 2;
            totalHeight += threadHeight;
        }
        for (int i = 0; i < divCount; ++i) {
            COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT0));
        }
    }
    else {
        toRGBA(rgbPtr, rgbaPtr, height, width, stride);
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageConvRgbaRgb::bgrToBgra(const CompVObjWrapper<CompVImage* >& bgr, CompVObjWrapper<CompVImage* >& bgra)
{
    // This is a private function, up to the caller to check the input parameters

    // the alpha channel is at the same index as rgb->rgba which means we can use the same function
    return CompVImageConvRgbaRgb::rgbToRgba(bgr, bgra);
}

COMPV_NAMESPACE_END()
