/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/compv_core_feature_canny_dete.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_convlt.h"
#include "compv/base/math/compv_math_distance.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_sse2.h"
#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_ssse3.h"
#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_avx2.h"

#define COMPV_THIS_CLASSNAME	"CompVEdgeDeteCanny"

#define COMPV_FEATURE_DETE_CANNY_GRAD_MIN_SAMPLES_PER_THREAD	3 // must be >= KernelSize because of the convolution ("rowsOverlapCount")
#define COMPV_FEATURE_DETE_CANNY_NMS_MIN_SAMPLES_PER_THREAD	(20*20)

COMPV_NAMESPACE_BEGIN()

static void CompVCannyNmsGatherRow_C(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, uint16_t tLow, size_t colStart, size_t width, size_t stride);
static void CompVCannyHysteresisRow_C(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0);

CompVEdgeDeteCanny::CompVEdgeDeteCanny(float tLow, float tHigh, size_t kernSize)
	: CompVEdgeDete(COMPV_CANNY_ID)
	, m_nImageWidth(0)
	, m_nImageHeight(0)
	, m_nImageStride(0)
	, m_pGx(NULL)
	, m_pGy(NULL)
	, m_pG(NULL)
	, m_pNms(NULL)	
	, m_pcKernelVt(kernSize == 3 ? CompVSobel3x3Gx_vt : CompVSobel5x5Gx_vt)
	, m_pcKernelHz(kernSize == 3 ? CompVSobel3x3Gx_hz : CompVSobel5x5Gx_hz)
	, m_nKernelSize(kernSize == 3 ? 3 : 5)
	, m_fThresholdLow(tLow)
	, m_fThresholdHigh(tHigh)
{

}

