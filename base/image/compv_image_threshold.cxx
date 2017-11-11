/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_threshold.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_convlt.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"

#define COMPV_IMAGE_THRESH_ADAPT_SAMPLES_PER_THREAD (40 * 40)

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVImageThreshold::fixed(const CompVMatPtr& input, CompVMatPtrPtr output, const double threshold)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

// Adaptive thresholding as explained at http://homepages.inf.ed.ac.uk/rbf/HIPR2/adpthrsh.htm
COMPV_ERROR_CODE CompVImageThreshold::adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const size_t blockSize, const double delta, const double maxVal COMPV_DEFAULT(255), bool invert COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!input || input->isEmpty() || !(blockSize & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("This function create the kernel each time you call it, You should create the kernel once and call the overrided one");
	
	// TODO(dmi): add support for Gaussian kernels

	// Create fixed point mean kernel. No need for floating point version because mean processing fxp version is largely enough.
	CompVMatPtr ptr32fNormalizedKernl, ptr16uFxpKernl;
	const compv_float32_t vvv = 1.f / static_cast<compv_float32_t>(blockSize);
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&ptr32fNormalizedKernl, 1, blockSize));
	compv_float32_t* ptr32fNormalizedKernlPtr = ptr32fNormalizedKernl->ptr<compv_float32_t>();
	for (size_t i = 0; i < blockSize; ++i) {
		ptr32fNormalizedKernlPtr[i] = vvv;
	}
	COMPV_CHECK_CODE_RETURN(CompVMathConvlt::fixedPointKernel(ptr32fNormalizedKernl, &ptr16uFxpKernl));
	// Processing
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::adaptive(input, output, ptr16uFxpKernl, delta, maxVal, invert));

	return COMPV_ERROR_CODE_S_OK;
}

// Adaptive thresholding as explained at http://homepages.inf.ed.ac.uk/rbf/HIPR2/adpthrsh.htm
// Kernel must be fixed-point 1xn or 2xn (16u): separable hz/vt kernels (e.g. mean, gaussian or median kernels)
COMPV_ERROR_CODE CompVImageThreshold::adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& kernel, const double delta, const double maxVal COMPV_DEFAULT(255), bool invert COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!input || input->isEmpty() || !output || !kernel || kernel->isEmpty() || kernel->subType() != COMPV_SUBTYPE_RAW_UINT16 || !(kernel->cols() & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(input->planeCount() != 1 || input->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input must be #1 dimension uint8 (e.g. grayscale image)");
	
#if 0 // TODO(dmi)
	COMPV_DEBUG_INFO_CODE_TODO("Add support for border type in convolution and use replicate");
#endif

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();

	const int deltaInt = COMPV_MATH_ROUNDFU_2_NEAREST_INT(
		COMPV_MATH_CLIP3(0x00, 0xff, delta), 
		int
	);
	const uint8_t maxValUInt8 = COMPV_MATH_ROUNDFU_2_NEAREST_INT(
		COMPV_MATH_CLIP3(0xff, 255, maxVal),
		int
	);

	// Build LookUp table
	uint8_t lut[768];
	const size_t maxValCount = (255 - deltaInt + 1);
	memset(&lut[0], invert ? maxValUInt8 : 0, maxValCount);
	memset(&lut[maxValCount], invert ? 0 : maxValUInt8, 768 - maxValCount);

	// Create output
	CompVMatPtr output_ = (input == *output) ? nullptr : *output;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));

	// Hz and Vt kernels
	const size_t kernelSize = kernel->cols();
	const uint16_t* kernelVtPtr = kernel->ptr<const uint16_t>(0);
	const uint16_t* kernelHzPtr = kernel->rows() > 1 ? kernel->ptr<const uint16_t>(1) : kernelVtPtr;

	// Get Number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, COMPV_IMAGE_THRESH_ADAPT_SAMPLES_PER_THREAD)
		: 1;

	// Check if if overlaping is small
	const bool bKernelSizeTooHighForMT = kernelSize > (static_cast<size_t>(height / threadsCount) >> 1);
	if (bKernelSizeTooHighForMT) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Kernel size too high for MT");
	}
	const bool mtEnabled = (threadsCount > 1 && !bKernelSizeTooHighForMT);

	// Create mean
	CompVMatPtr mean;
	if (mtEnabled) {
		// When MT is enabled we cannot perform convolution in one thread while the other ones are setting the final
		// values from the lookup tab
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&mean, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
	}
	else {
		mean = output_;
	}
		
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		const size_t h = (yend - ystart);
		const size_t rowsOverlapCount = ((kernelSize >> 1) << 1); // (kernelRadius times 2)
		const size_t mt_rowsOverlapCount = mtEnabled ? rowsOverlapCount : 0; // no overlap when there is no MT
		const size_t mt_h = h + mt_rowsOverlapCount;

		// Convolution
		CompVMatPtr tmpMat;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&tmpMat, mt_h, width, stride));
		const bool first = (ystart == 0);
		const bool last = (yend == height);
		const size_t padding = first ? 0 : rowsOverlapCount;
		const uint8_t* mt_inputPtr = input->ptr<const uint8_t>(ystart - padding);
		uint8_t* mt_outputPtr = output_->ptr<uint8_t>(ystart - padding);
		uint8_t* mt_meanPtr = mean->ptr<uint8_t>(ystart - padding); // same value as 'outputPtr' when MT is enabled
		CompVMathConvlt::convlt1HzFixedPoint(mt_inputPtr, tmpMat->ptr<uint8_t>(), width, mt_h, stride, kernelHzPtr, kernelSize, true);
		CompVMathConvlt::convlt1VtFixedPoint(tmpMat->ptr<const uint8_t>(), mt_meanPtr, width, mt_h, stride, kernelVtPtr, kernelSize, first, last);

		// Thresholding
		for (size_t j = 0; j < mt_h; ++j) {
			for (size_t i = 0; i < width; ++i) {
				mt_outputPtr[i] = lut[mt_inputPtr[i] - mt_meanPtr[i] + 255];
			}
			mt_inputPtr += stride;
			mt_outputPtr += stride;
			mt_meanPtr += stride;
		}
		
		return err;
	};
	
	// Processing
	if (mtEnabled) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		const size_t heights = (height / threadsCount);
		size_t YStart = 0, YEnd;
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			YEnd = (threadIdx == (threadsCount - 1)) ? height : (YStart + heights);
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, YStart, YEnd), taskIds), "Dispatching task failed");
			YStart += heights;
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		COMPV_CHECK_CODE_RETURN(funcPtr(0, height));
	}

	*output = output_;

	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()
