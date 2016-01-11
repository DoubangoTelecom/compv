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
#include "compv/image/compv_imageconv_rgba_i420.h"
#include "compv/image/compv_imageconv.h"
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_engine.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_mathutils.h"

#include "compv/intrinsics/x86/compv_imageconv_rgba_i420_intrin_sse.h"
#include "compv/intrinsics/x86/compv_imageconv_rgba_i420_intrin_avx2.h"

COMPV_NAMESPACE_BEGIN()

#if defined(COMPV_ARCH_X86) && defined(COMPV_ASM)
extern "C" void rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel11_CompY_Asm_X86_Aligned0x_SSSE3(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_SSSE3(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_SSSE3(const uint8_t* rgbaPtr, COMV_ALIGNED(SSE) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, COMV_ALIGNED(SSE) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

extern "C" void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_SSSE3(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned1xx_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned0xx_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

extern "C" void rgbaToI420Kernel11_CompY_Asm_Aligned0_AVX2(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel11_CompY_Asm_Aligned1_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_Aligned00_AVX2(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_Aligned01_AVX2(const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_Aligned10_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_Aligned11_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);

extern "C" void rgbaToI420Kernel11_CompUV_Asm_Aligned0xx_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel11_CompUV_Asm_Aligned1xx_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_Aligned000_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_Aligned100_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_Aligned110_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_Aligned111_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outUPtr, COMV_ALIGNED(AVX2) uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride);
#endif

static void rgbaToI420Kernel11_CompY_C(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	vcomp_scalar_t padSample = (stride - width);
	vcomp_scalar_t padRGBA = padSample << 2;
	vcomp_scalar_t padY = padSample;
	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			*outYPtr++ = (((33 * rgbaPtr[0]) + (65 * rgbaPtr[1]) + (13 * rgbaPtr[2])) >> 7) + 16;
			rgbaPtr += 4;
		}
		rgbaPtr += padRGBA;
		outYPtr += padY;
	}
}

static void rgbaToI420Kernel11_CompUV_C(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	vcomp_scalar_t i, j, maxI = ((width + 1) & -1), padUV = (stride - maxI) >> 1, padRGBA = ((stride - maxI) + stride) << 2;
	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 2) {
			*outUPtr++ = (((-38 * rgbaPtr[0]) + (-74 * rgbaPtr[1]) + (112 * rgbaPtr[2])) >> 8) + 128;
			*outVPtr++ = ((((112 * rgbaPtr[0]) + (-94 * rgbaPtr[1]) + (-18 * rgbaPtr[2]))) >> 8) + 128;
			rgbaPtr += 8; // 2 * 4
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

static void i420ToRGBAKernel11_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	vcomp_scalar_t padSample = (stride - width);
	vcomp_scalar_t padRGBA = padSample << 2;
	vcomp_scalar_t padY = padSample;
	vcomp_scalar_t padUV = ((padY + 1) >> 1);
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

// FIXME(dmi): move to ThreadDisp class
static int threadDivideAcrossY(int xcount, int ycount, int minSamplesPerThread, int maxThreads)
{
	int divCount = 1;
	for (int div = 2; div <= maxThreads; ++div) {
		divCount = div;
		if ((xcount * (ycount / divCount)) <= minSamplesPerThread) { // we started with the smallest div, which mean largest number of pixs and break the loop when we're below the threshold 
			break;
		}
	}
	return divCount;
}

void CompVImageConvRgbaI420::rgbaToI420(const uint8_t* rgbaPtr, int height, int width, int stride, uint8_t* outYPtr, uint8_t* outUPtr, uint8_t* outVPtr)
{
	rgbaToI420Kernel_CompY CompY = rgbaToI420Kernel11_CompY_C;
	rgbaToI420Kernel_CompUV CompUV = rgbaToI420Kernel11_CompUV_C;
	int strideRgbaBytes = (stride << 2); // #20 not SSE aligned but (20*4)=#80 is aligned
	int widthRgbaBytes = (width << 2);
	CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();

	// IS_ALIGNED(strideRgbaBytes, ALIGNV * 4) = IS_ALIGNED(stride * 4, ALIGNV * 4) = IS_ALIGNED(stride, ALIGNV)

#if defined(COMPV_ARCH_X86)
	if (COMPV_IS_ALIGNED_SSE(strideRgbaBytes)) {
		if (CompVCpu::isSupported(kCpuFlagSSSE3)) {
			COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel11_CompY_Asm_X86_Aligned0x_SSSE3);
			COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_SSSE3);
			if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
				COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3);
				COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3);
				COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3);
				COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_SSSE3);
			}
			if (COMPV_IS_ALIGNED(strideRgbaBytes, 4 * COMPV_SIMD_ALIGNV_SSE)) {
				COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_SSSE3);
				COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned0xx_SSSE3);
				if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
					COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel41_CompY_Intrin_Aligned_SSSE3);
					COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel41_CompUV_Intrin_Aligned_SSSE3);
					COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_SSSE3);
					COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel41_CompUV_Asm_X86_Aligned1xx_SSSE3);
					if (COMPV_IS_ALIGNED_SSE(outYPtr)) {
						COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_SSSE3);
					}
				}
				else if (COMPV_IS_ALIGNED_SSE(outYPtr)) {
					COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_SSSE3);
				}
			}
		} // end-of-SSSE3
	} // end-of-SSE
	
	if (COMPV_IS_ALIGNED_AVX2(strideRgbaBytes)) {
		if (CompVCpu::isSupported(kCpuFlagAVX2)) {
			COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel11_CompY_Asm_Aligned0_AVX2);
			COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel11_CompUV_Asm_Aligned0xx_AVX2);
			if (COMPV_IS_ALIGNED_AVX2(rgbaPtr)) {
				COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel11_CompY_Intrin_Aligned_AVX2);
				COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel11_CompUV_Intrin_Aligned_AVX2);
				COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel11_CompY_Asm_Aligned1_AVX2);
				COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel11_CompUV_Asm_Aligned1xx_AVX2);
			}
			if (COMPV_IS_ALIGNED(strideRgbaBytes, 4 * COMPV_SIMD_ALIGNV_AVX2)) {
				COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_Aligned00_AVX2);
				COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel41_CompUV_Asm_Aligned000_AVX2);
				if (COMPV_IS_ALIGNED_AVX2(rgbaPtr)) {
					COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel41_CompY_Intrin_Aligned_AVX2);
					COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel41_CompUV_Intrin_Aligned_AVX2);
					COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_Aligned10_AVX2);
					COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel41_CompUV_Asm_Aligned100_AVX2);
					if (COMPV_IS_ALIGNED_AVX2(outYPtr)) {
						COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_Aligned11_AVX2);
					}
					if (COMPV_IS_ALIGNED_SSE(outUPtr)) {  // vextractf128 -> SSE align
						COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel41_CompUV_Asm_Aligned110_AVX2);
						if (COMPV_IS_ALIGNED_SSE(outVPtr)) {  // vextractf128 -> SSE align
							COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel41_CompUV_Asm_Aligned111_AVX2);
						}
					}
				}
			}
		}
	}