CompVEdgeDeteCanny::~CompVEdgeDeteCanny()
{
	CompVMem::free(reinterpret_cast<void**>(&m_pGx));
	CompVMem::free(reinterpret_cast<void**>(&m_pGy));
	CompVMem::free(reinterpret_cast<void**>(&m_pG));
	CompVMem::free(reinterpret_cast<void**>(&m_pNms));
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_CANNY_SET_FLT32_THRESHOLD_LOW: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_fThresholdLow = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_CANNY_SET_FLT32_THRESHOLD_HIGH: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_fThresholdHigh = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_CANNY_SET_INT_KERNEL_SIZE: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || (*reinterpret_cast<const int*>(valuePtr) != 3 && *reinterpret_cast<const int*>(valuePtr) != 5), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nKernelSize = static_cast<size_t>(*reinterpret_cast<const int*>(valuePtr));
		switch (m_nKernelSize) {
		case 3:
			m_pcKernelVt = CompVSobel3x3Gx_vt, m_pcKernelHz = CompVSobel3x3Gx_hz;
			return COMPV_ERROR_CODE_S_OK;
		case 5:
			m_pcKernelVt = CompVSobel5x5Gx_vt, m_pcKernelHz = CompVSobel5x5Gx_hz;
			return COMPV_ERROR_CODE_S_OK;
		default:
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER, "In the current beta version only canny edge detector supports only kernel sizes 3x3 and 5x5");
			return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
		}
	}
	default:
		COMPV_CHECK_CODE_RETURN(CompVCaps::set(id, valuePtr, valueSize));
		return COMPV_ERROR_CODE_S_OK;
	}
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::process(const CompVMatPtr& image, CompVMatPtrPtr edges) /*Overrides(CompVEdgeDete)*/
{
	COMPV_CHECK_EXP_RETURN(!image || image->subType() != COMPV_SUBTYPE_PIXELS_Y || !edges, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");
	COMPV_CHECK_EXP_RETURN(m_fThresholdLow >= m_fThresholdHigh, COMPV_ERROR_CODE_E_INVALID_STATE, "Invalid state: m_fThresholdLow >= m_fThresholdHigh");

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// Force memory realloc if image size changes
	if (!m_pGx || !m_pGy || !m_pG || !m_pNms || image->cols() != m_nImageWidth || image->rows() != m_nImageHeight || image->stride() != m_nImageStride) {
		CompVMem::free(reinterpret_cast<void**>(&m_pGx));
		CompVMem::free(reinterpret_cast<void**>(&m_pGy));
		CompVMem::free(reinterpret_cast<void**>(&m_pG));
		CompVMem::free(reinterpret_cast<void**>(&m_pNms));
		m_nImageWidth = image->cols();
		m_nImageHeight = image->rows();
		m_nImageStride = image->stride();
		m_pGx = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight)));
		COMPV_CHECK_EXP_RETURN(!m_pGx, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to allocate m_pGx");
		m_pGy = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight)));
		COMPV_CHECK_EXP_RETURN(!m_pGy, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to allocate m_pGy");
		m_pG = reinterpret_cast<uint16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<uint16_t>(m_nImageStride, m_nImageHeight)));
		COMPV_CHECK_EXP_RETURN(!m_pG, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to allocate m_pG");
	}

	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip) and image (required by gaussian blur)
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(edges, COMPV_SUBTYPE_PIXELS_Y, m_nImageWidth, m_nImageHeight, m_nImageStride));

	// Compute mean and get tLow and tMin
	// TODO(dmi): add support for otsu and median
	uint32_t sum = 0; // sum used to compute mean

	// When multithreading is enabled the mean value could be "+-1" compared to the single threaded value.

	/* Convolution + Gradient + Mean */
	// Get Max number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(m_nImageStride, m_nImageHeight, maxThreads, COMPV_MATH_MAX(m_nKernelSize, COMPV_FEATURE_DETE_CANNY_GRAD_MIN_SAMPLES_PER_THREAD));

	//!\\ Computing mean per thread provides different result on MT and ST, this is why we compute sum then mean.

	if (threadsCount > 1) {
		const size_t rowsOverlapCount = ((m_nKernelSize >> 1) << 1); // (kernelRadius times 2)
		const size_t rowsOverlapPad = rowsOverlapCount * m_nImageStride;
		const size_t countAny = (size_t)(m_nImageHeight / threadsCount);
		const size_t countLast = (size_t)countAny + (m_nImageHeight % threadsCount);
		const size_t countAnyTimesStride = countAny * m_nImageStride;
		const uint8_t* inPtr_ = image->ptr<const uint8_t>();
		uint32_t* sums = reinterpret_cast<uint32_t*>(CompVMem::malloc(threadsCount * sizeof(uint32_t)));
		int16_t* outPtrGx_ = m_pGx;
		int16_t* outPtrGy_ = m_pGy;
		uint16_t* outPtrG_ = m_pG;
		CompVAsyncTaskIds taskIds;
		//!\\ Important: Our tests showed (both x86 and arm)d that it's faster to alloc temp memory for each thread rather than sharing global one -> false sharing issue.
		// This is an issue for the convolution only because there is no way to make the writing cache-friendly.
		// No such issue when multithreading 'CompVMathConvlt::convlt1' (perf tests done), so don't try to change the function.
		auto funcPtr = [&](const uint8_t* ptrIn, int16_t* ptrOutGx, int16_t* ptrOutGy, uint16_t* ptrOutG, uint32_t* ptrSum, size_t h, size_t threadIdx) -> COMPV_ERROR_CODE {
			int16_t* imgTmp = reinterpret_cast<int16_t*>(CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, h + rowsOverlapCount))); // local alloc to avoid false sharing
			COMPV_CHECK_EXP_RETURN(!imgTmp, COMPV_ERROR_CODE_E_OUT_OF_MEMORY, "Failed to alloc imgTmp");
			const bool first = (threadIdx == 0);
			const bool last = (threadIdx == (threadsCount - 1));
			const size_t padding = first ? 0 : rowsOverlapPad;
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - padding, imgTmp, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, true);
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(imgTmp, ptrOutGx - padding, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, first, last);
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - padding, imgTmp, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, true);
			CompVMathConvlt::convlt1Vt<int16_t, int16_t, int16_t>(imgTmp, ptrOutGy - padding, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, first, last);
			CompVMem::free((void**)&imgTmp);
			// Gradient using L1 distance (abs(gx) + abs(gy))
			COMPV_CHECK_CODE_RETURN((CompVMathDistance::l1<int16_t, uint16_t>(ptrOutGx - padding, ptrOutGy - padding, ptrOutG - padding, m_nImageWidth, h + rowsOverlapCount, m_nImageStride)));
			uint32_t sum_ = 0;
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum<uint8_t, uint32_t>(ptrIn, m_nImageWidth, h, m_nImageStride, sum_)));
			*ptrSum = sum_;
			return COMPV_ERROR_CODE_S_OK;
		};

		COMPV_CHECK_EXP_BAIL(!sums, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to alloc sums memory");
		taskIds.reserve(threadsCount);
		// convolution + gradient
		for (size_t threadIdx = 0, index = 0; threadIdx < threadsCount; ++threadIdx, index += countAnyTimesStride) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, &inPtr_[index], &outPtrGx_[index], &outPtrGy_[index], &outPtrG_[index], &sums[threadIdx],
				(threadIdx == (threadsCount - 1)) ? countLast : countAny, threadIdx),
				taskIds));
			;
		}
		// mean
		sum = 0;
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			sum += sums[threadIdx];
		}
