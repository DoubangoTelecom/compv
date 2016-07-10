/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#include "compv/features/edges/compv_feature_canny_dete.h"
#include "compv/math/compv_math_convlt.h"
#include "compv/math/compv_math_utils.h"
#include "compv/compv_engine.h"
#include "compv/compv_gauss.h"

#include "compv/intrinsics/x86/features/edges/compv_feature_canny_dete_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void CannyNMSApply_Asm_X86_SSE2(COMPV_ALIGNED(SSE) uint16_t* grad, COMPV_ALIGNED(SSE) uint8_t* nms, compv::compv_uscalar_t width, compv::compv_uscalar_t height, COMPV_ALIGNED(SSE) compv::compv_uscalar_t stride);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void CannyNMSApply_Asm_X64_SSE2(COMPV_ALIGNED(SSE) uint16_t* grad, COMPV_ALIGNED(SSE) uint8_t* nms, compv::compv_uscalar_t width, compv::compv_uscalar_t height, COMPV_ALIGNED(SSE) compv::compv_uscalar_t stride);
COMPV_EXTERNC void CannyNMSApply_Asm_X64_AVX2(COMPV_ALIGNED(AVX) uint16_t* grad, COMPV_ALIGNED(AVX) uint8_t* nms, compv::compv_uscalar_t width, compv::compv_uscalar_t height, COMPV_ALIGNED(AVX) compv::compv_uscalar_t stride);
#endif /* COMPV_ARCH_X64 && COMPV_ASM */

COMPV_NAMESPACE_BEGIN()

#define COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW	(0.68f)
#define COMPV_FEATURE_DETE_CANNY_THRESHOLD_HIGH	(COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW * 2.f)

#define COMPV_FEATURE_DETE_CANY_GRAD_MIN_SAMPLES_PER_THREAD	3 // must be >= 3 because of the convolution ("rowsOverlapCount")
#define COMPV_FEATURE_DETE_CANY_NMS_MIN_SAMPLES_PER_THREAD	(20*20)

static const float kTangentPiOver8 = 0.414213568f; // tan(22.5)
static const int32_t kTangentPiOver8Int = static_cast<int32_t>(kTangentPiOver8 * (1 << 16));
static const float kTangentPiTimes3Over8 = 2.41421366f; // tan(67.5)
static const int32_t kTangentPiTimes3Over8Int = static_cast<int32_t>(kTangentPiTimes3Over8 * (1 << 16));

struct CompVCandidateEdge{
	size_t row;
	size_t col;
	uint8_t* pixel;
	const uint16_t* grad;
	CompVCandidateEdge(size_t r, size_t c, uint8_t* p, const uint16_t* g) : row(r), col(c), pixel(p), grad(g) {  }
	CompVCandidateEdge() {  }
};

CompVEdgeDeteCanny::CompVEdgeDeteCanny()
	: CompVEdgeDete(COMPV_CANNY_ID)
	, m_nImageWidth(0)
	, m_nImageHeight(0)
	, m_nImageStride(0)
	, m_fThresholdLow(COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW)
	, m_fThresholdHigh(COMPV_FEATURE_DETE_CANNY_THRESHOLD_HIGH)
	, m_pGx(NULL)
	, m_pGy(NULL)
	, m_pG(NULL)
	, m_pNms(NULL)
	, m_pcKernelVt(CompVSobel3x3Gx_vt)
	, m_pcKernelHz(CompVSobel3x3Gx_hz)
	, m_nKernelSize(3)
{

}

CompVEdgeDeteCanny::~CompVEdgeDeteCanny()
{
	CompVMem::free((void**)&m_pGx);
	CompVMem::free((void**)&m_pGy);
	CompVMem::free((void**)&m_pG);
	CompVMem::free((void**)&m_pNms);
}

