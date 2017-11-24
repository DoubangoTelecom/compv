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
	static COMPV_ERROR_CODE convlt1(const InputType* dataPtr, size_t dataWidth, size_t dataHeight, size_t dataStride, const KernelType* vtKernPtr, const KernelType* hzKernPtr, size_t kernSize, OutputType*& outPtr, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt1_private<InputType, KernelType, OutputType>(dataPtr, dataWidth, dataHeight, dataStride, vtKernPtr, hzKernPtr, kernSize, outPtr, borderType, false);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick).
	static COMPV_ERROR_CODE convlt1FixedPoint(const uint8_t* dataPtr, size_t dataWidth, size_t dataHeight, size_t dataStride, const uint16_t* vtKernPtr, const uint16_t* hzKernPtr, size_t kernSize, uint8_t*& outPtr, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt1_private<uint8_t, uint16_t, uint8_t>(dataPtr, dataWidth, dataHeight, dataStride, vtKernPtr, hzKernPtr, kernSize, outPtr, borderType, true);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick).
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt1Hz(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* hzKernPtr, size_t kernSize, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt1Hz_private<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, stride, hzKernPtr, kernSize, borderType, false);
	}

	// *yes* arithmetic overflow check (up to the caller to normalize the data or use any trick).
	static COMPV_ERROR_CODE convlt1HzFixedPoint(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t stride, const uint16_t* hzKernPtr, size_t kernSize, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt1Hz_private<uint8_t, uint16_t, uint8_t>(inPtr, outPtr, width, height, stride, hzKernPtr, kernSize, borderType, true);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick).
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt1Vt(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* vtKernPtr, size_t kernSize, COMPV_BORDER_TYPE topBorderType = COMPV_BORDER_TYPE_ZERO, COMPV_BORDER_TYPE bottomBorderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt1Vt_private<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, stride, vtKernPtr, kernSize, topBorderType, bottomBorderType, false);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick).
	static COMPV_ERROR_CODE convlt1VtFixedPoint(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t stride, const uint16_t* vtKernPtr, size_t kernSize, COMPV_BORDER_TYPE topBorderType = COMPV_BORDER_TYPE_ZERO, COMPV_BORDER_TYPE bottomBorderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt1Vt_private<uint8_t, uint16_t, uint8_t>(inPtr, outPtr, width, height, stride, vtKernPtr, kernSize, topBorderType, bottomBorderType, true);
	}

	// Convolution using no separable kernel
	// sizeof(outPtr) should be at least equal to (dataHeight * dataStride)
	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt2(const InputType* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const KernelType* kernPtr, size_t kernSize, OutputType** outPtr, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt2_private<InputType, KernelType, OutputType>(dataPtr, dataWidth, dataStride, dataHeight, kernPtr, kernSize, outPtr, borderType, false);
	}

	// no arithmetic overflow check (up to the caller to normalize the data or use any trick)
	static COMPV_ERROR_CODE convlt2FixedPoint(const uint8_t* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const uint16_t* kernPtr, size_t kernSize, uint8_t** outPtr, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO) {
		return CompVMathConvlt::convlt2_private<uint8_t, uint16_t, uint8_t>(dataPtr, dataWidth, dataStride, dataHeight, kernPtr, kernSize, outPtr, borderType, false);
	}

	template <typename OutputType = uint8_t>
	static size_t outputSizeInBytes(size_t dataStride, size_t dataHeight) {
		return (dataHeight * dataStride) * sizeof(OutputType);
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
	static COMPV_ERROR_CODE convlt1_private(const InputType* dataPtr, size_t dataWidth, size_t dataHeight, size_t dataStride, const KernelType* vtKernPtr, const KernelType* hzKernPtr, size_t kernSize, OutputType*& outPtr, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO, bool fixedPoint = false) {
		// Check inputs
		COMPV_CHECK_EXP_RETURN(!dataPtr || (dataWidth < kernSize) || (dataHeight < kernSize) || (dataStride < dataWidth) || !vtKernPtr || !hzKernPtr || !(kernSize & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// The realloc_aligned() implementation memcpy() old data which is slow. Prefer, free_aligned() followed by malloc_aligned()

		/* Alloc memory */
		size_t neededSize = CompVMathConvlt::outputSizeInBytes<OutputType>(dataStride, dataHeight);
		bool outPtrAllocated = false;
		if (!outPtr) {
			outPtr = reinterpret_cast<OutputType*>(CompVMem::malloc(neededSize));
			COMPV_CHECK_EXP_RETURN(!outPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			outPtrAllocated = true;
		}
		OutputType* tmpPtr = NULL;

		// Compute number of threads
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
		const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
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
				COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
				OutputType* imgTmp = reinterpret_cast<OutputType*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<OutputType>(dataStride, h + rowsOverlapCount)));
				COMPV_CHECK_EXP_RETURN(!imgTmp, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to alloc imgTmp");
				const bool first = (threadIdx == 0);
				const bool last = (threadIdx == (threadsCount - 1));
				const size_t padding = first ? 0 : rowsOverlapPad;
				COMPV_CHECK_CODE_BAIL((err = CompVMathConvlt::convlt1Hz_private<InputType, KernelType, OutputType>(ptrIn - padding, imgTmp, dataWidth, h + rowsOverlapCount, dataStride, hzKernPtr, kernSize, borderType, fixedPoint)));
				COMPV_CHECK_CODE_BAIL((err = CompVMathConvlt::convlt1Vt_private<OutputType, KernelType, OutputType>(imgTmp, ptrOut - padding, dataWidth, h + rowsOverlapCount, dataStride, vtKernPtr, kernSize, (first ? borderType : COMPV_BORDER_TYPE_IGNORE), (last ? borderType : COMPV_BORDER_TYPE_IGNORE), fixedPoint)));
				bail:
				CompVMem::free((void**)&imgTmp);
				return err;
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
			COMPV_CHECK_CODE_BAIL((err = CompVMathConvlt::convlt1Hz_private<InputType, KernelType, OutputType>(dataPtr, tmpPtr, dataWidth, dataHeight, dataStride, hzKernPtr, kernSize, borderType, fixedPoint)));
			COMPV_CHECK_CODE_BAIL((err = CompVMathConvlt::convlt1Vt_private<OutputType, KernelType, OutputType>(tmpPtr, outPtr, dataWidth, dataHeight, dataStride, vtKernPtr, kernSize, borderType, borderType, fixedPoint)));
		}

	bail:
		CompVMem::free(reinterpret_cast<void**>(&tmpPtr));
		if (outPtrAllocated && COMPV_ERROR_CODE_IS_NOK(err)) {
			CompVMem::free(reinterpret_cast<void**>(&outPtr));
		}
		return err;
	}

	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt1Hz_private(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* hzKernPtr, size_t kernSize, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO, bool fixedPoint = false) {
		const size_t ker_size_div2 = (kernSize >> 1);
		const size_t imgpad = ((stride - width) + ker_size_div2 + ker_size_div2);
		// Set hz borders to zero
		// We must not accept garbage in the border (could be used by the calling function -e.g to find the max value for normalization)
		if (borderType == COMPV_BORDER_TYPE_ZERO) {
			OutputType *outPtr0 = outPtr, *outPtr1 = outPtr + (width - ker_size_div2);
			switch (ker_size_div2) { // 1 and 2 (kernel sizes 3 and 5 are very common)
			case 1: {
				const size_t kmax = (stride * height);
				for (size_t k = 0; k < kmax; k += stride) {
					outPtr0[k] = 0, outPtr1[k] = 0;
				}
				break;
			}
			case 2: {
				const size_t kmax = (stride * height);
				for (size_t k = 0; k < kmax; k += stride) {
					outPtr0[k] = outPtr0[k + 1] = 0, outPtr1[k] = outPtr1[k + 1] = 0;
				}
				break;
			}
			default: {
				for (size_t row = 0; row < height; ++row) {
					for (size_t col = 0; col < ker_size_div2; ++col) {
						outPtr0[col] = 0, outPtr1[col] = 0;
					}
					outPtr0 += stride;
					outPtr1 += stride;
				}
				break;
			}
			}
		}
		else if (borderType == COMPV_BORDER_TYPE_REPLICATE) {
			const InputType *inPtr0 = inPtr, *inPtr1 = inPtr + (width - ker_size_div2);
			OutputType *outPtr0 = outPtr, *outPtr1 = outPtr + (width - ker_size_div2);
			for (size_t row = 0; row < height; ++row) {
				for (size_t col = 0; col < ker_size_div2; ++col) {
					outPtr0[col] = static_cast<OutputType>(inPtr0[col]), outPtr1[col] = static_cast<OutputType>(inPtr1[col]);
				}
				outPtr0 += stride;
				outPtr1 += stride;
				inPtr0 += stride;
				inPtr1 += stride;
			}
		}
		else if (borderType != COMPV_BORDER_TYPE_IGNORE) {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		}
		
		// Perform horizontal convolution
		return CompVMathConvlt::convlt1VtHz_private<InputType, KernelType, OutputType>(inPtr, outPtr + ker_size_div2, static_cast<size_t>(width - kernSize), height, 1, imgpad, hzKernPtr, kernSize, fixedPoint);
	}

	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt1Vt_private(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t stride, const KernelType* vtKernPtr, size_t kernSize, COMPV_BORDER_TYPE topBorderType = COMPV_BORDER_TYPE_ZERO, COMPV_BORDER_TYPE bottomBorderType = COMPV_BORDER_TYPE_ZERO, bool fixedPoint = false) {
		size_t ker_size_div2 = (kernSize >> 1);
		size_t imgpad = (stride - width);
		// Set top and bottom vert borders to zero
		// We must not accept garbage in the border (coul be used by the calling function -e.g to find the max value for normalization)

		// Segmentation fault when the input image has an offset (e.g. bound to non-zero cols):
		//		-> for the last row reset 'width' only instead of 'stride'.
		const size_t bSizeInSamples = (((ker_size_div2 - 1) * stride) + width);

		// Top
		if (topBorderType == COMPV_BORDER_TYPE_ZERO) {
			CompVMem::zero(outPtr, bSizeInSamples * sizeof(OutputType));
		}
		else if (topBorderType == COMPV_BORDER_TYPE_REPLICATE) {
			const InputType* inPtr_ = inPtr;
			OutputType* outPtr_ = outPtr;
			if (sizeof(InputType) == sizeof(OutputType)) {
				memcpy(outPtr_, inPtr_, bSizeInSamples * sizeof(OutputType));
			}
			else {
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
				const OutputType minn = std::is_signed<OutputType>::value ? std::numeric_limits<OutputType>::lowest() : 0;
				const OutputType maxx = std::numeric_limits<OutputType>::max();
				for (size_t i = 0; i < bSizeInSamples; ++i) {
					outPtr_[i] = static_cast<OutputType>(COMPV_MATH_CLIP3(minn, maxx, inPtr_[i])); // SIMD: saturation
				}
			}
		}
		else if (topBorderType != COMPV_BORDER_TYPE_IGNORE) {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		}

		// Bottom
		if (bottomBorderType == COMPV_BORDER_TYPE_ZERO) {
			const size_t offsetInSamples = ((height - ker_size_div2) * stride);
			CompVMem::zero(outPtr + offsetInSamples, bSizeInSamples * sizeof(OutputType));
		}
		else if (bottomBorderType == COMPV_BORDER_TYPE_REPLICATE) {
			const size_t offsetInSamples = ((height - ker_size_div2) * stride);
			const InputType* inPtr_ = inPtr + offsetInSamples;
			OutputType* outPtr_ = outPtr + offsetInSamples;
			if (sizeof(InputType) == sizeof(OutputType)) {
				memcpy(outPtr_, inPtr_, bSizeInSamples * sizeof(OutputType));
			}
			else {
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
				const OutputType minn = std::is_signed<OutputType>::value ? std::numeric_limits<OutputType>::lowest() : 0;
				const OutputType maxx = std::numeric_limits<OutputType>::max();
				for (size_t i = 0; i < bSizeInSamples; ++i) {
					outPtr_[i] = static_cast<OutputType>(COMPV_MATH_CLIP3(minn, maxx, inPtr_[i])); // SIMD: saturation
				}
			}
		}
		else if (bottomBorderType != COMPV_BORDER_TYPE_IGNORE) {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		}

		// Perform vertical convolution
		return CompVMathConvlt::convlt1VtHz_private<InputType, KernelType, OutputType>(inPtr, outPtr + (ker_size_div2 * stride), width, (height - ker_size_div2 - ker_size_div2), stride, imgpad, vtKernPtr, kernSize, fixedPoint);
	}

	// Convolution using no separable kernel
	// sizeof(outPtr) must be at least equal to (dataHeight * dataStride)
	template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
	static COMPV_ERROR_CODE convlt2_private(const InputType* dataPtr, size_t dataWidth, size_t dataStride, size_t dataHeight, const KernelType* kernPtr, size_t kernSize, OutputType** outPtr, COMPV_BORDER_TYPE borderType = COMPV_BORDER_TYPE_ZERO, bool fixedPoint = false) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED); // see deprecated code
		return COMPV_ERROR_CODE_S_OK;
	}


	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE convlt1VtHz_private(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize, bool fixedPoint = false) {
		if (fixedPoint) {
			COMPV_CHECK_CODE_RETURN((convlt1VtHz_private_fxp_true<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
		}
		else {
			COMPV_CHECK_CODE_RETURN((convlt1VtHz_private_fxp_false<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE convlt1VtHz_private_fxp_true(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		// No need to check InputType, KernelType and OutputType
		COMPV_CHECK_CODE_RETURN(CompVMathConvlt::convlt1VtHzFixedPoint_C(reinterpret_cast<const uint8_t*>(inPtr), reinterpret_cast<uint8_t*>(outPtr), width, height, step, pad, reinterpret_cast<const uint16_t*>(vthzKernPtr), kernSize));
		return COMPV_ERROR_CODE_S_OK;
	}	

	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE convlt1VtHz_private_fxp_false(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		if (std::is_same<KernelType, compv_float32_t>::value || std::is_same<KernelType, compv_float64_t>::value) {
			COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1VtHzKernelFloat_C<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
		}
		else {
			COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1VtHzKernelInt_C<InputType, KernelType, OutputType>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE convlt1VtHzKernelInt_C(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		size_t i, j, k, row;
		int sum; // use int to accumulate any integer types (int18, int16 and int32)
		const OutputType minn = std::is_signed<OutputType>::value ? std::numeric_limits<OutputType>::lowest() : 0;
		const OutputType maxx = std::numeric_limits<OutputType>::max();
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = static_cast<int>(inPtr[0] * vthzKernPtr[0]);
				for (row = 1, k = step; row < kernSize; ++row, k += step) {
					sum += static_cast<int>(inPtr[k] * vthzKernPtr[row]);
				}
				*outPtr = static_cast<OutputType>(COMPV_MATH_CLIP3(minn, maxx, sum));
				++inPtr;
				++outPtr;
			}
			inPtr += pad;
			outPtr += pad;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// inPtr = any
	// outPtr = any
	// KernelType = float / double
	template <typename InputType, typename KernelType, typename OutputType>
	static COMPV_ERROR_CODE convlt1VtHzKernelFloat_C(const InputType* inPtr, OutputType* outPtr, size_t width, size_t height, size_t step, size_t pad, const KernelType* vthzKernPtr, size_t kernSize) {
		// KernelType is float32 or float64
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		size_t i, j, k, row;
		KernelType sum; // use (float / double) to accumulate
		const OutputType minn = std::is_signed<OutputType>::value ? std::numeric_limits<OutputType>::lowest() : 0;
		const OutputType maxx = std::numeric_limits<OutputType>::max();
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = inPtr[0] * vthzKernPtr[0];
				for (row = 1, k = step; row < kernSize; ++row, k += step) {
					sum += inPtr[k] * vthzKernPtr[row];
				}
#if 0
				*outPtr = COMPV_MATH_ROUNDFU_2_NEAREST_INT(sum, OutputType);
#else
				*outPtr = static_cast<OutputType>(COMPV_MATH_CLIP3(minn, maxx, sum)); // SIMD: saturation
#endif
				++inPtr;
				++outPtr;
			}
			inPtr += pad;
			outPtr += pad;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
	
	static COMPV_ERROR_CODE convlt1VtHzFixedPoint_C(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const uint16_t* vthzKernPtr, size_t kernSize) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		size_t i, j, k, row;
		unsigned int sum;

		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = static_cast<unsigned int>(inPtr[0] * vthzKernPtr[0]) >> 16;
				for (row = 1, k = step; row < kernSize; ++row, k += step) {
					sum += static_cast<unsigned int>(inPtr[k] * vthzKernPtr[row]) >> 16;
				}
				*outPtr = static_cast<uint8_t>(COMPV_MATH_CLIP3(0, 255, sum)); // SIMD: saturation
				++inPtr;
				++outPtr;
			}
			inPtr += pad;
			outPtr += pad;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
};

// InputType = uint8_t, KernelType = int16_t, OutputType = uint8_t, FixedPoint = true
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_true(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const uint16_t* vthzKernPtr, size_t kernSize);

// InputType = uint8_t, KernelType = compv_float32_t, OutputType = uint8_t, FixedPoint = false
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize);

// InputType = uint8_t, KernelType = int16_t, OutputType = int16_t, FixedPoint = false
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const int16_t* vthzKernPtr, size_t kernSize);

// InputType = int16_t, KernelType = int16_t, OutputType = int16_t, FixedPoint = false
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const int16_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const int16_t* vthzKernPtr, size_t kernSize);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_CONVLT_H_ */
