/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_histogram.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/math//intrin/x86/compv_math_histogram_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_histogram_intrin_neon.h"

#define COMPV_HISTOGRAM_BUILD_MIN_SAMPLES_PER_THREAD		(256 * 5)
#define COMPV_HISTOGRAM_EQUALIZ_MIN_SAMPLES_PER_THREAD		(256 * 4)
#define COMPV_HISTOGRAM_PROJX_MIN_SAMPLES_PER_THREAD		(256 * 256) // CPU-friendly and temporary values added at the end
#define COMPV_HISTOGRAM_PROJY_MIN_SAMPLES_PER_THREAD		(128 * 128) // CPU-friendly

COMPV_NAMESPACE_BEGIN()


#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathHistogramProcess_8u32s_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) uint32_t* histogramPtr);
COMPV_EXTERNC void CompVMathHistogramProcess_8u32s_Asm_X64(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionX_8u32s_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* ptrIn, COMPV_ALIGNED(SSE) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionX_8u32s_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* ptrIn, COMPV_ALIGNED(AVX) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* ptrIn, COMPV_ALIGNED(SSE) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* ptrIn, COMPV_ALIGNED(AVX) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathHistogramProcess_8u32s_Asm_NEON32(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* ptrIn, COMPV_ALIGNED(NEON) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* ptrIn, COMPV_ALIGNED(NEON) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathHistogramProcess_8u32s_Asm_NEON64(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* ptrIn, COMPV_ALIGNED(NEON) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* ptrIn, COMPV_ALIGNED(NEON) int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM64 */

COMPV_ERROR_CODE CompVMathHistogram::build(const CompVMatPtr& data, CompVMatPtrPtr histogram)
{
	COMPV_CHECK_EXP_RETURN(!data || data->isEmpty() || !histogram, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input data is null or empty");

	if (data->elmtInBytes() == sizeof(uint8_t)) { // no plane count check needed
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint32_t>(histogram, 1, 256));
		COMPV_CHECK_CODE_RETURN((*histogram)->zero_rows());
		uint32_t* histogramPtr = (*histogram)->ptr<uint32_t>();
		const int planes = static_cast<int>(data->planeCount());
		for (int p = 0; p < planes; ++p) {
			COMPV_CHECK_CODE_RETURN(CompVMathHistogram::process_8u32u(data->ptr<uint8_t>(0, 0, p), data->cols(p), data->rows(p), data->strideInBytes(p), histogramPtr));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_ERROR("Input format not supported, we expect 8u data");
	return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

static void CompVMathHistogramBuildProjectionY_8u32s_C(const uint8_t* ptrIn, int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);

// Project the image on the vertical axis (sum cols)
COMPV_ERROR_CODE CompVMathHistogram::buildProjectionY(const CompVMatPtr& dataIn, CompVMatPtrPtr ptr32sProjection)
{
	COMPV_CHECK_EXP_RETURN(!dataIn || !dataIn || dataIn->isEmpty() || dataIn->elmtInBytes() != sizeof(uint8_t) || dataIn->planeCount() != 1 || !ptr32sProjection,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t width = dataIn->cols();
	const size_t height = dataIn->rows();
	const size_t stride = dataIn->stride();

	CompVMatPtr ptr32sProjection_ = (*ptr32sProjection == dataIn) ? nullptr : *ptr32sProjection;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&ptr32sProjection_, 1, height));

	void (*CompVMathHistogramBuildProjectionY_8u32s)(const uint8_t* ptrIn, int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= CompVMathHistogramBuildProjectionY_8u32s_C;

#if COMPV_ARCH_X86
	if (width >= 16 && CompVCpu::isEnabled(kCpuFlagSSE2) && dataIn->isAlignedSSE() && ptr32sProjection_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathHistogramBuildProjectionY_8u32s = CompVMathHistogramBuildProjectionY_8u32s_Intrin_SSE2));
		COMPV_EXEC_IFDEF_ASM_X64((CompVMathHistogramBuildProjectionY_8u32s = CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_SSE2));
	}
	if (width >= 32 && CompVCpu::isEnabled(kCpuFlagAVX2) && dataIn->isAlignedAVX() && ptr32sProjection_->isAlignedAVX()) {
		COMPV_EXEC_IFDEF_ASM_X64((CompVMathHistogramBuildProjectionY_8u32s = CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_AVX2));
	}
#elif COMPV_ARCH_ARM
	if (width >= 16 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && dataIn->isAlignedNEON() && ptr32sProjection_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVMathHistogramBuildProjectionY_8u32s = CompVMathHistogramBuildProjectionY_8u32s_Intrin_NEON));
		COMPV_EXEC_IFDEF_ASM_ARM32((CompVMathHistogramBuildProjectionY_8u32s = CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32));
		COMPV_EXEC_IFDEF_ASM_ARM64((CompVMathHistogramBuildProjectionY_8u32s = CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64));
	}
#endif

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		CompVMathHistogramBuildProjectionY_8u32s(
			dataIn->ptr<const uint8_t>(ystart), ptr32sProjection_->ptr<int32_t>(0, ystart),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride)
		);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_HISTOGRAM_PROJY_MIN_SAMPLES_PER_THREAD
	));

	*ptr32sProjection = ptr32sProjection_;
	return COMPV_ERROR_CODE_S_OK;
}