// overrides CompVSettable::set
COMPV_ERROR_CODE CompVEdgeDeteCanny::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_CANNY_SET_FLOAT_THRESHOLD_LOW:{
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(float) || *((float*)valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_fThresholdLow = *((float*)valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_CANNY_SET_FLOAT_THRESHOLD_HIGH:{
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(float) || *((float*)valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_fThresholdHigh = *((float*)valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_CANNY_SET_INT32_KERNEL_SIZE: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t) || (*((int32_t*)valuePtr) != 3 && *((int32_t*)valuePtr) != 5), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nKernelSize = *((int32_t*)valuePtr);
		switch (m_nKernelSize) {
		case 3: m_pcKernelVt = CompVSobel3x3Gx_vt, m_pcKernelHz = CompVSobel3x3Gx_hz; return COMPV_ERROR_CODE_S_OK;
		case 5: m_pcKernelVt = CompVSobel5x5Gx_vt, m_pcKernelHz = CompVSobel5x5Gx_hz; return COMPV_ERROR_CODE_S_OK;
		default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER); return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
		}
	}
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

// overrides CompVEdgeDete::process
COMPV_ERROR_CODE CompVEdgeDeteCanny::process(const CompVPtr<CompVImage*>& image, CompVPtrArray(uint8_t)& edges)
{
	COMPV_CHECK_EXP_RETURN(!image || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(m_fThresholdLow >= m_fThresholdHigh, COMPV_ERROR_CODE_E_INVALID_STATE);

	// Force memory realloc if image size changes
	if (!m_pGx || !m_pGy || !m_pG || !m_pNms || image->getWidth() != m_nImageWidth || image->getHeight() != m_nImageHeight || image->getStride() != m_nImageStride) {
		CompVMem::free((void**)&m_pGx);
		CompVMem::free((void**)&m_pGy);
		CompVMem::free((void**)&m_pG);
		CompVMem::free((void**)&m_pNms);
		m_nImageWidth = static_cast<size_t>(image->getWidth());
		m_nImageHeight = static_cast<size_t>(image->getHeight());
		m_nImageStride = static_cast<size_t>(image->getStride());
		m_pGx = (int16_t*)CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight));
		COMPV_CHECK_EXP_RETURN(!m_pGx, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		m_pGy = (int16_t*)CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight));
		COMPV_CHECK_EXP_RETURN(!m_pGy, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		m_pG = (uint16_t*)CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<uint16_t>(m_nImageStride, m_nImageHeight));
		COMPV_CHECK_EXP_RETURN(!m_pG, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip) and image (required by gaussian blur)
	COMPV_CHECK_CODE_RETURN(CompVArray<uint8_t>::newObj(&edges, m_nImageHeight, m_nImageWidth, COMPV_SIMD_ALIGNV_DEFAULT, m_nImageStride));

	// Compute mean and get tLow and tMin
	// TODO(dmi): add support for otsu and median
	uint8_t mean = 1;

	// When multithreading is enabled the mean value could be "+-1" compared to the single threaded value.
	
	/* Convolution + Gradient + Mean */
	size_t threadsCount = 1;
	CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
	if (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) {
		threadsCount = m_nImageHeight / COMPV_FEATURE_DETE_CANY_GRAD_MIN_SAMPLES_PER_THREAD;
		threadsCount = COMPV_MATH_MIN_3(threadsCount, m_nImageHeight, (size_t)threadDisp->getThreadsCount());
	}

	if (threadsCount > 1) {
		const size_t rowsOverlapCount = ((m_nKernelSize >> 1) << 1); // (kernelRadius times 2)
		const size_t rowsOverlapPad = rowsOverlapCount * m_nImageStride;
		const size_t countAny = (size_t)(m_nImageHeight / threadsCount);
		const size_t countLast = (size_t)countAny + (m_nImageHeight % threadsCount);
		const size_t countAnyTimesStride = countAny * m_nImageStride;
		const uint8_t* inPtr_ = static_cast<const uint8_t*>(image->getDataPtr());
		int16_t* imgTmpGx = (int16_t*)CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight));
		COMPV_CHECK_EXP_RETURN(!imgTmpGx, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		int16_t* imgTmpGy = (int16_t*)CompVMem::malloc(CompVMathConvlt::outputSizeInBytes<int16_t>(m_nImageStride, m_nImageHeight));
		COMPV_CHECK_EXP_RETURN(!imgTmpGy, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		uint8_t* means = (uint8_t*)CompVMem::malloc(threadsCount * sizeof(uint8_t));
		int16_t* tmpPtrGx_ = imgTmpGx;
		int16_t* tmpPtrGy_ = imgTmpGy;
		int16_t* outPtrGx_ = m_pGx;
		int16_t* outPtrGy_ = m_pGy;
		uint16_t* outPtrG_ = m_pG;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtrFirst = [&](const uint8_t* ptrIn, int16_t* ptrOutGx, int16_t* ptrTmpGx, int16_t* ptrOutGy, int16_t* ptrTmpGy, uint16_t* ptrOutG, uint8_t* ptrMean, size_t h) -> COMPV_ERROR_CODE {
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn, ptrTmpGx, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize);
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn, ptrTmpGy, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize);
			CompVMathConvlt::convlt1Vert<int16_t, int16_t, int16_t>(ptrTmpGx, ptrOutGx, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, true, false);
			CompVMathConvlt::convlt1Vert<int16_t, int16_t, int16_t>(ptrTmpGy, ptrOutGy, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, true, false);
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(ptrOutGx, ptrOutGy, ptrOutG, m_nImageWidth, h + rowsOverlapCount, m_nImageStride)));
			uint8_t mean_ = 1;
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::mean<uint8_t>(ptrIn, m_nImageWidth, h, m_nImageStride, mean_)));
			*ptrMean = mean_;
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrOthers = [&](const uint8_t* ptrIn, int16_t* ptrOutGx, int16_t* ptrTmpGx, int16_t* ptrOutGy, int16_t* ptrTmpGy, uint16_t* ptrOutG, uint8_t* ptrMean, size_t h, bool last) -> COMPV_ERROR_CODE {
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - rowsOverlapPad, ptrTmpGx - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize);
			CompVMathConvlt::convlt1Hz<uint8_t, int16_t, int16_t>(ptrIn - rowsOverlapPad, ptrTmpGy - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize);
			CompVMathConvlt::convlt1Vert<int16_t, int16_t, int16_t>(ptrTmpGx - rowsOverlapPad, ptrOutGx - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelVt, m_nKernelSize, false, last);
			uint16_t* g_ = ptrOutG - rowsOverlapPad;
			CompVMathConvlt::convlt1Vert<int16_t, int16_t, int16_t>(ptrTmpGy - rowsOverlapPad, ptrOutGy - rowsOverlapPad, m_nImageWidth, h + rowsOverlapCount, m_nImageStride, m_pcKernelHz, m_nKernelSize, false, last);
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(ptrOutGx - rowsOverlapPad, ptrOutGy - rowsOverlapPad, g_, m_nImageWidth, h + rowsOverlapCount, m_nImageStride)));
			uint8_t mean_ = 1;
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::mean<uint8_t>(ptrIn, m_nImageWidth, h, m_nImageStride, mean_)));
			*ptrMean = mean_;
			return COMPV_ERROR_CODE_S_OK;
		};
		// first
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrFirst, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, &means[0], countAny), taskIds));
		inPtr_ += countAny * m_nImageStride;
		tmpPtrGx_ += countAnyTimesStride;
		outPtrGx_ += countAnyTimesStride;
		tmpPtrGy_ += countAnyTimesStride;
		outPtrGy_ += countAnyTimesStride;
		outPtrG_ += countAnyTimesStride;
		// others
		for (size_t threadIdx = 1; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, &means[threadIdx], countAny, false), taskIds));
			inPtr_ += countAnyTimesStride;
			tmpPtrGx_ += countAnyTimesStride;
			outPtrGx_ += countAnyTimesStride;
			tmpPtrGy_ += countAnyTimesStride;
			outPtrGy_ += countAnyTimesStride;
			outPtrG_ += countAnyTimesStride;
		}
		// last
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, &means[threadsCount - 1], countLast, true), taskIds));
		// wait
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		// mean
		uint32_t sum = 0;
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			sum += means[threadIdx];
		}
		mean = (sum + 1) / (uint32_t)threadsCount;
		CompVMem::free((void**)&imgTmpGx);
		CompVMem::free((void**)&imgTmpGy);
		CompVMem::free((void**)&means);
	}
	else {
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(static_cast<const uint8_t*>(image->getDataPtr()), m_nImageWidth, m_nImageStride, m_nImageHeight, m_pcKernelVt, m_pcKernelHz, m_nKernelSize, m_pGx)));
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(static_cast<const uint8_t*>(image->getDataPtr()), m_nImageWidth, m_nImageStride, m_nImageHeight, m_pcKernelHz, m_pcKernelVt, m_nKernelSize, m_pGy)));
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, m_nImageWidth, m_nImageHeight, m_nImageStride)));
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::mean<uint8_t>((const uint8_t*)image->getDataPtr(), m_nImageWidth, m_nImageHeight, m_nImageStride, mean)));
	}
	
	mean = COMPV_MATH_MAX(1, mean);
	uint16_t tLow = static_cast<uint16_t>(mean * m_fThresholdLow);
	uint16_t tHigh = static_cast<uint16_t>(mean * m_fThresholdHigh);
	tLow = COMPV_MATH_MAX(1, tLow);
	tHigh = COMPV_MATH_MAX(tLow + 2, tHigh);

	/* NMS + Hysteresis */
	if (!m_pNms) {
		m_pNms = (uint8_t*)CompVMem::calloc(m_nImageStride * m_nImageHeight, sizeof(uint8_t));
		COMPV_CHECK_EXP_RETURN(!m_pNms, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	if (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) {
		threadsCount = (m_nImageHeight * m_nImageWidth) / COMPV_FEATURE_DETE_CANY_NMS_MIN_SAMPLES_PER_THREAD;
		threadsCount = COMPV_MATH_MIN_3(threadsCount, m_nImageHeight, (size_t)threadDisp->getThreadsCount());
	}
	if (threadsCount > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		const size_t countAny = (size_t)(m_nImageHeight / threadsCount);
		const size_t countLast = (size_t)countAny + (m_nImageHeight % threadsCount);
		auto funcPtrNMS = [&](size_t rowStart, size_t rowCount) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(nms_gather(edges, tLow, rowStart, rowCount));
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrHysteresis = [&](size_t rowStart, size_t rowCount) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(hysteresis(edges, tLow, tHigh, rowStart, rowCount));
			return COMPV_ERROR_CODE_S_OK;
		};
		size_t rowStart, threadIdx;
		// NMS gathering
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCount - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNMS, rowStart, countAny), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNMS, rowStart, countLast), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		// NMS supp
		nms_supp();
		// Hysteresis
		taskIds.clear();
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCount - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrHysteresis, rowStart, countAny), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrHysteresis, rowStart, countLast), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
	}
	else {
		COMPV_CHECK_CODE_RETURN(nms_gather(edges, tLow, 0, m_nImageHeight));
		nms_supp();
		COMPV_CHECK_CODE_RETURN(hysteresis(edges, tLow, tHigh, 0, m_nImageHeight));
	}

	return COMPV_ERROR_CODE_S_OK;
}