bail:
		CompVMem::free(reinterpret_cast<void**>(&sums));
	}
	else {
		// Convolution (gx and gy)
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelVt, m_pcKernelHz, m_nKernelSize, m_pGx)));
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, m_pcKernelHz, m_pcKernelVt, m_nKernelSize, m_pGy)));
		// Gradient using L1 distance (abs(gx) + abs(gy))
		COMPV_CHECK_CODE_RETURN((CompVMathDistance::l1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, m_nImageWidth, m_nImageHeight, m_nImageStride)));
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum<uint8_t, uint32_t>(image->ptr<const uint8_t>(), m_nImageWidth, m_nImageHeight, m_nImageStride, sum)));
	}

	uint8_t mean = sum / static_cast<uint32_t>(m_nImageWidth * m_nImageHeight);
	mean = COMPV_MATH_CLIP3(1, 255, mean);
	uint16_t tLow = static_cast<uint16_t>(mean * m_fThresholdLow);
	uint16_t tHigh = static_cast<uint16_t>(mean * m_fThresholdHigh);
	tLow = COMPV_MATH_MAX(1, tLow);
	tHigh = COMPV_MATH_MAX(tLow + 2, tHigh);

	/* NMS + Hysteresis */
	if (!m_pNms) {
		m_pNms = reinterpret_cast<uint8_t*>(CompVMem::calloc(m_nImageStride * m_nImageHeight, sizeof(uint8_t)));
		COMPV_CHECK_EXP_RETURN(!m_pNms, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(m_nImageStride, m_nImageHeight, maxThreads, COMPV_FEATURE_DETE_CANNY_NMS_MIN_SAMPLES_PER_THREAD);
	if (threadsCount > 1) {
		CompVAsyncTaskIds taskIds;
		const size_t countAny = (m_nImageHeight / threadsCount);
		const size_t countLast = countAny + (m_nImageHeight % threadsCount);
		auto funcPtrNMS = [&](size_t rowStart, size_t rowCount) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(nms_gather(*edges, tLow, rowStart, rowCount));
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrHysteresis = [&](size_t rowStart, size_t rowCount) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(hysteresis(*edges, tLow, tHigh, rowStart, rowCount));
			return COMPV_ERROR_CODE_S_OK;
		};
		size_t rowStart, threadIdx;
		// NMS gathering
		taskIds.reserve(threadsCount);
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCount - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNMS, rowStart, countAny), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNMS, rowStart, countLast), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		// NMS supp
		nms_apply();
		// Hysteresis
		taskIds.clear();
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCount - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrHysteresis, rowStart, countAny), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrHysteresis, rowStart, countLast), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
	}
	else {
		COMPV_CHECK_CODE_RETURN(nms_gather(*edges, tLow, 0, m_nImageHeight));
		nms_apply();
		COMPV_CHECK_CODE_RETURN(hysteresis(*edges, tLow, tHigh, 0, m_nImageHeight));
	}

	return err;
}