static void CompVMathHistogramBuildProjectionX_8u32s_C(const uint8_t* ptrIn, int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride);

// Project the image on the horizontal axis (sum rows)
COMPV_ERROR_CODE CompVMathHistogram::buildProjectionX(const CompVMatPtr& dataIn, CompVMatPtrPtr ptr32sProjection)
{
	COMPV_CHECK_EXP_RETURN(!dataIn || !dataIn || dataIn->isEmpty() || dataIn->elmtInBytes() != sizeof(uint8_t) || dataIn->planeCount() != 1 || !ptr32sProjection,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t width = dataIn->cols();
	const size_t height = dataIn->rows();
	const size_t stride = dataIn->stride();

	CompVMatPtr ptr32sProjection_ = (*ptr32sProjection == dataIn) ? nullptr : *ptr32sProjection;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&ptr32sProjection_, 1, width));

	void(*CompVMathHistogramBuildProjectionX_8u32s)(const uint8_t* ptrIn, int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= CompVMathHistogramBuildProjectionX_8u32s_C;

#if COMPV_ARCH_X86
	if (width >= 16 && CompVCpu::isEnabled(kCpuFlagSSE2) && dataIn->isAlignedSSE() && ptr32sProjection_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathHistogramBuildProjectionX_8u32s = CompVMathHistogramBuildProjectionX_8u32s_Intrin_SSE2));
		COMPV_EXEC_IFDEF_ASM_X64((CompVMathHistogramBuildProjectionX_8u32s = CompVMathHistogramBuildProjectionX_8u32s_Asm_X64_SSE2));
	}
	if (width >= 32 && CompVCpu::isEnabled(kCpuFlagAVX2) && dataIn->isAlignedAVX() && ptr32sProjection_->isAlignedAVX()) {
		COMPV_EXEC_IFDEF_ASM_X64((CompVMathHistogramBuildProjectionX_8u32s = CompVMathHistogramBuildProjectionX_8u32s_Asm_X64_AVX2));
	}
#elif COMPV_ARCH_ARM
	if (width >= 16 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && dataIn->isAlignedNEON() && ptr32sProjection_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVMathHistogramBuildProjectionX_8u32s = CompVMathHistogramBuildProjectionX_8u32s_Intrin_NEON));
		COMPV_EXEC_IFDEF_ASM_ARM32((CompVMathHistogramBuildProjectionX_8u32s = CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON32));
		COMPV_EXEC_IFDEF_ASM_ARM64((CompVMathHistogramBuildProjectionX_8u32s = CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64));
	}
#endif

	// Compute number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, COMPV_HISTOGRAM_PROJX_MIN_SAMPLES_PER_THREAD);

	if (threadsCount > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		CompVMatPtrVector mt_vecPtrOut(threadsCount - 1);
		auto funcPtr = [&](const size_t ystart, const size_t yend, const size_t threadIdx) -> COMPV_ERROR_CODE {
			const uint8_t* ptrIn = dataIn->ptr<const uint8_t>(ystart);
			int32_t* ptrOut;
			if (threadIdx) {
				COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&mt_vecPtrOut[threadIdx - 1], ptr32sProjection_));
				ptrOut = mt_vecPtrOut[threadIdx - 1]->ptr<int32_t>();
			}
			else {
				ptrOut = ptr32sProjection_->ptr<int32_t>();
			}
			CompVMathHistogramBuildProjectionX_8u32s(
				ptrIn, ptrOut,
				static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride)
			);
			return COMPV_ERROR_CODE_S_OK;
		};
		
		const size_t heights = (height / threadsCount);
		size_t YStart = 0, YEnd;
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			YEnd = (threadIdx == (threadsCount - 1)) ? height : (YStart + heights);
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, YStart, YEnd, threadIdx), taskIds), "Dispatching task failed");
			YStart += heights;
		}
		// Wait and sum the histograms
		COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[0]));
		for (size_t threadIdx = 1; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum2<uint32_t, uint32_t>(mt_vecPtrOut[threadIdx - 1]->ptr<const uint32_t>(), ptr32sProjection_->ptr<const uint32_t>(), ptr32sProjection_->ptr<uint32_t>(), width, 1, stride))); // stride is useless as height is equal to 1
		}
	}
	else {
		CompVMathHistogramBuildProjectionX_8u32s(
			dataIn->ptr<const uint8_t>(), ptr32sProjection_->ptr<int32_t>(),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride)
		);
	}
	
	*ptr32sProjection = ptr32sProjection_;
	return COMPV_ERROR_CODE_S_OK;
}

