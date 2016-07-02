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

#define COMPV_FEATURE_DETE_CANNY_TMIN 90
#define COMPV_FEATURE_DETE_CANNY_TMAX 170

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

	// Compute gradiant using L1 distance
	// FIXME: gmax not needed
	uint16_t gmax = 1;
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradientL1<int16_t, uint16_t>(m_pGx, m_pGy, m_pG, gmax, m_nImageWidth, m_nImageHeight, m_nImageStride)));

#if 0
	// scale (normalization)
	float scale = 255.f / float(gmax);
	uint8_t* edgesPtr = const_cast<uint8_t*>(edges->ptr());
	COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip<uint16_t, float, uint8_t>(m_pG, scale, edgesPtr, 0, 255, m_nImageWidth, m_nImageHeight, m_nImageStride)));
#endif

#if 1
	// Directions
	COMPV_CHECK_CODE_RETURN(direction());

	// NMS
	COMPV_CHECK_CODE_RETURN(nms(edges));

	// Hysteresis
	COMPV_CHECK_CODE_RETURN(hysteresis(edges));
#endif

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

#if 0
			if (gx[i] == gy[i] && gx[i] == 0) {
				dirs[i] = 0;
			}
			else if (gx[i] > 0 && gy[i] == 0) {
				dirs[i] = 1;
			}
			else if (gx[i] > 0 && gy[i] > 0 && gx[i] == gy[i]) {
				dirs[i] = 3;
			}
			else if (gx[i] == 0 && gy[i] > 0) {
				dirs[i] = 5;
			}
			else if (gx[i] < 0 && gy[i] > 0 && -gx[i] == gy[i]) {
				dirs[i] = 7;
			}
			else if (gx[i] < 0 && gy[i] == 0) {
				dirs[i] = 9;
			}
			else if (gx[i] < 0 && gy[i] < 0 && gx[i] == gy[i]) {
				dirs[i] = 11;
			}
			else if (gx[i] == 0 && gy[i] < 0) {
				dirs[i] = 13;
			}
			else if (gx[i] > 0 && gy[i] < 0 && gx[i] == -gy[i]) {
				dirs[i] = 15;
			}
			else {
				float angle = atan2f(float(gy[i]), float(gx[i]));
				angle = fmodf(angle, 360.f);
				angle = COMPV_MATH_RADIAN_TO_DEGREE_FLOAT(angle);
				if (angle < 0)
					angle += 360.0;
				if (angle <= 45) {
					// dirs[i] = 2;
					dirs[i] = angle <= (45 - 22.5) ? 1 : 3;
				}
				else if (angle <= 90) {
					//dirs[i] = 4;
					dirs[i] = angle <= (90 - 22.5) ? 3 : 5;
				}
				else if (angle <= 135) {
					//dirs[i] = 6;
					dirs[i] = angle <= (135 - 22.5) ? 5 : 7;
				}
				else if (angle <= 180) {
					//dirs[i] = 8;
					dirs[i] = angle <= (180 - 22.5) ? 7 : 9;
				}
				else if (angle <= 225) {
					//dirs[i] = 10;
					dirs[i] = angle <= (225 - 22.5) ? 9 : 11;
				}
				else if (angle <= 270) {
					//dirs[i] = 12;
					dirs[i] = angle <= (270 - 22.5) ? 11 : 13;
				}
				else if (angle <= 315) {
					//dirs[i] = 14;
					dirs[i] = angle <= (315 - 22.5) ? 13 : 15;
				}
				else if (angle <= 360) {
					//dirs[i] = 16;
					dirs[i] = angle <= (360 - 22.5) ? 15 : 1;
				}
				else {
					COMPV_DEBUG_ERROR("Not expected, gx=%d, gy=%d", gx[i], gy[i]);
				}
			}
#else
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
			else if (gx[i] > 0 && gy[i] > 0 && gx[i] < gy[i]) {
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
#endif
		}

		gx += m_nImageStride;
		gy += m_nImageStride;
		dirs += m_nImageWidth;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// FIXME(dmi)
static double angle(int16_t gx, int16_t gy)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	if (gy == 0) {
		if (gx >= 0) {
			return 0.;
		}
		else {
			return COMPV_MATH_PI;
		}
	}

	float angle = atan2f(float(gy), float(gx));
	angle = fmodf(angle, kfMathTrigPiTimes2);
	if (angle < 0)
		angle += kfMathTrigPiTimes2;
	return angle;
}