// NonMaximaSuppression
COMPV_ERROR_CODE CompVEdgeDeteCanny::nms_gather(CompVMatPtr& edges, uint16_t tLow, size_t rowStart, size_t rowCount)
{
	// Private function -> do not check input parameters
	size_t maxRows = rowStart + rowCount;
	maxRows = COMPV_MATH_MIN(maxRows, m_nImageHeight - 1);
	rowStart = COMPV_MATH_MAX(1, rowStart);
	size_t rowStartTimesStride = rowStart * m_nImageStride;

	size_t colStart = 1;
	int xmpw = 1;
	size_t row;
	const size_t maxCols = edges->cols() - 1;
	const int16_t *gx = m_pGx + rowStartTimesStride, *gy = m_pGy + rowStartTimesStride;
	uint16_t *g = m_pG + rowStartTimesStride;
	uint8_t *nms = m_pNms + rowStartTimesStride;
	uint8_t *e = edges->ptr<uint8_t>(rowStart);

	// non-maximasupp is multi-threaded and we will use this property to zero the edge buffer with low cost (compared to edges->zeroall() which is not MT)
	if (rowStart == 1) { // First time ?
		COMPV_CHECK_CODE_RETURN(edges->zero_row(0)); // zero first line
		COMPV_CHECK_CODE_RETURN(edges->zero_row(m_nImageHeight - 1)); // zero last line
	}

	// 8mpw -> minpack 8 for words (int16) -> SSE or ARM NEON
	// 16mpw -> minpack 16 for words (int16) -> AVX
	void(*CompVCannyNMSGatherRow_xmpw)(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, const uint16_t* tLow1, compv_uscalar_t width, compv_uscalar_t stride)
		= NULL;

#if COMPV_ARCH_X86
	if (maxCols >= 8 && CompVCpu::isEnabled(compv::kCpuFlagSSSE3)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVCannyNMSGatherRow_xmpw = CompVCannyNMSGatherRow_8mpw_Intrin_SSSE3, xmpw = 8));
	}
	if (maxCols >= 16 && CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVCannyNMSGatherRow_xmpw = CompVCannyNMSGatherRow_16mpw_Intrin_AVX2, xmpw = 16));
	}
