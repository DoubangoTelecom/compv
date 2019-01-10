/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/calib/compv_core_calib_camera.h"
#include "compv/core/calib/compv_core_calib_utils.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_gauss.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_eigen.h"
#include "compv/base/math/compv_math_trig.h"
#include "compv/base/math/lmfit-6.1/lmmin.h" /* http://apps.jcns.fz-juelich.de/doku/sc/lmfit */
#include "compv/base/time/compv_time.h"

#include <iterator> /* std::back_inserter */
#include <limits> /* std::numeric_limits::smallest */

#define PATTERN_CORNERS_NUM					(PATTERN_ROW_CORNERS_NUM * PATTERN_COL_CORNERS_NUM) // Total number of corners
#define PATTERN_GROUP_MAXLINES				60 // Maximum number of lines per group (errors)
#define PATTERN_BLOCK_SIZE_PIXEL			40

#define CALIBRATION_MIN_PLANS				3

#define HOUGH_ID							COMPV_HOUGHSHT_ID
#define HOUGH_RHO							0.5f // "rho-delta" (half-pixel)
#define HOUGH_THETA							0.5f // "theta-delta" (half-radian)
#define HOUGH_SHT_THRESHOLD_FACT			0.1
#define HOUGH_SHT_THRESHOLD_MIN				5
#define HOUGH_SHT_THRESHOLD					30
#define HOUGH_KHT_THRESHOLD_FACT			1.0 // GS
#define HOUGH_KHT_THRESHOLD					1 // filter later when GS is known	
#define HOUGH_KHT_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGH_KHT_CLUSTER_MIN_SIZE			10
#define HOUGH_KHT_KERNEL_MIN_HEIGTH			0.002f // must be within [0, 1]

#define CANNY_LOW							1.33f
#define CANNY_HIGH							CANNY_LOW*2.f
#define CANNY_KERNEL_SIZE					3

#define COMPV_THIS_CLASSNAME	"CompVCalibCamera"

// Some documentation used in this implementation:
//	- [1] A Flexible New Technique for Camera Calibration: https://github.com/DoubangoTelecom/compv/blob/master/documentation/Camera%20calibration/Zhang_CamCal.pdf
//	- [2] Zhang's Camera Calibration Algorithm: In-Depth Tutorial and Implementation: https://github.com/DoubangoTelecom/compv/blob/master/documentation/Camera%20calibration/Burger-CameraCalibration-20160516.pdf
//	- [3] Photogrammetry I - 16b - DLT & Camera Calibration (2015): https://www.youtube.com/watch?v=Ou9Uj75DJX0

// TODO(dmi): use "ceres-solver" http://ceres-solver.org/index.html

COMPV_NAMESPACE_BEGIN()

#define kSmallRhoFactVt				0.035f /* small = (rho * fact) - this is for 8 cols */
#define kSmallRhoFactHz				0.035f /* small = (rho * fact) - this is for #10 rows */
#define kCheckerboardBoxDistFact	0.357f /* distance * fact */

/*
Groups holding close lines: most-likely to be merged into a single line
*/
struct CompVCabLineGroup {
	const CompVLineFloat32* pivot_cartesian;
	const CompVHoughLine* pivot_hough;
	compv_float32_t pivot_distance; // distance(pivot, origin)
	CompVCabLines lines;
};
typedef std::vector<CompVCabLineGroup, CompVAllocatorNoDefaultConstruct<CompVCabLineGroup> > CompVCabLineGroupVector;

/*
LevMarq user callback data
*/
struct levmarq_data {
	const compv_float64_t *cornersPtr;
	const CompVCalibCameraPlanVector& planes;
	const bool have_tangential_dist;
	const bool have_skew;
	CompVMatPtr K;
	CompVMatPtr d;
	CompVMatPtr t;
	compv_float64_t* K_fx = nullptr;
	compv_float64_t* K_fy = nullptr;
	compv_float64_t* K_cx = nullptr;
	compv_float64_t* K_cy = nullptr;
	compv_float64_t* K_skew = nullptr;
	compv_float64_t* d_k1 = nullptr;
	compv_float64_t* d_k2 = nullptr;
	compv_float64_t* d_p1 = nullptr;
	compv_float64_t* d_p2 = nullptr;
	compv_float64_t* t_x = nullptr;
	compv_float64_t* t_y = nullptr;
	compv_float64_t* t_z = nullptr;

public:
	levmarq_data(const CompVCalibCameraPlanVector& planes_, const compv_float64_t *cornersPtr_, const bool have_tangential_dist_, const bool have_skew_)
		: cornersPtr(cornersPtr_)
		, planes(planes_)
		, have_tangential_dist(have_tangential_dist_)
		, have_skew(have_skew_)
	{ }
};

/*
LevMarq evalution callback function
*/
static void levmarq_eval(const compv_float64_t *par, int m_dat, const void *data, compv_float64_t *fvec, int *userbreak);
/*
Function to check whether the segments intersect
*/
static bool segment_get_intersection(const CompVLineFloat32& line0, const CompVLineFloat32& line1, compv_float32_t *i_x, compv_float32_t *i_y = nullptr);

CompVCalibCamera::CompVCalibCamera(size_t nPatternRowsCount, size_t nPatternColsCount)
	: m_nPatternCornersNumRow(nPatternRowsCount)
	, m_nPatternCornersNumCol(nPatternColsCount)
	, m_nPatternLinesHz(m_nPatternCornersNumRow)
	, m_nPatternLinesVt(m_nPatternCornersNumCol)
	, m_nPatternBlockSizePixel(PATTERN_BLOCK_SIZE_PIXEL)
{
	m_nPatternCornersTotal = m_nPatternCornersNumRow * m_nPatternCornersNumCol;
	m_nPatternLinesTotal = (m_nPatternCornersNumRow + m_nPatternCornersNumCol);
}

CompVCalibCamera::~CompVCalibCamera()
{

}