// FIXME(dmi)
static void interp(int16_t gx, int16_t gy, int16_t gx0, int16_t gy0, int16_t gx1, int16_t gy1, uint16_t &g)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
#if 0
	uint16_t diffx0 = abs(abs(gx0) - abs(gx)) + 1;
	uint16_t diffx1 = abs(abs(gx1) - abs(gx)) + 1;
	uint16_t diffx = diffx0 + diffx1;
	float scalex0 = 1.f - (diffx0 / (float)diffx);
	float scalex1 = 1.f - scalex0;
	
	uint16_t diffy0 = abs(abs(gy0) - abs(gy)) + 1;
	uint16_t diffy1 = abs(abs(gy1) - abs(gy)) + 1;
	uint16_t diffy = diffy0 + diffy1;
	float scaley0 = 1.f - (diffy0 / (float)diffy);
	float scaley1 = 1.f - scaley0;
	g = (uint16_t)round((scalex0 * abs(gx0))
		+ (scalex1 * abs(gx1))
		+ (scaley0 * abs(gy0))
		+ (scaley1 * abs(gy1)));
#elif 1
	uint16_t diffx0 = ((gx0 - gx) * (gx0 - gx)) + 1;
	uint16_t diffx1 = ((gx1 - gx) * (gx1 - gx)) + 1;
	uint16_t diffx = diffx0 + diffx1;
	float scalex0 = 1.f - (diffx0 / (float)diffx);
	float scalex1 = 1.f - scalex0;

	uint16_t diffy0 = ((gy0 - gy) * (gy0 - gy)) + 1;
	uint16_t diffy1 = ((gy1 - gy) * (gy1 - gy)) + 1;
	uint16_t diffy = diffy0 + diffy1;
	float scaley0 = 1.f - (diffy0 / (float)diffy);
	float scaley1 = 1.f - scaley0;
	g = (uint16_t)round((scalex0 * abs(gx0))
		+ (scalex1 * abs(gx1))
		+ (scaley0 * abs(gy0))
		+ (scaley1 * abs(gy1)));
#else
	int diffx0 = ((gx0 - gx) * (gx0 - gx));
	int diffy0 = ((gy0 - gy) * (gy0 - gy));
	int diffx1 = ((gx1 - gx) * (gx1 - gx));
	int diffy1 = ((gy1 - gy) * (gy1 - gy));
	float diff0 = sqrtf(float(diffx0) + diffy0) + 1.f;
	float diff1 = sqrtf(float(diffx1) + diffy1) + 1.f;
	float diff = (diff0 + diff1);
	float scalex0 = 1.f - (diff0 / diff);
	float scalex1 = 1.f - scalex0;
	float gf = ((abs(gx0) + abs(gy0)) * scalex0) + ((abs(gx1) + abs(gy1)) * scalex1); // g0*s0 + g1*s1
	g = COMPV_MATH_ROUNDFU_2_INT(gf, uint16_t);
