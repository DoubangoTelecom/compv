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

COMPV_NAMESPACE_BEGIN()

static const int COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIZE = 5;
static const float COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIGMA = 1.4f;

#define COMPV_FEATURE_DETE_CANNY_TMIN 15
#define COMPV_FEATURE_DETE_CANNY_TMAX 65

CompVEdgeDeteCanny::CompVEdgeDeteCanny()
	: CompVEdgeDete(COMPV_CANNY_ID)
	, m_nImageWidth(0)
	, m_nImageHeight(0)
	, m_nImageStride(0)
	, m_pGx(NULL)
	, m_pGy(NULL)
	, m_pG(NULL)
	, m_pDirs(NULL)
	, m_pNms(NULL)
{

}

CompVEdgeDeteCanny::~CompVEdgeDeteCanny()
{
	CompVMem::free((void**)&m_pGx);
	CompVMem::free((void**)&m_pGy);
	CompVMem::free((void**)&m_pG);
	CompVMem::free((void**)&m_pDirs);
	CompVMem::free((void**)&m_pNms);
}

// overrides CompVSettable::set
COMPV_ERROR_CODE CompVEdgeDeteCanny::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case -1:
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

// overrides CompVEdgeDete::process
COMPV_ERROR_CODE CompVEdgeDeteCanny::process(const CompVPtr<CompVImage*>& image, CompVPtrArray(uint8_t)& edges)
{
	COMPV_CHECK_EXP_RETURN(!image || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // MT

	// Force memory realloc if image size changes
	if (!m_pGx || !m_pGy || !m_pG || !m_pDirs || !m_pNms || image->getWidth() != m_nImageWidth || image->getHeight() != m_nImageHeight || image->getStride() != m_nImageStride) {
		CompVMem::free((void**)&m_pGx);
		CompVMem::free((void**)&m_pGy);
		CompVMem::free((void**)&m_pG);
		CompVMem::free((void**)&m_pDirs);
		CompVMem::free((void**)&m_pNms);
		m_nImageWidth = static_cast<size_t>(image->getWidth());
		m_nImageHeight = static_cast<size_t>(image->getHeight());
		m_nImageStride = static_cast<size_t>(image->getStride());
	}
	
	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip) and image (required by gaussian blur)
	COMPV_CHECK_CODE_RETURN(CompVArray<uint8_t>::newObj(&edges, m_nImageHeight, m_nImageWidth, COMPV_SIMD_ALIGNV_DEFAULT, m_nImageStride));

	// Gaussian Blurr
	if (m_gblurKernFxp) {
		// Fixed-point
		COMPV_CHECK_CODE_RETURN(m_gblur->convlt1_fxp((uint8_t*)image->getDataPtr(), (int)m_nImageWidth, (int)m_nImageStride, (int)m_nImageHeight, m_gblurKernFxp->ptr(), m_gblurKernFxp->ptr(), COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIZE, (uint8_t*)edges->ptr()));
	}
	else {
		// Floating-point
		COMPV_CHECK_CODE_RETURN(m_gblur->convlt1((uint8_t*)image->getDataPtr(), (int)m_nImageWidth, (int)m_nImageStride, (int)m_nImageHeight, m_gblurKernFloat->ptr(), m_gblurKernFloat->ptr(), COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIZE, (uint8_t*)edges->ptr()));
	}

	// FIXME: use Scharr instead of sobel
	// Convolution
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)edges->ptr(), m_nImageWidth, m_nImageStride, m_nImageHeight, &CompVSobelGx_vt[0], &CompVSobelGx_hz[0], 3, m_pGx)));
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)edges->ptr(), m_nImageWidth, m_nImageStride, m_nImageHeight, &CompVSobelGx_hz[0], &CompVSobelGx_vt[0], 3, m_pGy)));

	// Compute gradiant using L1 distance
	// FIXME: gmax not needed
	uint16_t gmax = 1;
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, gmax, m_nImageWidth, m_nImageHeight, m_nImageStride)));

	// scale (normalization)
	//float scale = 255.f / float(gmax);
	//uint8_t* edgesPtr = const_cast<uint8_t*>(edges->ptr());
	//COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip<uint16_t, float, uint8_t>(m_pG, scale, edgesPtr, 0, 255, m_nImageWidth, m_nImageHeight, m_nImageStride)));

	// Directions
	COMPV_CHECK_CODE_RETURN(direction());

	// NMS
	COMPV_CHECK_CODE_RETURN(nms(edges));

	// Hysteresis
	COMPV_CHECK_CODE_RETURN(hysteresis(edges));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::direction()
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	/*
	gx == 0, gy == 0				-> dir = 0, angle = 0

	gx > 0,	gy = 0					-> dir = 1, angle = 0
	gx > 0, gy > 0, gx > gy			-> dir = 2, angle = [0, 45]
	gx > 0, gy > 0, gx == gy		-> dir = 3, angle = 45
	gx > 0, gy > 0, gx < gy			-> dir = 4, angle = [45, 90]
	gx = 0, gy > 0					-> dir = 5, angle = 90

	gx < 0, gy > 0, abs(gx) < gy	-> dir = 6, angle = [90, 135]
	gx < 0, gy > 0, abs(gx) == gy	-> dir = 7, angle = 135
	gx < 0, gy > 0, abs(gx) > gy	-> dir = 8, angle = [135, 180]
	gx < 0, gy = 0					-> dir = 9, angle = 180

	gx < 0, gy < 0, gx < gy			-> dir = 10, angle = [180, 225]
	gx < 0, gy < 0, gx == gy		-> dir = 11, angle = 225
	gx < 0, gy < 0, gx > gy			-> dir = 12, angle = [225, 270]
	gx = 0, gy < 0					-> dir = 13, angle = 270

	gx > 0, gy < 0, gx < abs(gy)	-> dir = 14, angle = [270, 315]
	gx > 0, gy < 0, gx == abs(gy)	-> dir = 15, angle = 315
	gx > 0, gy < 0, gx > abs(gy)	-> dir = 16, angle = [315, 360]
	*/

	if (!m_pDirs) {
		m_pDirs = (uint8_t*)CompVMem::malloc(m_nImageWidth * m_nImageHeight * sizeof(uint8_t));
		COMPV_CHECK_EXP_RETURN(!m_pDirs, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	uint8_t* dirs = m_pDirs;
	const int16_t* gx = m_pGx;
	const int16_t* gy = m_pGy;
	for (size_t j = 0; j < m_nImageHeight; ++j) {
		for (size_t i = 0; i < m_nImageWidth; ++i) {
			if (gx[i] == gy[i] && gx[i] == 0) {
				dirs[i] = 0;
			}
			else if (gx[i] > 0 && gy[i] == 0) {
				dirs[i] = 1;
			}
			else if (gx[i] > 0 && gy[i] > 0 && gx[i] > gy[i]) {
				dirs[i] = 2;
			}
			else if (gx[i] > 0 && gy[i] > 0 && gx[i] == gy[i]) {
				dirs[i] = 3;
			}
			else if (gx[i] > 0 && gy[i] > 0, gx[i] < gy[i]) {
				dirs[i] = 4;
			}
			else if (gx[i] == 0 && gy[i] > 0) {
				dirs[i] = 5;
			}
			else if (gx[i] < 0 && gy[i] > 0 && -gx[i] < gy[i]) {
				dirs[i] = 6;
			}
			else if (gx[i] < 0 && gy[i] > 0 && -gx[i] == gy[i]) {
				dirs[i] = 7;
			}
			else if (gx[i] < 0 && gy[i] > 0 && -gx[i] > gy[i]) {
				dirs[i] = 8;
			}
			else if (gx[i] < 0 && gy[i] == 0) {
				dirs[i] = 9;
			}
			else if (gx[i] < 0 && gy[i] < 0 && gx[i] < gy[i]) {
				dirs[i] = 10;
			}
			else if (gx[i] < 0 && gy[i] < 0 && gx[i] == gy[i]) {
				dirs[i] = 11;
			}
			else if (gx[i] < 0 && gy[i] < 0 && gx[i] > gy[i]) {
				dirs[i] = 12;
			}
			else if (gx[i] == 0 && gy[i] < 0) {
				dirs[i] = 13;
			}
			else if (gx[i] > 0 && gy[i] < 0 && gx[i] < -gy[i]) {
				dirs[i] = 14;
			}
			else if (gx[i] > 0 && gy[i] < 0 && gx[i] == -gy[i]) {
				dirs[i] = 15;
			}
			else if (gx[i] > 0 && gy[i] < 0 && gx[i] > -gy[i]) {
				dirs[i] = 16;
			}
			else {
				COMPV_DEBUG_ERROR("Not expected, gx=%d, gy=%d", gx[i], gy[i]);
			}
		}
		gx += m_nImageStride;
		gy += m_nImageStride;
		dirs += m_nImageWidth;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// NonMaximaSuppression
COMPV_ERROR_CODE CompVEdgeDeteCanny::nms(CompVPtrArray(uint8_t)& edges)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	if (!m_pNms) {
		m_pNms = (uint8_t*)CompVMem::calloc(edges->rows() * edges->cols(), sizeof(uint8_t));
		COMPV_CHECK_EXP_RETURN(!m_pNms, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	uint8_t currDir;
	uint16_t currGrad;
	size_t idxStrideless, idxStridefull;
	
	// mark points to supp
	for (size_t row = 1; row < edges->rows() - 1; ++row) {
		for (size_t col = 1; col < edges->cols() - 1; ++col) {
			idxStrideless = (row * m_nImageWidth) + col;
			idxStridefull = (row * m_nImageStride) + col;
			currGrad = m_pG[idxStridefull];
			currDir = m_pDirs[idxStrideless];
#if 0
			if (m_pG[idxStridefull - 1] >= currGrad && m_pDirs[idxStrideless - 1] == currDir) { // left
				m_pNms[idxStrideless] = 1;
			}
			else if (m_pG[idxStridefull + 1] >= currGrad && m_pDirs[idxStrideless + 1] == currDir) { // right
				m_pNms[idxStrideless] = 1;
			}
			else if (m_pG[idxStridefull - m_nImageStride - 1] >= currGrad && m_pDirs[idxStrideless - m_nImageWidth - 1] == currDir) { // left-top
				m_pNms[idxStrideless] = 1;
			}
			else if (m_pG[idxStridefull - m_nImageStride] >= currGrad && m_pDirs[idxStrideless - m_nImageWidth] == currDir) { // top
				m_pNms[idxStrideless] = 1;
			}
			else if (m_pG[idxStridefull - m_nImageStride + 1] >= currGrad && m_pDirs[idxStrideless - m_nImageWidth + 1] == currDir) { // right-top
				m_pNms[idxStrideless] = 1;
			}
			else if (m_pG[idxStridefull + m_nImageStride - 1] >= currGrad && m_pDirs[idxStrideless + m_nImageWidth - 1] == currDir) { // left-bottom
				m_pNms[idxStrideless] = 1;
			}
			else if (m_pG[idxStridefull + m_nImageStride] >= currGrad && m_pDirs[idxStrideless + m_nImageWidth] == currDir) { // bottom
				m_pNms[idxStrideless] = 1;
			}
			else if (m_pG[idxStridefull + m_nImageStride + 1] >= currGrad && m_pDirs[idxStrideless + m_nImageWidth + 1] == currDir) { // right-bottom
				m_pNms[idxStrideless] = 1;
			}
#elif 1
			switch (currDir) {
			case 0:
				// no-direction
				break;
			case 1: case 9:
			case 2: case 10:
				// left, right
				if (m_pG[idxStridefull - 1] >= currGrad || m_pG[idxStridefull + 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 3: case 11:
			case 4: case 12:
				// top-right, bottom-left
				if (m_pG[idxStridefull - m_nImageStride + 1] >= currGrad || m_pG[idxStridefull + m_nImageStride - 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;

			case 5: case 13:
			case 6: case 14:
				// top, bottom
				if (m_pG[idxStridefull - m_nImageStride] >= currGrad || m_pG[idxStridefull + m_nImageStride] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;

			case 7: case 15:
			case 8: case 16:
				// top-left, bottom-right
				if (m_pG[idxStridefull - m_nImageStride - 1] >= currGrad || m_pG[idxStridefull + m_nImageStride + 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;

			default:
				COMPV_DEBUG_ERROR("Not expected. Dir=%u", currDir);
				break;
			}
#else
			switch (currDir) {
			case 0:
				// no-direction
				break;
			case 1: case 9:
				// left, right
				if (m_pG[idxStridefull - 1] >= currGrad || m_pG[idxStridefull + 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 2: case 10:
				// right, top-right, left, bottom-left
				if (m_pG[idxStridefull + 1] >= currGrad || m_pG[idxStridefull - m_nImageStride + 1] >= currGrad || m_pG[idxStridefull - 1] >= currGrad || m_pG[idxStridefull + m_nImageStride - 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 3: case 11:
				// top-right, bottom-left
				if (m_pG[idxStridefull - m_nImageStride + 1] >= currGrad || m_pG[idxStridefull + m_nImageStride - 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 4: case 12:
				// top-right, top, bottom, bottom-left
				if (m_pG[idxStridefull - m_nImageStride + 1] >= currGrad || m_pG[idxStridefull - m_nImageStride] >= currGrad || m_pG[idxStridefull + m_nImageStride] >= currGrad || m_pG[idxStridefull + m_nImageStride - 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 5: case 13:
				// top, bottom
				if (m_pG[idxStridefull - m_nImageStride] >= currGrad || m_pG[idxStridefull + m_nImageStride] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 6: case 14:
				// top, top-left, bottom, bottom-right
				if (m_pG[idxStridefull - m_nImageStride] >= currGrad || m_pG[idxStridefull - m_nImageStride - 1] >= currGrad || m_pG[idxStridefull + m_nImageStride] >= currGrad || m_pG[idxStridefull + m_nImageStride + 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 7: case 15:
				// top-left, bottom-right
				if (m_pG[idxStridefull - m_nImageStride - 1] >= currGrad || m_pG[idxStridefull + m_nImageStride + 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
			case 8: case 16:
				// top-left, left, right, bottom-right
				if (m_pG[idxStridefull - m_nImageStride - 1] >= currGrad || m_pG[idxStridefull - 1] >= currGrad || m_pG[idxStridefull + 1] >= currGrad || m_pG[idxStridefull + m_nImageStride + 1] >= currGrad) {
					m_pNms[idxStrideless] = 1;
				}
				break;
				
			default:
				COMPV_DEBUG_ERROR("Not expected. Dir=%u", currDir);
				break;
			}
#endif
		}
	}

	// supp marked points
	for (size_t row = 1; row < edges->rows() - 1; ++row) {
		for (size_t col = 1; col < edges->cols() - 1; ++col) {
			idxStrideless = (row * m_nImageWidth) + col;
			idxStridefull = (row * m_nImageStride) + col;
			if (m_pNms[idxStrideless] || m_pG[idxStridefull] < COMPV_FEATURE_DETE_CANNY_TMIN) {
				m_pNms[idxStrideless] = 0; // reset for the next call
				m_pG[idxStridefull] = 0;
			}
			else {
				COMPV_DEBUG_INFO_CODE_FOR_TESTING();
				//*const_cast<uint8_t*>(edges->ptr(row, col)) = 255;
			}
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::hysteresis(CompVPtrArray(uint8_t)& edges)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // check for 255 not correct as could be there

	size_t idxStridefull;

	COMPV_CHECK_CODE_RETURN(edges->zero_all()); // FIXME

#if 1
	for (size_t row = 1; row < edges->rows() - 1; ++row) {
		for (size_t col = 1; col < edges->cols() - 1; ++col) {
			idxStridefull = (row * m_nImageStride) + col;
			if (m_pG[idxStridefull] >= COMPV_FEATURE_DETE_CANNY_TMAX) { // strong edge
				connectEdge(edges, row, col);
			}
		}
	}
#else
	for (size_t row = 1; row < edges->rows() - 1; ++row) {
		for (size_t col = 1; col < edges->cols() - 1; ++col) {
			idxStridefull = (row * m_nImageStride) + col;
			if (m_pG[idxStridefull] > COMPV_FEATURE_DETE_CANNY_TMIN) { // strong edge
				*const_cast<uint8_t*>(edges->ptr(row, col)) = 255;
			}
		}
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::connectEdge(CompVPtrArray(uint8_t)& edges, size_t rowIdx, size_t colIdx)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	// at the border ?
	if (!rowIdx || !colIdx) {
		return COMPV_ERROR_CODE_S_OK;
	}

	// already strong edge ?
	if (*edges->ptr(rowIdx, colIdx) == 255) {
		return COMPV_ERROR_CODE_S_OK;
	}
	*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
	
	size_t idxStridefull;

	for (size_t row = rowIdx; row < edges->rows() - 1; ++row) {
		for (size_t col = colIdx; col < edges->cols() - 1; ++col) {
			if (row == rowIdx && col == colIdx) continue; // FIXME
			idxStridefull = (row * m_nImageStride) + col;
			if (m_pG[idxStridefull - 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row, col - 1);
			}
			else if (m_pG[idxStridefull + 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row, col + 1);
			}
			else if (m_pG[idxStridefull - m_nImageStride - 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left-top
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row - 1, col - 1);
			}
			else if (m_pG[idxStridefull - m_nImageStride] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // top
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row - 1, col);
			}
			else if (m_pG[idxStridefull - m_nImageStride + 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right-top
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row - 1, col + 1);
			}
			else if (m_pG[idxStridefull + m_nImageStride - 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left-bottom
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row + 1, col - 1);
			}
			else if (m_pG[idxStridefull + m_nImageStride] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // bottom
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row + 1, col);
			}
			else if (m_pG[idxStridefull + m_nImageStride + 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right-bottom
				//*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
				connectEdge(edges, row + 1, col + 1);
			}
			else {
				return COMPV_ERROR_CODE_S_OK;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::newObj(CompVPtr<CompVEdgeDete* >* dete)
{
	COMPV_CHECK_EXP_RETURN(!dete, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVEdgeDeteCanny* >dete_ = new CompVEdgeDeteCanny();
	COMPV_CHECK_EXP_RETURN(!dete_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVConvlt<float>::newObj(&dete_->m_gblur));
	if (CompVEngine::isMathFixedPoint()) {
		COMPV_CHECK_CODE_RETURN(CompVGaussKern<float>::buildKern1_fxp(&dete_->m_gblurKernFxp, COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIZE, COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIGMA));
	}
	if (!dete_->m_gblurKernFxp) {
		COMPV_CHECK_CODE_RETURN(CompVGaussKern<float>::buildKern1(&dete_->m_gblurKernFloat, COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIZE, COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIGMA));
	}

	*dete = *dete_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
