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
#include "compv/compv_md5.h" // FIXME(dmi): remove

COMPV_NAMESPACE_BEGIN()

#define COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW	(0.68f)
#define COMPV_FEATURE_DETE_CANNY_THRESHOLD_HIGH	(COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW * 2.f)

#define COMPV_FEATURE_DETE_CANY_MIN_SAMPLES_PER_THREAD	3 // must be >= 3 because of the convolution ("rowsOverlapCount")

static const float kTangentPiOver8 = 0.414213568f; // tan(22.5)
static const int32_t kTangentPiOver8Int = static_cast<int32_t>(kTangentPiOver8 * (1 << 16));
static const float kTangentPiTimes3Over8 = 2.41421366f; // tan(67.5)
static const int32_t kTangentPiTimes3Over8Int = static_cast<int32_t>(kTangentPiTimes3Over8 * (1 << 16));

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
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // MT

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
	
	int32_t threadsCount = 1;
	CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
	if (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) {
		threadsCount = static_cast<int32_t>(m_nImageHeight / COMPV_FEATURE_DETE_CANY_MIN_SAMPLES_PER_THREAD);
		threadsCount = COMPV_MATH_MIN(threadsCount, threadDisp->getThreadsCount());
	}

	// When multithreading is enabled the mean value could be "+-1" compared to the single threaded value.

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
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::mean<uint8_t, uint32_t>(ptrIn, m_nImageWidth, h, m_nImageStride, mean_)));
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
			COMPV_CHECK_CODE_RETURN((CompVMathUtils::mean<uint8_t, uint32_t>(ptrIn, m_nImageWidth, h, m_nImageStride, mean_)));
			*ptrMean = mean_;
			return COMPV_ERROR_CODE_S_OK;
		};
		/* first */
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrFirst, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, &means[0], countAny), taskIds));
		inPtr_ += countAny * m_nImageStride;
		tmpPtrGx_ += countAnyTimesStride;
		outPtrGx_ += countAnyTimesStride;
		tmpPtrGy_ += countAnyTimesStride;
		outPtrGy_ += countAnyTimesStride;
		outPtrG_ += countAnyTimesStride;
		/* others */
		for (int32_t threadIdx = 1; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, &means[threadIdx], countAny, false), taskIds));
			inPtr_ += countAnyTimesStride;
			tmpPtrGx_ += countAnyTimesStride;
			outPtrGx_ += countAnyTimesStride;
			tmpPtrGy_ += countAnyTimesStride;
			outPtrGy_ += countAnyTimesStride;
			outPtrG_ += countAnyTimesStride;
		}
		/* last */
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrOthers, inPtr_, outPtrGx_, tmpPtrGx_, outPtrGy_, tmpPtrGy_, outPtrG_, &means[threadsCount - 1], countLast, true), taskIds));
		/* wait */
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		/* mean */
		uint32_t sum = 0;
		for (int32_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			sum += means[threadIdx];
		}
		mean = (sum + 1) / threadsCount;
		CompVMem::free((void**)&imgTmpGx);
		CompVMem::free((void**)&imgTmpGy);
		CompVMem::free((void**)&means);
	}
	else {
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(static_cast<const uint8_t*>(image->getDataPtr()), m_nImageWidth, m_nImageStride, m_nImageHeight, m_pcKernelVt, m_pcKernelHz, m_nKernelSize, m_pGx)));
		COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>(static_cast<const uint8_t*>(image->getDataPtr()), m_nImageWidth, m_nImageStride, m_nImageHeight, m_pcKernelHz, m_pcKernelVt, m_nKernelSize, m_pGy)));
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, m_nImageWidth, m_nImageHeight, m_nImageStride)));
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::mean<uint8_t, uint32_t>((const uint8_t*)image->getDataPtr(), m_nImageWidth, m_nImageHeight, m_nImageStride, mean)));
	}
	
	mean = COMPV_MATH_MAX(1, mean);
	uint16_t tLow = static_cast<uint16_t>(mean * m_fThresholdLow);
	uint16_t tHigh = static_cast<uint16_t>(mean * m_fThresholdHigh);
	tLow = COMPV_MATH_MAX(1, tLow);
	tHigh = COMPV_MATH_MAX(tLow + 2, tHigh);

	// NMS
	COMPV_CHECK_CODE_RETURN(nms(edges, tLow));
	// Hysteresis
	hysteresis(edges, tLow, tHigh);

	return COMPV_ERROR_CODE_S_OK;
}