#endif
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
			// FIXME: interpolation
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
				//if ((m_pG[idxStridefull + 1] + m_pG[idxStridefull - m_nImageStride + 1])>>1 >= currGrad || (m_pG[idxStridefull - 1] + m_pG[idxStridefull + m_nImageStride - 1])>>1 >= currGrad) {
				//	m_pNms[idxStrideless] = 1;
				//}
				grad0 = (uint16_t)(m_pG[idxStridefull + 1] * 0.7f + m_pG[idxStridefull - m_nImageStride + 1] * 0.3f);
				grad1 = (uint16_t)(m_pG[idxStridefull - 1] * 0.7f + m_pG[idxStridefull + m_nImageStride - 1] * 0.3f);
				if (grad0 >= currGrad || grad1 >= currGrad) {
					//m_pNms[idxStrideless] = 1;
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
				//if ((m_pG[idxStridefull - m_nImageStride + 1] + m_pG[idxStridefull - m_nImageStride])>>1 >= currGrad || (m_pG[idxStridefull + m_nImageStride] + m_pG[idxStridefull + m_nImageStride - 1])>>1 >= currGrad) {
				//m_pNms[idxStrideless] = 1;
				//}
				grad0 = (uint16_t)(m_pG[idxStridefull - m_nImageStride] * 0.5f + m_pG[idxStridefull - m_nImageStride + 1] * 0.5f);
				grad1 = (uint16_t)(m_pG[idxStridefull + m_nImageStride] * 0.5f + m_pG[idxStridefull + m_nImageStride - 1] * 0.5f);
				if (grad0 >= currGrad || grad1 >= currGrad) {
					//m_pNms[idxStrideless] = 1;
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
				if ((m_pG[idxStridefull - m_nImageStride] + m_pG[idxStridefull - m_nImageStride - 1])>>1 >= currGrad || (m_pG[idxStridefull + m_nImageStride] + m_pG[idxStridefull + m_nImageStride + 1])>>1 >= currGrad) {
					//m_pNms[idxStrideless] = 1;
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
				if ((m_pG[idxStridefull - m_nImageStride - 1] + m_pG[idxStridefull - 1])>>1 >= currGrad || (m_pG[idxStridefull + 1] + m_pG[idxStridefull + m_nImageStride + 1])>>1 >= currGrad) {
					//m_pNms[idxStrideless] = 1;
				}
				break;

			default:
				COMPV_DEBUG_ERROR("Not expected. Dir=%u", currDir);
				break;
			}
#elif 1
			switch (currDir) {
			case 8: case 16:
			default:
				uint16_t g0 = 0, g1 = 0;
				int x0, y0, x1, y1;
				double angle_ = angle(m_pGx[idxStridefull], m_pGy[idxStridefull]);
				angle_ = COMPV_MATH_RADIAN_TO_DEGREE(angle_);
				if (angle_ <= 0.5) {
					// no angle
					x0 = 1, y0 = 0;
				}
				else if (angle_ >= 45. - 22.5 && angle_ <= 45. + 22.5) {
					x0 = 1, y0 = 1;
				}
				else if (angle_ >= 90. - 22.5 && angle_ <= 90. + 22.5) {
					x0 = 0, y0 = 1;
				}
				else if (angle_ >= 135. - 22.5 && angle_ <= 135. + 22.5) {
					x0 = -1, y0 = 1;
				}
				else if (angle_ >= 180. - 22.5 && angle_ <= 180. + 22.5) {
					x0 = -1, y0 = 0;
				}
				else if (angle_ >= 225. - 22.5 && angle_ <= 225. + 22.5) {
					x0 = -1, y0 = -1;
				}
				else if (angle_ >= 270. - 22.5 && angle_ <= 270. + 22.5) {
					x0 = 0, y0 = -1;
				}
				else if (angle_ >= 315. - 22.5 && angle_ <= 315. + 22.5) {
					x0 = 1, y0 = -1;
				}
				else if (angle_ >= 360. - 22.5 || angle_ <= 22.5) { // else
					x0 = 1, y0 = 0;
				}
				else {
					COMPV_DEBUG_ERROR("Not expected");
				}
				x1 = -x0;
				y1 = -y0;

				x0 += (int)col;
				x1 += (int)col;
				y0 += (int)row;
				y1 += (int)row;

				x0 = COMPV_MATH_CLIP3(0, (int)m_nImageWidth - 1, x0);
				x1 = COMPV_MATH_CLIP3(0, (int)m_nImageWidth - 1, x1);
				y0 = COMPV_MATH_CLIP3(0, (int)m_nImageHeight - 1, y0);
				y1 = COMPV_MATH_CLIP3(0, (int)m_nImageHeight - 1, y1);
				
				if (x0 >= 0. && x0 < m_nImageWidth && y0 >= 0. && y0 < m_nImageHeight) {
					g0 = m_pG[(x0 + (y0 * m_nImageStride))];
				}
				if (x1 >= 0. && x1 < m_nImageWidth && y1 >= 0. && y1 < m_nImageHeight) {
					g1 = m_pG[(x1 + (y1 * m_nImageStride))];
				}
				if (g0 > currGrad || g1 > currGrad) {
					m_pNms[idxStrideless] = 1;
					//m_pG[idxStridefull] = 0;
				}
				break;
			}
			
			
#elif 0
			double angle_ = angle(m_pGx[idxStridefull], m_pGy[idxStridefull]);
			static const int x = 1, y = 0;
			double x0 = round(x * ::cos(angle_) - y * ::sin(angle_));
			double y0 = round(x * ::sin(angle_) + y * ::cos(angle_));
			angle_ += COMPV_MATH_PI;
			double x1 = round(x * ::cos(angle_) - y * ::sin(angle_));
			double y1 = round(x * ::sin(angle_) + y * ::cos(angle_));
			uint16_t g0 = 0, g1 = 0;
			x0 += (double)col;
			x1 += (double)col;
			y0 += (double)row;
			y1 += (double)row;

			/*if (x0 >= 0. && x0 < m_nImageWidth && y0 >= 0. && y0 < m_nImageHeight)*/ {
				g0 = m_pG[(int)round(x0 + (y0 * m_nImageStride))];
			}
			/*if (x1 >= 0. && x1 < m_nImageWidth && y1 >= 0. && y1 < m_nImageHeight)*/ {
				g1 = m_pG[(int)round(x1 + (y1 * m_nImageStride))];
			}
			if (g0 >= currGrad || g1 >= currGrad) {
				m_pG[idxStridefull] = 0;
			}
#elif 1
			switch (currDir) {
			case 0:
				// no-direction
				//m_pG[idxStridefull] = 0; // FIXME(dmi): correct?
				break;
			case 1: case 9:
				// left, right
				if (m_pG[idxStridefull - 1] >= currGrad || m_pG[idxStridefull + 1] >= currGrad) {
					m_pG[idxStridefull] = 0;
				}
				break;
			case 2: case 10:
				// right, top-right, left, bottom-left
				if (true) {
					uint16_t g0, g1;
					int16_t gx = m_pGx[idxStridefull];
					int16_t gy = m_pGy[idxStridefull];
					interp(gx, gy, m_pGx[idxStridefull + 1], m_pGy[idxStridefull + 1], m_pGx[idxStridefull - m_nImageStride + 1], m_pGy[idxStridefull - m_nImageStride + 1], g0);
					interp(gx, gy, m_pGx[idxStridefull - 1], m_pGy[idxStridefull - 1], m_pGx[idxStridefull + m_nImageStride - 1], m_pGy[idxStridefull + m_nImageStride - 1], g1);
					if (g0 >= currGrad || g1 >= currGrad) {
						//m_pG[idxStridefull] = 0;
					}
				}
				break;
			case 3: case 11:
				// top-right, bottom-left
				if (m_pG[idxStridefull - m_nImageStride + 1] >= currGrad || m_pG[idxStridefull + m_nImageStride - 1] >= currGrad) {
					m_pG[idxStridefull] = 0;
				}
				break;
			case 4: case 12:
				// top-righ, top, bottom, bottom-left
				if (true) {
					uint16_t g0, g1;
					int16_t gx = m_pGx[idxStridefull];
					int16_t gy = m_pGy[idxStridefull];
					interp(gx, gy, m_pGx[idxStridefull - m_nImageStride + 1], m_pGy[idxStridefull - m_nImageStride + 1], m_pGx[idxStridefull - m_nImageStride], m_pGy[idxStridefull - m_nImageStride], g0);
					interp(gx, gy, m_pGx[idxStridefull + m_nImageStride], m_pGy[idxStridefull + m_nImageStride], m_pGx[idxStridefull + m_nImageStride - 1], m_pGy[idxStridefull + m_nImageStride - 1], g1);
					if (g0 >= currGrad || g1 >= currGrad) {
						//m_pG[idxStridefull] = 0;
					}
				}
				break;
			case 5: case 13:
				// top, bottom
				if (m_pG[idxStridefull - m_nImageStride] >= currGrad || m_pG[idxStridefull + m_nImageStride] >= currGrad) {
					m_pG[idxStridefull] = 0;
				}
				break;
			case 6: case 14:
				// top, top-left, bottom, bottom-right
				if (true) {
					uint16_t g0, g1;
					int16_t gx = m_pGx[idxStridefull];
					int16_t gy = m_pGy[idxStridefull];
					interp(gx, gy, m_pGx[idxStridefull - m_nImageStride], m_pGy[idxStridefull - m_nImageStride], m_pGx[idxStridefull - m_nImageStride - 1], m_pGy[idxStridefull - m_nImageStride - 1], g0);
					interp(gx, gy, m_pGx[idxStridefull + m_nImageStride], m_pGy[idxStridefull + m_nImageStride], m_pGx[idxStridefull + m_nImageStride + 1], m_pGy[idxStridefull + m_nImageStride + 1], g1);
					if (g0 >= currGrad || g1 >= currGrad) {
						//m_pG[idxStridefull] = 0;
					}
				}
				break;
			case 7: case 15:
				// top-left, bottom-right
				if (m_pG[idxStridefull - m_nImageStride - 1] >= currGrad || m_pG[idxStridefull + m_nImageStride + 1] >= currGrad) {
					m_pG[idxStridefull] = 0;
				}
				break;
			case 8: case 16:
				// top-left, left, right, bottom-right
				if (true) {
					/*uint16_t g0, g1;
					int16_t gx = m_pGx[idxStridefull];
					int16_t gy = m_pGy[idxStridefull];
					interp(gx, gy, m_pGx[idxStridefull - m_nImageStride - 1], m_pGy[idxStridefull - m_nImageStride - 1], m_pGx[idxStridefull - 1], m_pGy[idxStridefull - 1], g0);
					interp(gx, gy, m_pGx[idxStridefull + 1], m_pGy[idxStridefull + 1], m_pGx[idxStridefull + m_nImageStride + 1], m_pGy[idxStridefull + m_nImageStride + 1], g1);
					if (g0 >= currGrad || g1 >= currGrad) {
						m_pG[idxStridefull] = 0;
					}*/
				}
				break;

			default:
				// FIXME(dmi):
				COMPV_DEBUG_INFO_CODE_NOT_TESTED();
				//COMPV_DEBUG_ERROR("Not expected. Dir=%u", currDir);
				break;
			}
#elif 0
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
#if 1
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
#endif

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVEdgeDeteCanny::hysteresis(CompVPtrArray(uint8_t)& edges)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // check for 255 not correct as could be there

	size_t idxStridefull, numEdges = 0;

	COMPV_CHECK_CODE_RETURN(edges->zero_all()); // FIXME

#if 1
	// FIXME(dmi): is count == 1 -> supp
	for (size_t row = 1; row < edges->rows() - 1; ++row) {
		for (size_t col = 1; col < edges->cols() - 1; ++col) {
			idxStridefull = (row * m_nImageStride) + col;
			if (m_pG[idxStridefull] >= COMPV_FEATURE_DETE_CANNY_TMAX) { // strong edge
				numEdges = 0;
				connectEdge(edges, row, col, numEdges);
				if (numEdges < 2) { // FIXME: find better
					*const_cast<uint8_t*>(edges->ptr(row, col)) = 0;
				}
			}
		}
	}
#else
	for (size_t row = 1; row < edges->rows() - 1; ++row) {
		for (size_t col = 1; col < edges->cols() - 1; ++col) {
			idxStridefull = (row * m_nImageStride) + col;
			if (m_pG[idxStridefull] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // weak edge
				*const_cast<uint8_t*>(edges->ptr(row, col)) = 255;
			}
		}
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVEdgeDeteCanny::connectEdge(CompVPtrArray(uint8_t)& edges, size_t rowIdx, size_t colIdx, size_t& numEdges)
{
	// Private function -> do not check input parameters
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	// at the border ?
	if (rowIdx == 0 || colIdx == 0 || rowIdx >= m_nImageHeight - 1 || colIdx >= m_nImageWidth - 1) {
		return false;
	}

	// already strong edge ?
	if (*edges->ptr(rowIdx, colIdx) == 255) {
		numEdges = SIZE_MAX;
		return true;
	}
	
	*const_cast<uint8_t*>(edges->ptr(rowIdx, colIdx)) = 255;
	if (m_pG[(rowIdx * m_nImageStride) + colIdx] >= COMPV_FEATURE_DETE_CANNY_TMAX) {
		++numEdges;
	}

	size_t idxStridefull;

	// FIXME: use grid

	/*for (size_t row = rowIdx; row < edges->rows() - 1; ++row)*/ {
		/*for (size_t col = colIdx; col < edges->cols() - 1; ++col)*/ {
			// if (row == rowIdx && col == colIdx) continue; // FIXME
			idxStridefull = (rowIdx * m_nImageStride) + colIdx;
			if (m_pG[idxStridefull - 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left
				connectEdge(edges, rowIdx, colIdx - 1, numEdges);
			}
			if (m_pG[idxStridefull + 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right
				connectEdge(edges, rowIdx, colIdx + 1, numEdges);
			}
			if (m_pG[idxStridefull - m_nImageStride - 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left-top
				connectEdge(edges, rowIdx - 1, colIdx - 1, numEdges);
			}
			if (m_pG[idxStridefull - m_nImageStride] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // top
				connectEdge(edges, rowIdx - 1, colIdx, numEdges);
			}
			if (m_pG[idxStridefull - m_nImageStride + 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right-top
				connectEdge(edges, rowIdx - 1, colIdx + 1, numEdges);
			}
			if (m_pG[idxStridefull + m_nImageStride - 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // left-bottom
				connectEdge(edges, rowIdx + 1, colIdx - 1, numEdges);
			}
			if (m_pG[idxStridefull + m_nImageStride] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // bottom
				connectEdge(edges, rowIdx + 1, colIdx, numEdges);
			}
			if (m_pG[idxStridefull + m_nImageStride + 1] >= COMPV_FEATURE_DETE_CANNY_TMIN) { // right-bottom
				connectEdge(edges, rowIdx + 1, colIdx + 1, numEdges);
			}
		}
	}
	return false;
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