// NonMaximaSuppression
COMPV_ERROR_CODE CompVEdgeDeteCanny::nms_gather(CompVPtrArray(uint8_t)& edges, uint16_t tLow, size_t rowStart, size_t rowCount)
{
	// Private function -> do not check input parameters
	size_t maxRows = rowStart + rowCount;
	maxRows = COMPV_MATH_MIN(maxRows, m_nImageHeight - 1);
	rowStart = COMPV_MATH_MAX(1, rowStart);
	size_t rowStartTimesStride = rowStart * m_nImageStride;

	size_t row, col;
	const size_t maxCols = edges->cols() - 1;
	const int16_t *gx = m_pGx + rowStartTimesStride + 1, *gy = m_pGy + rowStartTimesStride + 1;
	uint16_t *g = m_pG + rowStartTimesStride + 1;
	const uint16_t *top = g - m_nImageStride, *bottom = g + m_nImageStride, *left = g - 1, *right = g + 1;
	uint8_t *nms = m_pNms + rowStartTimesStride + 1;
	uint8_t *e = const_cast<uint8_t*>(edges->ptr(rowStart));
	int32_t gxInt, gyInt, absgyInt, absgxInt;
	const int stride = static_cast<const int>(m_nImageStride);
	const int pad = static_cast<const int>(m_nImageStride - m_nImageWidth) + 2;

	// non-maximasupp is multi-threaded and we will use this property to zero the edge buffer with low cost (compared to edges->zeroall() which is not MT)
	if (rowStart == 1) { // First time ?
		CompVMem::zero(const_cast<uint8_t*>(edges->ptr(0)), m_nImageWidth); // zero first line
		CompVMem::zero(const_cast<uint8_t*>(edges->ptr(m_nImageHeight - 1)), m_nImageWidth); // zero last line
	}

	// mark points to supp
	for (row = rowStart; row < maxRows; ++row) {
		CompVMem::zero(e, m_nImageWidth);
		// TODO(dmi): add support SIMD ->  "nms_gather_row"
		for (col = 1; col < maxCols; ++col, ++nms, ++gx, ++gy, ++g, ++top, ++bottom, ++left, ++right) {			
			if (*g >= tLow) {
				gxInt = static_cast<int32_t>(*gx);
				gyInt = static_cast<int32_t>(*gy);
				absgyInt = ((gyInt ^ (gyInt >> 31)) - (gyInt >> 31)) << 16;
				absgxInt = ((gxInt ^ (gxInt >> 31)) - (gxInt >> 31));
				if (absgyInt < (kTangentPiOver8Int * absgxInt)) { // angle = "0° / 180°"
					if (*left > *g || *right > *g) {
						*nms = 0xff;
					}
				}
				else if (absgyInt < (kTangentPiTimes3Over8Int * absgxInt)) { // angle = "45° / 225°" or "135 / 315"
					const int c = (gxInt ^ gyInt) < 0 ? 1 - stride : 1 + stride;
					if (g[-c] > *g || g[c] > *g) {
						*nms = 0xff;
					}
				}
				else { // angle = "90° / 270°"
					if (*top > *g || *bottom > *g) {
						*nms = 0xff;
					}
				}
			}
		}
		top += pad;
		bottom += pad;
		left += pad;
		right += pad;
		gx += pad;
		gy += pad;
		g += pad;
		e += m_nImageStride;
		nms += pad;
	}

	return COMPV_ERROR_CODE_S_OK;
}

