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
#include "compv/base/math//compv_math_histogram.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/image/intrin/arm/compv_image_threshold_intrin_neon.h"
#include "compv/base/image/intrin/x86/compv_image_threshold_intrin_sse2.h"
#include "compv/base/image/intrin/x86/compv_image_threshold_intrin_sse41.h"
#include "compv/base/image/intrin/x86/compv_image_threshold_intrin_avx2.h"

#define COMPV_IMAGE_THRESH_ADAPT_SAMPLES_PER_THREAD (40 * 40)
#define COMPV_IMAGE_THRESH_GLOBAL_SAMPLES_PER_THREAD (50 * 50)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X86
#	endif /* COMPV_ARCH_X86 */
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVImageThresholdGlobal_8u8u_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* inPtr, COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, compv_uscalar_t threshold);
	COMPV_EXTERNC void CompVImageThresholdGlobal_8u8u_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* inPtr, COMPV_ALIGNED(AVX) uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride, compv_uscalar_t threshold);
	COMPV_EXTERNC void CompVImageThresholdOtsuSum_32s32s_Asm_X64_SSE41(COMPV_ALIGNED(SSE) const uint32_t* ptr32uHistogram, COMPV_ALIGNED(SSE) uint32_t* sumA256, uint32_t* sumB1);
#	endif /* COMPV_ARCH_X64 */
#	if COMPV_ARCH_ARM32
	COMPV_EXTERNC void CompVImageThresholdGlobal_8u8u_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* inPtr, COMPV_ALIGNED(NEON) uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, compv_uscalar_t threshold);
	COMPV_EXTERNC void CompVImageThresholdOtsuSum_32s32s_Asm_NEON32(COMPV_ALIGNED(NEON) const uint32_t* ptr32uHistogram, COMPV_ALIGNED(NEON) uint32_t* sumA256, uint32_t* sumB1);
#	endif /* COMPV_ARCH_ARM32 */
#	if COMPV_ARCH_ARM64
	COMPV_EXTERNC void CompVImageThresholdGlobal_8u8u_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* inPtr, COMPV_ALIGNED(AVX) uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, compv_uscalar_t threshold);
	COMPV_EXTERNC void CompVImageThresholdOtsuSum_32s32s_Asm_NEON64(COMPV_ALIGNED(NEON) const uint32_t* ptr32uHistogram, COMPV_ALIGNED(NEON) uint32_t* sumA256, uint32_t* sumB1);
#	endif /* COMPV_ARCH_ARM64 */
#endif /* COMPV_ASM */

static void CompVImageThresholdGlobal_8u8u_C(
	const uint8_t* inPtr,
	uint8_t* outPtr,
	compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride,
	compv_uscalar_t threshold
);
static void CompVImageThresholdOtsuSum_32s32s_C(const uint32_t* ptr32uHistogram, uint32_t* sumA256, uint32_t* sumB1);

COMPV_ERROR_CODE CompVImageThreshold::otsu(const CompVMatPtr& input, double& threshold, CompVMatPtrPtr output COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_EXP_RETURN(!input || input->isEmpty() || input->planeCount() != 1 || input->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	// TODO(dmi): optiz issues on ARM64 (MediaPad 2) - image size=1282x720, loops=1k, threads=1
	//		- asm: 2774.ms, intrin: 3539

	/* Histogram */
	CompVMatPtr ptr32sHistogram;
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::build(input, &ptr32sHistogram));

	/* Otsu */
	uint32_t sum32s = 0;
	COMPV_ALIGN(16) uint32_t sumA256[256];
	const uint32_t* ptr32sHistogramPtr = ptr32sHistogram->ptr<const uint32_t>();
	void(*CompVImageThresholdOtsuSum_32s32s)(const uint32_t* ptr32uHistogram, uint32_t* sumA256, uint32_t* sumB1)
		= CompVImageThresholdOtsuSum_32s32s_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE41) && ptr32sHistogram->isAlignedSSE() && COMPV_IS_ALIGNED_SSE(sumA256)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVImageThresholdOtsuSum_32s32s = CompVImageThresholdOtsuSum_32s32s_Intrin_SSE41);
		COMPV_EXEC_IFDEF_ASM_X64(CompVImageThresholdOtsuSum_32s32s = CompVImageThresholdOtsuSum_32s32s_Asm_X64_SSE41);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && ptr32sHistogram->isAlignedNEON() && COMPV_IS_ALIGNED_NEON(sumA256)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVImageThresholdOtsuSum_32s32s = CompVImageThresholdOtsuSum_32s32s_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(CompVImageThresholdOtsuSum_32s32s = CompVImageThresholdOtsuSum_32s32s_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(CompVImageThresholdOtsuSum_32s32s = CompVImageThresholdOtsuSum_32s32s_Asm_NEON64);
	}