// NonMaximaSuppression
COMPV_ERROR_CODE CompVEdgeDeteCanny::nms(CompVPtrArray(uint8_t)& edges, uint16_t tLow, size_t rowStart, size_t rowCount)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // MT and SIMD
	if (!m_pNms) {
		m_pNms = (uint8_t*)CompVMem::calloc(edges->rows() * edges->cols(), sizeof(uint8_t));
		COMPV_CHECK_EXP_RETURN(!m_pNms, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	rowStart = COMPV_MATH_MAX(1, rowStart);
	size_t maxRows = rowStart + rowCount;
	maxRows = COMPV_MATH_MIN(maxRows, rowStart);

	size_t row, col;
	const size_t maxRows = edges->rows() - 1, maxCols = edges->cols() - 1;
	const int16_t *gx = m_pGx + m_nImageStride + 1, *gy = m_pGy + m_nImageStride + 1;
	uint16_t *g = m_pG + m_nImageStride + 1;
	const uint16_t *top = g - m_nImageStride, *bottom = g + m_nImageStride, *left = g - 1, *right = g + 1;
	uint8_t *nms = m_pNms + m_nImageWidth + 1;
	uint8_t *e = const_cast<uint8_t*>(edges->ptr(1));
	int32_t gxInt, gyInt, absgyInt, absgxInt;
	const int stride = static_cast<const int>(m_nImageStride);
	const int pad = static_cast<const int>(m_nImageStride - m_nImageWidth) + 2;

	// non-maximasupp is multi-threaded and we will use this property to zero the edge buffer with low cost (compared to edges->zeroall() which is not MT)
	// TODO(dmi): perform next op only if startIndex == 0
	CompVMem::zero(e - m_nImageStride, m_nImageWidth); // zero first line
	CompVMem::zero(e + ((maxRows - 1) * m_nImageStride), m_nImageWidth); // zero last line

	// mark points to supp
	for (row = 1; row < maxRows; ++row) {
		CompVMem::zero(e, m_nImageWidth);
		for (col = 1; col < maxCols; ++col, ++nms, ++gx, ++gy, ++g, ++top, ++bottom, ++left, ++right) {
			// The angle is guessed using the first quadrant ([0-45] degree) only then completed.
			// We want "arctan(gy/gx)" to be within [0, 22.5], [22.5, 67.5], ]67.5, inf[, with 67.5 = (45. + 22.5)
			//!\\ All NMS values must be set to reset all values (no guarantee it contains zeros).
			
			// "theta = arctan(gy/gx)" -> "tan(theta) = gy/gx" -> compare "gy/gx" with "tan(quadrants)"
			// Instead of comparing "abs(gy/gx)" with "tanTheta" we will compare "abs(gy)" with "tanTheta*abs(gx)" to avoid division.
			
			if (*g >= tLow) {
				gxInt = static_cast<int32_t>(*gx);
				gyInt = static_cast<int32_t>(*gy);
				absgyInt = ((gyInt ^ (gyInt >> 31)) - (gyInt >> 31)) << 16;
				absgxInt = ((gxInt ^ (gxInt >> 31)) - (gxInt >> 31));
				if (absgyInt < (kTangentPiOver8Int * absgxInt)) { // angle = "0° / 180°"
					if (*left > *g || *right > *g) {
						*nms = 1;
					}
				}
				else if (absgyInt < (kTangentPiTimes3Over8Int * absgxInt)) { // angle = "45° / 225°" or "135 / 315"
					const int c = (gxInt ^ gyInt) < 0 ? 1 - stride : 1 + stride;
					if (g[-c] > *g || g[c] > *g) {
						*nms = 1;
					}
				}
				else { // angle = "90° / 270°"
					if (*top > *g || *bottom > *g) {
						*nms = 1;
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
		nms += 2;
	}

	// supp marked points
	nms = m_pNms + m_nImageWidth + 1;
	g = m_pG + m_nImageStride + 1;
	for (row = 1; row < maxRows; ++row) {
		for (col = 1; col < maxCols; ++col, ++g, ++nms) {
			if (*nms) {
				*g = 0;
				*nms = 0;
			}
		}
		nms += 2;
		g += pad;
	}

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_INLINE void connectEdge(uint8_t* pixel, const uint16_t* g, size_t rowIdx, size_t colIdx, int stride, size_t maxRows, size_t maxCols, uint16_t tLow)
{	
	// Private function -> do not check input parameters
	if (rowIdx && colIdx && rowIdx < maxRows && colIdx < maxCols) {
		*pixel = 0xff; // set as strong edge
		if (pixel[-1] ^ 0xff && g[-1] >= tLow) { // left
			connectEdge(pixel - 1, g - 1, rowIdx, colIdx - 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[1] ^ 0xff && g[1] >= tLow) { // right
			connectEdge(pixel + 1, g + 1, rowIdx, colIdx + 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[-stride - 1] ^ 0xff && g[-stride - 1] >= tLow) { // left-top
			connectEdge(pixel - stride - 1, g - stride - 1, rowIdx - 1, colIdx - 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[-stride] ^ 0xff && g[-stride] >= tLow) { // top
			connectEdge(pixel - stride, g - stride, rowIdx - 1, colIdx, stride, maxRows, maxCols, tLow);
		}
		if (pixel[-stride + 1] ^ 0xff && g[-stride + 1] >= tLow) { // right-top
			connectEdge(pixel - stride + 1, g - stride + 1, rowIdx - 1, colIdx + 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[stride - 1] ^ 0xff && g[stride - 1] >= tLow) { // left-bottom
			connectEdge(pixel + stride - 1, g + stride - 1, rowIdx + 1, colIdx - 1, stride, maxRows, maxCols, tLow);
		}
		if (pixel[stride] ^ 0xff && g[stride] >= tLow) { // bottom
			connectEdge(pixel + stride, g + stride, rowIdx + 1, colIdx, stride, maxRows, maxCols, tLow);
		}
		if (pixel[stride + 1] ^ 0xff && g[stride + 1] >= tLow) { // right-bottom
			connectEdge(pixel + stride + 1, g + stride + 1, rowIdx + 1, colIdx + 1, stride, maxRows, maxCols, tLow);
		}
	}
}

void CompVEdgeDeteCanny::hysteresis(CompVPtrArray(uint8_t)& edges, uint16_t tLow, uint16_t tHigh)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();  // MT

	size_t row, col;
	const size_t maxRows = edges->rows() - 1, maxCols = edges->cols() - 1;
	const uint16_t *g = m_pG + m_nImageStride;
	uint8_t* e = const_cast<uint8_t*>(edges->ptr(1));
	const int stride = static_cast<const int>(m_nImageStride);
	for (row = 1; row < maxRows; ++row) {
		for (col = 1; col < maxCols; ++col) {
			if (g[col] >= tHigh) { // strong edge
				connectEdge((e + col), (g + col), row, col, stride, maxRows, maxCols, tLow);
			}
		}
		g += m_nImageStride;
		e += m_nImageStride;
	}
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
