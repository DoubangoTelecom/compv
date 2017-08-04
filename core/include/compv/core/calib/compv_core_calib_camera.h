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
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_HOMOGRAPHIES,
	COMPV_CALIB_CAMERA_RESULT_TOO_MUCH_LINES,
	COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS,
};

struct CompVCalibCameraPlan {
	CompVMatPtr pattern;
	size_t pattern_width;
	size_t pattern_height;
	CompVPointFloat32Vector intersections;
	CompVMatPtr homography;
	CompVMatPtr R; // Rotation matrix(3x3) - extrinsic
	CompVMatPtr t; // Translation matrix (3x1)t vector - extrinsic
public:
	COMPV_INLINE void reset() {
		pattern = nullptr;
		pattern_width = 0;
		pattern_height = 0;
		intersections.clear();
		homography = nullptr;
		R = nullptr;
		t = nullptr;
	}
};
typedef std::vector<CompVCalibCameraPlan, CompVAllocatorNoDefaultConstruct<CompVCalibCameraPlan> > CompVCalibCameraPlanVector;

struct CompVCalibCameraResult {
	COMPV_CALIB_CAMERA_RESULT_CODE code;
	CompVCabLines lines_raw;
	CompVCabLines lines_grouped;
	CompVCalibCameraPlan plane_curr;
	bool rotated;
	CompVMatPtr edges;
	CompVCalibCameraPlanVector planes;
	CompVMatPtr K; // Camera matrix (3x3) matrix - intrinsic
	CompVMatPtr d; // Radial/Tangential distorsions (4x1) vector [k1, k2, p1, p2]t - "intrinsic"
public:
	COMPV_INLINE void reset() {
		code = COMPV_CALIB_CAMERA_RESULT_NONE;
		lines_raw.lines_cartesian.clear();
		lines_raw.lines_hough.clear();
		lines_grouped.lines_cartesian.clear();
		lines_grouped.lines_hough.clear();
		plane_curr.reset();
	}
	COMPV_INLINE bool isOK() const {
		return code == COMPV_CALIB_CAMERA_RESULT_OK;
	}
	COMPV_INLINE bool isDone() const {
		return (K && d);
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
	COMPV_ERROR_CODE test(CompVCalibCameraResult& result_calib, CompVPointFloat32Vector& corrected, size_t file_index = 47);

	COMPV_INLINE CompVEdgeDetePtr edgeDetector() { return m_ptrCanny; }
	COMPV_INLINE CompVHoughPtr houghTransform() { return m_ptrHough; }
	
	static COMPV_ERROR_CODE newObj(CompVCalibCameraPtrPtr calib);

private:
	COMPV_ERROR_CODE subdivision(const size_t image_width, const size_t image_height, const CompVCabLines& lines, CompVCabLines& lines_hz, CompVCabLines& lines_vt);
	COMPV_ERROR_CODE grouping(const size_t image_width, const size_t image_height, const CompVCabLines& lines_parallel, const bool vt, const size_t max_strength, CompVCabLineFloat32Vector& lines_parallel_grouped);
	COMPV_ERROR_CODE lineBestFit(const CompVLineFloat32Vector& points_cartesian, const CompVHoughLineVector& points_hough, CompVLineFloat32& line);
	COMPV_ERROR_CODE buildPatternCorners(const CompVCalibCameraResult& result_calib);
	COMPV_ERROR_CODE homography(CompVCalibCameraResult& result_calib, CompVHomographyResult& result_homography, CompVMatPtrPtr homographyMat);
	COMPV_ERROR_CODE calibrate(CompVCalibCameraResult& result_calib);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	size_t m_nPatternCornersNumRow;
	size_t m_nPatternCornersNumCol;
	size_t m_nPatternCornersTotal;
	size_t m_nPatternLinesTotal;
	size_t m_nPatternLinesHz;
	size_t m_nPatternLinesVt;
	size_t m_nPatternBlockSizePixel;
	size_t m_nMinPanesCountBeforeCalib;
	CompVEdgeDetePtr m_ptrCanny;
	CompVHoughPtr m_ptrHough;
	CompVMatPtr m_ptrPatternCornersInPlace;
	CompVMatPtr m_ptrPatternCornersRotated;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CALIB_CAMERA_H_ */
