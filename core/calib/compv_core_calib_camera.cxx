/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/calib/compv_core_calib_camera.h"
#include "compv/base/math/compv_math_utils.h"

#define PATTERN_ROW_CORNERS_NUM			10 // Number of corners per row
#define PATTERN_COL_CORNERS_NUM			8  // Number of corners per column
#define PATTERN_CORNERS_NUM				(PATTERN_ROW_CORNERS_NUM * PATTERN_COL_CORNERS_NUM) // Total number of corners
#define PATTERN_GROUP_MAXLINES			5 // Maximum number of lines per group (errors)

#define HOUGH_RHO						(1.0f * 0.5f) // "rho-delta" (half-pixel)
#define HOUGH_THETA						(kfMathTrigPiOver180 * 0.5f) // "theta-delta" (half-radian)
#define HOUGH_THRESHOLD					150
#define HOUGH_THRESHOLD_FACT			0.0004828					
#define HOUGH_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGH_CLUSTER_MIN_SIZE			10
#define HOUGH_KERNEL_MIN_HEIGTH			0.002f // must be within [0, 1]

#define CANNY_LOW						2.11f
#define CANNY_HIGH						CANNY_LOW*2.f
#define CANNY_KERNEL_SIZE				3

COMPV_NAMESPACE_BEGIN()

CompVCalibCamera::CompVCalibCamera()
	: m_nPatternCornersNumRow(PATTERN_ROW_CORNERS_NUM)
	, m_nPatternCornersNumCol(PATTERN_COL_CORNERS_NUM)
{
	m_nPatternCornersTotal = m_nPatternCornersNumRow * m_nPatternCornersNumCol;
	m_nPatternLinesTotal = (m_nPatternCornersNumRow + m_nPatternCornersNumCol);
}

CompVCalibCamera::~CompVCalibCamera()
{

}

COMPV_ERROR_CODE CompVCalibCamera::process(const CompVMatPtr& image, CompVCalibCameraResult& result)
{
	COMPV_CHECK_EXP_RETURN(!image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Reset the previous result
	result.reset();

	// Canny edge detection
	COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(image, &result.edges));

	// Hough lines
	COMPV_CHECK_CODE_RETURN(m_ptrHough->setInt(COMPV_HOUGH_SET_INT_THRESHOLD, static_cast<int>(static_cast<double>(image->cols() * image->rows()) * HOUGH_THRESHOLD_FACT) + 1));
	COMPV_CHECK_CODE_RETURN(m_ptrHough->process(result.edges, result.hough_lines));
	if (result.hough_lines.size() < m_nPatternLinesTotal) {
		result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		return COMPV_ERROR_CODE_S_OK;
	}
#if 0
	CompVHoughLineVector lines;
	static const float k10DegreeInRad = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(10.0);
	bool alreadyAdded;

	for (CompVHoughLineVector::iterator i = result.hough_lines.begin(); i < result.hough_lines.end(); ++i) {
		alreadyAdded = false;
		for (CompVHoughLineVector::iterator j = lines.begin(); j < lines.end(); ++j) {
			if (std::abs(j->theta - i->theta) < k10DegreeInRad) {
				alreadyAdded = true;
				break;
			}
		}
		if (!alreadyAdded) {
			lines.push_back(*i);
		}
	}

	result.hough_lines = lines;
#endif

	//result.hough_lines.resize(1);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibCamera::newObj(CompVCalibCameraPtrPtr calib)
{
	COMPV_CHECK_EXP_RETURN(!calib, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVCalibCameraPtr calib_ = new CompVCalibCamera();
	COMPV_CHECK_EXP_RETURN(!calib_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	/* Hough transform */
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&calib_->m_ptrHough, COMPV_HOUGHKHT_ID, HOUGH_RHO, HOUGH_THETA, HOUGH_THRESHOLD));
	COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setInt(COMPV_HOUGH_SET_INT_MAXLINES, static_cast<int>(calib_->m_nPatternLinesTotal * PATTERN_GROUP_MAXLINES)));
	COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION, HOUGH_CLUSTER_MIN_DEVIATION));
	COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setInt(COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE, HOUGH_CLUSTER_MIN_SIZE));
	COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH, HOUGH_KERNEL_MIN_HEIGTH));
	
	/* Canny edge detector */
	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&calib_->m_ptrCanny, COMPV_CANNY_ID, CANNY_LOW, CANNY_HIGH, CANNY_KERNEL_SIZE));

	*calib = *calib_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