void CompVEdgeDeteCanny::nms_supp()
{
	// Private function -> do not check input parameters

	uint8_t *nms_ = m_pNms + m_nImageStride;
	uint16_t *g_ = m_pG + m_nImageStride;
	size_t imageWidthMinus1_ = m_nImageWidth - 1;
	size_t imageHeightMinus1_ = m_nImageHeight - 1;

#if COMPV_ARCH_X86
	const size_t gStrideInBytes = m_nImageStride * sizeof(uint16_t);
	void(*CannyNMSApply)(COMPV_ALIGNED(X) uint16_t* grad, COMPV_ALIGNED(X) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(X) compv_uscalar_t stride) = NULL;
	if (imageWidthMinus1_ >= 8 && CompVCpu::isEnabled(compv::kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(nms_) && COMPV_IS_ALIGNED_SSE(g_) && COMPV_IS_ALIGNED_SSE(m_nImageStride) && COMPV_IS_ALIGNED_SSE(gStrideInBytes)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CannyNMSApply = CannyNMSApply_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X86(CannyNMSApply = CannyNMSApply_Asm_X86_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CannyNMSApply = CannyNMSApply_Asm_X64_SSE2);
	}
	if (imageWidthMinus1_ >= 16 && CompVCpu::isEnabled(compv::kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX2(nms_) && COMPV_IS_ALIGNED_AVX2(g_) && COMPV_IS_ALIGNED_AVX2(m_nImageStride) && COMPV_IS_ALIGNED_AVX2(gStrideInBytes)) {
		COMPV_EXEC_IFDEF_ASM_X64(CannyNMSApply = CannyNMSApply_Asm_X64_AVX2);
	}
	if (CannyNMSApply) {
		CannyNMSApply(g_, nms_, (compv_uscalar_t)imageWidthMinus1_, (compv_uscalar_t)imageHeightMinus1_, (compv_uscalar_t)m_nImageStride);
		return;
	}
#endif /* COMPV_ARCH_X86 */

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

static COMPV_INLINE void connectEdgeInPlace(uint8_t* pixel, const uint16_t* grad, size_t rowIdx, size_t colIdx, int stride, size_t maxRows, size_t maxCols, uint16_t tLow, CompVPtr<CompVBox<CompVCandidateEdge>* >& edges)
{
	*pixel = 0xff;
	const CompVCandidateEdge* e;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r;
	uint8_t *pb, *pt;

	edges->push(CompVCandidateEdge(rowIdx, colIdx, pixel, grad));

	while ((e = edges->pop_back())) {
		c = e->col;
		r = e->row;
		if (r && c && r < maxRows && c < maxCols) {
			p = e->pixel;
			g = e->grad;
			pb = p + stride;
			pt = p - stride;
			gb = g + stride;
			gt = g - stride;
			if (!p[-1] && g[-1] >= tLow) { // left
				p[-1] = 0xff;
				edges->push(CompVCandidateEdge(r, c - 1, p - 1, g - 1));
			}
			if (!p[1] && g[1] >= tLow) { // right
				p[1] = 0xff;
				edges->push(CompVCandidateEdge(r, c + 1, p + 1, g + 1));
			}
			if (!pt[-1] && gt[-1] >= tLow) { // left-top
				pt[-1] = 0xff;
				edges->push(CompVCandidateEdge(r - 1, c - 1, pt - 1, gt - 1));
			}
			if (!*pt && *gt >= tLow) { // top
				*pt = 0xff;
				edges->push(CompVCandidateEdge(r - 1, c, pt, gt));
			}
			if (!pt[1] && gt[1] >= tLow) { // right-top
				pt[1] = 0xff;
				edges->push(CompVCandidateEdge(r - 1, c + 1, pt + 1, gt + 1));
			}
			if (!pb[-1] && gb[-1] >= tLow) { // left-bottom
				pb[-1] = 0xff;
				edges->push(CompVCandidateEdge(r + 1, c - 1, pb - 1, gb - 1));
			}
			if (!*pb && *gb >= tLow) { // bottom
				*pb = 0xff;
				edges->push(CompVCandidateEdge(r + 1, c, pb, gb));
			}
			if (!pb[1] && gb[1] >= tLow) { // right-bottom
				pb[1] = 0xff;
				edges->push(CompVCandidateEdge(r + 1, c + 1, pb + 1, gb + 1));
			}
		}
	}
}

// StackOverflow when there are more than 4k connected edges
static COMPV_INLINE void connectEdgeRecursive(uint8_t* pixel, const uint16_t* grad, size_t rowIdx, size_t colIdx, int stride, size_t maxRows, size_t maxCols, uint16_t tLow)
{	
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	if (rowIdx && colIdx && rowIdx < maxRows && colIdx < maxCols) {
		*pixel = 0xff; // set as strong edge
		if (pixel[-1] ^ 0xff && grad[-1] >= tLow) { // left
			connectEdgeRecursive(pixel - 1, grad - 1, rowIdx, colIdx - 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[1] ^ 0xff && grad[1] >= tLow) { // right
			connectEdgeRecursive(pixel + 1, grad + 1, rowIdx, colIdx + 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[-stride - 1] ^ 0xff && grad[-stride - 1] >= tLow) { // left-top
			connectEdgeRecursive(pixel - stride - 1, grad - stride - 1, rowIdx - 1, colIdx - 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[-stride] ^ 0xff && grad[-stride] >= tLow) { // top
			connectEdgeRecursive(pixel - stride, grad - stride, rowIdx - 1, colIdx, stride, maxRows, maxCols, tLow);
		}
		if (pixel[-stride + 1] ^ 0xff && grad[-stride + 1] >= tLow) { // right-top
			connectEdgeRecursive(pixel - stride + 1, grad - stride + 1, rowIdx - 1, colIdx + 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[stride - 1] ^ 0xff && grad[stride - 1] >= tLow) { // left-bottom
			connectEdgeRecursive(pixel + stride - 1, grad + stride - 1, rowIdx + 1, colIdx - 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[stride] ^ 0xff && grad[stride] >= tLow) { // bottom
			connectEdgeRecursive(pixel + stride, grad + stride, rowIdx + 1, colIdx, stride, maxRows, maxCols, tLow);
		}
		if (pixel[stride + 1] ^ 0xff && grad[stride + 1] >= tLow) { // right-bottom
			connectEdgeRecursive(pixel + stride + 1, grad + stride + 1, rowIdx + 1, colIdx + 1, stride, maxRows, maxCols, tLow);
		}
	}
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::hysteresis(CompVPtrArray(uint8_t)& edges, uint16_t tLow, uint16_t tHigh, size_t rowStart, size_t rowCount)
{
	// Private function -> do not check input parameters
	size_t rowEnd = rowStart + rowCount;
	const size_t imageHeightMinus1 = m_nImageHeight - 1;
	rowEnd = COMPV_MATH_MIN(rowEnd, imageHeightMinus1);
	rowStart = COMPV_MATH_MAX(1, rowStart);

	CompVPtr<CompVBox<CompVCandidateEdge>* >candidates;
	CompVBox<CompVCandidateEdge>::newObj(&candidates);

	size_t row, col;
	const size_t imageWidthMinus1 = m_nImageWidth - 1;
	const uint16_t *g = m_pG + (rowStart * m_nImageStride);
	uint8_t* e = const_cast<uint8_t*>(edges->ptr(rowStart));
	const int stride = static_cast<const int>(m_nImageStride);

	for (row = rowStart; row < rowEnd; ++row) {
		for (col = 1; col < imageWidthMinus1; ++col) {
			if (!*e && g[col] >= tHigh) { // strong edge
#if 1
				connectEdgeInPlace((e + col), (g + col), row, col, stride, imageHeightMinus1, imageWidthMinus1, tLow, candidates);
#else
				connectEdgeRecursive((e + col), (g + col), row, col, stride, imageHeightMinus1, imageWidthMinus1, tLow);
#endif
			}
		}
		g += m_nImageStride;
		e += m_nImageStride;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::newObj(CompVPtr<CompVEdgeDete* >* dete)
{
	COMPV_CHECK_EXP_RETURN(!dete, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVEdgeDeteCanny* >dete_ = new CompVEdgeDeteCanny();
	COMPV_CHECK_EXP_RETURN(!dete_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*dete = *dete_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
