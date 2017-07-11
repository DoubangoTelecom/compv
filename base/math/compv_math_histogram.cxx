/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_histogram.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#define COMPV_HISTOGRAM_MIN_SAMPLES_PER_THREAD		(200 * 10)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVMathHistogramProcess_8u32s_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) uint32_t* histogramPtr);
	COMPV_EXTERNC void CompVMathHistogramProcess_8u32s_Asm_X64(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr);
#	endif
#endif /* COMPV_ASM */

COMPV_ERROR_CODE CompVMathHistogram::process(const CompVMatPtr& data, CompVMatPtrPtr histogram)
{
	COMPV_CHECK_EXP_RETURN(!data || data->isEmpty() || !histogram, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input data is null or empty");

	if (data->elmtInBytes() == sizeof(uint8_t)) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint32_t>(histogram, 1, 256));
		COMPV_CHECK_CODE_RETURN((*histogram)->zero_rows());
		uint32_t* histogramPtr = (*histogram)->ptr<uint32_t>();
		const int planes = static_cast<int>(data->planeCount());
		for (int p = 0; p < planes; ++p) {
			COMPV_CHECK_CODE_RETURN(CompVMathHistogram::process_8u32u(data->ptr<uint8_t>(0, 0, p), data->cols(p), data->rows(p), data->strideInBytes(p), histogramPtr));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_ERROR("Input format not supported");
	return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

static void CompVMathHistogramProcess_8u32u_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr);

// Up to the caller to set 'histogramPtr' values to zeros
COMPV_ERROR_CODE CompVMathHistogram::process_8u32u(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, uint32_t* histogramPtr)
{
	// Private function, no need to check imput parameters

	void(*CompVMathHistogramProcess_8u32u)(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr)
		= CompVMathHistogramProcess_8u32u_C;

	if (width > 7) {
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathHistogramProcess_8u32u = CompVMathHistogramProcess_8u32s_Asm_X64);
		// Tried using #4 histograms but not faster at all
		//if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(dataPtr) && COMPV_IS_ALIGNED_SSE(stride) && COMPV_IS_ALIGNED_SSE(histogramPtr)) {
		//	COMPV_EXEC_IFDEF_ASM_X64(CompVMathHistogramProcess_8u32u = CompVMathHistogramProcess_8u32s_Asm_X64_SSE2);
		//}
	}

	// Compute number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, COMPV_HISTOGRAM_MIN_SAMPLES_PER_THREAD);

	if (threadsCount > 1) {
		size_t threadIdx, padding;
		std::vector<CompVMatPtr> mt_histograms;
		const size_t countAny = (height / threadsCount);
		const size_t countLast = countAny + (height % threadsCount);
		auto funcPtr = [&](const uint8_t* mt_dataPtr, size_t mt_height, size_t mt_threadIdx) -> COMPV_ERROR_CODE {
			uint32_t* mt_histogramPtr;
			if (mt_threadIdx == 0) {
				mt_histogramPtr = histogramPtr;
			}
			else {
				const size_t mt_histogram_index = (mt_threadIdx - 1);
				COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint32_t>(&mt_histograms[mt_histogram_index], 1, 256));
				COMPV_CHECK_CODE_RETURN(mt_histograms[mt_histogram_index]->zero_rows());
				mt_histogramPtr = mt_histograms[mt_histogram_index]->ptr<uint32_t>();
			}
			CompVMathHistogramProcess_8u32u(mt_dataPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(mt_height), static_cast<compv_uscalar_t>(stride), mt_histogramPtr);
			return COMPV_ERROR_CODE_S_OK;
		};
		const uint8_t* mt_dataPtr = dataPtr;
		const size_t paddingPerThread = (countAny * stride);
		CompVAsyncTaskIds taskIds;
		mt_histograms.resize(threadsCount - 1);
		// Invoke
		for (threadIdx = 0, padding = 0; threadIdx < (threadsCount - 1); ++threadIdx, padding += paddingPerThread) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, &mt_dataPtr[padding], countAny, threadIdx), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, &mt_dataPtr[padding], countLast, (threadsCount - 1)), taskIds));
		// Wait and sum the histograms
		COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[0]));
		for (threadIdx = 1; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum2<uint32_t, uint32_t>(mt_histograms[threadIdx - 1]->ptr<const uint32_t>(), histogramPtr, histogramPtr, 256, 1, 256))); // stride is useless as height is equal to 1
		}
	}
	else {
		CompVMathHistogramProcess_8u32u(dataPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride), histogramPtr);
	}

	return COMPV_ERROR_CODE_S_OK;
}

static void CompVMathHistogramProcess_8u32u_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint32_t* histogramPtr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	compv_uscalar_t i, j;
	const compv_uscalar_t maxWidthStep1 = width & -4;
	int a, b, c, d;
	for (j = 0; j < height; ++j) {
		for (i = 0; i < maxWidthStep1; i += 4) {
			// TODO(dmi): for asm code, use #4 different histograms (to hide memory latency) and sum them at the end
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

COMPV_NAMESPACE_END()
