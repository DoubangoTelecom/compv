/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CALIB_COMMON_H_)
#define _COMPV_CORE_CALIB_COMMON_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_allocators.h"

#if !defined(COMPV_CALIB_PATTERN_ROWS_COUNT)
#	define COMPV_CALIB_PATTERN_ROWS_COUNT			10 // Number of rows
#endif /* COMPV_CALIB_PATTERN_ROWS_COUNT */
#if !defined(COMPV_CALIB_PATTERN_COLS_COUNT)
#	define COMPV_CALIB_PATTERN_COLS_COUNT			8  // Number of corners per column
#endif /* COMPV_CALIB_PATTERN_COLS_COUNT */

COMPV_NAMESPACE_BEGIN()

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
	COMPV_CALIB_CAMERA_RESULT_NO_CHANGES, /// almost same plane. Only if "check_plans" is set to true
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INLIERS,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES,
	COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_HOMOGRAPHIES,
	COMPV_CALIB_CAMERA_RESULT_TOO_MUCH_LINES,
	COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS,
	COMPV_CALIB_CAMERA_RESULT_PATTERN_ROTATED, // This is a TODO(dmi) to fix
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

struct CompVCalibContex {
	int verbosity = 0;
	bool levenberg_marquardt = true; // whether to perform non-linear levenberg marquardt optimisation after getting initial calib results
	bool check_plans = false; // whether to check if the current and previous plans are almost the same. If they are almost the same, reject!
	bool compute_tangential_dist = false; // whether to compute tangential distorsions (p1, p2) in addition to radial distorsions (k1, k2)
	bool compute_skew = false; // whether to compute skew value (part of camera matrix K) or set value to 0. 99.99% cameras should have skew equal 0 and this speedup levmarq process (less params)
	compv_float32_t check_plans_min_sad = 13.f;
	COMPV_CALIB_CAMERA_RESULT_CODE code = COMPV_CALIB_CAMERA_RESULT_NONE;
	CompVCabLines lines_raw;
	CompVCabLines lines_grouped;
	CompVCalibCameraPlan plane_curr;
	CompVMatPtr edges;
	compv_float64_t reproj_error = DBL_MAX; // reprojection error, should be < 0.8
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
		reproj_error = DBL_MAX;
	}
	COMPV_INLINE void clean() {
		reset();
		planes.clear();
		K = nullptr;
		d = nullptr;
	}
	COMPV_INLINE bool isOK() const {
		return code == COMPV_CALIB_CAMERA_RESULT_OK;
	}
	COMPV_INLINE bool isDone() const {
		return (K && d);
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CALIB_COMMON_H_ */