COMPV_ERROR_CODE CompVCalibCamera::process(const CompVMatPtr& image, CompVCalibContex& context)
{
	COMPV_CHECK_EXP_RETURN(!image || image->elmtInBytes() != sizeof(uint8_t) || image->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");

	// Reset the previous result
	context.reset();
	CompVCalibCameraPlan& plan_curr = context.plane_curr;

	const size_t image_width = image->cols();
	const size_t image_height = image->rows();
	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	compv_float32_t intersect_x, intersect_y;
	static const compv_float32_t intersect_z = 1.f;

	static const compv_float32_t x_axis_right = 1e15f;
	static const compv_float32_t x_axis_left = -1e15f;
	static const CompVLineFloat32 x_axis(CompVPointFloat32(x_axis_left, 0.f), CompVPointFloat32(x_axis_right, 0.f));
	
	/* Canny edge detection */
	COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(image, &context.edges));

	/* Hough lines */
	// For SHT, set the threshold before processing. But for KHT, we need the global scale (GS) which is defined only *after* processing
	if (m_ptrHough->id() == COMPV_HOUGHSHT_ID) {
		COMPV_CHECK_CODE_RETURN(m_ptrHough->setInt(COMPV_HOUGH_SET_INT_THRESHOLD, HOUGH_SHT_THRESHOLD_MIN));
	}
	// Process
	COMPV_CHECK_CODE_RETURN(m_ptrHough->process(context.edges, context.lines_raw.lines_hough));

	/* Return if no enough points */
	if (context.lines_raw.lines_hough.size() < m_nPatternLinesTotal) {
		if (context.verbosity > 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough lines after hough transform: %zu", context.lines_raw.lines_hough.size());
		}
		context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Remove weak lines (round #1) */
	const size_t strongestStrength = context.lines_raw.lines_hough.begin()->strength;
	if (context.lines_raw.lines_hough.size() > m_nPatternLinesTotal) {
		if (m_ptrHough->id() == COMPV_HOUGHKHT_ID) {
			// Remove weak lines using global scale (GS)
			compv_float64_t gs;
			COMPV_CHECK_CODE_RETURN(m_ptrHough->getFloat64(COMPV_HOUGHKHT_GET_FLT64_GS, &gs));
			const size_t min_strength = static_cast<size_t>(gs * HOUGH_KHT_THRESHOLD_FACT);
			auto fncShortLines = std::remove_if(context.lines_raw.lines_hough.begin(), context.lines_raw.lines_hough.end(), [&](const CompVHoughLine& line) {
				return line.strength < min_strength;
			});
			context.lines_raw.lines_hough.erase(fncShortLines, context.lines_raw.lines_hough.end());
		}
		else {
			// Remove weak lines using global max_strength
			const size_t min_strength = static_cast<size_t>((strongestStrength << 1) / std::max(m_nPatternLinesHz, m_nPatternLinesVt));
			auto fncShortLines = std::remove_if(context.lines_raw.lines_hough.begin(), context.lines_raw.lines_hough.end(), [&](const CompVHoughLine& line) {
				return line.strength < min_strength;
			});
			context.lines_raw.lines_hough.erase(fncShortLines, context.lines_raw.lines_hough.end());
		}
	}

	/* Return if no enough points */
	if (context.lines_raw.lines_hough.size() < m_nPatternLinesTotal) {
		if (context.verbosity > 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough lines after removing weak lines: %zu", context.lines_raw.lines_hough.size());
		}
		context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Convert from polar to cartesian coordinates */
	COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(image_width, image_height, context.lines_raw.lines_hough, context.lines_raw.lines_cartesian));

	/* Lines subdivision */
	CompVCabLines lines_hz, lines_vt;
	COMPV_CHECK_CODE_RETURN(subdivision(image_width, image_height, context.lines_raw, lines_hz, lines_vt));
	if (lines_hz.lines_cartesian.size() < m_nPatternLinesHz) {
		if (context.verbosity > 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [hz] lines after subdivision: %zu", lines_hz.lines_cartesian.size());
		}
		context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}
	if (lines_vt.lines_cartesian.size() < m_nPatternLinesVt) {
		if (context.verbosity > 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [vt] lines after subdivision: %zu", lines_vt.lines_cartesian.size());
		}
		context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}
	if ((lines_hz.lines_cartesian.size() + lines_vt.lines_cartesian.size()) < m_nPatternLinesTotal) {
		if (context.verbosity > 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [vt+hz] lines after subdivision: %zu+%zu", lines_hz.lines_cartesian.size(), lines_vt.lines_cartesian.size());
		}
		context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	// Hz-grouping
	const size_t max_strength = context.lines_raw.lines_hough.begin()->strength; // lines_hough is sorted (by hought->process)
	CompVCabLineFloat32Vector lines_cab_hz_grouped, lines_cab_vt_grouped;
	if (lines_hz.lines_cartesian.size() > m_nPatternLinesHz) {
		COMPV_CHECK_CODE_RETURN(grouping(image_width, image_height, lines_hz, false, max_strength, std::max(m_nPatternLinesVt, m_nPatternLinesHz), lines_cab_hz_grouped));
		if (lines_cab_hz_grouped.size() < m_nPatternLinesHz) {
			if (context.verbosity > 0) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [hz] grouping we got less lines than what is requires, not a good news at all");
			}
			context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	if (lines_cab_hz_grouped.size() != m_nPatternLinesHz) {
		if (context.verbosity > 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [hz] grouping we don't have exactly %zu lines but (%zu). Maybe our grouping function missed some orphans", m_nPatternLinesHz, lines_cab_hz_grouped.size());
		}
	}
	
	// Vt-grouping
	if (lines_vt.lines_cartesian.size() > m_nPatternLinesVt) {
		COMPV_CHECK_CODE_RETURN(grouping(image_width, image_height, lines_vt, true, max_strength, std::max(m_nPatternLinesVt, m_nPatternLinesHz), lines_cab_vt_grouped));
		if (lines_cab_vt_grouped.size() < m_nPatternLinesVt) {
			if (context.verbosity > 0) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [vt] grouping we got less lines than what is requires. Not a good news at all");
			}
			context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	if (lines_cab_vt_grouped.size() != m_nPatternLinesVt) {
		if (context.verbosity > 0) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [vt] grouping we don't have exactly %zu lines but (%zu). Maybe our grouping function missed some orphans", m_nPatternLinesVt, lines_cab_vt_grouped.size());
		}
	}

	/* Pack all lines, sort and keep the best */
	if (lines_cab_hz_grouped.size() > m_nPatternLinesHz) {
		std::sort(lines_cab_hz_grouped.begin(), lines_cab_hz_grouped.end(), [](const CompVCabLineFloat32 &line1, const CompVCabLineFloat32 &line2) {
			return (line1.strength > line2.strength);
		});
		lines_cab_hz_grouped.resize(m_nPatternLinesHz);
	}
	if (lines_cab_vt_grouped.size() > m_nPatternLinesVt) {
		std::sort(lines_cab_vt_grouped.begin(), lines_cab_vt_grouped.end(), [](const CompVCabLineFloat32 &line1, const CompVCabLineFloat32 &line2) {
			return (line1.strength > line2.strength);
		});
		lines_cab_vt_grouped.resize(m_nPatternLinesVt);
	}
	
	CompVLineFloat32Vector lines_hz_grouped, lines_vt_grouped;
	std::transform(lines_cab_hz_grouped.begin(), lines_cab_hz_grouped.end(), std::back_inserter(lines_hz_grouped), [](const CompVCabLineFloat32& line) {
		return line.line;
	});
	std::transform(lines_cab_vt_grouped.begin(), lines_cab_vt_grouped.end(), std::back_inserter(lines_vt_grouped), [](const CompVCabLineFloat32& line) {
		return line.line;
	});

	/* Sorting the lines */
	// Sort top-bottom (hz lines)
	std::sort(lines_hz_grouped.begin(), lines_hz_grouped.end(), [](const CompVLineFloat32 &line1, const CompVLineFloat32 &line2) {
		return std::tie(line1.a.y, line1.b.y) < std::tie(line2.a.y, line2.b.y); // coorect only because x-components are constant when using CompV's hough implementations (otherwise use intersection with y-axis)
	});
	// Sort left-right (vt lines): sort the intersection with x-axis
	std::vector<std::pair<compv_float32_t, CompVLineFloat32> > intersections;
	intersections.reserve(lines_vt_grouped.size());
	for (CompVLineFloat32Vector::const_iterator i = lines_vt_grouped.begin(); i < lines_vt_grouped.end(); ++i) {
		if (!segment_get_intersection(*i, x_axis, &intersect_x)) {
			// Must never happen
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Vertical line must intersect with x-axis (%f, %f)-(%f, %f)", i->a.x, i->a.y, i->b.x, i->b.y);
			context.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
			return COMPV_ERROR_CODE_S_OK;
		}
		intersections.push_back(std::make_pair(intersect_x, *i));
	}
	std::sort(intersections.begin(), intersections.end(), [](const std::pair<compv_float32_t, const CompVLineFloat32> &pair1, const std::pair<compv_float32_t, const CompVLineFloat32> &pair2) {
		return pair1.first < pair2.first;
	});
	lines_vt_grouped.clear();
	std::transform(intersections.begin(), intersections.end(), std::back_inserter(lines_vt_grouped), [](const std::pair<compv_float32_t, CompVLineFloat32>& p) { 
		return p.second; 
	});

	/* Push grouped lines */
	lines_vt_grouped.reserve(lines_hz_grouped.size() + lines_vt_grouped.size());
	context.lines_grouped.lines_cartesian.assign(lines_hz_grouped.begin(), lines_hz_grouped.end());
	context.lines_grouped.lines_cartesian.insert(context.lines_grouped.lines_cartesian.end(), lines_vt_grouped.begin(), lines_vt_grouped.end());

	/* Compute intersections */
	for (CompVLineFloat32Vector::const_iterator i = lines_hz_grouped.begin(); i < lines_hz_grouped.end(); ++i) {
		for (CompVLineFloat32Vector::const_iterator j = lines_vt_grouped.begin(); j < lines_vt_grouped.end(); ++j) {
			// Compute intersection
			if (!segment_get_intersection(*i, *j, &intersect_x, &intersect_y)) {
				if (context.verbosity > 0) {
					COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No intersection between the lines. Stop processing");
				}
				context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS;
				return COMPV_ERROR_CODE_S_OK;
			}
			if (intersect_x < 0.f || intersect_y < 0.f || intersect_x >= image_widthF || intersect_y >= image_heightF) {
				if (context.verbosity > 0) {
					COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersection outside the image domain. Stop processing");
				}
				context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS;
				return COMPV_ERROR_CODE_S_OK;
			}
			// Push intersection
			plan_curr.intersections.push_back(CompVPointFloat32(intersect_x, intersect_y, intersect_z)); // z is fake and contain distance to origine (to avoid computing distance several times)
		}

		// Computing homography on garbage is very sloow and to make sure this is a checkerboard, we check
		// that the intersections with the (x/y)-axis are almost equidistant and going forward (increasing)
		// !!The boxes in the checkerboard MUST BE SQUARE!!
		if (i != lines_hz_grouped.begin()) { // not first line
			compv_float32_t dist, dist_err, dist_approx_x = 0.f, dist_err_max_x = 0.f, dist_approx_y = 0.f, dist_err_max_y = 0.f; // not same distortion across x and y -> use different distance estimation
			CompVPointFloat32Vector::const_iterator k = plan_curr.intersections.end() - (m_nPatternLinesVt << 1); // #2 last rows
			CompVPointFloat32Vector::const_iterator m = (k + m_nPatternLinesVt); // move to next line
			for (size_t index = 0; index < m_nPatternLinesVt; ++k, ++m, ++index) {
				/* x-axis */
				if (index == 1) {
					// compute default approx distance (x-axis)
					dist_approx_x = (k->x - (k - 1)->x);
					dist_err_max_x = (dist_approx_x * kCheckerboardBoxDistFact) + 1.f;
				}
				if (index) { // not first index
					// Previous x-intersection must be < current x-intersection (increasing)
					if ((k->x <= (k - 1)->x) || (m->x <= (m - 1)->x)) {
						if (context.verbosity > 0) {
							COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not going forward (x-axis). Stop processing");
						}
						context.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
						return COMPV_ERROR_CODE_S_OK;
					}
					// Check if equidistant (x-axis)
					if (index > 1) {
						dist = (m->x - (m - 1)->x);
						dist_err = std::abs(dist - dist_approx_x);
						if (dist_err > dist_err_max_x) {
							if (context.verbosity > 0) {
								COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not equidistant (xm-axis). Stop processing");
							}
							context.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
							return COMPV_ERROR_CODE_S_OK;
						}
						dist = (k->x - (k - 1)->x);
						dist_err = std::abs(dist - dist_approx_x);
						if (dist_err > dist_err_max_x) {
							if (context.verbosity > 0) {
								COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not equidistant (xk-axis). Stop processing");
							}
							context.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
							return COMPV_ERROR_CODE_S_OK;
						}
						// Refine dist-approx
						dist_approx_x = dist;
						dist_err_max_x = (dist_approx_x * kCheckerboardBoxDistFact) + 1.f;
					}
				}

				/* y-axis */
				dist = (m->y - k->y);
				if (!index) {
					// compute default approx distance (y-axis)
					dist_approx_y = dist;
					dist_err_max_y = (dist_approx_y * kCheckerboardBoxDistFact) + 1.f;
				}
				// Top y-intersection must be < bottom y-intersection (increasing)
				if (dist <= 0.f) {
					if (context.verbosity > 0) {
						COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not going forward (y-axis). Stop processing");
					}
					context.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
					return COMPV_ERROR_CODE_S_OK;
				}
				// Check if equidistant (y-axis)
				if (index > 0) {
					// y-axis
					dist_err = std::abs(dist - dist_approx_y);
					if (dist_err > dist_err_max_y) {
						if (context.verbosity > 0) {
							COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not equidistant (y-axis). Stop processing");
						}
						context.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
						return COMPV_ERROR_CODE_S_OK;
					}
					// Refine dist-approx
					dist_approx_y = dist;
					dist_err_max_y = (dist_approx_y * kCheckerboardBoxDistFact) + 1.f;
				}
			}
		}
	}

	/* Transpose the intersections (no longer needed) */
#if 0
	if (rotated) {
		COMPV_ASSERT(m_nPatternCornersTotal == plan_curr.intersections.size());
		CompVPointFloat32Vector intersections(m_nPatternCornersTotal);
		CompVPointFloat32Vector::const_iterator it0 = plan_curr.intersections.begin() + (m_nPatternCornersTotal - m_nPatternCornersNumRow);
		size_t i, j, k, m;
		for (j = 0, m = 0; j < m_nPatternCornersNumRow; ++j, ++it0) {
			for (i = 0, k = 0; i < m_nPatternCornersNumCol; ++i, ++m, k += m_nPatternCornersNumRow) {
				intersections[m] = *(it0 - k);
			}
		}
		plan_curr.intersections.assign(intersections.begin(), intersections.end());
	}
#endif

	/* Check if it's the same plane or not */
	if (context.check_plans && !context.planes.empty()) {
		const CompVCalibCameraPlan& plan_last = context.planes.back();
		compv_float32_t sad = 0.f;
		CompVPointFloat32Vector::const_iterator i, j;
		const size_t count = std::min(plan_last.intersections.size(), plan_curr.intersections.size()); // should be the same number of intersections
		size_t k;
		for (i = plan_last.intersections.begin(), j = plan_curr.intersections.begin(), k = 0; k < count; ++k, ++i, ++j) {
			sad += std::abs(i->x - j->x) + std::abs(i->y - j->y);
		}
		sad /= static_cast<compv_float32_t>(count); // mean
		if (sad < context.check_plans_min_sad) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Almost same plane. mean(SAD)=%f < %f", sad, context.check_plans_min_sad);
			context.code = COMPV_CALIB_CAMERA_RESULT_NO_CHANGES;
			return COMPV_ERROR_CODE_S_OK;
		}
	}

	/* Build patterns */
	COMPV_CHECK_CODE_RETURN(buildPatternCorners(context));
	plan_curr.pattern = m_ptrPatternCorners;
	plan_curr.pattern_width = ((m_nPatternCornersNumCol - 1) * m_nPatternBlockSizePixel);
	plan_curr.pattern_height = ((m_nPatternCornersNumRow - 1) * m_nPatternBlockSizePixel);

	/* Compute homography: [2] 3.2.1 Homography estimation with the Direct Linear Transformation(DLT) */
	CompVHomographyResult result_homography;
	COMPV_CHECK_CODE_RETURN(homography(plan_curr, result_homography, &plan_curr.homography));
	if (result_homography.inlinersCount < (m_nPatternCornersTotal - std::max(m_nPatternCornersNumRow, m_nPatternCornersNumCol))) { // We allow at most #1 row/col error (outliers)
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough inliners after homography computation (%zu / %zu)", result_homography.inlinersCount, m_nPatternCornersTotal);
		context.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INLIERS;
		return COMPV_ERROR_CODE_S_OK;
	}
	
	// Save the plane for future calibration
	context.planes.push_back(plan_curr);

	// Set result code to OK
	context.code = COMPV_CALIB_CAMERA_RESULT_OK;

	// Return
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibCamera::calibrate(CompVCalibContex& context)
{
	COMPV_CHECK_EXP_RETURN(context.planes.size() < CALIBRATION_MIN_PLANS, COMPV_ERROR_CODE_E_INVALID_CALL, "Calibration process requires at least #3 images");

	const size_t numPlanes = context.planes.size();
	CompVCalibCameraPlanVector& planes = context.planes;
	CompVCalibCameraPlanVector::iterator it_planes;
	const compv_float64_t *h0, *h1, *h2;
	compv_float64_t h00, h01, h10, h11, h20, h21;

	/* [2] 3.3 Step 2: Determining the intrinsic camera parameters */
	// For multiple images (2n x 6) matrix: https://youtu.be/Ou9Uj75DJX0?t=22m13s
	// Each image have #2 contribution and we require at least #3 images
	CompVMatPtr V;
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<compv_float64_t>(&V, (numPlanes << 1), 6, 1)));
	compv_float64_t* V0 = V->ptr<compv_float64_t>(0); // v01(H)
	compv_float64_t* V1 = V->ptr<compv_float64_t>(1); // (v00(H) - v11(H))
	const size_t vStrideTimes2 = V->stride() << 1;
	// Computing V matrix use to solve V.b = 0 equation
	for (it_planes = planes.begin(); it_planes < planes.end(); ++it_planes) {
		// [2] equation (96)
		const CompVMatPtr& homography = it_planes->homography;
		h0 = homography->ptr<const compv_float64_t>(0);
		h1 = homography->ptr<const compv_float64_t>(1);
		h2 = homography->ptr<const compv_float64_t>(2);
		h00 = h0[0];
		h01 = h0[1];
		h10 = h1[0];
		h11 = h1[1];
		h20 = h2[0];
		h21 = h2[1];
		// v01(H)
		V0[0] = (h00*h01);
		V0[1] = (h00*h11) + (h10*h01);
		V0[2] = (h10*h11);
		V0[3] = (h20*h01) + (h00*h21);
		V0[4] = (h20*h11) + (h10*h21);
		V0[5] = (h20*h21);
		// v00(H) - v11(H)
		V1[0] = (h00*h00) - (h01*h01);
		V1[1] = (h00*h10 + h10*h00) - (h01*h11 + h11*h01);
		V1[2] = (h10*h10) - (h11*h11);
		V1[3] = (h20*h00 + h00*h20) - (h21*h01 + h01*h21);
		V1[4] = (h20*h10 + h10*h20) - (h21*h11 + h11*h21);
		V1[5] = (h20*h20) - (h21*h21);
		// Next
		V0 += vStrideTimes2;
		V1 += vStrideTimes2;
	}

	/* Find b by solving Vb = 0: [2] equation (98)*/
	// Compute S = Vt*V, 6x6 symetric matrix
	CompVMatPtr S;
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(V, &S));
	COMPV_GCC_DISABLE_WARNINGS_END()
	// Find Eigen values and vectors
	CompVMatPtr eigenValues, eigneVectors;
	static const bool sortEigenValuesVectors = true;
	static const bool transposeEigenVectors = true; // row-vector?
	static const bool promoteZerosInEigenValues = false; // set to zero when < eps
	COMPV_CHECK_CODE_RETURN(CompVMathEigen<compv_float64_t>::findSymm(S, &eigenValues, &eigneVectors, sortEigenValuesVectors, transposeEigenVectors, promoteZerosInEigenValues));
	const compv_float64_t* bPtr = eigneVectors->ptr<const compv_float64_t>(5); // 6x6 matrix -> index of the smallest eigen value is last one = #5

	// [2] Algorithm 4.4 Calculation of intrinsic camera parameters (Version A).
	const compv_float64_t B0 = bPtr[0];
	const compv_float64_t B1 = bPtr[1];
	const compv_float64_t B2 = bPtr[2];
	const compv_float64_t B3 = bPtr[3];
	const compv_float64_t B4 = bPtr[4];
	const compv_float64_t B5 = bPtr[5];
	const compv_float64_t B1s = B1*B1;
	const compv_float64_t B0B2 = B0*B2;
	const compv_float64_t B0B4 = B0*B4;
	const compv_float64_t B1B4 = B1*B4;
	const compv_float64_t B2B3 = B2*B3;
	const compv_float64_t ww = (B0B2 * B5) - (B1s * B5) - (B0B4*B4) + (2 * B1B4*B3) - (B2B3*B3); // // [2] equation (104)
	const compv_float64_t dd = B0B2 - B1s; // [2] equation (105)
	const compv_float64_t dd2 = dd * dd;
	const compv_float64_t dds = 1.0 / dd;
	const compv_float64_t alpha = std::sqrt(std::abs(ww / (dd * B0))); // [2] equation (99)
	const compv_float64_t beta = std::sqrt(std::abs((ww / dd2) * B0)); // [2] equation (100)
	const compv_float64_t gamma = context.compute_skew ? (std::sqrt(std::abs(ww / (dd2 * B0))) * B1) : 0.0; // [2] equation (101)
	const compv_float64_t uc = (B1B4 - B2B3) * dds; // [2] equation (102)
	const compv_float64_t vc = ((B1*B3) - B0B4) * dds; // // [2] equation (103)
	CompVMatPtr& K = context.K;
	COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<compv_float64_t>(&K, 3, 3)));
	*K->ptr<compv_float64_t>(0, 0) = alpha; // fx
	*K->ptr<compv_float64_t>(0, 1) = gamma; // skew
	*K->ptr<compv_float64_t>(0, 2) = uc; // cx
	*K->ptr<compv_float64_t>(1, 0) = 0.0;
	*K->ptr<compv_float64_t>(1, 1) = beta; // fy
	*K->ptr<compv_float64_t>(1, 2) = vc; // cy
	*K->ptr<compv_float64_t>(2, 0) = 0.0;
	*K->ptr<compv_float64_t>(2, 1) = 0.0;
	*K->ptr<compv_float64_t>(2, 2) = 1.0;
	/* Compute Kinv (3x3) */
	// Inverse (3x3): http://mathworld.wolfram.com/MatrixInverse.html
	// Compute determinant(Kinv)
	compv_float64_t detKinv = (alpha * beta);
	COMPV_CHECK_EXP_RETURN(!detKinv, COMPV_ERROR_CODE_E_INVALID_CALL, "Camera matrix is singular");
	// Compute Kinv values
	detKinv = (1.0 / detKinv);
	const compv_float64_t k11 = (beta * detKinv);
	const compv_float64_t k21 = -(gamma * detKinv);
	const compv_float64_t k31 = ((gamma * vc) - (beta * uc)) * detKinv;
	const compv_float64_t k22 = (alpha * detKinv);
	const compv_float64_t k32 = -(vc * alpha * detKinv);
	const compv_float64_t k33 = (alpha * beta) * detKinv;

	/*
		[2] Algorithm 4.6 Calculation of extrinsic view parameters
		[2] Algorithm 4.7 Estimation of the radial lens distortion parameters.
	*/
	compv_float64_t r11, r12, r13, r21, r22, r23, r31, r32, r33, t11, t12, t13, k;
	CompVMatPtr D, d;
	const bool compute_tangential_dist = context.compute_tangential_dist;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&D, (numPlanes * m_nPatternCornersTotal) << 1, compute_tangential_dist ? 4 : 2));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&d, (numPlanes * m_nPatternCornersTotal) << 1, 1));
	const size_t DStrideTimes2 = D->stride() << 1;
	const size_t dStrideTimes2 = d->stride() << 1;
	compv_float64_t* DPtr0 = D->ptr<compv_float64_t>();
	compv_float64_t* DPtr1 = DPtr0 + D->stride();
	compv_float64_t* dPtr0 = d->ptr<compv_float64_t>();
	compv_float64_t* dPtr1 = dPtr0 + d->stride();
	for (it_planes = planes.begin(); it_planes < planes.end(); ++it_planes) {
		const CompVMatPtr& homography = it_planes->homography;
		// r0 = k.(Kinv.h0)
		h00 = *homography->ptr<const compv_float64_t>(0, 0);
		h10 = *homography->ptr<const compv_float64_t>(1, 0);
		h20 = *homography->ptr<const compv_float64_t>(2, 0);
		r11 = ((k11 * h00) + (k21 * h10) + (k31 * h20));
		r12 = ((k22 * h10) + (k32 * h20));
		r13 = (k33 * h20);
		k = 1.0 / std::sqrt((r11 * r11) + (r12 * r12) + (r13 * r13));
		r11 *= k;
		r12 *= k;
		r13 *= k;

		// r1 = k.(Kinv.h1)
		h00 = *homography->ptr<const compv_float64_t>(0, 1);
		h10 = *homography->ptr<const compv_float64_t>(1, 1);
		h20 = *homography->ptr<const compv_float64_t>(2, 1);
		r21 = ((k11 * h00) + (k21 * h10) + (k31 * h20));
		r22 = ((k22 * h10) + (k32 * h20));
		r23 = (k33 * h20);
		r21 *= k;
		r22 *= k;
		r23 *= k;

		// r2 = r0 x r1 (https://www.mathsisfun.com/algebra/vectors-cross-product.html)
		r31 = r12 * r23 - r13 * r22;
		r32 = r13 * r21 - r11 * r23;
		r33 = r11 * r22 - r12 * r21;

		// t = k.(kinv.h2)
		h00 = *homography->ptr<const compv_float64_t>(0, 2);
		h10 = *homography->ptr<const compv_float64_t>(1, 2);
		h20 = *homography->ptr<const compv_float64_t>(2, 2);
		t11 = ((k11 * h00) + (k21 * h10) + (k31 * h20));
		t12 = ((k22 * h10) + (k32 * h20));
		t13 = (k33 * h20);
		t11 *= k;
		t12 *= k;
		t13 *= k;

		/*
		Computing radial distortion
		[1] 3.3 Dealing with radial distortion
		[2] 3.5 Step 4: Estimating radial lens distortion
		*/
		const CompVPointFloat32Vector& intersections = it_planes->intersections;
		const size_t numPoints = intersections.size();
		CompVPointFloat32Vector::const_iterator it_intersection;
		size_t i;
		const compv_float64_t* patternX = it_planes->pattern->ptr<compv_float64_t>(0);
		const compv_float64_t* patternY = it_planes->pattern->ptr<compv_float64_t>(1);

		/* [2] Algorithm 4.7 Estimation of the radial lens distortion parameters. */
		for (i = 0, it_intersection = intersections.begin(); i < numPoints; ++i, ++it_intersection) {
			const compv_float64_t z = (patternX[i] * r13) + (patternY[i] * r23) + t13;
			const compv_float64_t scale = z ? (1.0 / z) : 1.0;
			const compv_float64_t x = ((patternX[i] * r11) + (patternY[i] * r21) + t11) * scale;
			const compv_float64_t y = ((patternX[i] * r12) + (patternY[i] * r22) + t12) * scale;
			const compv_float64_t r2 = (x * x) + (y * y);
			const compv_float64_t r4 = r2 * r2;
			const compv_float64_t u = static_cast<compv_float64_t>(it_intersection->x);
			const compv_float64_t v = static_cast<compv_float64_t>(it_intersection->y);
			const compv_float64_t uh = (alpha * x) + (gamma * y) + uc;
			const compv_float64_t vh = (beta * y) + vc;
			const compv_float64_t du = (u - uc);
			const compv_float64_t dv = (v - vc);

			// Radial distortion part (D)
			DPtr0[0] = (du * r2);
			DPtr0[1] = (du * r4);
			DPtr1[0] = (dv * r2);
			DPtr1[1] = (dv * r4);
			// Tangential distortion part (D) - probably not correct, not fully checked
			if (compute_tangential_dist) {
				const compv_float64_t two_dudv = 2 * du * dv;
				DPtr0[2] = two_dudv;
				DPtr0[3] = r2 + (2 * (du * du));
				DPtr1[2] = r2 + (dv * dv);
				DPtr1[3] = two_dudv;
			}
			// d
			dPtr0[0] = (uh - u);
			dPtr1[0] = (vh - v);

			// Move to next
			DPtr0 += DStrideTimes2;
			DPtr1 += DStrideTimes2;
			dPtr0 += dStrideTimes2;
			dPtr1 += dStrideTimes2;
		} // for (it_intersection....

		// Packing rotation matrix R (extrinsic)
		compv_float64_t *ptr;
		CompVMatPtr& R = it_planes->R;
		CompVMatPtr U, D;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&R, 3, 3));
		ptr = R->ptr<compv_float64_t>(0), ptr[0] = r11, ptr[1] = r21, ptr[2] = r31;
		ptr = R->ptr<compv_float64_t>(1), ptr[0] = r12, ptr[1] = r22, ptr[2] = r32;
		ptr = R->ptr<compv_float64_t>(2), ptr[0] = r13, ptr[1] = r23, ptr[2] = r33;
		// [1] Appendix C Approximating a 3x3 matrix by a Rotation Matrix
		// TODO(dmi): in the future if you've any error based on high MSE after computing proj2DError then,
		// blame the next code. Just, comment the next lines to avoid overriding R and check
		// if this solve the issue.
		COMPV_CHECK_CODE_RETURN(CompVMatrix::svd(R, &U, &D, &V, false));
		COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulABt(U, V, &R));
		COMPV_GCC_DISABLE_WARNINGS_END()

		// Packing translation vector t (extrinsic)
		CompVMatPtr& t = it_planes->t;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&t, 1, 3));
		ptr = t->ptr<compv_float64_t>(0), ptr[0] = t11, ptr[1] = t12, ptr[2] = t13;
	} // for (it_planes...

	/*
	Final radial distortions (Least Square minimization):
	- [1] 3.3 Dealing with radial distortion
	- [2] 3.5 Step 4: Estimating radial lens distortion
	k = (Dt.D)*.Dt.d
	*/
	CompVMatPtr DtD, kd;
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(D, &DtD)); // Dt.D
	COMPV_GCC_DISABLE_WARNINGS_END()
	if (compute_tangential_dist) {
		CompVMatPtr DtDinv, k;
		COMPV_CHECK_CODE_RETURN(CompVMatrix::pseudoinv(DtD, &DtDinv)); // (Dt.D)*
		COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulABt(DtDinv, D, &kd)); // (Dt.D)*.Dt
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(kd, d, &context.d)); // (Dt.D)*.Dt.d
		COMPV_GCC_DISABLE_WARNINGS_END()
	}
	else {
		const compv_float64_t aa = *DtD->ptr<const compv_float64_t>(0, 0);
		const compv_float64_t bb = *DtD->ptr<const compv_float64_t>(0, 1);
		const compv_float64_t cc = *DtD->ptr<const compv_float64_t>(1, 0);
		const compv_float64_t dd = *DtD->ptr<const compv_float64_t>(1, 1);
		// (2x2) matrix inverse: https://www.mathsisfun.com/algebra/matrix-inverse.html
		compv_float64_t det = (aa*dd) - (bb*cc);
		det = 1.0 / det;
		*DtD->ptr<compv_float64_t>(0, 0) = (dd * det);
		*DtD->ptr<compv_float64_t>(0, 1) = -(bb * det);
		*DtD->ptr<compv_float64_t>(1, 0) = -(cc * det);
		*DtD->ptr<compv_float64_t>(1, 1) = (aa * det);
		// (DtDinv.Dt).d
		COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
		COMPV_CHECK_CODE_RETURN(CompVMath::mulABt(DtD, D, &kd));
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(kd, d, &context.d));
		COMPV_GCC_DISABLE_WARNINGS_END()
	}

	/* Compute reproj error */
	COMPV_CHECK_CODE_RETURN(CompVCalibUtils::proj2DError(context, context.reproj_error));	
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Reproj error before levmarq: %f", context.reproj_error);
	if (context.levenberg_marquardt) {
		CompVMatPtr levmarq_K, levmarq_d;
		CompVMatPtrVector levmarq_R, levmarq_t;
		compv_float64_t levmarq_error;
		COMPV_CHECK_CODE_RETURN(levmarq(context, &levmarq_K, &levmarq_d, levmarq_R, levmarq_t));
		COMPV_CHECK_CODE_RETURN(CompVCalibUtils::proj2DError(context.planes, levmarq_K, levmarq_d, levmarq_R, levmarq_t, levmarq_error));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Reproj error after levmarq: %f", levmarq_error);
		if (levmarq_error > context.reproj_error) {
			// TODO(dmi): because of skew (exclude in proj?)??
			COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Reproj error after levmarq is higher (%f > %f)", levmarq_error, context.reproj_error);
		}
		else {
			// Update result with refined parameters
			context.reproj_error = levmarq_error;
			context.K = levmarq_K; // Camera matrix (fx, fy, cx, cy, skew)
			context.d = levmarq_d; // Distorsion coefficients (k1, k2, p1, p2)
			CompVMatPtrVector::const_iterator it_R = levmarq_R.begin(), it_t = levmarq_t.begin();
			CompVCalibCameraPlanVector::iterator it_planes = context.planes.begin();
			for (; it_planes < context.planes.end(); ++it_planes, ++it_R, ++it_t) {
				it_planes->R = *it_R; // Rotation matrices (3x3)
				it_planes->t = *it_t; // Translation vectors (tx, ty, tz)
			}
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Subdivide the lines in two groups: almost vt and almost hz
// This function is thread-safe
COMPV_ERROR_CODE CompVCalibCamera::subdivision(const size_t image_width, const size_t image_height, const CompVCabLines& lines, CompVCabLines& lines_hz, CompVCabLines& lines_vt)
{
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < m_nPatternLinesTotal, COMPV_ERROR_CODE_E_INVALID_STATE, "No enought points");
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < lines.lines_hough.size(), COMPV_ERROR_CODE_E_INVALID_STATE, "Must have same number of cartesian and polar lines");

	CompVHoughLineVector::const_iterator it_hough;
	CompVLineFloat32Vector::const_iterator it_cartesian;
	compv_float32_t angle;
	for (it_hough = lines.lines_hough.begin(), it_cartesian = lines.lines_cartesian.begin(); it_cartesian < lines.lines_cartesian.end(); ++it_cartesian, ++it_hough) {
		angle = std::atan2((it_cartesian->b.y - it_cartesian->a.y), (it_cartesian->b.x - it_cartesian->a.x)); // inclinaison angle, within [-pi/2, pi/2]
		if (std::abs(std::sin(angle)) > 0.7071067691f) { // sin(angle) within [-1, 1] > sin(45)
			lines_vt.lines_hough.push_back(*it_hough);
			lines_vt.lines_cartesian.push_back(*it_cartesian);
		}
		else {
			lines_hz.lines_hough.push_back(*it_hough);
			lines_hz.lines_cartesian.push_back(*it_cartesian);
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Grouping using cartesian distances
// "lines_hough_parallel" must contains lines almost parallel so that the distances are meaningful
// This function is thread-safe
COMPV_ERROR_CODE CompVCalibCamera::grouping(const size_t image_width, const size_t image_height, const CompVCabLines& lines_parallel, const bool vt, const size_t max_strength, const size_t max_lines, CompVCabLineFloat32Vector& lines_parallel_grouped)
{
	lines_parallel_grouped.clear();

	// Group using distance to the origine point (x0,y0) = (0, 0)
	// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
	CompVCabLineGroupVector groups;
	CompVHoughLineVector::const_iterator it_hough;

	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	const compv_float32_t rsmall = (static_cast<compv_float32_t>(max_strength >> 1) / static_cast<compv_float32_t>(max_lines)); // 0.5f because black squares are alterning
	
	compv_float32_t distance, c, d, distance_diff;
	compv_float32_t i_x, i_y;

	// Grouping
	it_hough = lines_parallel.lines_hough.begin();
	for (CompVLineFloat32Vector::const_iterator i = lines_parallel.lines_cartesian.begin(); i < lines_parallel.lines_cartesian.end(); ++i, ++it_hough) {
		// Compute the distance from the origine (x0, y0) to the line
		// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
		// x1 = a.x, y1 = a.y
		// x2 = b.x, y2 = b.y
		c = (i->b.y - i->a.y);
		d = (i->b.x - i->a.x);
		distance = std::abs(((i->b.x * i->a.y) - (i->b.y * i->a.x)) / std::sqrt((c * c) + (d * d)));

		// Get the group associated to the curent line
		CompVCabLineGroup* group = nullptr;
		for (CompVCabLineGroupVector::iterator g = groups.begin(); g < groups.end(); ++g) {
			bool same_group = segment_get_intersection(*g->pivot_cartesian, *i, &i_x, &i_y)
				&& ((i_x >= 0.f && i_y >= 0.f && i_x < image_widthF && i_y < image_heightF));
			if (vt) { // Because of the perspective
				distance_diff = std::abs(g->pivot_distance - distance);
				same_group |= (distance_diff < (rsmall) * 1.f);
			}
			if (same_group) {
				group = &(*g);
				break;
			}
		}
		if (!group) {
			CompVCabLineGroup g;
			g.pivot_cartesian = &(*i);
			g.pivot_hough = &(*it_hough);
			g.pivot_distance = distance;
			groups.push_back(g);
			group = &groups[groups.size() - 1];
		}
		group->lines.lines_cartesian.push_back(*i);
		group->lines.lines_hough.push_back(*it_hough);
	}

	lines_parallel_grouped.reserve(groups.size());
	CompVHoughLineVector::const_iterator i;
	CompVLineFloat32Vector::const_iterator j;
	for (CompVCabLineGroupVector::const_iterator g = groups.begin(); g < groups.end(); ++g) {
		if (g->lines.lines_hough.size() > 1) {
			size_t strength_sum = 0;
			for (i = g->lines.lines_hough.begin(); i < g->lines.lines_hough.end(); ++i) {
				strength_sum += i->strength;
			}
			CompVCabLineFloat32 line_cab_cartesian;
			COMPV_CHECK_CODE_RETURN(lineBestFit(g->lines.lines_cartesian, g->lines.lines_hough, line_cab_cartesian.line));
			line_cab_cartesian.strength = strength_sum;
			line_cab_cartesian.vt = vt;
			lines_parallel_grouped.push_back(line_cab_cartesian);
		}
		else {
			CompVCabLineFloat32 line_cab_cartesian;
			line_cab_cartesian.line = *g->lines.lines_cartesian.begin();
			line_cab_cartesian.strength = g->lines.lines_hough.begin()->strength;
			line_cab_cartesian.vt = vt;
			lines_parallel_grouped.push_back(line_cab_cartesian);
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

// This function is thread-safe
COMPV_ERROR_CODE CompVCalibCamera::lineBestFit(const CompVLineFloat32Vector& points_cartesian, const CompVHoughLineVector& points_hough, CompVLineFloat32& line)
{
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() < 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Need at least #2 points");
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() != points_hough.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Must have same number of points for polar and cartesian points");

#if 0
	// This function doesn't work for 'almost-perfect' vertical and horizontal lines. Use Total Least Squares
	// as done in compv_math_stats_fit.cxx
	COMPV_DEBUG_INFO_CODE_TODO("Use Total Least Squares. See CompVMathStatsFitGeneric::buildModelParamsLine");
#endif

	// Implementing "Least Square Method" (https://www.varsitytutors.com/hotmath/hotmath_help/topics/line-of-best-fit)
	// while ignoring the x-component for the simple reason that they are always constant
	// when using CompV's KHT and SHT implementations (a.x = 0 and b.x = image_width)

	CompVLineFloat32Vector::const_iterator i;
	CompVHoughLineVector::const_iterator j;
	std::vector<compv_float32_t>::const_iterator k;
#if 0
	bool have_perfect_vt_lines = false;
#endif

	// Compute the sum of the strengths and the global scaling factor
	compv_float32_t sum_strengths = 0.f;
	for (j = points_hough.begin(); j < points_hough.end(); ++j) {
		sum_strengths += j->strength;
#if 0
		if (j->theta == 0.f) {
			have_perfect_vt_lines = true;
		}
#endif
	}
	const compv_float32_t scale_strengths = (1.f / (sum_strengths * 2.f)); // times #2 because we have #2 points (a & b) for each step.

	// Compute the strengths (for each point)
	std::vector<compv_float32_t> strengths(points_hough.size());
	size_t index;
	for (j = points_hough.begin(), index = 0; j < points_hough.end(); ++j, ++index) {
		strengths[index] = (j->strength * scale_strengths);
	}

	// Compute mean(y)
	compv_float32_t mean_y = 0.f;
	compv_float32_t scale_strength;
	for (i = points_cartesian.begin(), k = strengths.begin(); i < points_cartesian.end(); ++i, ++k) {
		mean_y += (i->a.y + i->b.y) * (*k);
	}

	// Compute t0
	compv_float32_t t0 = 0.f;
	for (i = points_cartesian.begin(), j = points_hough.begin(), k = strengths.begin(); i < points_cartesian.end(); ++i, ++j, ++k) {
		scale_strength = (j->strength * scale_strengths);
		t0 += (((i->a.y - mean_y)) + ((i->b.y - mean_y))) * (*k);
	}

	// set result
	line = points_cartesian[0]; // set x, y, z
	line.a.y += (line.a.y * t0);
	line.b.y += (line.b.y * t0);

#if 0 // image 50 fails bigly
	// perfect vt lines have "a.x == b.x == rho" while all other lines have "a.x == 0, b.x == width".
	// when there is no perfect vt lines then mean_ax == 0 and mean_bx == width
	if (have_perfect_vt_lines) {
		compv_float32_t mean_ax = 0.f, mean_bx = 0.f;
		for (i = points_cartesian.begin(), k = strengths.begin(); i < points_cartesian.end(); ++i, ++k) {
			mean_ax += i->a.x * (*k);
			mean_bx += i->b.x * (*k);
		}
		// mul mean by 2.f to get ride of the div 2.f in 'scale_strengths'
		line.a.x = (mean_ax * 2.f);
		line.b.x = (mean_bx * 2.f);
	}
#endif
	
	return COMPV_ERROR_CODE_S_OK;
}

/* 
* Build pattern's corners
*/
COMPV_ERROR_CODE CompVCalibCamera::buildPatternCorners(const CompVCalibContex& context)
{
	if (!m_ptrPatternCorners) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Building pattern corners");
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&m_ptrPatternCorners, 3, m_nPatternCornersTotal));
		compv_float64_t* corX0 = m_ptrPatternCorners->ptr<compv_float64_t>(0);
		compv_float64_t* corY0 = m_ptrPatternCorners->ptr<compv_float64_t>(1);
		COMPV_CHECK_CODE_RETURN(m_ptrPatternCorners->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
		const compv_float64_t patternBlockSizePixel = static_cast<compv_float64_t>(m_nPatternBlockSizePixel);
		compv_float64_t x0, y0;
		size_t i, j, k;
		const size_t nPatternCornersNumRow0 = m_nPatternCornersNumRow;
		const size_t nPatternCornersNumCol0 = m_nPatternCornersNumCol;
		for (j = 0, y0 = 0.0, k = 0; j < nPatternCornersNumRow0; ++j, y0 += patternBlockSizePixel) {
			for (i = 0, x0 = 0.0; i < nPatternCornersNumCol0; ++i, x0 += patternBlockSizePixel, ++k) {
				corX0[k] = x0;
				corY0[k] = y0;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

/*
* [2] 3.2.1 Homography estimation with the Direct Linear Transformation(DLT)
*/
COMPV_ERROR_CODE CompVCalibCamera::homography(const CompVCalibCameraPlan& plan, CompVHomographyResult& result_homography, CompVMatPtrPtr homographyMat)
{
	const CompVPointFloat32Vector& intersections = plan.intersections;
	COMPV_CHECK_EXP_RETURN(intersections.size() != m_nPatternCornersTotal, COMPV_ERROR_CODE_E_INVALID_CALL, "Invalid number of corners");

	// Convert the intersections from float32 to float64 for homagraphy
	CompVMatPtr query;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&query, 3, m_nPatternCornersTotal));
	compv_float64_t* queryX = query->ptr<compv_float64_t>(0);
	compv_float64_t* queryY = query->ptr<compv_float64_t>(1);
	COMPV_CHECK_CODE_RETURN(query->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
	size_t index = 0;
	for (CompVPointFloat32Vector::const_iterator i = intersections.begin(); i < intersections.end(); ++i, ++index) {
		queryX[index] = static_cast<compv_float64_t>(i->x);
		queryY[index] = static_cast<compv_float64_t>(i->y);
	}

	// Find homography
	COMPV_CHECK_CODE_RETURN(CompVHomography<compv_float64_t>::find(plan.pattern, query, homographyMat, &result_homography));

	return COMPV_ERROR_CODE_S_OK;
}

// Parameters refinement:
//	- [2] 3.6 Step 5: Refining all parameters
//	- [2] 3.6.3 Non-linear optimization
COMPV_ERROR_CODE CompVCalibCamera::levmarq(const CompVCalibContex& context, CompVMatPtrPtr K, CompVMatPtrPtr d, CompVMatPtrVector& R, CompVMatPtrVector& t)
{
	const size_t nplanes = context.planes.size();
	const bool have_skew = context.compute_skew;
	const bool have_tangential_dist = context.compute_tangential_dist;

	// Measurements
	const size_t ncorners = context.planes.begin()->intersections.size();
	const size_t xyCount = (nplanes * ncorners) << 1; // (x,y)
	CompVMatPtr corners;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&corners, 2, xyCount));
	compv_float64_t* cornersPtr = corners->ptr<compv_float64_t>(0);
	size_t index = 0;
	for (CompVCalibCameraPlanVector::const_iterator it_plans = context.planes.begin(); it_plans < context.planes.end(); ++it_plans) {
		for (CompVPointFloat32Vector::const_iterator it_intersections = it_plans->intersections.begin(); it_intersections < it_plans->intersections.end(); ++it_intersections) {
			cornersPtr[index++] = static_cast<compv_float64_t>(it_intersections->x);
			cornersPtr[index++] = static_cast<compv_float64_t>(it_intersections->y);
		}
	}

	// Parameters
	CompVMatPtr parametersMat;
	const size_t parametersCount =
		(have_skew ? 5 : 4) // fx, fy, cx, cy[, skew]
		+ (have_tangential_dist ? 4 : 2) // k1, k2[, p1, p2]
		+ ((3 + 3) * nplanes) // R, t
		;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObj<compv_float64_t>(&parametersMat, 1, parametersCount, 1));
	compv_float64_t* parametersPtr = parametersMat->ptr<compv_float64_t>();
	index = 0;
	// Camera matrix (K)
	parametersPtr[index++] = *context.K->ptr<const compv_float64_t>(0, 0); // fx
	parametersPtr[index++] = *context.K->ptr<const compv_float64_t>(1, 1); // fy
	parametersPtr[index++] = *context.K->ptr<const compv_float64_t>(0, 2); //cx
	parametersPtr[index++] = *context.K->ptr<const compv_float64_t>(1, 2); // cy
	if (have_skew) {
		parametersPtr[index++] = *context.K->ptr<const compv_float64_t>(0, 1); // skew
	}
	// Distorsions (k1, k2, p1, p2)
	parametersPtr[index++] = *context.d->ptr<const compv_float64_t>(0, 0); // k1
	parametersPtr[index++] = *context.d->ptr<const compv_float64_t>(1, 0); // k2
	if (have_tangential_dist) {
		parametersPtr[index++] = *context.d->ptr<const compv_float64_t>(2, 0); // p1
		parametersPtr[index++] = *context.d->ptr<const compv_float64_t>(3, 0); // p2
	}
	// Rotation (R) and translation (t) vectors
	compv_float64x3_t rVector;
	for (CompVCalibCameraPlanVector::const_iterator it_plans = context.planes.begin(); it_plans < context.planes.end(); ++it_plans, index += 6) {
		// [2] 3.6.2 Parameterizing the extrinsic rotation matrices Ri
		COMPV_CHECK_CODE_RETURN(CompVMathTrig::rodriguesMatrixToVector(it_plans->R, rVector));
		parametersPtr[index] = rVector[0]; // r0
		parametersPtr[index + 1] = rVector[1]; // r1
		parametersPtr[index + 2] = rVector[2]; // r2
		// Translation vector
		parametersPtr[index + 3] = *it_plans->t->ptr<const compv_float64_t>(0, 0); // tx
		parametersPtr[index + 4] = *it_plans->t->ptr<const compv_float64_t>(0, 1); // ty
		parametersPtr[index + 5] = *it_plans->t->ptr<const compv_float64_t>(0, 2); // tz
	}

	/* https://en.wikipedia.org/wiki/Levenberg%E2%80%93Marquardt_algorithm */
	// LM data
	lm_status_struct status;
	lm_control_struct control = lm_control_double;
	control.verbosity = 0;
	control.patience = 2000;
	levmarq_data data(context.planes, cornersPtr, have_tangential_dist, have_skew);
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&data.K, 3, 3));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&data.d, have_tangential_dist ? 4 : 2, 1));
	COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<compv_float64_t>(&data.t, 1, 3));
	*data.K->ptr<compv_float64_t>(0, 1) = 0.0;
	*data.K->ptr<compv_float64_t>(1, 0) = 0.0;
	*data.K->ptr<compv_float64_t>(2, 0) = 0.0;
	*data.K->ptr<compv_float64_t>(2, 1) = 0.0;
	*data.K->ptr<compv_float64_t>(2, 2) = 1.0;
	data.K_fx = data.K->ptr<compv_float64_t>(0, 0); // fx
	data.K_fy = data.K->ptr<compv_float64_t>(1, 1); // fy
	data.K_cx = data.K->ptr<compv_float64_t>(0, 2); // cx
	data.K_cy = data.K->ptr<compv_float64_t>(1, 2); // cy
	data.K_skew = data.K->ptr<compv_float64_t>(0, 1); // skew
	data.d_k1 = data.d->ptr<compv_float64_t>(0, 0); // k1
	data.d_k2 = data.d->ptr<compv_float64_t>(1, 0); // k2
	if (have_tangential_dist) {
		data.d_p1 = data.d->ptr<compv_float64_t>(2, 0); // p1
		data.d_p2 = data.d->ptr<compv_float64_t>(3, 0); // p2
	}
	data.t_x = data.t->ptr<compv_float64_t>(0, 0);
	data.t_y = data.t_x + 1;
	data.t_z = data.t_y + 1;
	// LM process
	lmmin(static_cast<int>(parametersCount), parametersPtr, static_cast<int>(xyCount), (const void*)&data, levmarq_eval,
		&control, &status);

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "LM status after %d function evaluations:  %s",
		status.nfev, lm_infmsg[status.outcome]);

	/* Copy results */
	index = 0;

	// Camera matrix
	*data.K_fx = parametersPtr[index++]; // fx
	*data.K_fy = parametersPtr[index++]; // fy
	*data.K_cx = parametersPtr[index++]; // cx
	*data.K_cy = parametersPtr[index++]; // cy
	*data.K_skew = have_skew ? parametersPtr[index++] : 0.0; // skew
	*K = data.K;
	
	// Distorsions (radial and tangential)
	*data.d_k1 = parametersPtr[index++]; // k1
	*data.d_k2 = parametersPtr[index++]; // k2
	if (have_tangential_dist) {
		*data.d_p1 = parametersPtr[index++]; // p1
		*data.d_p2 = parametersPtr[index++]; // p2
	}
	*d = data.d;

	// Rotations and translations
	for (CompVCalibCameraPlanVector::const_iterator i = context.planes.begin(); i < context.planes.end(); ++i, index += 6) {
		// Rotation
		rVector[0] = parametersPtr[index]; // r0
		rVector[1] = parametersPtr[index + 1]; // r1
		rVector[2] = parametersPtr[index + 2]; // r2
		CompVMatPtr rMatrix; //!\\ must be local (for-scope) variable to avoid reusing same object reference
		COMPV_CHECK_CODE_RETURN(CompVMathTrig::rodriguesVectorToMatrix(rVector, &rMatrix));
		R.push_back(rMatrix);
		// Translation
		CompVMatPtr tVector; //!\\ must be local (for-scope) variable to avoid reusing same object reference
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&tVector, 1, 3));
		compv_float64_t* tVectorPtr = tVector->ptr<compv_float64_t>();
		tVectorPtr[0] = parametersPtr[index + 3]; // tx
		tVectorPtr[1] = parametersPtr[index + 4]; // ty
		tVectorPtr[2] = parametersPtr[index + 5]; // tz
		t.push_back(tVector);
	}

	return COMPV_ERROR_CODE_S_OK;
}

