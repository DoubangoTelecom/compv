/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_CONVLT_H_)
#define _COMPV_BASE_MATH_CONVLT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_mat.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/parallel/compv_parallel.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathConvlt
{
public:
	// Convolution using separable kernel
	// sizeof(outPtr) should be computed using CompVMathConvlt::outputSizeInBytes()
	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt1(const InputType* dataPtr, size_t dataWidth, size_t dataHeight, size_t dataStride, const KernelType* vtKernPtr, const KernelType* hzKernPtr, size_t kernSize, OutputType*& outPtr, size_t dataBorder = 0) {
		return CompVMathConvlt::convlt1_private<InputType, KernelType, OutputType>(dataPtr, dataWidth, dataHeight, dataStride, vtKernPtr, hzKernPtr, kernSize, outPtr, dataBorder, false);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	static COMPV_ERROR_CODE convlt1FixedPoint(const uint8_t* dataPtr, size_t dataWidth, size_t dataHeight, size_t dataStride, const uint16_t* vtKernPtr, const uint16_t* hzKernPtr, size_t kernSize, uint8_t*& outPtr, size_t dataBorder = 0) {
		return CompVMathConvlt::convlt1_private<uint8_t, uint16_t, uint8_t>(dataPtr, dataWidth, dataHeight, dataStride, vtKernPtr, hzKernPtr, kernSize, outPtr, dataBorder, true);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static void convlt1Hz(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* hzKernPtr, size_t kernSize, bool resetBorders = true) {
		CompVMathConvlt::convlt1Hz_private<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, stride, hzKernPtr, kernSize, resetBorders, false);
	}

	// *yes* arithmetic overflow check (up to the caller to normalize the data or use any trick)
	static void convlt1HzFixedPoint(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t stride, const uint16_t* hzKernPtr, size_t kernSize, bool resetBorders = true) {
		CompVMathConvlt::convlt1Hz_private<uint8_t, uint16_t, uint8_t>(inPtr, outPtr, width, height, stride, hzKernPtr, kernSize, resetBorders, true);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static void convlt1Vt(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* vtKernPtr, size_t kernSize, bool resetTopBorder = true, bool resetBottomBorder = true) {
		CompVMathConvlt::convlt1Vt_private<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, stride, vtKernPtr, kernSize, resetTopBorder, resetBottomBorder, false);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	static void convlt1VtFixedPoint(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t stride, const uint16_t* vtKernPtr, size_t kernSize, bool resetTopBorder = true, bool resetBottomBorder = true) {
		CompVMathConvlt::convlt1Vt_private<uint8_t, uint16_t, uint8_t>(inPtr, outPtr, width, height, stride, vtKernPtr, kernSize, resetTopBorder, resetBottomBorder, false);
	}

	// Convolution using no separable kernel
	// sizeof(outPtr) should be at least equal to (dataHeight * dataStride)
	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt2(const InputType* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const KernelType* kernPtr, size_t kernSize, OutputType** outPtr, size_t dataBorder = 0) {
		return CompVMathConvlt::convlt2_private<InputType, KernelType, OutputType>(dataPtr, dataWidth, dataStride, dataHeight, kernPtr, kernSize, outPtr, dataBorder, false);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	static COMPV_ERROR_CODE convlt2FixedPoint(const uint8_t* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const uint16_t* kernPtr, size_t kernSize, uint8_t** outPtr, size_t dataBorder = 0) {
		return CompVMathConvlt::convlt2_private<uint8_t, uint16_t, uint8_t>(dataPtr, dataWidth, dataStride, dataHeight, kernPtr, kernSize, outPtr, dataBorder, false);
	}

	template <typename OutputType = uint8_t>
	static size_t outputSizeInBytes(size_t dataStride, size_t dataHeight, size_t dataBorder = 0) {
		return ((dataHeight + (dataBorder << 1)) * (dataStride + (dataBorder << 1))) * sizeof(OutputType);
	}

	// kernel should be normalized and must be > 0
	template <typename KernelType = compv_float32_t>
	static COMPV_ERROR_CODE fixedPointKernel(CompVMatPtr normalizedKernel, CompVMatPtrPtr fixedPointKernel) {
		COMPV_CHECK_EXP_RETURN(!normalizedKernel || !fixedPointKernel, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<uint16_t>(fixedPointKernel, normalizedKernel->rows(), normalizedKernel->cols())));
		const KernelType* aPtr;
		uint16_t* bPtr;
		size_t row, col;
		for (row = 0; row < normalizedKernel->rows(); ++row) {
			aPtr = normalizedKernel->ptr<const KernelType>();
			bPtr = (*fixedPointKernel)->ptr<uint16_t>();
			for (col = 0; col < normalizedKernel->cols(); ++col) {
				COMPV_CHECK_EXP_RETURN(aPtr[col] < 0, COMPV_ERROR_CODE_E_INVALID_CALL, "Kernel values for fixedpoint convolution must be > 0");
				bPtr[col] = static_cast<uint16_t>(aPtr[col] * 0xffff);
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	// Convolution using separable kernel
	// sizeof(outPtr) must be computed using CompVMathConvlt::outputSizeInBytes()
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt1_private(const InputType* dataPtr, size_t dataWidth, size_t dataHeight, size_t dataStride, const KernelType* vtKernPtr, const KernelType* hzKernPtr, size_t kernSize, OutputType*& outPtr, size_t dataBorder = 0, bool fixedPoint = false) {
		// Check inputs
		COMPV_CHECK_EXP_RETURN(!dataPtr || (dataWidth < kernSize * 2) || (dataHeight < kernSize * 2) || (dataStride < dataWidth) || !vtKernPtr || !hzKernPtr || dataBorder < 0 || !(kernSize & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// The realloc_aligned() implementation memcpy() old data which is slow. Prefer, free_aligned() followed by malloc_aligned()

		/* Alloc memory */
		size_t neededSize = CompVMathConvlt::outputSizeInBytes<OutputType>(dataStride, dataHeight, dataBorder);
		bool outPtrAllocated = false;
		if (!outPtr) {
			outPtr = reinterpret_cast<OutputType*>(CompVMem::malloc(neededSize));
			COMPV_CHECK_EXP_RETURN(!outPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			outPtrAllocated = true;
		}
		OutputType* tmpPtr = NULL;
		size_t threadsCount;
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

		// Compute number of threads
		threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
			? COMPV_MATH_MIN(maxThreads, (dataHeight / (kernSize << 1))) /* at least "rowsOverlapCount" */
			: 1;

		// Check if if overlaping is small
		const bool bKernelSizeTooHighForMT = kernSize > (static_cast<size_t>(dataHeight / threadsCount) >> 1);
		if (bKernelSizeTooHighForMT) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Kernel size too high for MT");
		}

		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		
		if (threadsCount > 1 && !bKernelSizeTooHighForMT) {
			CompVAsyncTaskIds taskIds;
			const size_t rowsOverlapCount = ((kernSize >> 1) << 1); // (kernelRadius times 2)
			const size_t rowsOverlapPad = rowsOverlapCount * dataStride;
			const size_t countAny = dataHeight / threadsCount;
			const size_t countLast = dataHeight - ((threadsCount - 1) * countAny);
			const size_t countAnyTimesStride = countAny * dataStride;
			const InputType* inPtr_ = dataPtr;
			OutputType* outPtr_ = outPtr;
			taskIds.reserve(threadsCount);
			auto funcPtr = [&](const InputType* ptrIn, OutputType* ptrOut, size_t h, size_t threadIdx) -> COMPV_ERROR_CODE {
				OutputType* imgTmp = reinterpret_cast<OutputType*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<OutputType>(dataStride, h + rowsOverlapCount)));
				COMPV_CHECK_EXP_RETURN(!imgTmp, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to alloc imgTmp");
				const bool first = (threadIdx == 0);
				const bool last = (threadIdx == (threadsCount - 1));
				const size_t padding = first ? 0 : rowsOverlapPad;
				CompVMathConvlt::convlt1Hz_private<InputType, KernelType, OutputType>(ptrIn - padding, imgTmp, dataWidth, h + rowsOverlapCount, dataStride, hzKernPtr, kernSize, true, fixedPoint);
				CompVMathConvlt::convlt1Vt_private<OutputType, KernelType, OutputType>(imgTmp, ptrOut - padding, dataWidth, h + rowsOverlapCount, dataStride, vtKernPtr, kernSize, first, last, fixedPoint);
				CompVMem::free((void**)&imgTmp);
				return COMPV_ERROR_CODE_S_OK;
			};
			// execute
			for (size_t threadIdx = 0, index = 0; threadIdx < threadsCount; ++threadIdx, index += countAnyTimesStride) {
				COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtr, &inPtr_[index], &outPtr_[index],
					(threadIdx == (threadsCount - 1)) ? countLast : countAny, threadIdx),
					taskIds));
			}
			COMPV_CHECK_CODE_BAIL(err = threadDisp->wait(taskIds));
		}
		else {
			tmpPtr = reinterpret_cast<OutputType*>(CompVMem::malloc(neededSize));
			COMPV_CHECK_EXP_BAIL(!tmpPtr, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to allocate temporary memory");
			CompVMathConvlt::convlt1Hz_private<InputType, KernelType, OutputType>(dataPtr, tmpPtr, dataWidth, dataHeight, dataStride, hzKernPtr, kernSize, true, fixedPoint);
			CompVMathConvlt::convlt1Vt_private<OutputType, KernelType, OutputType>(tmpPtr, outPtr, dataWidth, dataHeight, dataStride, vtKernPtr, kernSize, true, true, fixedPoint);
		}

	bail:
		CompVMem::free(reinterpret_cast<void**>(&tmpPtr));
		if (outPtrAllocated && COMPV_ERROR_CODE_IS_NOK(err)) {
			CompVMem::free(reinterpret_cast<void**>(&outPtr));
		}
		return err;
	}

	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static void convlt1Hz_private(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* hzKernPtr, size_t kernSize, bool resetBorders = true, bool fixedPoint = false) {
		size_t ker_size_div2 = (kernSize >> 1);
		size_t imgpad = ((stride - width) + ker_size_div2 + ker_size_div2);
		// Set hz borders to zero
		// We must not accept garbage in the border (could be used by the calling function -e.g to find the max value for normalization)
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
		CompVMathConvlt::convlt1VtHz_private<InputType, KernelType, OutputType>(inPtr, outPtr + ker_size_div2, static_cast<size_t>(width - ker_size_div2 - ker_size_div2), height, 1, imgpad, hzKernPtr, kernSize, fixedPoint);
	}

	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static void convlt1Vt_private(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* vtKernPtr, size_t kernSize, bool resetTopBorder = true, bool resetBottomBorder = true, bool fixedPoint = false) {
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
		CompVMathConvlt::convlt1VtHz_private<InputType, KernelType, OutputType>(inPtr, outPtr + (ker_size_div2 * stride), width, (height - ker_size_div2 - ker_size_div2), stride, imgpad, vtKernPtr, kernSize, fixedPoint);
	}

	// Convolution using no separable kernel
	// sizeof(outPtr) must be at least equal to (dataHeight * dataStride)
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt2_private(const InputType* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const KernelType* kernPtr, size_t kernSize, OutputType** outPtr, size_t dataBorder = 0, bool fixedPoint = false) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED); // see deprecated code
		return COMPV_ERROR_CODE_S_OK;
	}


	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1VtHz_private(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize, bool fixedPoint = false) {
		if (fixedPoint) {
			convlt1VtHz_private_fxp_true<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize);
		}
		else {
			convlt1VtHz_private_fxp_false<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize);
		}
	}

	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1VtHz_private_fxp_true(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		// No need to check InputType, KernelType and OutputType
		CompVMathConvlt::convlt1VtHzFixedPoint_C(reinterpret_cast<const uint8_t*>(inPtr), reinterpret_cast<uint8_t*>(outPtr), width, height, step, pad, reinterpret_cast<const uint16_t*>(vthzKernPtr), kernSize);
	}	

	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1VtHz_private_fxp_false(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		if ((std::is_same<KernelType, compv_float32_t>::value || std::is_same<KernelType, compv_float64_t>::value)
			&& (std::is_same<InputType, int32_t>::value || std::is_same<InputType, uint32_t>::value || std::is_same<InputType, int16_t>::value || std::is_same<InputType, uint16_t>::value || std::is_same<InputType, int8_t>::value || std::is_same<InputType, uint8_t>::value)
			&& (std::is_same<OutputType, int32_t>::value || std::is_same<OutputType, uint32_t>::value || std::is_same<OutputType, int16_t>::value || std::is_same<OutputType, uint16_t>::value || std::is_same<OutputType, int8_t>::value || std::is_same<OutputType, uint8_t>::value))
		{
			CompVMathConvlt::convlt1VtHzKernelFloat_C<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize);
		}
		else {
			CompVMathConvlt::convlt1VtHzKernelInt_C<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize);
		}
	}

	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1VtHzKernelInt_C(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		size_t i, j, k, row;
		OutputType sum;
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = static_cast<OutputType>(inPtr[0] * vthzKernPtr[0]);
				for (row = 1, k = step; row < kernSize; ++row, k += step) {
					sum += static_cast<OutputType>(inPtr[k] * vthzKernPtr[row]);
				}
				*outPtr = static_cast<OutputType>(sum);
				++inPtr;
				++outPtr;
			}
			inPtr += pad;
			outPtr += pad;
		}
	}

	// inPtr = (u)int8_t / (u)int16_t / (u)int32_t
	// outPtr = (u)int8_t / (u)int16_t / (u)int32_t
	// KernelType = float / double
	template <typename InputType, typename KernelType, typename OutputType>
	static void convlt1VtHzKernelFloat_C(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		// KernelType is float32 or float64
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		size_t i, j, k, row;
		KernelType sum; // use (float / double) to accumulate
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = inPtr[0] * vthzKernPtr[0];
				for (row = 1, k = step; row < kernSize; ++row, k += step) {
					sum += inPtr[k] * vthzKernPtr[row];
				}
#if 0 // SIMD instruction -> out = cvtt(add(sum, 0.5f))
				*outPtr = COMPV_MATH_ROUNDFU_2_NEAREST_INT(sum, OutputType);
#else // SIMD instruction -> out = cvtt(sum)
				*outPtr = static_cast<OutputType>(sum); // Truncation introduce very small error, not a big deal for convolution operation
#endif
				++inPtr;
				++outPtr;
			}
			inPtr += pad;
			outPtr += pad;
		}
	}
	
	static void convlt1VtHzFixedPoint_C(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const uint16_t* vthzKernPtr, size_t kernSize) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		size_t i, j, k, row;
		unsigned int sum;

		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = static_cast<unsigned int>(inPtr[0] * vthzKernPtr[0]) >> 16;
				for (row = 1, k = step; row < kernSize; ++row, k += step) {
					sum += static_cast<unsigned int>(inPtr[k] * vthzKernPtr[row]) >> 16;
				}
				*outPtr = static_cast<uint8_t>(sum);
				++inPtr;
				++outPtr;
			}
			inPtr += pad;
			outPtr += pad;
		}
	}
};

// InputType = uint8_t, KernelType = int16_t, OutputType = uint8_t, FixedPoint = true
COMPV_TEMPLATE_EXTERN COMPV_BASE_API void CompVMathConvlt::convlt1VtHz_private_fxp_true(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const uint16_t* vthzKernPtr, size_t kernSize);

// InputType = uint8_t, KernelType = compv_float32_t, OutputType = uint8_t, FixedPoint = false
COMPV_TEMPLATE_EXTERN COMPV_BASE_API void CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize);

// InputType = uint8_t, KernelType = int16_t, OutputType = int16_t, FixedPoint = false
COMPV_TEMPLATE_EXTERN COMPV_BASE_API void CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const int16_t* vthzKernPtr, size_t kernSize);

// InputType = int16_t, KernelType = int16_t, OutputType = int16_t, FixedPoint = false
COMPV_TEMPLATE_EXTERN COMPV_BASE_API void CompVMathConvlt::convlt1VtHz_private_fxp_false(const int16_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const int16_t* vthzKernPtr, size_t kernSize);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_CONVLT_H_ */
