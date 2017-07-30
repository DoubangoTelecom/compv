/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CALIB_CAMERA_H_)
#define _COMPV_CORE_CALIB_CAMERA_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/core/calib/compv_core_calib_homography.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_allocators.h"
#include "compv/base/compv_features.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CalibCamera)

struct CompVCabLines {
	CompVHoughLineVector lines_hough;
	CompVLineFloat32Vector lines_cartesian;
};

struct CompVCabLineFloat32 {
	CompVLineFloat32 line;
	bool vt;
	size_t strength;
};
typedef std::vector<CompVCabLineFloat32, CompVAllocatorNoDefaultConstruct<CompVCabLineFloat32> > CompVCabLineFloat32Vector;

enum COMPV_CALIB_CAMERA_RESULT_CODE {
	COMPV_CALIB_CAMERA_RESULT_NONE,
	COMPV_CALIB_CAMERA_RESULT_OK,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INLIERS,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES,
	COMPV_CALIB_CAMERA_RESULT_TOO_MUCH_LINES,
	COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS,
};

struct CompVCalibCameraResult {
	COMPV_CALIB_CAMERA_RESULT_CODE code;
	CompVCabLines lines_raw;
	CompVCabLines lines_grouped;
	CompVPointFloat32Vector points_intersections;
	bool rotated;
	CompVMatPtr edges;
	CompVMatPtr homography;
public:
	void reset() {
		code = COMPV_CALIB_CAMERA_RESULT_NONE;
		lines_raw.lines_cartesian.clear();
		lines_raw.lines_hough.clear();
		lines_grouped.lines_cartesian.clear();
		lines_grouped.lines_hough.clear();
		points_intersections.clear();
	}
	COMPV_INLINE bool isOK() const {
		return code == COMPV_CALIB_CAMERA_RESULT_OK;
	}
};

class COMPV_CORE_API CompVCalibCamera : public CompVObj
{
protected:
	CompVCalibCamera();

public:
	virtual ~CompVCalibCamera();
	COMPV_OBJECT_GET_ID(CompVBoxInterestPoint);

	COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVCalibCameraResult& result);

	COMPV_INLINE CompVEdgeDetePtr edgeDetector() { return m_ptrCanny; }
	COMPV_INLINE CompVHoughPtr houghTransform() { return m_ptrHough; }

	COMPV_INLINE size_t patternWidth() const { return ((m_bPatternCornersRotated ? m_nPatternCornersNumRow : m_nPatternCornersNumCol) - 1) * m_nPatternBlockSizePixel; }
	COMPV_INLINE size_t patternHeight() const { return ((m_bPatternCornersRotated ? m_nPatternCornersNumCol : m_nPatternCornersNumRow) - 1) * m_nPatternBlockSizePixel; }
	
	static COMPV_ERROR_CODE newObj(CompVCalibCameraPtrPtr calib);

private:
	COMPV_ERROR_CODE subdivision(const size_t image_width, const size_t image_height, const CompVCabLines& lines, CompVCabLines& lines_hz, CompVCabLines& lines_vt);
	COMPV_ERROR_CODE grouping(const size_t image_width, const size_t image_height, const CompVCabLines& lines_parallel, const bool vt, CompVCabLineFloat32Vector& lines_parallel_grouped);
	COMPV_ERROR_CODE lineBestFit(const CompVLineFloat32Vector& points_cartesian, const CompVHoughLineVector& points_hough, CompVLineFloat32& line);
	COMPV_ERROR_CODE buildPatternCorners(const CompVCalibCameraResult& result_calib);
	COMPV_ERROR_CODE homography(CompVCalibCameraResult& result_calib, CompVHomographyResult& result_homography);

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
	bool m_bPatternCornersRotated;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CALIB_CAMERA_H_ */
