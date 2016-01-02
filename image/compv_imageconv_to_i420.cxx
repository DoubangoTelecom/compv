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

#include "compv/intrinsics/x86/compv_imageconv_to_i420_intrin_sse.h"

COMPV_NAMESPACE_BEGIN()

static void rgbaToI420Kernel11_CompY_C(const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t stride, size_t rows, size_t cols)
{
	// colStep=1, rowStep=1
	size_t padRGBA = ((stride - cols) << 2); // (stride - cols) * 4 * colStep
	size_t padY = (stride - cols); // (stride - cols) * 1 * colStep
	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (size_t j = 0; j < rows; ++j) {
		for (size_t i = 0; i < cols; ++i) {
			*outYPtr++ = (((33 * rgbaPtr[0]) + (65 * rgbaPtr[1]) + (13 * rgbaPtr[2])) >> 7) + 16;
			rgbaPtr += 4;
		}
		rgbaPtr += padRGBA;
		outYPtr += padY;
	}
}

static void rgbaToI420Kernel11_CompUV_C(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t stride, size_t rows, size_t cols)
{
	// colStep=1, rowStep=1
	size_t padRGBA = ((stride - cols) << 2) + (stride << 2); // ((stride - cols) * 4 * colStep) + (stride * 4)
	size_t padUV = (stride - cols) << 2; // (stride - cols) * 1 * colStep
	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (size_t j = 0; j < rows; j += 2) {
		for (size_t i = 0; i < cols; i += 2) {
			*outUPtr++ = (((-38 * rgbaPtr[0]) + (-74 * rgbaPtr[1]) + (112 * rgbaPtr[2])) >> 8) + 128;
			*outVPtr++ = ((((112 * rgbaPtr[0]) + (-94 * rgbaPtr[1]) + (-18 * rgbaPtr[2]))) >> 8) + 128;
			rgbaPtr += 8; // 2 * 4
		}
		rgbaPtr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

void CompVImageConvToI420::fromRGBA(const uint8_t* rgbaPtr, size_t width, size_t height, size_t stride, uint8_t* outYPtr, uint8_t* outUPtr, uint8_t* outVPtr)
{
	void(*rgbaToI420Kernel_CompY)(const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t stride, size_t rows, size_t cols) = rgbaToI420Kernel11_CompY_C;
	void(*rgbaToI420Kernel_CompUV)(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t stride, size_t rows, size_t cols) = rgbaToI420Kernel11_CompUV_C;
	const uint8_t* _rgbaPtr = rgbaPtr;
	size_t colMissedSamples = 0;
	size_t cols = width, rows = height, rowStep = 1, colStep = 1;



#if COMPV_ARCH_X86
	if (CompVCpu::isSupported(kCpuFlagSSSE3) && width >= COMPV_SIMD_ALIGNV_SSE) {
		colStep = 4;
		COMPV_SET_IFDEF_INTRINSIC(rgbaToI420Kernel_CompY, rgbaToI420Kernel11_CompY_Intrin_Unaligned_SSSE3);
		if (COMPV_IS_ALIGNED_SSE(rgbaPtr)) {
			COMPV_SET_IFDEF_INTRINSIC(rgbaToI420Kernel_CompY, rgbaToI420Kernel11_CompY_Intrin_Aligned_SSSE3);
			if (COMPV_IS_ALIGNED_SSE(outYPtr)) {

			}
			else {

			}
			if (COMPV_IS_ALIGNED_SSE(outUPtr) && COMPV_IS_ALIGNED_SSE(outVPtr)) {

			}
			else {

			}
		}
	} // end-of-SSSE3
#endif

	// Compute "cols", "rows" and the "colMissedSamples"
	cols = width / colStep;
	rows = height / rowStep;
	if (!COMPV_IS_ALIGNED(width, colStep) && stride > width) {
		cols += ((stride - width) + (width - (cols * colStep))) / colStep;
		colMissedSamples = (width - (cols * colStep));
	}

	// Process Y and UV lines
	rgbaToI420Kernel_CompY(rgbaPtr, outYPtr, stride, rows, cols);
	//rgbaToI420Kernel_CompUV(rgbaPtr, outUPtr, outVPtr, stride, rows, cols);

	// Now process mised samples
	if (colMissedSamples != 0) {
		size_t colCatchedSamples = (width - colMissedSamples);
		const uint8_t* rgbaPtr_ = rgbaPtr + (4 * colCatchedSamples);
		uint8_t* outYPtr_ = outYPtr + colCatchedSamples;
		uint8_t* outUPtr_ = outUPtr + (colCatchedSamples >> 1);
		uint8_t* outVPtr_ = outVPtr + (colCatchedSamples >> 1);

		rgbaToI420Kernel11_CompY_C(rgbaPtr_, outYPtr_, stride, rows, colMissedSamples);
		rgbaToI420Kernel11_CompUV_C(rgbaPtr_, outUPtr_, outVPtr_, stride, rows, colMissedSamples);
	}
}

COMPV_NAMESPACE_END()