#endif
	CompVImageThresholdOtsuSum_32s32s(ptr32sHistogramPtr, sumA256, &sum32s);
	const compv_float32_t sumf = static_cast<compv_float32_t>(sum32s);

	const int N = static_cast<int>(input->cols() * input->rows());
	compv_float32_t sumB = 0;
	int q1 = 0, q2 = 0, thresholdInt = 0;
	compv_float32_t varMax = 0, q1f, q2f;
	for (int i = 0; i < 256; i++) {
		q1 += ptr32sHistogramPtr[i];
		if (q1) {
			q2 = N - q1;
			if (!q2) {
				break;
			}
			q1f = static_cast<compv_float32_t>(q1);
			q2f = static_cast<compv_float32_t>(q2);
			sumB += static_cast<compv_float32_t>(sumA256[i]);
			const compv_float32_t mf = (sumB / q1f) - ((sumf - sumB) / q2f);
			const compv_float32_t varB = q1f * q2f * mf * mf;
			if (varB > varMax) {
				varMax = varB;
				thresholdInt = i;
			}
		}
	}

	/* Set return value */
	threshold = static_cast<double>(thresholdInt);

	/* Thresholding if asked */
	if (output) {
		COMPV_CHECK_CODE_RETURN(CompVImageThreshold::global(input, output, threshold));
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageThreshold::global(const CompVMatPtr& input, CompVMatPtrPtr output, const double threshold)
{
	COMPV_CHECK_EXP_RETURN(!input || input->isEmpty() || input->planeCount() != 1 || input->elmtInBytes() != sizeof(uint8_t) || !output || threshold < 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();

	// output could be equal to input, not an issue for this function
	// create new output only if doesn't match the required format (needed by ADAS road binarization)
	CompVMatPtr output_ = *output;
	if (output_ != input && (!output_ || output_->planeCount() != 1 || output_->elmtInBytes() != sizeof(uint8_t) || output_->cols() != width || output_->rows() != height || output_->stride() != stride)) {
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&output_, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
	}

	const uint8_t thresholdUInt8 = COMPV_MATH_ROUNDFU_2_NEAREST_INT(
		COMPV_MATH_CLIP3(0x00, 0xff, threshold),
		uint8_t
	);

	// Get function
	void(*CompVImageThresholdGlobal_8u8u)(
		COMPV_ALIGNED(SSE) const uint8_t* inPtr,
		COMPV_ALIGNED(SSE) uint8_t* outPtr,
		compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
		compv_uscalar_t threshold
		) = CompVImageThresholdGlobal_8u8u_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && input->isAlignedSSE() && output_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVImageThresholdGlobal_8u8u = CompVImageThresholdGlobal_8u8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVImageThresholdGlobal_8u8u = CompVImageThresholdGlobal_8u8u_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && input->isAlignedAVX() && output_->isAlignedAVX()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVImageThresholdGlobal_8u8u = CompVImageThresholdGlobal_8u8u_Intrin_AVX2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVImageThresholdGlobal_8u8u = CompVImageThresholdGlobal_8u8u_Asm_X64_AVX2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && input->isAlignedNEON() && output_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVImageThresholdGlobal_8u8u = CompVImageThresholdGlobal_8u8u_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(CompVImageThresholdGlobal_8u8u = CompVImageThresholdGlobal_8u8u_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(CompVImageThresholdGlobal_8u8u = CompVImageThresholdGlobal_8u8u_Asm_NEON64);
	}
#endif

	// Get Number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, COMPV_IMAGE_THRESH_GLOBAL_SAMPLES_PER_THREAD)
		: 1;

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const uint8_t* ptr8uInput = input->ptr<const uint8_t>(ystart);
		uint8_t* ptr8uOutput = output_->ptr<uint8_t>(ystart);
		const compv_uscalar_t h = static_cast<compv_uscalar_t>(yend - ystart);
		CompVImageThresholdGlobal_8u8u(ptr8uInput, ptr8uOutput,
			static_cast<compv_uscalar_t>(width), h, static_cast<compv_uscalar_t>(stride), 
			static_cast<compv_uscalar_t>(thresholdUInt8));
		return COMPV_ERROR_CODE_S_OK;
	};

	if (threadsCount > 1) {
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

// Adaptive thresholding as explained at http://homepages.inf.ed.ac.uk/rbf/HIPR2/adpthrsh.htm
COMPV_ERROR_CODE CompVImageThreshold::adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const size_t blockSize, const double delta, const double maxVal COMPV_DEFAULT(255), bool invert COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!input || input->isEmpty() || !(blockSize & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("This function create the kernel each time you call it, You should create the kernel once (using CompVImageThreshold::kernelMean) and call the overrided one");
	
	// TODO(dmi): add support for Gaussian kernels

	CompVMatPtr ptr16uFxpKernl;
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::kernelMean(blockSize, &ptr16uFxpKernl));
	// Processing
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::adaptive(input, output, ptr16uFxpKernl, delta, maxVal, invert));

	return COMPV_ERROR_CODE_S_OK;
}

// Adaptive thresholding as explained at http://homepages.inf.ed.ac.uk/rbf/HIPR2/adpthrsh.htm
// Kernel must be fixed-point 1xn or 2xn (16u): separable hz/vt kernels (e.g. mean, gaussian or median kernels)
COMPV_ERROR_CODE CompVImageThreshold::adaptive(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& kernel, const double delta, const double maxVal COMPV_DEFAULT(255), bool invert COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!input || input->isEmpty() || !output || !kernel || kernel->isEmpty() || kernel->subType() != COMPV_SUBTYPE_RAW_UINT16 || !(kernel->cols() & 1) || maxVal < 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
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
		COMPV_MATH_CLIP3(0x00, 0xff, maxVal),
		uint8_t
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
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t minSamplesPerThreads = COMPV_MATH_MAX(
		(kernelSize * width),
		COMPV_IMAGE_THRESH_ADAPT_SAMPLES_PER_THREAD
	); // num rows per threads must be >= kernel size
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, minSamplesPerThreads)
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
		uint8_t* mt_meanPtr = mean->ptr<uint8_t>(ystart - padding); // same value as 'outputPtr' when MT is disabled
		COMPV_CHECK_CODE_RETURN(CompVMathConvlt::convlt1HzFixedPoint(mt_inputPtr, tmpMat->ptr<uint8_t>(), width, mt_h, stride, kernelHzPtr, kernelSize, COMPV_BORDER_TYPE_ZERO));
		COMPV_CHECK_CODE_RETURN(CompVMathConvlt::convlt1VtFixedPoint(tmpMat->ptr<const uint8_t>(), mt_meanPtr, width, mt_h, stride, kernelVtPtr, kernelSize, first ? COMPV_BORDER_TYPE_ZERO : COMPV_BORDER_TYPE_IGNORE, last ? COMPV_BORDER_TYPE_ZERO : COMPV_BORDER_TYPE_IGNORE));

		// Thresholding
		for (size_t j = 0; j < mt_h; ++j) {
			for (size_t i = 0; i < width; ++i) {
				mt_outputPtr[i] = lut[mt_inputPtr[i] - mt_meanPtr[i] + 255];
			}
			mt_inputPtr += stride;
			mt_outputPtr += stride;
			mt_meanPtr += stride;
		}
		
		return COMPV_ERROR_CODE_S_OK;
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

COMPV_ERROR_CODE CompVImageThreshold::kernelMean(const size_t blockSize, CompVMatPtrPtr kernel)
{
	COMPV_CHECK_EXP_RETURN(!kernel || !(blockSize & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr ptr32fNormalizedKernl;
	const compv_float32_t vvv = 1.f / static_cast<compv_float32_t>(blockSize);
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&ptr32fNormalizedKernl, 1, blockSize));
	compv_float32_t* ptr32fNormalizedKernlPtr = ptr32fNormalizedKernl->ptr<compv_float32_t>();
	for (size_t i = 0; i < blockSize; ++i) {
		ptr32fNormalizedKernlPtr[i] = vvv;
	}
	// Create fixed point mean kernel. No need for floating point version because mean processing fxp version is largely enough.
	COMPV_CHECK_CODE_RETURN(CompVMathConvlt::fixedPointKernel(ptr32fNormalizedKernl, kernel));
	return COMPV_ERROR_CODE_S_OK;
}

template <typename InputType, typename OutputType>
static void CompVImageThresholdGlobal_Generic_C(
	const InputType* inPtr,
	OutputType* outPtr,
	compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride,
	const double threshold, const double maxVal = 255.0
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	const InputType threshold_ = static_cast<InputType>(threshold);
	const OutputType maxVal_ = static_cast<OutputType>(maxVal);
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			outPtr[i] = static_cast<OutputType>((inPtr[i] > threshold_) ? maxVal_ : 0);
		}
		inPtr += stride;
		outPtr += stride;
	}
}

