/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CALIB_CAMERA_H_)
#define _COMPV_CORE_CALIB_CAMERA_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/calib/compv_core_calib_common.h"
#include "compv/core/calib/compv_core_calib_homography.h"
#include "compv/base/compv_features.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CalibCamera)

class COMPV_CORE_API CompVCalibCamera : public CompVObj
{
protected:
	CompVCalibCamera(size_t nPatternRowsCount, size_t nPatternColsCount);

public:
	virtual ~CompVCalibCamera();
	COMPV_OBJECT_GET_ID(CompVCalibCamera);

	COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVCalibContex& result);
	COMPV_ERROR_CODE calibrate(CompVCalibContex& context);

	COMPV_INLINE CompVEdgeDetePtr edgeDetector() { return m_ptrCanny; }
	COMPV_INLINE CompVHoughPtr houghTransform() { return m_ptrHough; }
	
	static COMPV_ERROR_CODE newObj(CompVCalibCameraPtrPtr calib, size_t nPatternRowsCount = COMPV_CALIB_PATTERN_ROWS_COUNT, size_t nPatternColsCount = COMPV_CALIB_PATTERN_COLS_COUNT);

private:
	COMPV_ERROR_CODE subdivision(const size_t image_width, const size_t image_height, const CompVCabLines& lines, CompVCabLines& lines_hz, CompVCabLines& lines_vt);
	COMPV_ERROR_CODE grouping(const size_t image_width, const size_t image_height, const CompVCabLines& lines_parallel, const bool vt, const size_t max_strength, const size_t max_lines, CompVCabLineFloat32Vector& lines_parallel_grouped);
	COMPV_ERROR_CODE lineBestFit(const CompVLineFloat32Vector& points_cartesian, const CompVHoughLineVector& points_hough, CompVLineFloat32& line);
	COMPV_ERROR_CODE buildPatternCorners(const CompVCalibContex& context);
	COMPV_ERROR_CODE homography(const CompVCalibCameraPlan& plan, CompVHomographyResult& result_homography, CompVMatPtrPtr homographyMat);
	COMPV_ERROR_CODE levmarq(const CompVCalibContex& context, CompVMatPtrPtr K, CompVMatPtrPtr d, CompVMatPtrVector& R, CompVMatPtrVector& t);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	size_t m_nPatternCornersNumRow;
	size_t m_nPatternCornersNumCol;
	size_t m_nPatternCornersTotal;
	size_t m_nPatternLinesTotal;
	size_t m_nPatternLinesHz;
	size_t m_nPatternLinesVt;
	size_t m_nPatternBlockSizePixel;
	CompVEdgeDetePtr m_ptrCanny;
	CompVHoughPtr m_ptrHough;
	CompVMatPtr m_ptrPatternCorners;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CALIB_CAMERA_H_ */
