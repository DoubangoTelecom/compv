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
static const float COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIGMA = 1.4f; // FIXME: 1.4

#define COMPV_FEATURE_DETE_CANNY_TMIN	90
#define COMPV_FEATURE_DETE_CANNY_TMAX	170

static const float kTangentPiOver8 = 0.414213568f; // tan(22.5)
static const float kTangentPiTimes3Over8 = 2.41421366f; // tan(67.5)
static const float kTangentDiv = (kTangentPiTimes3Over8 / kTangentPiOver8); // tan(67.5) / tan(22.5)
static const float kTangentPiOver8M = -kTangentPiOver8;
static const float kTangentPiTimes3Over8M = -kTangentPiTimes3Over8;

CompVEdgeDeteCanny::CompVEdgeDeteCanny()
	: CompVEdgeDete(COMPV_CANNY_ID)
	, m_nImageWidth(0)
	, m_nImageHeight(0)
	, m_nImageStride(0)
	, m_pGx(NULL)
	, m_pGy(NULL)
	, m_pG(NULL)
	, m_pNms(NULL)
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
	if (!m_pGx || !m_pGy || !m_pG || !m_pNms || image->getWidth() != m_nImageWidth || image->getHeight() != m_nImageHeight || image->getStride() != m_nImageStride) {
		CompVMem::free((void**)&m_pGx);
		CompVMem::free((void**)&m_pGy);
		CompVMem::free((void**)&m_pG);
		CompVMem::free((void**)&m_pNms);
		m_nImageWidth = static_cast<size_t>(image->getWidth());
		m_nImageHeight = static_cast<size_t>(image->getHeight());
		m_nImageStride = static_cast<size_t>(image->getStride());
	}

	// Create edges buffer
	// edges must have same stride than m_pG (required by scaleAndClip) and image (required by gaussian blur)
	COMPV_CHECK_CODE_RETURN(CompVArray<uint8_t>::newObj(&edges, m_nImageHeight, m_nImageWidth, COMPV_SIMD_ALIGNV_DEFAULT, m_nImageStride));

	// FIXME(dmi): combine gaussianblur with sobel/scharr to one op
	// Gaussian Blurr
#if 0
	if (m_gblurKernFxp) {
		// Fixed-point
		COMPV_CHECK_CODE_RETURN(m_gblur->convlt1_fxp((uint8_t*)image->getDataPtr(), (int)m_nImageWidth, (int)m_nImageStride, (int)m_nImageHeight, m_gblurKernFxp->ptr(), m_gblurKernFxp->ptr(), COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIZE, (uint8_t*)edges->ptr()));
	}
	else {
		// Floating-point
		COMPV_CHECK_CODE_RETURN(m_gblur->convlt1((uint8_t*)image->getDataPtr(), (int)m_nImageWidth, (int)m_nImageStride, (int)m_nImageHeight, m_gblurKernFloat->ptr(), m_gblurKernFloat->ptr(), COMPV_FEATURE_DETE_CANNY_GAUSS_KERN_SIZE, (uint8_t*)edges->ptr()));
	}
#endif

	// FIXME(dmi): use Scharr instead of sobel
	// FIXME(dmi): restore gaussian blur
	// Convolution
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)image->getDataPtr(), m_nImageWidth, m_nImageStride, m_nImageHeight, &CompVSobelGx_vt[0], &CompVSobelGx_hz[0], 3, m_pGx)));
	COMPV_CHECK_CODE_RETURN((CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)image->getDataPtr(), m_nImageWidth, m_nImageStride, m_nImageHeight, &CompVSobelGx_hz[0], &CompVSobelGx_vt[0], 3, m_pGy)));

	// Compute gradiant using L1 distance.
	//!\\ Important: NMS function requires distance metric to be L1 instead of L2 (see comment in nms())
	// FIXME'dmi): gmax not needed
	uint16_t gmax = 1;
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, gmax, m_nImageWidth, m_nImageHeight, m_nImageStride)));

#if 0
	// scale (normalization)
	float scale = 255.f / float(gmax);
	uint8_t* edgesPtr = const_cast<uint8_t*>(edges->ptr());
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip<uint16_t, float, uint8_t>(m_pG, scale, edgesPtr, 0, 255, m_nImageWidth, m_nImageHeight, m_nImageStride)));
#endif

	// NMS
	COMPV_CHECK_CODE_RETURN(nms(edges));
	// Hysteresis
	hysteresis(edges);

	return COMPV_ERROR_CODE_S_OK;
}

