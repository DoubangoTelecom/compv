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
#include "compv/image/compv_imageconv_to_i420.h"
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"

#include "compv/intrinsics/x86/compv_imageconv_to_i420_intrin_sse.h"
#include "compv/intrinsics/x86/compv_imageconv_to_i420_intrin_avx2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_ASM)
extern "C" void rgbaToI420Kernel11_CompY_Asm_X86_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride);
extern "C" void rgbaToI420Kernel41_CompY_ASM_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride);
extern "C" void rgbaToI420Kernel11_CompY_Asm_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride);
extern "C" void rgbaToI420Kernel41_CompY_Asm_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride);
extern "C" void rgbaToI420Kernel11_CompUV_Asm_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t height, size_t width, size_t stride);
extern "C" void rgbaToI420Kernel41_CompUV_Asm_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t height, size_t width, size_t stride);
#endif

COMPV_NAMESPACE_BEGIN()

static void rgbaToI420Kernel11_CompY_C(const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
{
	size_t padSample = (stride - width);
	size_t padRGBA = padSample << 2;
	size_t padY = padSample;
	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			*outYPtr++ = (((33 * rgbaPtr[0]) + (65 * rgbaPtr[1]) + (13 * rgbaPtr[2])) >> 7) + 16;
			rgbaPtr += 4;
		}
		rgbaPtr += padRGBA;
		outYPtr += padY;
	}
}

static void rgbaToI420Kernel11_CompUV_C(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t height, size_t width, size_t stride)
{
	size_t padSample = (stride - width) - (width & 1);
	size_t padRGBA = (padSample << 2) + (stride << 2); // "+ (stride << 2)" -> because one line out of two
	size_t padUV = padSample >> 1;
	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (size_t j = 0; j < height; j += 2) {
		for (size_t i = 0; i < width; i += 2) {
			*outUPtr++ = (((-38 * rgbaPtr[0]) + (-74 * rgbaPtr[1]) + (112 * rgbaPtr[2])) >> 8) + 128;
			*outVPtr++ = ((((112 * rgbaPtr[0]) + (-94 * rgbaPtr[1]) + (-18 * rgbaPtr[2]))) >> 8) + 128;
			rgbaPtr += 8; // 2 * 4
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

typedef void(*rgbaToI420Kernel_CompY)(const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride);
typedef void(*rgbaToI420Kernel_CompUV)(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t height, size_t width, size_t stride);

#if 0
static bool computeColsAndMissedBytes(size_t widthInBytes, size_t strideInBytes, size_t alignementInBytes, size_t &cols, int32_t &colMissedBytes, int32_t &colIgnoredBytes)
{
	if (widthInBytes < alignementInBytes) {
		return false;
	}
	cols = (widthInBytes / alignementInBytes);
	colMissedBytes = (int32_t)(widthInBytes - (cols * alignementInBytes)); // less than "alignementInBytes" which means we should look for a single new col at max
	if (colMissedBytes > 0 && (strideInBytes - widthInBytes) >= alignementInBytes) {
		cols += 1;
		colMissedBytes = 0;
	}
	colIgnoredBytes = (int32_t)(strideInBytes - (cols * alignementInBytes));
	return true;
}

static bool computeRowsAndMissedRows(size_t rowsTotal, size_t rowsPerKernel, size_t &rows, int32_t &rowsMissed)
{
	if (rowsTotal < rowsPerKernel) {
		return false;
	}
	rows = (rowsTotal / rowsPerKernel);
	rowsMissed = (int32_t)(rowsTotal - (rows * rowsPerKernel));
	return true;
}
#endif

void CompVImageConvToI420::fromRGBA(const uint8_t* rgbaPtr, size_t width, size_t height, size_t stride, uint8_t* outYPtr, uint8_t* outUPtr, uint8_t* outVPtr)
{
	rgbaToI420Kernel_CompY CompY = rgbaToI420Kernel11_CompY_C;
	rgbaToI420Kernel_CompUV CompUV = rgbaToI420Kernel11_CompUV_C;
	size_t widthRgbaBytes = (width << 2);

#if defined(COMPV_ARCH_X86)
	if (widthRgbaBytes > COMPV_SIMD_ALIGNV_SSE && COMPV_IS_ALIGNED_SSE(stride)) {
		if (CompVCpu::isSupported(kCpuFlagSSSE3)) {
			COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel11_CompY_Intrin_Unaligned_SSSE3);
			// TODO(dmi): add unaligned versions
			if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
				COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3);
				COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel11_CompUV_Intrin_Aligned_SSSE3);
				COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel11_CompY_Asm_X86_Aligned_SSSE3);
				if (COMPV_IS_ALIGNED(stride, 4 * COMPV_SIMD_ALIGNV_SSE)) {
					COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel41_CompY_Intrin_Aligned_SSSE3);
					COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel41_CompUV_Intrin_Aligned_SSSE3);
					COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_ASM_Aligned_SSSE3);
				}
			}
		} // end-of-SSSE3
	} // end-of-SSE
	if (widthRgbaBytes > COMPV_SIMD_ALIGNV_AVX2 && COMPV_IS_ALIGNED_AVX2(stride)) {
		if (CompVCpu::isSupported(kCpuFlagAVX2)) {
			// TODO(dmi): add unaligned versions
			if (COMPV_IS_ALIGNED_AVX2(rgbaPtr)) {
				COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel11_CompY_Intrin_Aligned_AVX2);
				COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel11_CompUV_Intrin_Aligned_AVX2);
				COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel11_CompY_Asm_Aligned_AVX2);
				COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel11_CompUV_Asm_Aligned_AVX2);
				if (COMPV_IS_ALIGNED(stride, 4 * COMPV_SIMD_ALIGNV_AVX2)) {
					COMPV_EXEC_IFDEF_INTRINSIC(CompY = rgbaToI420Kernel41_CompY_Intrin_Aligned_AVX2);
					COMPV_EXEC_IFDEF_INTRINSIC(CompUV = rgbaToI420Kernel41_CompUV_Intrin_Aligned_AVX2);
					COMPV_EXEC_IFDEF_ASM(CompY = rgbaToI420Kernel41_CompY_Asm_Aligned_AVX2);
					COMPV_EXEC_IFDEF_ASM(CompUV = rgbaToI420Kernel41_CompUV_Asm_Aligned_AVX2);
				}
			}
		}
	}
#endif

	// Process Y and UV lines
	CompY(rgbaPtr, outYPtr, height, width, stride);
	CompUV(rgbaPtr, outUPtr, outVPtr, height, width, stride);
}

COMPV_NAMESPACE_END()
