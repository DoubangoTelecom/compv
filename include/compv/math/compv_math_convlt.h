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
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVMathConvlt
{
public:
	template <typename T>
	struct convltData {
		CompVPtrArray(T) tmp;
	};

	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE initData(convltData<OutputType>&data)
	{
		return COMPV_ERROR_CODE_S_OK;
	}
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
		OutputType* imgTmp = NULL;
		imgTmp = (OutputType*)CompVMem::malloc(neededSize);
		if (!imgTmp) {
			if (outPtrAllocated) {
				CompVMem::free((void**)&outPtr);
			}
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}
		int32_t threadsCount = 1;
		CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
		if (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) {
			threadsCount = static_cast<int32_t>(dataHeight / (kernSize << 1)); // at least "rowsOverlapCount"
			threadsCount = COMPV_MATH_MIN(threadsCount, threadDisp->getThreadsCount());
		}

		if (threadsCount > 1 && kernSize < 20) { // only if overlaping is small
			const size_t rowsOverlapCount = ((kernSize >> 1) << 1); // (kernelRadius times 2)
			const size_t rowsOverlapPad = rowsOverlapCount * dataStride;
			const size_t countAny = (size_t)(dataHeight / threadsCount);
			const size_t countLast = (size_t)countAny + (dataHeight % threadsCount);
			const InputType* inPtr_ = dataPtr;
			OutputType* tmpPtr_ = imgTmp;
			OutputType* outPtr_ = outPtr;
			CompVAsyncTaskIds taskIds;
			taskIds.reserve(threadsCount);
			auto funcPtrFirst = [&](const InputType* ptrIn, OutputType* ptrOut, OutputType* ptrTmp, size_t h) -> COMPV_ERROR_CODE {
				CompVMathConvlt::convlt1Hz<InputType, KernelType, OutputType>(ptrIn, ptrTmp, dataWidth, h + rowsOverlapCount, dataStride, hkernPtr, kernSize);
				CompVMathConvlt::convlt1Vert<OutputType, KernelType, OutputType>(ptrTmp, ptrOut, dataWidth, h + rowsOverlapCount, dataStride, vkernPtr, kernSize, true, false);
				return COMPV_ERROR_CODE_S_OK;
			};
			auto funcPtrOthers = [&](const InputType* ptrIn, OutputType* ptrOut, OutputType* ptrTmp, size_t h, bool last) -> COMPV_ERROR_CODE {
				CompVMathConvlt::convlt1Hz<InputType, KernelType, OutputType>(ptrIn - rowsOverlapPad, ptrTmp - rowsOverlapPad, dataWidth, h + rowsOverlapCount, dataStride, hkernPtr, kernSize);
				CompVMathConvlt::convlt1Vert<OutputType, KernelType, OutputType>(ptrTmp - rowsOverlapPad, ptrOut - rowsOverlapPad, dataWidth, h + rowsOverlapCount, dataStride, vkernPtr, kernSize, false, last);
				return COMPV_ERROR_CODE_S_OK;
			};
			/* first */
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrFirst, inPtr_, outPtr_, tmpPtr_, countAny), taskIds));
			inPtr_ += countAny * dataStride;
			tmpPtr_ += countAny * dataStride;
			outPtr_ += countAny * dataStride;
			/* others */
			for (int32_t threadIdx = 1; threadIdx < threadsCount - 1; ++threadIdx) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtr_, tmpPtr_, countAny, false), taskIds));
				inPtr_ += countAny * dataStride;
				tmpPtr_ += countAny * dataStride;
				outPtr_ += countAny * dataStride;
			}
			/* last */
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtr_, tmpPtr_, countLast, true), taskIds));
			/* wait */
			COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		}
		else {
			CompVMathConvlt::convlt1Hz<InputType, KernelType, OutputType>(dataPtr, imgTmp, dataWidth, dataHeight, dataStride, hkernPtr, kernSize);
			CompVMathConvlt::convlt1Vert<OutputType, KernelType, OutputType>(imgTmp, outPtr, dataWidth, dataHeight, dataStride, vkernPtr, kernSize);
		}

		CompVMem::free((void**)&imgTmp);

		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1Hz(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* hkernPtr, size_t kernSize, bool resetBorders = true)
	{
		size_t ker_size_div2 = (kernSize >> 1);
		size_t imgpad = ((stride - width) + ker_size_div2 + ker_size_div2);
		// Set hz borders to zero
		// We must not accept garbage in the border (coul be used by the calling function -e.g to find the max value for normalization)
		if (resetBorders) {
			OutputType *outPtr0 = outPtr, *outPtr1 = outPtr + (width - ker_size_div2);
			for (size_t row = 0; row < height; ++row) {
				for (size_t col = 0; col < ker_size_div2; ++col) {
					outPtr0[col] = 0, outPtr1[col] = 0;
				}
				outPtr0 += stride;
				outPtr1 += stride;
			}
		}
		// Perform horizontal convolution
		CompVMathConvlt::convlt1VertHz<InputType, KernelType, OutputType>(inPtr, outPtr + ker_size_div2, (size_t)(width - ker_size_div2 - ker_size_div2), height, 1, imgpad, hkernPtr, kernSize);
	}

	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1Vert(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* vkernPtr, size_t kernSize, bool resetTopBorder = true, bool resetBottomBorder = true)
	{
		size_t ker_size_div2 = (kernSize >> 1);
		size_t imgpad = (stride - width);
		// Set top and bottom vert borders to zero
		// We must not accept garbage in the border (coul be used by the calling function -e.g to find the max value for normalization)
		const size_t bSize = (ker_size_div2 * stride) * sizeof(OutputType);
		if (resetTopBorder) {
			CompVMem::zero(outPtr, bSize);
		}
		if (resetBottomBorder) {
			CompVMem::zero(outPtr + ((height - ker_size_div2) * stride), bSize);
		}
		// Perform vertical convolution
		CompVMathConvlt::convlt1VertHz<InputType, KernelType, OutputType>(inPtr, outPtr + (ker_size_div2 * stride), width, (height - ker_size_div2 - ker_size_div2), stride, imgpad, vkernPtr, kernSize);
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
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
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

extern template void CompVMathConvlt::convlt1VertHz(const uint8_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t stride, size_t pad, const int16_t* vhkernPtr, size_t kernSize);
extern template void CompVMathConvlt::convlt1VertHz(const int16_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t stride, size_t pad, const int16_t* vhkernPtr, size_t kernSize);

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_CONVLT_H_ */
