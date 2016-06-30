/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_CONVLT_H_)
#define _COMPV_MATH_CONVLT_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVMathConvlt
{
public:
	// Convolution using separable kernel
	// sizeof(outPtr) must be computed using CompVMathConvlt::outputSizeInBytes()
	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE convlt1(const InputType* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const KernelType* vkernPtr, const KernelType* hkernPtr, size_t kernSize, OutputType*& outPtr, size_t dataBorder = 0)
	{
		// Check inputs
		COMPV_CHECK_EXP_RETURN(!dataPtr || (dataWidth < kernSize * 2) || (dataHeight < kernSize * 2) || (dataStride < dataWidth) || !vkernPtr || !hkernPtr || dataBorder < 0 || !(kernSize & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// The realloc_aligned() implementation memcpy() old data which is slow. Prefer, free_aligned() followed by malloc_aligned()

		/* Alloc memory */
		size_t neededSize = CompVMathConvlt::outputSizeInBytes<OutputType>(dataStride, dataHeight, dataBorder);
		bool outPtrAllocated = false;
		if (!outPtr) {
			outPtr = (OutputType*)CompVMem::malloc(neededSize);
			COMPV_CHECK_EXP_RETURN(!outPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			outPtrAllocated = true;
		}
		// Allocate tmp memory
		OutputType* imgTmp0 = NULL;
		imgTmp0 = (OutputType*)CompVMem::malloc(neededSize);
		if (!imgTmp0) {
			if (outPtrAllocated) {
				CompVMem::free((void**)&outPtr);
			}
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}

		OutputType *imgTmp, *imgOut, *imgPtr;
		size_t imgpad;
		size_t ker_size_div2 = kernSize >> 1;
		int start_margin = (int)(dataBorder >= ker_size_div2) ? -(int)ker_size_div2 : -(int)dataBorder;
		int start_center = (int)(start_margin + ker_size_div2);

		// We must not accept garbage in the border (coul be used by the calling function -e.g to find the max value for normalization)
		if (dataBorder < ker_size_div2) {
			// Set hz borders to zero
			OutputType *outPtr0 = outPtr, *outPtr1 = outPtr + (dataWidth - ker_size_div2);
			for (size_t row = 0; row < dataHeight; ++row) {
				for (size_t col = 0; col < ker_size_div2; ++col) {
					outPtr0[col] = 0, outPtr1[col] = 0;
				}
				outPtr0 += dataStride;
				outPtr1 += dataStride;
			}
			// Set vert borders to zero
			outPtr0 = outPtr;
			outPtr1 = outPtr + ((dataHeight - ker_size_div2) * dataStride);
			size_t bSize = (ker_size_div2 * dataStride) * sizeof(OutputType);
			CompVMem::zero(outPtr, bSize);
			CompVMem::zero(outPtr1, bSize);
		}

		imgTmp = imgTmp0 + (dataBorder * dataStride) + dataBorder;
		imgOut = outPtr + (dataBorder * dataStride) + dataBorder;

		/* Horizontal */
		const InputType *topleft0 = dataPtr + start_margin;
		imgpad = (size_t)((dataStride - dataWidth) + start_center + start_center);
		imgPtr = imgTmp + start_center;
		CompVMathConvlt::convlt1VertHz<InputType, KernelType, OutputType>(topleft0, imgPtr, (size_t)(dataWidth - start_center - start_center), dataHeight, /*stride*/1, imgpad, hkernPtr, kernSize);

		/* Vertical */
		const OutputType* topleft1 = imgTmp + (start_margin * dataStride); // output from hz filtering is now used as input
		imgpad = (dataStride - dataWidth);
		imgPtr = imgOut + (start_center * dataStride);
		CompVMathConvlt::convlt1VertHz<OutputType, KernelType, OutputType>(topleft1, imgPtr, dataWidth, (size_t)(dataHeight - start_center - start_center), dataStride, imgpad, vkernPtr, kernSize);

		CompVMem::free((void**)&imgTmp0);

		return COMPV_ERROR_CODE_S_OK;
	}
	
	// Convolution using no separable kernel
	// sizeof(outPtr) must be at least equal to (dataHeight * dataStride)
	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE convlt2(const InputType* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const KernelType* kernPtr, size_t kernSize, OutputType* &outPtr, size_t dataBorder = 0)
	{
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename OutputType>
	static size_t outputSizeInBytes(size_t dataStride, size_t dataHeight, size_t dataBorder = 0)
	{
		return ((dataHeight + (dataBorder << 1)) * (dataStride + (dataBorder << 1))) * sizeof(OutputType);
	}

private:
	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1VertHz(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, size_t pad, const KernelType* vhkernPtr, size_t kernSize)
	{
		int minpack = 0; // Minimum number of pixels the function can handle for each operation (must be pof 2)
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

		// Check missed pixels
		if (minpack > 0) {
			int missed = (width & (minpack - 1));
			if (missed == 0) {
				return;
			}
			inPtr += (width - missed);
			outPtr += (width - missed);
			pad += (width - missed);
			width = missed;
		}
		else {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
		}

		CompVMathConvlt::convlt1VertHz_C<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, stride, pad, vhkernPtr, kernSize);
	}

	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1VertHz_C(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, size_t pad, const KernelType* vhkernPtr, size_t kernSize)
	{
		size_t i, j, row;
		OutputType sum;
		const InputType *ptr_;

		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = 0;
				ptr_ = inPtr;
				for (row = 0; row < kernSize; ++row) {
					sum += OutputType(*ptr_ * vhkernPtr[row]);
					ptr_ += stride;
				}
				*outPtr = OutputType(sum);
				++inPtr;
				++outPtr;
			}
			inPtr += pad;
			outPtr += pad;
		}
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_CONVLT_H_ */