#endif /* COMPV_ARCH_X86 */

	if (CompVCannyNMSGatherRow_xmpw) {
		const int16_t *gx_xmpw = gx, *gy_xmpw = gy;
		const uint16_t *g_xmpw = g;
		uint8_t *nms_xmpw = nms, *e_xmpw = e;
		for (row = rowStart; row < maxRows; ++row) {
			CompVMem::zero(e_xmpw, m_nImageWidth);
			CompVCannyNMSGatherRow_xmpw(nms_xmpw, g_xmpw, gx_xmpw, gy_xmpw, &tLow, maxCols, m_nImageStride);
			gx_xmpw += m_nImageStride, gy_xmpw += m_nImageStride, g_xmpw += m_nImageStride, e_xmpw += m_nImageStride, nms_xmpw += m_nImageStride;
		}
		colStart = (maxCols & -(xmpw - 1)); // modulo xmpw
	}
	else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	}
	if (colStart < maxCols) { // samples missing after SIMD function execution
		for (row = rowStart; row < maxRows; ++row) {
			if (!CompVCannyNMSGatherRow_xmpw) { // "e" zero'ed in "CompVCannyNMSGatherRow_xmpw"
				CompVMem::zero(e, m_nImageWidth);
			}
			CompVCannyNmsGatherRow_C(nms, g, gx, gy, tLow, colStart, maxCols, m_nImageStride);
			gx += m_nImageStride, gy += m_nImageStride, g += m_nImageStride, e += m_nImageStride, nms += m_nImageStride;
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

void CompVEdgeDeteCanny::nms_apply()
{
	// Private function -> do not check input parameters

	uint8_t *nms_ = m_pNms + m_nImageStride;
	uint16_t *g_ = m_pG + m_nImageStride;
	size_t imageWidthMinus1_ = m_nImageWidth - 1;
	size_t imageHeightMinus1_ = m_nImageHeight - 1;

	void(*CompVCannyNMSApply)(COMPV_ALIGNED(X) uint16_t* grad, COMPV_ALIGNED(X) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(X) compv_uscalar_t stride)
		= NULL;

#if COMPV_ARCH_X86
	const size_t gStrideInBytes = m_nImageStride * sizeof(uint16_t);
	if (imageWidthMinus1_ >= 8 && CompVCpu::isEnabled(compv::kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(nms_) && COMPV_IS_ALIGNED_SSE(g_) && COMPV_IS_ALIGNED_SSE(m_nImageStride) && COMPV_IS_ALIGNED_SSE(gStrideInBytes)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVCannyNMSApply = CompVCannyNMSApply_Intrin_SSE2);
		//COMPV_EXEC_IFDEF_ASM_X86(CompVCannyNMSApply = CompVCannyNMSApply_Asm_X86_SSE2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVCannyNMSApply = CompVCannyNMSApply_Asm_X64_SSE2);
	}
	if (imageWidthMinus1_ >= 16 && CompVCpu::isEnabled(compv::kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX2(nms_) && COMPV_IS_ALIGNED_AVX2(g_) && COMPV_IS_ALIGNED_AVX2(m_nImageStride) && COMPV_IS_ALIGNED_AVX2(gStrideInBytes)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVCannyNMSApply = CompVCannyNMSApply_Intrin_AVX2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVCannyNMSApply = CompVCannyNMSApply_Asm_X64_AVX2);
	}
#endif /* COMPV_ARCH_X86 */

	if (CompVCannyNMSApply) {
		CompVCannyNMSApply(g_, nms_, static_cast<compv_uscalar_t>(imageWidthMinus1_), static_cast<compv_uscalar_t>(imageHeightMinus1_), static_cast<compv_uscalar_t>(m_nImageStride));
	}
	else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		for (size_t row_ = 1; row_ < imageHeightMinus1_; ++row_) {
			for (size_t col_ = 1; col_ < imageWidthMinus1_; ++col_) { // SIMD, starts at 0 to have memory aligned
				if (nms_[col_]) {
					g_[col_] = 0, nms_[col_] = 0;
				}
			}
			nms_ += m_nImageStride;
			g_ += m_nImageStride;
		}
	}
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::hysteresis(CompVMatPtr& edges, uint16_t tLow, uint16_t tHigh, size_t rowStart, size_t rowCount)
{
	// Private function -> do not check input parameters
	size_t rowEnd = rowStart + rowCount;
	const size_t imageHeightMinus1 = m_nImageHeight - 1;
	rowEnd = COMPV_MATH_MIN(rowEnd, imageHeightMinus1);
	rowStart = COMPV_MATH_MAX(1, rowStart);
	
	size_t colStart = 1;
	size_t row;
	size_t imageWidthMinus1 = m_nImageWidth - 1;
	const uint16_t *grad = m_pG + (rowStart * m_nImageStride), *g0 = m_pG;
	uint8_t *e = edges->ptr<uint8_t>(rowStart), *e0 = edges->ptr<uint8_t>(0);

	// 8mpw -> minpack 8 for words (int16)
	void(*CompVCannyHysteresis_8mpw)(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
		= NULL;

#if COMPV_ARCH_X86
	if (imageWidthMinus1 >= 8 && CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVCannyHysteresis_8mpw = CompVCannyHysteresisRow_8mpw_Intrin_SSE2);
	}
#endif /* COMPV_ARCH_X86 */

	// SIMD execution
	if (CompVCannyHysteresis_8mpw) {
		const uint16_t *grad_8mpw = grad;
		uint8_t *e_8mpw = e;
		for (row = rowStart; row < rowEnd; ++row) {
			CompVCannyHysteresis_8mpw(row, colStart, imageWidthMinus1, imageHeightMinus1, m_nImageStride, tLow, tHigh, grad_8mpw, g0, e_8mpw, e0);
			grad_8mpw += m_nImageStride, e_8mpw += m_nImageStride;
		}
		colStart = (imageWidthMinus1 & -7);
	}
	else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	}

	// Serial execution
	if (colStart < imageWidthMinus1) { // samples missing after SIMD function execution
		for (row = rowStart; row < rowEnd; ++row) {
			CompVCannyHysteresisRow_C(row, colStart, imageWidthMinus1, imageHeightMinus1, m_nImageStride, tLow, tHigh, grad, g0, e, e0);
			grad += m_nImageStride, e += m_nImageStride;
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::newObj(CompVEdgeDetePtrPtr dete, float tLow, float tHigh, size_t kernSize)
{
	COMPV_CHECK_EXP_RETURN(!dete, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVEdgeDeteCannyPtr dete_ = new CompVEdgeDeteCanny(tLow, tHigh, kernSize);
	COMPV_CHECK_EXP_RETURN(!dete_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*dete = *dete_;
	return COMPV_ERROR_CODE_S_OK;
}

static void CompVCannyNmsGatherRow_C(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, uint16_t tLow, size_t colStart, size_t maxCols, size_t stride)
{
#if 0 // already printed in the caller function
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
#endif
	int32_t gxInt, gyInt, absgyInt, absgxInt;
	const int s = static_cast<const int>(stride);
	const int c0 = 1 - s, c1 = 1 + s;
	for (size_t col = colStart; col < maxCols; ++col) {
		if (g[col] > tLow) {
			gxInt = static_cast<int32_t>(gx[col]);
			gyInt = static_cast<int32_t>(gy[col]);
			absgyInt = COMPV_MATH_ABS_INT32(gyInt) << 16;
			absgxInt = COMPV_MATH_ABS_INT32(gxInt);
			if (absgyInt < (kCannyTangentPiOver8Int * absgxInt)) { // angle = "0° / 180°"
				if (g[col - 1] > g[col] || g[col + 1] > g[col]) {
					nms[col] = 0xff;
				}
			}
			else if (absgyInt < (kCannyTangentPiTimes3Over8Int * absgxInt)) { // angle = "45° / 225°" or "135 / 315"
				const int c = (gxInt ^ gyInt) < 0 ? c0 : c1;
				if (g[col - c] > g[col] || g[col + c] > g[col]) {
					nms[col] = 0xff;
				}
			}
			else { // angle = "90° / 270°"
				if (g[col - s] > g[col] || g[col + s] > g[col]) {
					nms[col] = 0xff;
				}
			}
		}
	}
}

static void CompVCannyHysteresisRow_C(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
#if 0 // Printed in the calling function
	if (width > 15) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	}
#endif
	CompVMatIndex edge;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	uint32_t cmp32;
	std::vector<CompVMatIndex> edges;

	for (size_t col = colStart; col < width; ++col) {
		if (grad[col] > tHigh && !e[col]) { // strong edge and not connected yet
			e[col] = 0xff;
			edges.push_back(CompVMatIndex(row, col));
			while (!edges.empty()) {
				edge = edges.back();
				edges.pop_back();
				c = edge.col;
				r = edge.row;
				if (r && c && r < height && c < width) {
					s = (r * stride) + c;
					p = e0 + s;
					g = g0 + s;
					pb = p + stride;
					pt = p - stride;
					gb = g + stride;
					gt = g - stride;
					if (g[-1] > tLow && !p[-1]) { // left
						p[-1] = 0xff;
						edges.push_back(CompVMatIndex(r, c - 1));
					}
					if (g[1] > tLow && !p[1]) { // right
						p[1] = 0xff;
						edges.push_back(CompVMatIndex(r, c + 1));
					}
					/* TOP */
					cmp32 = *reinterpret_cast<const uint32_t*>(&pt[-1]) ^ 0xffffff;
					if (cmp32) {
						if (cmp32 & 0xff && gt[-1] > tLow) { // left
							pt[-1] = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c - 1));
						}
						if (cmp32 & 0xff00 && gt[0] > tLow) { // center
							*pt = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c));
						}
						if (cmp32 & 0xff0000 && gt[1] > tLow && !pt[1]) { // right
							pt[1] = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c + 1));
						}
					}
					/* BOTTOM */
					cmp32 = *reinterpret_cast<const uint32_t*>(&pb[-1]) ^ 0xffffff;
					if (cmp32) {
						if (cmp32 & 0xff && gb[-1] > tLow) { // left
							pb[-1] = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c - 1));
						}
						if (cmp32 & 0xff00 && gb[0] > tLow) { // center
							*pb = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c));
						}
						if (cmp32 & 0xff0000 && gb[1] > tLow) { // right
							pb[1] = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c + 1));
						}
					}
				}
			}
		}
	}
}

COMPV_NAMESPACE_END()