// NonMaximaSuppression
COMPV_ERROR_CODE CompVEdgeDeteCanny::nms(CompVPtrArray(uint8_t)& edges)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // MT
	if (!m_pNms) {
		m_pNms = (uint8_t*)CompVMem::malloc(edges->rows() * edges->cols() * sizeof(uint8_t));
		COMPV_CHECK_EXP_RETURN(!m_pNms, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	size_t row, col;
	const size_t maxRows = edges->rows() - 1, maxCols = edges->cols() - 1;
	const int16_t *gx = m_pGx + m_nImageStride, *gy = m_pGy + m_nImageStride;
	uint16_t *g = m_pG + m_nImageStride;
	const uint16_t *gcol;
	uint16_t absgy;
	uint8_t *nms = m_pNms + m_nImageWidth;
	uint8_t *e = const_cast<uint8_t*>(edges->ptr(1));
	int c;
	const int stride = (int)m_nImageStride;
	float absGxTimesPiOver8;

	// non-maximasupp is multi-threaded and we will use this property to zero the edge buffer with low cost (compared to edges->zeroall() which is not MT)
	// TODO(dmi): perform next op only if startIndex == 0
	CompVMem::zero(e - m_nImageStride, m_nImageWidth); // zero first line
	CompVMem::zero(e + ((maxRows - 1)*m_nImageStride), m_nImageWidth); // zero last line

	// mark points to supp
	for (row = 1; row < maxRows; ++row) {
		CompVMem::zero(e, m_nImageWidth);
		for (col = 1, gcol = (g + 1); col < maxCols; ++col, ++gcol) {
			// The angle is guessed using the first quadrant ([0-45] degree) only then completed.
			// We want "arctan(gy/gx)" to be within [0, 22.5], [22.5, 67.5], ]67.5, inf[, with 67.5 = (45. + 22.5)
			//!\\ All NMS values must be set to reset all values (no guarantee it contains zeros).
			
#if 1 // Slower
			if (!gy[col]) {
				// angle = 0° or 180°
				nms[col] = (gcol[1] > *gcol || gcol[-1] > *gcol);
			}
			else if (!gx[col]) {
				// angle = 90° or 270°
				nms[col] = (gcol[m_nImageStride] > *gcol || *(gcol-m_nImageStride) > *gcol);
			}
			else
#endif
			{
				// "theta = arctan(gy/gx)" -> "tan(theta) = gy/gx" -> compare "gy/gx" with "tan(quadrants)"
				// Instead of comparing "abs(gy/gx)" with "tanTheta" we will compare "abs(gy)" with "tanTheta*abs(gx)" to avoid division.
				// G = "abs(GX) + abs(GY)" -> "abs(GY) = G - abs(GX)"

				if (gx[col] < 0) {
					absgy = *gcol + gx[col];
					absGxTimesPiOver8 = (kTangentPiOver8M * gx[col]);
					c = gy[col] < 0 ? (-1 - stride) : (-1 + stride);
				}
				else {
					absgy = *gcol - gx[col];
					absGxTimesPiOver8 = (kTangentPiOver8 * gx[col]);
					c = gy[col] < 0 ? (1 - stride) : (1 + stride);
				}
				//nms[col] = (absgy < absGxTimesPiOver8)
				//	? (gcol[1] > *gcol || gcol[-1] > *gcol)
				//	: ((absgy < (absGxTimesPiOver8 * kTangentDiv)) ? (gcol[c] > *gcol || gcol[-c] > *gcol) : (gcol[m_nImageStride] > *gcol || *(gcol - m_nImageStride) > *gcol));
				if (absgy < absGxTimesPiOver8) { // angle = 0° or 180°
					nms[col] = (gcol[1] > *gcol || gcol[-1] > *gcol);
				}
				else if (absgy < (absGxTimesPiOver8 * kTangentDiv)) { // angle = 45° or 225°
					nms[col] = (gcol[c] > *gcol || gcol[-c] > *gcol);
				}
				else { // angle = 90° or 270°
					nms[col] = (gcol[m_nImageStride] > *gcol || *(gcol-m_nImageStride) > *gcol);
				}
			}
		}
		gx += m_nImageStride;
		gy += m_nImageStride;
		g += m_nImageStride;
		e += m_nImageStride;
		nms += m_nImageWidth;
	}

	// supp marked points
	nms = m_pNms + m_nImageWidth;
	g = m_pG + m_nImageStride;
	for (row = 1; row < maxRows; ++row) {
		for (col = 1; col < maxCols; ++col) {
			if (nms[col]) {
				g[col] = 0;
			}
		}
		nms += m_nImageWidth;
		g += m_nImageStride;
	}

	return COMPV_ERROR_CODE_S_OK;
}

void CompVEdgeDeteCanny::hysteresis(CompVPtrArray(uint8_t)& edges)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();  // MT

	size_t row, col;
	const size_t maxRows = edges->rows() - 1, maxCols = edges->cols() - 1;
	const uint16_t *g = m_pG + m_nImageStride;
	for (row = 1; row < maxRows; ++row) {
		for (col = 1; col < maxCols; ++col) {
			if (g[col] >= COMPV_FEATURE_DETE_CANNY_TMAX) { // strong edge
				connectEdge(edges, row, col);
			}
		}
		g += m_nImageStride;
	}
}

void CompVEdgeDeteCanny::connectEdge(CompVPtrArray(uint8_t)& edges, size_t rowIdx, size_t colIdx)
{
	// Private function -> do not check input parameters
	// at the border ?
	if (!rowIdx || !colIdx || rowIdx > m_nImageHeight - 2 || colIdx > m_nImageWidth - 2) {
		return;
	}
	uint8_t* pixel = const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx));
	// already strong edge ?
	if (*pixel == 255) {
		return;
	}
	*pixel = 255; // set as strong edge
	const uint16_t* g = &m_pG[(rowIdx * m_nImageStride) + colIdx];
	if (*(g-1) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left
		connectEdge(edges, rowIdx, colIdx - 1);
	}
	if (*(g+1) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right
		connectEdge(edges, rowIdx, colIdx + 1);
	}
	if (*(g-m_nImageStride-1) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left-top
		connectEdge(edges, rowIdx - 1, colIdx - 1);
	}
	if (*(g-m_nImageStride) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // top
		connectEdge(edges, rowIdx - 1, colIdx);
	}
	if (*(g-m_nImageStride+1) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right-top
		connectEdge(edges, rowIdx - 1, colIdx + 1);
	}
	if (*(g+ m_nImageStride-1) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left-bottom
		connectEdge(edges, rowIdx + 1, colIdx - 1);
	}
	if (*(g+m_nImageStride) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // bottom
		connectEdge(edges, rowIdx + 1, colIdx);
	}
	if (*(g+m_nImageStride+1) >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right-bottom
		connectEdge(edges, rowIdx + 1, colIdx + 1);
	}
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