static void CompVMathHistogramEqualiz_32u8u_C(const uint32_t* ptr32uHistogram, uint8_t* ptr8uLut, const compv_float32_t* scale1);

COMPV_ERROR_CODE CompVMathHistogram::equaliz(const CompVMatPtr& dataIn, CompVMatPtrPtr dataOut)
{
	COMPV_CHECK_EXP_RETURN(!dataOut || !dataIn || dataIn->isEmpty() || dataIn->elmtInBytes() != sizeof(uint8_t) || dataIn->planeCount() != 1,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr histogram;
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::build(dataIn, &histogram));
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::equaliz(dataIn, histogram, dataOut));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathHistogram::equaliz(const CompVMatPtr& dataIn, const CompVMatPtr& histogram, CompVMatPtrPtr dataOut)
{
	COMPV_CHECK_EXP_RETURN(!dataOut || !dataIn || dataIn->isEmpty() || dataIn->elmtInBytes() != sizeof(uint8_t) || dataIn->planeCount() != 1
		|| !histogram || histogram->isEmpty() || histogram->cols() != 256 || histogram->subType() != COMPV_SUBTYPE_RAW_UINT32,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t width = dataIn->cols();
	const size_t height = dataIn->rows();
	const size_t stride = dataIn->stride();

	// This function allows dataOut to be equal to dataIn
	CompVMatPtr dataOut_ = *dataOut;
	if (!dataOut_ || dataOut_->cols() != width || dataOut_->rows() != height || dataOut_->stride() != stride || dataOut_->elmtInBytes() != sizeof(uint8_t) || dataOut_->planeCount() != 1) {
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&dataOut_, COMPV_SUBTYPE_PIXELS_Y, width, height, stride));
	}

	const compv_float32_t scale = 255.f / (width * height); 
	COMPV_ALIGN(16) uint8_t lut[256];

	void(*CompVMathHistogramEqualiz_32u8u)(const uint32_t* ptr32uHistogram, uint8_t* ptr8uLut, const compv_float32_t* scale1)
		= CompVMathHistogramEqualiz_32u8u_C;

	CompVMathHistogramEqualiz_32u8u(histogram->ptr<const uint32_t>(), lut, &scale);

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {		
		const uint8_t* ptr8uDataIn = dataIn->ptr<const uint8_t>(ystart);
		uint8_t* ptr8uDataOut_ = dataOut_->ptr<uint8_t>(ystart);
		for (size_t j = ystart; j < yend; ++j) {
			for (size_t i = 0; i < width; ++i) {
				ptr8uDataOut_[i] = lut[ptr8uDataIn[i]];
			}
			ptr8uDataIn += stride;
			ptr8uDataOut_ += stride;
		}

		return COMPV_ERROR_CODE_S_OK;
	};

	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_HISTOGRAM_EQUALIZ_MIN_SAMPLES_PER_THREAD
	));

	*dataOut = dataOut_;
	return COMPV_ERROR_CODE_S_OK;
}

static void CompVMathHistogramProcess_8u32u_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr);

// Up to the caller to set 'histogramPtr' values to zeros
COMPV_ERROR_CODE CompVMathHistogram::process_8u32u(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, uint32_t* histogramPtr)
{
	// Private function, no need to check imput parameters

	void(*CompVMathHistogramProcess_8u32u)(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr)
		= CompVMathHistogramProcess_8u32u_C;
    
#   if COMPV_ARCH_ARM32
    if (width > 3) {
        COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathHistogramProcess_8u32u = CompVMathHistogramProcess_8u32s_Asm_NEON32);
    }
#   endif /* COMPV_ARCH_ARM32 */

	if (width > 7) {
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathHistogramProcess_8u32u = CompVMathHistogramProcess_8u32s_Asm_X64);
        COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathHistogramProcess_8u32u = CompVMathHistogramProcess_8u32s_Asm_NEON64);
#       if 0
        COMPV_DEBUG_INFO_CODE_FOR_TESTING("Tried using #4 histograms but not faster at all and code not correct");
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(dataPtr) && COMPV_IS_ALIGNED_SSE(stride) && COMPV_IS_ALIGNED_SSE(histogramPtr)) {
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathHistogramProcess_8u32u = CompVMathHistogramProcess_8u32s_Asm_X64_SSE2);
		}