#endif
	
	// Process Y and UV lines
	if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
		int divCount, rgbaIdx = 0, YIdx = 0, UVIdx = 0, threadHeight, totalHeight = 0;
		divCount = threadDivideAcrossY(stride, height, COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD, threadDip->getThreadsCount());
		uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
		for (int i = 0; i < divCount; ++i) {
			threadHeight = ((height - totalHeight) / (divCount - i)) & -2; // the & -2 is to make sure we'll deal with even heights
			COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT0, ImageConvKernelxx_AsynExec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(COMPV_IMAGECONV_FUNCID_RGBAToI420_Y, CompY, (rgbaPtr + rgbaIdx), (outYPtr + YIdx), threadHeight, width, stride),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
			COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT1, ImageConvKernelxx_AsynExec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(COMPV_IMAGECONV_FUNCID_RGBAToI420_UV, CompUV, (rgbaPtr + rgbaIdx), (outUPtr + UVIdx), (outVPtr + UVIdx), threadHeight, width, stride),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
			rgbaIdx += (threadHeight * stride) << 2;
			YIdx += (threadHeight * stride);
			UVIdx += ((threadHeight * stride) >> 2);
			totalHeight += threadHeight;
		}
		for (int i = 0; i < divCount; ++i) {
			COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT0));
			COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT1));
		}
	}
	else {
		CompY(rgbaPtr, outYPtr, height, width, stride);
		CompUV(rgbaPtr, outUPtr, outVPtr, height, width, stride);
	}
}

void CompVImageConvRgbaI420::i420ToRgba(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, int height, int width, int stride)
{
	i420ToRGBAKernel toRGBA = i420ToRGBAKernel11_C;
	int strideRgbaBytes = (stride << 2);
	int strideYBytes = stride;
	int widthRgbaBytes = (width << 2);
	CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();

	// IS_ALIGNED(strideRgbaBytes, ALIGNV * 4) = IS_ALIGNED(stride * 4, ALIGNV * 4) = IS_ALIGNED(stride, ALIGNV)

#if defined(COMPV_ARCH_X86)

	if (COMPV_IS_ALIGNED_SSE(stride)) {
		if (CompVCpu::isSupported(kCpuFlagSSSE3)) {
			if (COMPV_IS_ALIGNED_SSE(yPtr)) {
				if (COMPV_IS_ALIGNED_SSE(outRgbaPtr)) {
					COMPV_EXEC_IFDEF_INTRINSIC(toRGBA = i420ToRGBAKernel11_Intrin_Aligned_SSSE3);
				}
			}
		}
	}
	if (COMPV_IS_ALIGNED_AVX2(stride)) {
		if (CompVCpu::isSupported(kCpuFlagAVX2)) {
			if (COMPV_IS_ALIGNED_AVX2(yPtr)) {
				if (COMPV_IS_ALIGNED_AVX2(outRgbaPtr)) {
					COMPV_EXEC_IFDEF_INTRINSIC(toRGBA = i420ToRGBAKernel11_Intrin_Aligned_AVX2);
				}
			}
		}
	}
#endif

	if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
		int divCount, rgbaIdx = 0, YIdx = 0, UVIdx = 0, threadHeight, totalHeight = 0;
		divCount = threadDivideAcrossY(stride, height, COMPV_IMAGCONV_MIN_SAMPLES_PER_THREAD, threadDip->getThreadsCount());
		uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
		for (int i = 0; i < divCount; ++i) {
			threadHeight = ((height - totalHeight) / (divCount - i)) & -2; // the & -2 is to make sure we'll deal with even heights
			COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT0, ImageConvKernelxx_AsynExec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(COMPV_IMAGECONV_FUNCID_I420ToRGBA, toRGBA, (yPtr + YIdx), (uPtr + UVIdx), (vPtr + UVIdx), (outRgbaPtr + rgbaIdx), threadHeight, width, stride),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
			rgbaIdx += (threadHeight * stride) << 2;
			YIdx += (threadHeight * stride);
			UVIdx += ((threadHeight * stride) >> 2);
			totalHeight += threadHeight;
		}
		for (int i = 0; i < divCount; ++i) {
			COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX_IMAGE_CONVERT0));
		}
	}
	else {
		toRGBA(yPtr, uPtr, vPtr, outRgbaPtr, height, width, stride);
	}
}

COMPV_NAMESPACE_END()