/*
* LevMarq evaluation function (callback)
*/
static void levmarq_eval(const compv_float64_t *par, int m_dat, const void *data, compv_float64_t *fvec, int *userbreak)
{
	levmarq_data *Data = const_cast<levmarq_data*>(reinterpret_cast<const levmarq_data*>(data));
	const compv_float64_t* cornersPtr = Data->cornersPtr;

	size_t nplanes = Data->planes.size();
	size_t index = 0;

	/*  Parameters */
	//	5 // fx, fy, cx, cy, skew
	//	+ 4 // k1, k2, [p1, p2]
	//	+ ((3 + 3) * nplanes) // Rt
	// Camera matrix
	*Data->K_fx = par[index++]; // fx
	*Data->K_fy = par[index++]; // fy
	*Data->K_cx = par[index++]; // cx
	*Data->K_cy = par[index++]; // cy
	*Data->K_skew = Data->have_skew ? par[index++] : 0.0; // skew
	// Distorsions
	*Data->d_k1 = par[index++]; // k1
	*Data->d_k2 = par[index++]; // k2
	if (Data->have_tangential_dist) {
		*Data->d_p1 = par[index++]; // p1
		*Data->d_p2 = par[index++]; // p2
	}

	CompVMatPtr& K = Data->K;
	CompVMatPtr& d = Data->d;
	CompVMatPtr& t = Data->t;
	CompVMatPtr R, reprojected;
	size_t i, ncorners;
	compv_float64x3_t r;
	const compv_float64_t *reprojectedX, *reprojectedY;
	CompVCalibCameraPlanVector::const_iterator it_planes;
	for (i = 0, it_planes = Data->planes.begin(); i < nplanes; ++i, ++it_planes) {
		// Rotation vector
		r[0] = par[index++]; // r0
		r[1] = par[index++]; // r1
		r[2] = par[index++]; // r2
		// Translation
		*Data->t_x = par[index++]; // tx
		*Data->t_y = par[index++]; // ty
		*Data->t_z = par[index++]; // tz
		// Convert rotation vector to 3x3 matrix (Rodrigues)
		COMPV_CHECK_CODE_ASSERT(CompVMathTrig::rodriguesVectorToMatrix(r, &R));
		// Compute projection
		COMPV_CHECK_CODE_ASSERT(CompVCalibUtils::proj2D(it_planes->pattern, K, d, R, t, &reprojected));
		COMPV_ASSERT(it_planes->pattern->cols() == reprojected->cols());
		// Compute the residual
		ncorners = it_planes->pattern->cols();
		reprojectedX = reprojected->ptr<const compv_float64_t>(0);
		reprojectedY = reprojected->ptr<const compv_float64_t>(1);
		for (size_t j = 0, z = (i * ncorners * 2); j < ncorners; ++j, z += 2) {
			fvec[z + 0] = (cornersPtr[z + 0] - reprojectedX[j]);
			fvec[z + 1] = (cornersPtr[z + 1] - reprojectedY[j]);
		}
	}
}