#       endif /* 0 */
	}

	// Compute number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, COMPV_HISTOGRAM_BUILD_MIN_SAMPLES_PER_THREAD);

	if (threadsCount > 1) {
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		size_t threadIdx, padding;
		const size_t threadsCountMinus1 = (threadsCount - 1);
		uint32_t* mt_histogramsPtr = reinterpret_cast<uint32_t*>(CompVMem::calloc(threadsCountMinus1 << 8, sizeof(uint32_t)));
		COMPV_CHECK_EXP_RETURN(!mt_histogramsPtr, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
		const size_t countAny = (height / threadsCount);
		const size_t countLast = countAny + (height % threadsCount);
		auto funcPtr = [&](const uint8_t* mt_dataPtr, size_t mt_height, size_t mt_threadIdx) -> COMPV_ERROR_CODE {
			CompVMathHistogramProcess_8u32u(mt_dataPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(mt_height), static_cast<compv_uscalar_t>(stride), 
				mt_threadIdx ? &mt_histogramsPtr[(mt_threadIdx - 1) << 8] : histogramPtr);
			return COMPV_ERROR_CODE_S_OK;
		};
		const uint8_t* mt_dataPtr = dataPtr;
		const size_t paddingPerThread = (countAny * stride);
		CompVAsyncTaskIds taskIds;
		// Invoke
		for (threadIdx = 0, padding = 0; threadIdx < (threadsCount - 1); ++threadIdx, padding += paddingPerThread) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtr, &mt_dataPtr[padding], countAny, threadIdx), taskIds));
		}
		COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtr, &mt_dataPtr[padding], countLast, (threadsCount - 1)), taskIds));
		// Wait and sum the histograms
		COMPV_CHECK_CODE_BAIL(err = threadDisp->waitOne(taskIds[0]));
		for (threadIdx = 1, padding = 0; threadIdx < threadsCount; ++threadIdx, padding += 256) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->waitOne(taskIds[threadIdx]));
			COMPV_CHECK_CODE_BAIL(err = (CompVMathUtils::sum2<uint32_t, uint32_t>(&mt_histogramsPtr[padding], histogramPtr, histogramPtr, 256, 1, 256))); // stride is useless as height is equal to 1
		}
	bail:
		CompVMem::free(reinterpret_cast<void**>(&mt_histogramsPtr));
	}
	else {
		CompVMathHistogramProcess_8u32u(dataPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride), histogramPtr);
	}
    
	return COMPV_ERROR_CODE_S_OK;
}

static void CompVMathHistogramProcess_8u32u_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr)
{
	//!\\ TODO(dmi): optiz issue -> on AArch64 MediaPad2 the asm code is almost two times faster than the intrin code.
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	compv_uscalar_t i, j;
	const compv_uscalar_t maxWidthStep1 = width & -4;
	int a, b, c, d;
	for (j = 0; j < height; ++j) {
		for (i = 0; i < maxWidthStep1; i += 4) {
			a = dataPtr[i + 0];
			b = dataPtr[i + 1];
			c = dataPtr[i + 2];
			d = dataPtr[i + 3];
			++histogramPtr[a];
			++histogramPtr[b];
			++histogramPtr[c];
			++histogramPtr[d];
		}
		for (; i < width; ++i) {
			++histogramPtr[dataPtr[i]];
		}
		dataPtr += stride;
	}
}

static void CompVMathHistogramEqualiz_32u8u_C(const uint32_t* ptr32uHistogram, uint8_t* ptr8uLut, const compv_float32_t* scale1)
{
#if 1 // Doesn't worth SIMD (in all cases, not used in any commercial product for now)
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
#endif
	const compv_float32_t scale = *scale1;
	uint32_t sum = 0;
	for (size_t i = 0; i < 256; ++i) {
		sum += ptr32uHistogram[i];
		ptr8uLut[i] = static_cast<uint8_t>(sum * scale); // no need for saturation (thanks to scaling factor)
	}
}

static void CompVMathHistogramBuildProjectionY_8u32s_C(const uint8_t* ptrIn, int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		int32_t sum = *ptrIn; // int32_t <- uint8_t
		for (compv_uscalar_t i = 1; i < width; ++i) {
			sum += ptrIn[i]; // int32_t <- uint8_t
		}
		ptrIn += stride;
		ptrOut[j] = sum;
	}
}

static void CompVMathHistogramBuildProjectionX_8u32s_C(const uint8_t* ptrIn, int32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	/* Copy first row (to avoid using memset(0)) */
	for (compv_uscalar_t i = 0; i < width; ++i) {
		ptrOut[i] = ptrIn[i]; // int32_t <- uint8_t
	}
	ptrIn += stride;
	/* Other rows */
	for (compv_uscalar_t j = 1; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			ptrOut[i] += ptrIn[i]; // int32_t <- uint8_t
		}
		ptrIn += stride;
	}
}

COMPV_NAMESPACE_END()