static void CompVImageThresholdGlobal_8u8u_C(
	const uint8_t* inPtr,
	uint8_t* outPtr,
	compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride,
	compv_uscalar_t threshold
)
{
	CompVImageThresholdGlobal_Generic_C(inPtr, outPtr, width, height, stride, static_cast<double>(threshold), 0xff);
}

static void CompVImageThresholdOtsuSum_32s32s_C(const uint32_t* ptr32uHistogram, uint32_t* sumA256, uint32_t* sumB1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	uint32_t sumB = 0;
	for (uint32_t i = 0; i < 256; i += 8) {
		sumA256[i] = (i * ptr32uHistogram[i]);
		sumA256[i + 1] = ((i + 1) * ptr32uHistogram[i + 1]);
		sumA256[i + 2] = ((i + 2) * ptr32uHistogram[i + 2]);
		sumA256[i + 3] = ((i + 3) * ptr32uHistogram[i + 3]);
		sumA256[i + 4] = ((i + 4) * ptr32uHistogram[i + 4]);
		sumA256[i + 5] = ((i + 5) * ptr32uHistogram[i + 5]);
		sumA256[i + 6] = ((i + 6) * ptr32uHistogram[i + 6]);
		sumA256[i + 7] = ((i + 7) * ptr32uHistogram[i + 7]);
		sumB += sumA256[i] + sumA256[i + 1] + sumA256[i + 2] + sumA256[i + 3]
			+ sumA256[i + 4] + sumA256[i + 5] + sumA256[i + 6] + sumA256[i + 7];
	}
	*sumB1 = sumB;
}

COMPV_NAMESPACE_END()