/*
* Check whether the segments intersect.
*/
static bool segment_get_intersection(const CompVLineFloat32& line0, const CompVLineFloat32& line1, compv_float32_t *i_x, compv_float32_t *i_y COMPV_DEFAULT(nullptr))
{
	const compv_float32_t a1 = line0.b.y - line0.a.y;
	const compv_float32_t b1 = line0.a.x - line0.b.x;

	const compv_float32_t a2 = line1.b.y - line1.a.y;
	const compv_float32_t b2 = line1.a.x - line1.b.x;

	// Compute determinent
	const compv_float32_t det = a1 * b2 - a2 * b1;
	if (det == 0) {
		return false; // segments are parallel (no intersection)
	}

	const compv_float32_t scale = (1.f / det);
	const compv_float32_t c1 = a1 * line0.a.x + b1 * line0.a.y;
	const compv_float32_t c2 = a2 * line1.a.x + b2 * line1.a.y;
	*i_x = (b2 * c1 - b1 * c2) * scale;
	if (i_y) {
		*i_y = (a1 * c2 - a2 * c1) * scale;
	}

	return true;
}

COMPV_ERROR_CODE CompVCalibCamera::newObj(CompVCalibCameraPtrPtr calib, size_t nPatternRowsCount COMPV_DEFAULT(COMPV_CALIB_PATTERN_ROWS_COUNT), size_t nPatternColsCount COMPV_DEFAULT(COMPV_CALIB_PATTERN_COLS_COUNT))
{
	COMPV_CHECK_EXP_RETURN(!calib || !nPatternRowsCount || !nPatternColsCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVCalibCameraPtr calib_ = new CompVCalibCamera(nPatternRowsCount, nPatternColsCount);
	COMPV_CHECK_EXP_RETURN(!calib_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	/* Hough transform */
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&calib_->m_ptrHough, HOUGH_ID,
		(HOUGH_ID == COMPV_HOUGHKHT_ID) ? HOUGH_RHO : 1.f,
		HOUGH_THETA,
		(HOUGH_ID == COMPV_HOUGHKHT_ID) ? HOUGH_KHT_THRESHOLD : HOUGH_SHT_THRESHOLD
	));
	COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setInt(COMPV_HOUGH_SET_INT_MAXLINES, static_cast<int>(calib_->m_nPatternLinesTotal * PATTERN_GROUP_MAXLINES)));
	if (HOUGH_ID == COMPV_HOUGHKHT_ID) {
		COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION, HOUGH_KHT_CLUSTER_MIN_DEVIATION));
		COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setInt(COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE, HOUGH_KHT_CLUSTER_MIN_SIZE));
		COMPV_CHECK_CODE_RETURN(calib_->m_ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH, HOUGH_KHT_KERNEL_MIN_HEIGTH));
	}

	/* Canny edge detector */
	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&calib_->m_ptrCanny, COMPV_CANNY_ID, CANNY_LOW, CANNY_HIGH, CANNY_KERNEL_SIZE));

	*calib = *calib_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
