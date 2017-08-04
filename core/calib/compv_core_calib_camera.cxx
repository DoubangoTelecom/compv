/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/calib/compv_core_calib_camera.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/math/compv_math_gauss.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_eigen.h"
#include "compv/base/math/lmfit-6.1/lmmin.h" /* http://apps.jcns.fz-juelich.de/doku/sc/lmfit */
#include "compv/base/time/compv_time.h"

#include <iterator> /* std::back_inserter */
#include <limits> /* std::numeric_limits::smallest */

#define PATTERN_ROW_CORNERS_NUM				10 // Number of corners per row
#define PATTERN_COL_CORNERS_NUM				8  // Number of corners per column
#define PATTERN_CORNERS_NUM					(PATTERN_ROW_CORNERS_NUM * PATTERN_COL_CORNERS_NUM) // Total number of corners
#define PATTERN_GROUP_MAXLINES				10 // Maximum number of lines per group (errors)
#define PATTERN_BLOCK_SIZE_PIXEL			40

#define CALIBRATION_MIN_IMAGES				20 // Minimum images before computing calibrartion (must be >= 3)

#define HOUGH_ID							COMPV_HOUGHSHT_ID
#define HOUGH_RHO							0.5f // "rho-delta" (half-pixel)
#define HOUGH_THETA							0.5f // "theta-delta" (half-radian)
#define HOUGH_SHT_THRESHOLD_FACT			0.3
#define HOUGH_SHT_THRESHOLD_MAX				120
#define HOUGH_SHT_THRESHOLD_MIN				30
#define HOUGH_SHT_THRESHOLD					10
#define HOUGH_KHT_THRESHOLD_FACT			1.0 // GS
#define HOUGH_KHT_THRESHOLD					1 // filter later when GS is known	
#define HOUGH_KHT_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGH_KHT_CLUSTER_MIN_SIZE			10
#define HOUGH_KHT_KERNEL_MIN_HEIGTH			0.002f // must be within [0, 1]

#define CANNY_LOW							1.33f
#define CANNY_HIGH							CANNY_LOW*2.f
#define CANNY_KERNEL_SIZE					3

#define COMPV_THIS_CLASSNAME	"CompVCalibCamera"

// Implementation based on "Photogrammetry I - 16b - DLT & Camera Calibration (2015)": https://www.youtube.com/watch?v=Ou9Uj75DJX0
// TODO(dmi): use "ceres-solver" http://ceres-solver.org/index.html

COMPV_NAMESPACE_BEGIN()

#define kSmallRhoFactVt				0.035f /* small = (rho * fact) */
#define kSmallRhoFactHz				0.035f /* small = (rho * fact) */
#define kCheckerboardBoxDistFact	0.357f /* distance * fact */

struct CompVCabLineGroup {
	const CompVLineFloat32* pivot_cartesian;
	const CompVHoughLine* pivot_hough;
	compv_float32_t pivot_distance; // distance(pivot, origin)
	CompVCabLines lines;
};
typedef std::vector<CompVCabLineGroup, CompVAllocatorNoDefaultConstruct<CompVCabLineGroup> > CompVCabLineGroupVector;

static COMPV_ERROR_CODE proj(const CompVMatPtr& inPoints, const CompVMatPtr& K, const CompVMatPtr& k, const CompVMatPtr& p, const CompVMatPtr& R, const CompVMatPtr&t, CompVMatPtrPtr outPoints);
static COMPV_ERROR_CODE projError(const CompVMatPtr& inPoints, const CompVMatPtr& outPoints, const CompVMatPtr& K, const CompVMatPtr& k, const CompVMatPtr& p, const CompVMatPtr& R, const CompVMatPtr&t, compv_float64_t& error);
static COMPV_ERROR_CODE projError(const CompVCalibCameraResult& result_calib, compv_float64_t& error);

template<typename T = compv_float32_t>
static bool CompVMathLineSegmentGetIntersection(T p0_x, T p0_y, T p1_x, T p1_y,
	compv_float32_t p2_x, compv_float32_t p2_y, T p3_x, T p3_y, T *i_x, T *i_y = nullptr)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Re-write code from StackOverflow");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	const compv_float32_t a1 = p1_y - p0_y;
	const compv_float32_t b1 = p0_x - p1_x;

	const compv_float32_t a2 = p3_y - p2_y;
	const compv_float32_t b2 = p2_x - p3_x;

	const compv_float32_t det = a1 * b2 - a2 * b1;
	if (det == 0) {
		// lines are parallel
		return false;
	}

	const compv_float32_t scale = (1.f / det);
	const compv_float32_t c1 = a1 * p0_x + b1 * p0_y;
	const compv_float32_t c2 = a2 * p2_x + b2 * p2_y;
	*i_x = (b2 * c1 - b1 * c2) * scale;
	if (i_y) {
		*i_y = (a1 * c2 - a2 * c1) * scale;
	}

	return true;
}

CompVCalibCamera::CompVCalibCamera()
	: m_nPatternCornersNumRow(PATTERN_ROW_CORNERS_NUM)
	, m_nPatternCornersNumCol(PATTERN_COL_CORNERS_NUM)
	, m_nPatternLinesHz(PATTERN_ROW_CORNERS_NUM)
	, m_nPatternLinesVt(PATTERN_COL_CORNERS_NUM)
	, m_nPatternBlockSizePixel(PATTERN_BLOCK_SIZE_PIXEL)
	, m_nMinPanesCountBeforeCalib(CALIBRATION_MIN_IMAGES)
{
	m_nPatternCornersTotal = m_nPatternCornersNumRow * m_nPatternCornersNumCol;
	m_nPatternLinesTotal = (m_nPatternCornersNumRow + m_nPatternCornersNumCol);
}

CompVCalibCamera::~CompVCalibCamera()
{

}

COMPV_ERROR_CODE CompVCalibCamera::process(const CompVMatPtr& image, CompVCalibCameraResult& result_calib)
{
	COMPV_CHECK_EXP_RETURN(!image || image->elmtInBytes() != sizeof(uint8_t) || image->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");

	// Reset the previous result
	result_calib.reset();
	CompVCalibCameraPlan& plan_curr = result_calib.plane_curr;

	const size_t image_width = image->cols();
	const size_t image_height = image->rows();
	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	bool intersect;
	compv_float32_t intersect_x, intersect_y;
	static const compv_float32_t intersect_z = 1.f;

	static const compv_float32_t x_axis_right = 1e15f;
	static const compv_float32_t x_axis_left = -1e15f;
	
	/* Canny edge detection */
	COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(image, &result_calib.edges));

	/* Hough lines */
	// For SHT, set the threshold before processing. But for KHT, we need the global scale (GS) which is defined only *after* processing
	if (m_ptrHough->id() == COMPV_HOUGHSHT_ID) {
		COMPV_CHECK_CODE_RETURN(m_ptrHough->setInt(COMPV_HOUGH_SET_INT_THRESHOLD, HOUGH_SHT_THRESHOLD_MIN));
	}
	// Process
	COMPV_CHECK_CODE_RETURN(m_ptrHough->process(result_calib.edges, result_calib.lines_raw.lines_hough));

	/* Return if no enough points */
	if (result_calib.lines_raw.lines_hough.size() < m_nPatternLinesTotal) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough lines after hough transform: %zu", result_calib.lines_raw.lines_hough.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Remove weak lines */
	if (result_calib.lines_raw.lines_hough.size() > m_nPatternLinesTotal) {
		if (m_ptrHough->id() == COMPV_HOUGHKHT_ID) {
			// Remove weak lines using global scale (GS)
			compv_float64_t gs;
			COMPV_CHECK_CODE_RETURN(m_ptrHough->getFloat64(COMPV_HOUGHKHT_GET_FLT64_GS, &gs));
			const size_t min_strength = static_cast<size_t>(gs * HOUGH_KHT_THRESHOLD_FACT);
			auto fncShortLines = std::remove_if(result_calib.lines_raw.lines_hough.begin(), result_calib.lines_raw.lines_hough.end(), [&](const CompVHoughLine& line) {
				return line.strength < min_strength;
			});
			result_calib.lines_raw.lines_hough.erase(fncShortLines, result_calib.lines_raw.lines_hough.end());
		}
		else {
			// Remove weak lines using global max_strength
			const size_t min_strength = static_cast<size_t>(result_calib.lines_raw.lines_hough.begin()->strength * HOUGH_SHT_THRESHOLD_FACT);
			auto fncShortLines = std::remove_if(result_calib.lines_raw.lines_hough.begin(), result_calib.lines_raw.lines_hough.end(), [&](const CompVHoughLine& line) {
				return line.strength < min_strength;
			});
			result_calib.lines_raw.lines_hough.erase(fncShortLines, result_calib.lines_raw.lines_hough.end());
		}
	}

	/* Return if no enough points */
	if (result_calib.lines_raw.lines_hough.size() < m_nPatternLinesTotal) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough lines after removing weak lines: %zu", result_calib.lines_raw.lines_hough.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Convert from polar to cartesian coordinates */
	COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(image_width, image_height, result_calib.lines_raw.lines_hough, result_calib.lines_raw.lines_cartesian));

	const size_t nPatternLinesHzVtMax = std::max(m_nPatternLinesHz, m_nPatternLinesVt);
	const size_t nPatternLinesHzVtMin = std::min(m_nPatternLinesHz, m_nPatternLinesVt);

	/* Lines subdivision */
	CompVCabLines lines_hz, lines_vt;
	COMPV_CHECK_CODE_RETURN(subdivision(image_width, image_height, result_calib.lines_raw, lines_hz, lines_vt));
	if (lines_hz.lines_cartesian.size() < nPatternLinesHzVtMin) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [hz] lines after subdivision: %zu", lines_hz.lines_cartesian.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}
	if (lines_vt.lines_cartesian.size() < nPatternLinesHzVtMin) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [vt] lines after subdivision: %zu", lines_vt.lines_cartesian.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}
	if ((lines_hz.lines_cartesian.size() + lines_vt.lines_cartesian.size()) < m_nPatternLinesTotal) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [vt+hz] lines after subdivision: %zu+%zu", lines_hz.lines_cartesian.size(), lines_vt.lines_cartesian.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	// Hz-grouping
	const size_t max_strength = result_calib.lines_raw.lines_hough.begin()->strength; // lines_hough is sorted (by hought->process)
	CompVCabLineFloat32Vector lines_cab_hz_grouped, lines_cab_vt_grouped;
	if (lines_hz.lines_cartesian.size() > nPatternLinesHzVtMin) {
		COMPV_CHECK_CODE_RETURN(grouping(image_width, image_height, lines_hz, false, max_strength, lines_cab_hz_grouped));
		if (lines_cab_hz_grouped.size() < nPatternLinesHzVtMin) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [hz] grouping we got less lines than what is requires, not a good news at all");
			result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	if (lines_cab_hz_grouped.size() != m_nPatternLinesHz && lines_cab_hz_grouped.size() != m_nPatternLinesVt) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [hz] grouping we don't have exactly %zu/%zu lines but more (%zu). Maybe our grouping function missed some orphans", m_nPatternLinesHz, m_nPatternLinesVt, lines_cab_hz_grouped.size());
	}
	
	// Vt-grouping
	if (lines_vt.lines_cartesian.size() > nPatternLinesHzVtMin) {
		COMPV_CHECK_CODE_RETURN(grouping(image_width, image_height, lines_vt, true, max_strength, lines_cab_vt_grouped));
		if (lines_cab_vt_grouped.size() < nPatternLinesHzVtMin) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [vt] grouping we got less lines than what is requires. Not a good news at all");
			result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	if (lines_cab_vt_grouped.size() != m_nPatternLinesHz && lines_cab_vt_grouped.size() != m_nPatternLinesVt) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [vt] grouping we don't have exactly %zu lines but more (%zu). Maybe our grouping function missed some orphans", m_nPatternLinesVt, lines_cab_vt_grouped.size());
	}

	if ((lines_cab_vt_grouped.size() + lines_cab_hz_grouped.size()) < m_nPatternLinesTotal) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [vt+hz] lines after grouping: %zu+%zu", lines_cab_vt_grouped.size(), lines_cab_hz_grouped.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Pack all lines, sort and keep the best */
	if (lines_cab_hz_grouped.size() > nPatternLinesHzVtMax) {
		std::sort(lines_cab_hz_grouped.begin(), lines_cab_hz_grouped.end(), [](const CompVCabLineFloat32 &line1, const CompVCabLineFloat32 &line2) {
			return (line1.strength > line2.strength);
		});
		lines_cab_hz_grouped.resize(nPatternLinesHzVtMax);
	}
	if (lines_cab_vt_grouped.size() > nPatternLinesHzVtMax) {
		std::sort(lines_cab_vt_grouped.begin(), lines_cab_vt_grouped.end(), [](const CompVCabLineFloat32 &line1, const CompVCabLineFloat32 &line2) {
			return (line1.strength > line2.strength);
		});
		lines_cab_vt_grouped.resize(nPatternLinesHzVtMax);
	}
	if ((lines_cab_vt_grouped.size() + lines_cab_hz_grouped.size()) < m_nPatternLinesTotal) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [vt+hz] lines after grouping: %zu+%zu", lines_cab_vt_grouped.size(), lines_cab_hz_grouped.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}
	CompVLineFloat32Vector lines_hz_grouped, lines_vt_grouped;
	lines_cab_hz_grouped.insert(lines_cab_hz_grouped.end(), lines_cab_vt_grouped.begin(), lines_cab_vt_grouped.end()); // Pack [hz] and [vt] lines together
	std::sort(lines_cab_hz_grouped.begin(), lines_cab_hz_grouped.end(), [](const CompVCabLineFloat32 &line1, const CompVCabLineFloat32 &line2) {
		return (line1.strength > line2.strength);
	});
	lines_cab_hz_grouped.resize(m_nPatternLinesTotal); // keep best only
	lines_hz_grouped.reserve(nPatternLinesHzVtMax);
	lines_vt_grouped.reserve(nPatternLinesHzVtMax);
	for (CompVCabLineFloat32Vector::const_iterator i = lines_cab_hz_grouped.begin(); i < lines_cab_hz_grouped.end(); ++i) {
		if (i->vt) {
			lines_vt_grouped.push_back(i->line);
		}
		else {
			lines_hz_grouped.push_back(i->line);
		}
	}
	if (lines_hz_grouped.size() < nPatternLinesHzVtMin) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [hz] lines after packing: %zu", lines_hz_grouped.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}
	if (lines_vt_grouped.size() < nPatternLinesHzVtMin) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough [vt] lines after packing: %zu", lines_vt_grouped.size());
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* The pattern could be rectanglar (hz != vt) and it's time to decide what is really hz and vt */
	size_t vt_size = lines_vt_grouped.size();
	size_t hz_size = lines_hz_grouped.size();
	result_calib.rotated = ((m_nPatternCornersNumCol > m_nPatternCornersNumRow) && (lines_vt_grouped.size() < lines_hz_grouped.size()))
		|| ((m_nPatternCornersNumCol < m_nPatternCornersNumRow) && (lines_vt_grouped.size() > lines_hz_grouped.size()));
	if (result_calib.rotated) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Checkerboard probably rotated: close to 90deg position");
	}
	const size_t nPatternLinesHzExpected = result_calib.rotated ? m_nPatternLinesVt : m_nPatternLinesHz;
	const size_t nPatternLinesVtExpected = result_calib.rotated ? m_nPatternLinesHz : m_nPatternLinesVt;

	/* Make sure we have the right number of lines in each group or keep the bests */
	if (lines_hz_grouped.size() != nPatternLinesHzExpected) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No expected number of [hz] lines (%zu != %zu)", lines_hz_grouped.size(), nPatternLinesHzExpected);
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}
	if (lines_vt_grouped.size() != nPatternLinesVtExpected) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No expected number of [vt] lines (%zu != %zu)", lines_vt_grouped.size(), nPatternLinesVtExpected);
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_LINES;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Sorting the lines */
	// Sort top-bottom (hz lines)
	std::sort(lines_hz_grouped.begin(), lines_hz_grouped.end(), [](const CompVLineFloat32 &line1, const CompVLineFloat32 &line2) {
		return std::tie(line1.a.y, line1.b.y) < std::tie(line2.a.y, line2.b.y); // coorect only because x-components are constant when using CompV's hough implementations (otherwise use intersection with y-axis)
	});
	// Sort left-right (vt lines): sort the intersection with x-axis
	std::vector<std::pair<compv_float32_t, CompVLineFloat32> > intersections;
	intersections.reserve(lines_vt_grouped.size());
	for (CompVLineFloat32Vector::const_iterator i = lines_vt_grouped.begin(); i < lines_vt_grouped.end(); ++i) {
		intersect = CompVMathLineSegmentGetIntersection(i->a.x, i->a.y, i->b.x, i->b.y,
			x_axis_left, 0.f, x_axis_right, 0.f, &intersect_x);
		if (!intersect) {
			// Must never happen
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Vertical line must intersect with x-axis (%f, %f)-(%f, %f)", i->a.x, i->a.y, i->b.x, i->b.y);
			result_calib.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
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
	result_calib.lines_grouped.lines_cartesian.assign(lines_hz_grouped.begin(), lines_hz_grouped.end());
	result_calib.lines_grouped.lines_cartesian.insert(result_calib.lines_grouped.lines_cartesian.end(), lines_vt_grouped.begin(), lines_vt_grouped.end());

	/* Compute intersections */
	for (CompVLineFloat32Vector::const_iterator i = lines_hz_grouped.begin(); i < lines_hz_grouped.end(); ++i) {
		for (CompVLineFloat32Vector::const_iterator j = lines_vt_grouped.begin(); j < lines_vt_grouped.end(); ++j) {
			// Compute intersection
			intersect = CompVMathLineSegmentGetIntersection(i->a.x, i->a.y, i->b.x, i->b.y,
				j->a.x, j->a.y, j->b.x, j->b.y, &intersect_x, &intersect_y);
			if (!intersect) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No intersection between the lines. Stop processing");
				result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS;
				return COMPV_ERROR_CODE_S_OK;
			}
			if (intersect_x < 0.f || intersect_y < 0.f || intersect_x >= image_widthF || intersect_y >= image_heightF) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersection outside the image domain. Stop processing");
				result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS;
				return COMPV_ERROR_CODE_S_OK;
			}
			// Push intersection
			plan_curr.intersections.push_back(CompVPointFloat32(intersect_x, intersect_y, intersect_z)); // z is fake and contain distance to origine (to avoid computing distance several times)
		}

		// Computing homography on garbage is very sloow and to make sure this is a checkerboard, we check
		// that the intersections with the (x/y)-axis are almost equidistant and going forward (increasing)
		// !!The boxes in the checkerboard MUST BE SQUARE!!
		if (i != lines_hz_grouped.begin()) { // not first line
			compv_float32_t dist, dist_err, dist_approx_x, dist_err_max_x, dist_approx_y, dist_err_max_y; // not same distortion across x and y -> use different distance estimation
			CompVPointFloat32Vector::const_iterator k = plan_curr.intersections.end() - (nPatternLinesVtExpected << 1); // #2 last rows
			CompVPointFloat32Vector::const_iterator m = (k + nPatternLinesVtExpected); // move to next line
			for (size_t index = 0; index < nPatternLinesVtExpected; ++k, ++m, ++index) {
				/* x-axis */
				if (index == 1) {
					// compute default approx distance (x-axis)
					dist_approx_x = (k->x - (k - 1)->x);
					dist_err_max_x = (dist_approx_x * kCheckerboardBoxDistFact) + 1.f;
				}
				if (index) { // not first index
					// Previous x-intersection must be < current x-intersection (increasing)
					if ((k->x <= (k - 1)->x) || (m->x <= (m - 1)->x)) {
						COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not going forward (x-axis). Stop processing");
						result_calib.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
						return COMPV_ERROR_CODE_S_OK;
					}
					// Check if equidistant (x-axis)
					if (index > 1) {
						dist = (m->x - (m - 1)->x);
						dist_err = std::abs(dist - dist_approx_x);
						if (dist_err > dist_err_max_x) {
							COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not equidistant (xm-axis). Stop processing");
							result_calib.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
							return COMPV_ERROR_CODE_S_OK;
						}
						dist = (k->x - (k - 1)->x);
						dist_err = std::abs(dist - dist_approx_x);
						if (dist_err > dist_err_max_x) {
							COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not equidistant (xk-axis). Stop processing");
							result_calib.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
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
					COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not going forward (y-axis). Stop processing");
					result_calib.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
					return COMPV_ERROR_CODE_S_OK;
				}
				// Check if equidistant (y-axis)
				if (index > 0) {
					// y-axis
					dist_err = std::abs(dist - dist_approx_y);
					if (dist_err > dist_err_max_y) {
						COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersections not equidistant (y-axis). Stop processing");
						result_calib.code = COMPV_CALIB_CAMERA_RESULT_INCOHERENT_INTERSECTIONS;
						return COMPV_ERROR_CODE_S_OK;
					}
					// Refine dist-approx
					dist_approx_y = dist;
					dist_err_max_y = (dist_approx_y * kCheckerboardBoxDistFact) + 1.f;
				}
			}
		}
	}

	COMPV_DEBUG_INFO_CODE_TODO("Do not waste time computing homography and calibration if the image is almost same plan than previous one. To check same plane: compare SAD(intersections(n, (n-1)))");

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove FIXME_once");
	static bool FIXME_once = false;

	if (FIXME_once) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "FIXME_once is true");
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Build patterns */
	COMPV_CHECK_CODE_RETURN(buildPatternCorners(result_calib));
	plan_curr.pattern = result_calib.rotated ? m_ptrPatternCornersRotated : m_ptrPatternCornersInPlace;
	plan_curr.pattern_width = (((result_calib.rotated ? m_nPatternCornersNumRow : m_nPatternCornersNumCol) - 1) * m_nPatternBlockSizePixel);
	plan_curr.pattern_height = (((result_calib.rotated ? m_nPatternCornersNumCol : m_nPatternCornersNumRow) - 1) * m_nPatternBlockSizePixel);

	/* Compute homography */
	CompVHomographyResult result_homography;
	COMPV_CHECK_CODE_RETURN(homography(result_calib, result_homography, &plan_curr.homography));
	if (result_homography.inlinersCount < (m_nPatternCornersTotal - std::max(m_nPatternCornersNumRow, m_nPatternCornersNumCol))) { // We allow at most #1 row/col error (outliers)
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough inliners after homography computation (%zu / %zu)", result_homography.inlinersCount, m_nPatternCornersTotal);
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INLIERS;
		return COMPV_ERROR_CODE_S_OK;
	}
	// Save the plane for future calibration
	result_calib.planes.push_back(plan_curr);

	/* Compute calibration */
	if (result_calib.planes.size() < m_nMinPanesCountBeforeCalib) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough homographies (%zu < %zu) yet, delaying calibration", result_calib.planes.size(), m_nMinPanesCountBeforeCalib);
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_HOMOGRAPHIES;
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_CODE_RETURN(calibrate(result_calib));

	FIXME_once = true;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("result.code must not be set to ok now");
	result_calib.code = COMPV_CALIB_CAMERA_RESULT_OK;

	return COMPV_ERROR_CODE_S_OK;
}

typedef compv_float64_t compv_float64x3_t[3];

// (1x3) -> (3x3)
static COMPV_ERROR_CODE CompVMathTrigRodrigues3x3(const compv_float64x3_t& rot3, CompVMatPtrPtr rot3x3)
{
	// http://mathworld.wolfram.com/RodriguesRotationFormula.html
	// https://www.cs.duke.edu/courses/compsci527/fall13/notes/rodrigues.pdf

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(rot3x3, 3, 3));
	compv_float64_t* R0 = (*rot3x3)->ptr<compv_float64_t>(0);
	compv_float64_t* R1 = (*rot3x3)->ptr<compv_float64_t>(1);
	compv_float64_t* R2 = (*rot3x3)->ptr<compv_float64_t>(2);

	const compv_float64_t theta = std::sqrt((rot3[0] * rot3[0]) + (rot3[1] * rot3[1]) + (rot3[2] * rot3[2]));

	// TODO(dmi): maybe use closeToZero instead of comparing with 0.0
	if (theta == 0) {
		R0[0] = 1, R0[1] = 0, R0[2] = 0;
		R1[0] = 0, R1[1] = 1, R1[2] = 0;
		R2[0] = 0, R2[1] = 0, R2[2] = 1;
	}
	else {
		const compv_float64_t scale = (1 / theta);
		const compv_float64_t wx = rot3[0] * scale;
		const compv_float64_t wy = rot3[1] * scale;
		const compv_float64_t wz = rot3[2] * scale;
		const compv_float64_t cos_theta = std::cos(theta);
		const compv_float64_t sin_theta = std::sin(theta);
		const compv_float64_t one_minus_cos_theta = (1 - cos_theta);
		const compv_float64_t wxwy = (wx * wy);
		const compv_float64_t wxwz = (wx * wz);
		const compv_float64_t wywz = (wy * wz);

		R0[0] = cos_theta + ((wx * wx) * one_minus_cos_theta);
		R0[1] = (wxwy * one_minus_cos_theta) - (wz * sin_theta);
		R0[2] = (wy * sin_theta) + (wxwz * one_minus_cos_theta);

		R1[0] = (wz * sin_theta) + (wxwy * one_minus_cos_theta);
		R1[1] = cos_theta + ((wy * wy) * one_minus_cos_theta);
		R1[2] = (wywz * one_minus_cos_theta) - (wx * sin_theta);

		R2[0] = (wxwz * one_minus_cos_theta) - (wy * sin_theta);
		R2[1] = (wx * sin_theta) + (wywz * one_minus_cos_theta);
		R2[2] = cos_theta + ((wz * wz) * one_minus_cos_theta);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// (3x3) -> (1x3)
// "rot3x3" must be a rotation matrix: a square matrix R is a rotation matrix if and only if Rt = R* and det(R) = 1 -> Rt.R = R.Rt = I
static COMPV_ERROR_CODE CompVMathTrigRodrigues1x3(const CompVMatPtr& rot3x3, compv_float64x3_t& rot3)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Move to compv_math_trig.cxx");
	COMPV_CHECK_EXP_RETURN(!rot3x3 || rot3x3->cols() != 3 || rot3x3->rows() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	// https://www.cs.duke.edu/courses/compsci527/fall13/notes/rodrigues.pdf

	const compv_float64_t* R = rot3x3->ptr<const compv_float64_t>(0);
	const compv_float64_t R11 = R[0];
	const compv_float64_t R12 = R[1];
	const compv_float64_t R13 = R[2];
	R += rot3x3->stride();
	const compv_float64_t R21 = R[0];
	const compv_float64_t R22 = R[1];
	const compv_float64_t R23 = R[2];
	R += rot3x3->stride();
	const compv_float64_t R31 = R[0];
	const compv_float64_t R32 = R[1];
	const compv_float64_t R33 = R[2];

	const compv_float64_t a32 = (R32 - R23) * 0.5;
	const compv_float64_t a13 = (R13 - R31) * 0.5;
	const compv_float64_t a21 = (R21 - R12) * 0.5;

	const compv_float64_t s = std::sqrt((a32 * a32) + (a13 * a13) + (a21 * a21));
	const compv_float64_t c = (R11 + R22 + R33 - 1) * 0.5;

	// TODO(dmi): maybe use closeToZero instead of comparing with 0.0

	if (s == 0 && c == 1) {
		rot3[0] = rot3[1] = rot3[2] = 0;
	}
	else {
		compv_float64_t u0, u1, u2;
		if (s == 0 && c == -1) {
			if ((R11 + 1) != 0 || R21 != 0 || R31 != 0) {
				u0 = (R11 + 1), u1 = R21, u2 = R31;
			}
			else if (R12 != 0 || (R22 + 1) != 0 || R32 != 0) {
				u0 = R12, u1 = (R22 + 1), u2 = R32;
			}
			else {
				u0 = R13, u1 = R23, u2 = (R33 + 1);
			}
			const compv_float64_t scale = (COMPV_MATH_PI / std::sqrt((u0 * u0) + (u1 * u1) + (u2 * u2)));
			u0 *= scale, u1 *= scale, u2 *= scale;

			const compv_float64_t normr = std::sqrt((u0 * u0) + (u1 * u1) + (u2 * u2));
			if ((normr == COMPV_MATH_PI) && ((u0 == 0 && u1 == 0 && u2 < 0) || (u0 == 0 && u1 == 0) || (u0 < 0))) {
				u0 = -u0, u1 = -u1, u2 = -u2;
			}
		}
		else {
			const compv_float64_t theta = std::atan2(s, c);
			const compv_float64_t scale = (theta / s);
			u0 = a32 * scale, u1 = a13 * scale, u2 = a21 * scale;
		}

		rot3[0] = u0, rot3[1] = u1, rot3[2] = u2;
	}

	return COMPV_ERROR_CODE_S_OK;
}

/* data structure to transmit data arays and fit model */
typedef struct {
	const double *cornersPtr;
	CompVMatPtr pattern; // FIXME(dmi): not correct when upside down
	size_t nplanes;
	double(*f)(double corX, double corY, const double *p);
} data_struct;

/* function evaluation, determination of residues */
static void lmmin_eval(const double *par, int m_dat, const void *data, double *fvec, int *userbreak)
{
	/* for readability, explicit type conversion */
	data_struct *D;
	D = (data_struct*)data;

	const compv_float64_t* cornersPtr = D->cornersPtr;

	const size_t ncorners = D->pattern->cols();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Recreating matrices");

	size_t index;

	COMPV_ASSERT(m_dat == 3200); // FIXME(dmi): remove

	//const size_t m =
	//	5 // fx, fy, cx, cy, skew
	//	+ 2 // k1, k2[, p1, p2]
	//	+ ((3 + 3) * nplanes) // Rt
	size_t nplanes = D->nplanes;
	CompVMatPtr K;
	COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<compv_float64_t>(&K, 3, 3));
	COMPV_CHECK_CODE_ASSERT(K->zero_all());
	index = 0;
	*K->ptr<compv_float64_t>(0, 0) = par[index++]; // fx
	*K->ptr<compv_float64_t>(1, 1) = par[index++]; // fy
	*K->ptr<compv_float64_t>(0, 2) = par[index++]; //cx
	*K->ptr<compv_float64_t>(1, 2) = par[index++]; // cy
	*K->ptr<compv_float64_t>(0, 1) = par[index++]; // skew
	CompVMatPtr k/*, pp*/;
	COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<compv_float64_t>(&k, 2, 1));
	//COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<compv_float64_t>(&pp, 2, 1));
	*k->ptr<compv_float64_t>(0, 0) = par[index++]; // k1
	*k->ptr<compv_float64_t>(1, 0) = par[index++]; // k2
	//*pp->ptr<compv_float64_t>(0, 0) = p[index++]; // p1
	//*pp->ptr<compv_float64_t>(1, 0) = p[index++]; // p2

	CompVMatPtr R, t, reprojected;
	compv_float64x3_t r;
	COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<compv_float64_t>(&t, 1, 3));
	size_t nindex = 0;
	for (size_t i = 0; i < nplanes; ++i) {
		r[0] = par[index++]; // r0
		r[1] = par[index++]; // r1
		r[2] = par[index++]; // r2

		*t->ptr<compv_float64_t>(0, 0) = par[index++]; // tx
		*t->ptr<compv_float64_t>(0, 1) = par[index++]; // ty
		*t->ptr<compv_float64_t>(0, 2) = par[index++]; // tz

		COMPV_CHECK_CODE_ASSERT(CompVMathTrigRodrigues3x3(r, &R));
		COMPV_CHECK_CODE_ASSERT(proj(D->pattern, K, k, nullptr, R, t, &reprojected));
		COMPV_ASSERT(ncorners == reprojected->cols());
		const compv_float64_t* reprojectedX = reprojected->ptr<const compv_float64_t>(0);
		const compv_float64_t* reprojectedY = reprojected->ptr<const compv_float64_t>(1);
		for (size_t j = 0, z = (i * ncorners * 2); j < ncorners; ++j, z += 2) {
			// compute the residual
			fvec[z + 0] = (cornersPtr[z + 0] - reprojectedX[j]);
			fvec[z + 1] = (cornersPtr[z + 1] - reprojectedY[j]);
		}
	}

	COMPV_ASSERT(index == 127); // FIXME(dmi): remove
}

// FIXME(dmi): remove
COMPV_ERROR_CODE CompVCalibCamera::test(CompVCalibCameraResult& result_calib, CompVPointFloat32Vector& corrected, size_t file_index COMPV_DEFAULT(47))
{
	const size_t base_file_index = file_index - 47;
	COMPV_CHECK_EXP_RETURN(base_file_index >= result_calib.planes.size() || base_file_index >= result_calib.planes.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	corrected.clear();

	CompVMatPtr correctedMat;
	const CompVCalibCameraPlan& plan = result_calib.planes[base_file_index];
	COMPV_CHECK_CODE_RETURN(proj(plan.pattern, result_calib.K, result_calib.k, result_calib.p, plan.R, plan.t, &correctedMat));
	const compv_float64_t* projX = correctedMat->ptr<compv_float64_t>(0);
	const compv_float64_t* projY = correctedMat->ptr<compv_float64_t>(1);
	const compv_float64_t* projZ = correctedMat->ptr<compv_float64_t>(2);

	const size_t numPoints = correctedMat->cols();
	size_t i;
	corrected.resize(numPoints);
	CompVPointFloat32Vector::iterator it;
	for (i = 0, it = corrected.begin(); it < corrected.end(); ++it, ++i) {
		it->x = static_cast<compv_float32_t>(projX[i]);
		it->y = static_cast<compv_float32_t>(projY[i]);
		it->z = static_cast<compv_float32_t>(projZ[i]);
	}

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove next code");
	compv_float64_t error;
	COMPV_CHECK_CODE_RETURN(projError(result_calib, error));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "error before LM: %f", error);


	/*{
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "plan.R = %f, %f, %f\n %f, %f, %f\n %f, %f, %f",
			*plan.R->ptr<const compv_float64_t>(0, 0), *plan.R->ptr<const compv_float64_t>(0, 1), *plan.R->ptr<const compv_float64_t>(0, 2),
			*plan.R->ptr<const compv_float64_t>(1, 0), *plan.R->ptr<const compv_float64_t>(1, 1), *plan.R->ptr<const compv_float64_t>(1, 2),
			*plan.R->ptr<const compv_float64_t>(2, 0), *plan.R->ptr<const compv_float64_t>(2, 1), *plan.R->ptr<const compv_float64_t>(2, 2)
		);
		compv_float64x3_t r;
		COMPV_CHECK_CODE_RETURN(CompVMathTrigRodrigues1x3(plan.R, r));
		COMPV_CHECK_CODE_RETURN(CompVMathTrigRodrigues3x3(r, (CompVMatPtrPtr)&plan.R));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "plan.R' = %f, %f, %f\n %f, %f, %f\n %f, %f, %f",
			*plan.R->ptr<const compv_float64_t>(0, 0), *plan.R->ptr<const compv_float64_t>(0, 1), *plan.R->ptr<const compv_float64_t>(0, 2),
			*plan.R->ptr<const compv_float64_t>(1, 0), *plan.R->ptr<const compv_float64_t>(1, 1), *plan.R->ptr<const compv_float64_t>(1, 2),
			*plan.R->ptr<const compv_float64_t>(2, 0), *plan.R->ptr<const compv_float64_t>(2, 1), *plan.R->ptr<const compv_float64_t>(2, 2)
		);
	}*/

	const size_t nplanes = result_calib.planes.size();

	//Measurement
	const size_t ncorners = result_calib.planes.begin()->intersections.size();
	const size_t n = (nplanes * ncorners) << 1; // (x,y)
	CompVMatPtr corners;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&corners, 2, n));
	compv_float64_t* cornersPtr = corners->ptr<compv_float64_t>(0);
	size_t index = 0;
	for (CompVCalibCameraPlanVector::const_iterator it_plans = result_calib.planes.begin(); it_plans < result_calib.planes.end(); ++it_plans) {
		for (CompVPointFloat32Vector::const_iterator it_intersections = it_plans->intersections.begin(); it_intersections < it_plans->intersections.end(); ++it_intersections) {
			cornersPtr[index++] = static_cast<compv_float64_t>(it_intersections->x);
			cornersPtr[index++] = static_cast<compv_float64_t>(it_intersections->y);
		}
	}
	COMPV_ASSERT(index == n);

	//Parameter
	CompVMatPtr parameters;
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Too much parameters for R, use rodrigues to diminish");
	const size_t m =
		5 // fx, fy, cx, cy, skew
		+ 2 // k1, k2[, p1, p2]
		+ ((3 + 3) * nplanes) // R, t
		;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&parameters, 1, m));
	compv_float64_t* parametersPtr = parameters->ptr<compv_float64_t>();
	index = 0;
	parametersPtr[index++] = *result_calib.K->ptr<const compv_float64_t>(0, 0); // fx
	parametersPtr[index++] = *result_calib.K->ptr<const compv_float64_t>(1, 1); // fy
	parametersPtr[index++] = *result_calib.K->ptr<const compv_float64_t>(0, 2); //cx
	parametersPtr[index++] = *result_calib.K->ptr<const compv_float64_t>(1, 2); // cy
	parametersPtr[index++] = *result_calib.K->ptr<const compv_float64_t>(0, 1); // skew

	parametersPtr[index++] = *result_calib.k->ptr<const compv_float64_t>(0, 0); // k1
	parametersPtr[index++] = *result_calib.k->ptr<const compv_float64_t>(1, 0); // k2
	//parametersPtr[index++] = result_calib.p ? *result_calib.p->ptr<const compv_float64_t>(0, 0) : 0.0; // p1
	//parametersPtr[index++] = result_calib.p ? *result_calib.p->ptr<const compv_float64_t>(1, 0) : 0.0; // p2

	// Rt
	compv_float64x3_t r;
	for (CompVCalibCameraPlanVector::const_iterator it_plans = result_calib.planes.begin(); it_plans < result_calib.planes.end(); ++it_plans) {
		COMPV_CHECK_CODE_RETURN(CompVMathTrigRodrigues1x3(it_plans->R, r));
		parametersPtr[index++] = r[0]; // r0
		parametersPtr[index++] = r[1]; // r1
		parametersPtr[index++] = r[2]; // r2

		parametersPtr[index++] = *it_plans->t->ptr<const compv_float64_t>(0, 0); // tx
		parametersPtr[index++] = *it_plans->t->ptr<const compv_float64_t>(0, 1); // ty
		parametersPtr[index++] = *it_plans->t->ptr<const compv_float64_t>(0, 2); // tz
	}
	COMPV_ASSERT(index == m);

	lm_status_struct status;
	lm_control_struct control = lm_control_double;
	control.verbosity = 3;

	/* parameter vector */
	int n_par = static_cast<int>(m); /* number of parameters in model function f */
	double* par = parametersPtr; /* arbitrary starting value */

	/* data points */
	int m_dat = static_cast<int>(n);
	data_struct data;
	data.cornersPtr = cornersPtr;
	data.pattern = result_calib.planes.begin()->pattern;
	data.nplanes = nplanes;

	/* perform the fit */
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Fitting:");
	// https://en.wikipedia.org/wiki/Levenberg%E2%80%93Marquardt_algorithm
	lmmin(n_par, par, m_dat, (const void*)&data, lmmin_eval,
		&control, &status);

	/* print results */
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "\nResults:");
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "status after %d function evaluations:\n  %s",
		status.nfev, lm_infmsg[status.outcome]);

	index = 0;
	*result_calib.K->ptr<compv_float64_t>(0, 0) = parametersPtr[index++]; // fx
	*result_calib.K->ptr<compv_float64_t>(1, 1) = parametersPtr[index++]; // fy
	*result_calib.K->ptr<compv_float64_t>(0, 2) = parametersPtr[index++]; //cx
	*result_calib.K->ptr<compv_float64_t>(1, 2) = parametersPtr[index++]; // cy
	*result_calib.K->ptr<compv_float64_t>(0, 1) = parametersPtr[index++]; // skew
	*result_calib.k->ptr<compv_float64_t>(0, 0) = parametersPtr[index++]; // k1
	*result_calib.k->ptr<compv_float64_t>(1, 0) = parametersPtr[index++]; // k2
	/*if (result_calib.p) {
		*result_calib.p->ptr<compv_float64_t>(0, 0) = p[index++]; // p1
		*result_calib.p->ptr<compv_float64_t>(1, 0) = p[index++]; // p2
	}
	else {
		index += 2;
	}*/

	for (size_t i = 0; i < nplanes; ++i) {
		CompVCalibCameraPlan& plan = result_calib.planes[i];
		r[0] = parametersPtr[index++]; // r0
		r[1] = parametersPtr[index++]; // r1
		r[2] = parametersPtr[index++]; // r2
		COMPV_CHECK_CODE_RETURN(CompVMathTrigRodrigues3x3(r, &plan.R));

		*plan.t->ptr<compv_float64_t>(0, 0) = parametersPtr[index++]; // tx
		*plan.t->ptr<compv_float64_t>(0, 1) = parametersPtr[index++]; // ty
		*plan.t->ptr<compv_float64_t>(0, 2) = parametersPtr[index++]; // tz
	}
	COMPV_ASSERT(index == m);
	
	COMPV_CHECK_CODE_RETURN(projError(result_calib, error));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "error afert LM: %f", error);
	

	return COMPV_ERROR_CODE_S_OK;
}

// Subdivide the lines in two groups: those parallel to the strongest lines and those vertical
COMPV_ERROR_CODE CompVCalibCamera::subdivision(const size_t image_width, const size_t image_height, const CompVCabLines& lines, CompVCabLines& lines_hz, CompVCabLines& lines_vt)
{
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < m_nPatternLinesTotal, COMPV_ERROR_CODE_E_INVALID_STATE, "No enought points");
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < lines.lines_hough.size(), COMPV_ERROR_CODE_E_INVALID_STATE, "Must have same number of cartesian and polar lines");

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT impementation found");

	CompVHoughLineVector::const_iterator it_hough;
	CompVLineFloat32Vector::const_iterator it_cartesian;

	// Find the strongest line
	size_t lines_cartesian_strongestIndex = 0, strongestStrength = 0, index;
	index = 0;
	for (it_hough = lines.lines_hough.begin(); it_hough < lines.lines_hough.end(); ++it_hough, ++index) {
		if (it_hough->strength > strongestStrength) {
			strongestStrength = it_hough->strength;
			lines_cartesian_strongestIndex = index;
		}
	}
	const CompVLineFloat32& lines_cartesian_strongest = lines.lines_cartesian[lines_cartesian_strongestIndex];
	CompVCabLines *p_lines_hz, *p_lines_vt;

	// FIXME(dmi): use cross-product instead of slopes https://math.stackexchange.com/questions/1858274/how-to-determine-if-two-lines-are-parallel-almost-parallel
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Use cross-product which is faster -> no arctan or sin comparisons");

	// Find the angle between the strongest line and other lines
	// When a line is vertical, (a.x == b.x) and slope is undefined (infinine which means atan(slope) is pi/2)
	it_hough = lines.lines_hough.begin();
	compv_float32_t angle, angle_diff, angle_sin;
	const compv_float32_t st_angle = std::atan2((lines_cartesian_strongest.b.y - lines_cartesian_strongest.a.y), (lines_cartesian_strongest.b.x - lines_cartesian_strongest.a.x)); // inclinaison angle, within [-pi/2, pi/2]
	if (std::sin(std::abs(st_angle)) > 0.5f) {
		p_lines_hz = &lines_hz;
		p_lines_vt = &lines_vt;
	}
	else {
		p_lines_hz = &lines_vt;
		p_lines_vt = &lines_hz;
	}
	for (it_hough = lines.lines_hough.begin(), it_cartesian = lines.lines_cartesian.begin(); it_cartesian < lines.lines_cartesian.end(); ++it_cartesian, ++it_hough) {
		angle = std::atan2((it_cartesian->b.y - it_cartesian->a.y), (it_cartesian->b.x - it_cartesian->a.x)); // inclinaison angle, within [-pi/2, pi/2]
		angle_diff = std::abs(st_angle - angle); // within [0, pi]
		angle_sin = std::sin(angle_diff); // within [0, 1]
		if (angle_sin > 0.5f) {
			// almost parallel
			p_lines_hz->lines_hough.push_back(*it_hough);
			p_lines_hz->lines_cartesian.push_back(*it_cartesian);
		}
		else {
			p_lines_vt->lines_hough.push_back(*it_hough);
			p_lines_vt->lines_cartesian.push_back(*it_cartesian);
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Grouping using cartesian distances
// "lines_hough_parallel" must contains lines almost parallel so that the distances are meaningful
COMPV_ERROR_CODE CompVCalibCamera::grouping(const size_t image_width, const size_t image_height, const CompVCabLines& lines_parallel, const bool vt, const size_t max_strength, CompVCabLineFloat32Vector& lines_parallel_grouped)
{
	lines_parallel_grouped.clear();

	// Group using distance to the origine point (x0,y0) = (0, 0)
	// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
	CompVCabLineGroupVector groups;
	CompVHoughLineVector::const_iterator it_hough;

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT impementation found");

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Using distance approximation: sqrt (c**2 + d**2) = abs(c + d)");

	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	const compv_float32_t rsmall = (vt ? kSmallRhoFactVt : kSmallRhoFactHz) * static_cast<compv_float32_t>(max_strength);
	const compv_float32_t rmedium = rsmall * 4.f;
	
	compv_float32_t distance, c, d, distance_diff;

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
			distance_diff = std::abs(g->pivot_distance - distance);
			if (distance_diff < rsmall) {
				group = &(*g);
				break;
			}
			else if (distance_diff < rmedium) {
				// If the distance isn't small but reasonably close (medium) then, check
				// if the lines intersect in the image domain
				compv_float32_t i_x, i_y;
				bool intersect = CompVMathLineSegmentGetIntersection(g->pivot_cartesian->a.x, g->pivot_cartesian->a.y, g->pivot_cartesian->b.x, g->pivot_cartesian->b.y,
					i->a.x, i->a.y, i->b.x, i->b.y, &i_x, &i_y);
				COMPV_DEBUG_INFO_CODE_FOR_TESTING("Also check intersection angle");
				// No need to check for the intersection angle because the lines are the same type (hz or vt)
				if (intersect && i_x >= 0.f && i_y >= 0.f && i_x < image_widthF && i_y < image_heightF) { // FIXME(dmi): also check intersection angle
					group = &(*g);
					break;
				}
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

COMPV_ERROR_CODE CompVCalibCamera::lineBestFit(const CompVLineFloat32Vector& points_cartesian, const CompVHoughLineVector& points_hough, CompVLineFloat32& line)
{
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() < 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Need at least #2 points");
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() != points_hough.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Must have same number of points for polar and cartesian points");
	
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT impementation found"); // Not really needed (number of points should always be very low)

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

// Build pattern's corners
COMPV_ERROR_CODE CompVCalibCamera::buildPatternCorners(const CompVCalibCameraResult& result_calib)
{
	if (!m_ptrPatternCornersInPlace || !m_ptrPatternCornersRotated) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Building pattern corners");
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&m_ptrPatternCornersInPlace, 3, m_nPatternCornersTotal));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&m_ptrPatternCornersRotated, 3, m_nPatternCornersTotal));
		compv_float64_t* corX0 = m_ptrPatternCornersInPlace->ptr<compv_float64_t>(0);
		compv_float64_t* corY0 = m_ptrPatternCornersInPlace->ptr<compv_float64_t>(1);
		compv_float64_t* corX1 = m_ptrPatternCornersRotated->ptr<compv_float64_t>(0);
		compv_float64_t* corY1 = m_ptrPatternCornersRotated->ptr<compv_float64_t>(1);
		COMPV_CHECK_CODE_RETURN(m_ptrPatternCornersInPlace->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
		COMPV_CHECK_CODE_RETURN(m_ptrPatternCornersRotated->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
		const compv_float64_t patternBlockSizePixel = static_cast<compv_float64_t>(m_nPatternBlockSizePixel);
		compv_float64_t x0, y0, x1, y1;
		size_t i, j, k;
		const size_t nPatternCornersNumRow0 = m_nPatternCornersNumRow;
		const size_t nPatternCornersNumCol0 = m_nPatternCornersNumCol;
		const size_t nPatternCornersNumRow1 = m_nPatternCornersNumCol;
		const size_t nPatternCornersNumCol1 = m_nPatternCornersNumRow;
		for (j = 0, y0 = 0.0, k = 0; j < nPatternCornersNumRow0; ++j, y0 += patternBlockSizePixel) {
			for (i = 0, x0 = 0.0; i < nPatternCornersNumCol0; ++i, x0 += patternBlockSizePixel, ++k) {
				corX0[k] = x0;
				corY0[k] = y0;
			}
		}
		for (j = 0, y1 = 0.0, k = 0; j < nPatternCornersNumRow1; ++j, y1 += patternBlockSizePixel) {
			for (i = 0, x1 = 0.0; i < nPatternCornersNumCol1; ++i, x1 += patternBlockSizePixel, ++k) {
				corX1[k] = x1;
				corY1[k] = y1;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Compute homography
COMPV_ERROR_CODE CompVCalibCamera::homography(CompVCalibCameraResult& result_calib, CompVHomographyResult& result_homography, CompVMatPtrPtr homographyMat)
{
	const CompVCalibCameraPlan& plane_curr = result_calib.plane_curr;
	const CompVPointFloat32Vector& intersections = plane_curr.intersections;
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
	COMPV_CHECK_CODE_RETURN(CompVHomography<compv_float64_t>::find(plane_curr.pattern, query, homographyMat, &result_homography));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibCamera::calibrate(CompVCalibCameraResult& result_calib)
{
	COMPV_CHECK_EXP_RETURN(result_calib.planes.size() < 3, COMPV_ERROR_CODE_E_INVALID_CALL, "Calibration process requires at least #3 images");

	const size_t numPlanes = result_calib.planes.size();
	CompVCalibCameraPlanVector& planes = result_calib.planes;
	CompVCalibCameraPlanVector::iterator it_planes;
	const compv_float64_t *h1, *h2, *h3;
	compv_float64_t h11, h12, h21, h22, h31, h32;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Upside-down (image #50 renamed) produce odd result");
	
	/* Compute the V matrix */
	// For multiple images (2n x 6) matrix: https://youtu.be/Ou9Uj75DJX0?t=22m13s
	// Each image have #2 contribution and we require at least #3 images
	CompVMatPtr V;
	COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<compv_float64_t>(&V, (numPlanes << 1), 6)));
	compv_float64_t* V0 = V->ptr<compv_float64_t>(0); // v12
	compv_float64_t* V1 = V->ptr<compv_float64_t>(1); // (v11 - v22)
	const size_t vStrideTimes2 = V->stride() << 1;

	for (it_planes = planes.begin(); it_planes < planes.end(); ++it_planes) {
		// For one image, computing v12, v11 and v22: https://youtu.be/Ou9Uj75DJX0?t=22m
		const CompVMatPtr& homography = it_planes->homography;
		h1 = homography->ptr<const compv_float64_t>(0);
		h2 = homography->ptr<const compv_float64_t>(1);
		h3 = homography->ptr<const compv_float64_t>(2);
		h11 = h1[0];
		h12 = h1[1];
		h21 = h2[0];
		h22 = h2[1];
		h31 = h3[0];
		h32 = h3[1];
		// const compv_float64_t v12[6] = { h11*h12, (h11*h22) + (h21*h12), (h31*h12) + (h11*h32), h21*h22, (h31*h22) + (h21*h32), h31*h32 };
		// const compv_float64_t v11[6] = { h11*h11, (h11*h21) + (h21*h12), (h31*h11) + (h11*h31), h21*h21, (h31*h21) + (h21*h31), h31*h31 };
		// const compv_float64_t v22[6] = { h12*h12, (h12*h22) + (h22*h12), (h32*h12) + (h12*h32), h22*h22, (h32*h22) + (h22*h32), h32*h32 };
		V0[0] = h11*h12, V0[1] = (h11*h22) + (h21*h12), V0[2] = (h31*h12) + (h11*h32), V0[3] = h21*h22, V0[4] = (h31*h22) + (h21*h32); V0[5] = h31*h32;
		V1[0] = (h11*h11) - (h12*h12), V1[1] = ((h11*h21) + (h21*h12)) - ((h12*h22) + (h22*h12)), V1[2] = ((h31*h11) + (h11*h31)) - ((h32*h12) + (h12*h32)), V1[3] = (h21*h21) - (h22*h22), V1[4] = ((h31*h21) + (h21*h31)) - ((h32*h22) + (h22*h32)); V1[5] = (h31*h31) - (h32*h32);
		
		V0 += vStrideTimes2;
		V1 += vStrideTimes2;
	}

	/* Find b by solving Vb = 0: https://youtu.be/Ou9Uj75DJX0?t=23m47s */
	// Vector B = (K*t.K*) = (Lt.L) https://youtu.be/Ou9Uj75DJX0?t=21m14s
	// Compute S = Vt*V, 6x6 symetric matrix
	CompVMatPtr S;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(V, &S));
	// Find Eigen values and vectors
	CompVMatPtr eigenValues, eigneVectors;
	static const bool sortEigenValuesVectors = true;
	static const bool transposeEigenVectors = true; // row-vector?
	static const bool promoteZerosInEigenValues = false; // set to zero when < eps
	COMPV_CHECK_CODE_RETURN(CompVMathEigen<compv_float64_t>::findSymm(S, &eigenValues, &eigneVectors, sortEigenValuesVectors, transposeEigenVectors, promoteZerosInEigenValues));
	const compv_float64_t* bPtr = eigneVectors->ptr<const compv_float64_t>(5); // 6x6 matrix -> index of the smallest eigen value is last one = #5
	const compv_float64_t b11 = bPtr[0];
	const compv_float64_t b12 = bPtr[1];
	const compv_float64_t b13 = bPtr[2];
	const compv_float64_t b22 = bPtr[3];
	const compv_float64_t b23 = bPtr[4];
	const compv_float64_t b33 = bPtr[5];

	/* Cholesky decomposition: https://youtu.be/Ou9Uj75DJX0?t=19m24s */
	// Chol(B) = L.Lt, with L = K*t -> Lt = K*
	// Cholesky decomposition for 3x3 matrix: https://rosettacode.org/wiki/Cholesky_decomposition
	const compv_float64_t k11 = std::sqrt(b11);
	const compv_float64_t k22 = std::sqrt(b22 - (b12 * b12)); // b21 = b12
	const compv_float64_t k33 = std::sqrt(b33 - ((b13 * b13) + (b23 * b23))); // b31 = b13, b32 = b23
	const compv_float64_t k11s = (1.0 / k11);
	const compv_float64_t k21 = k11s * b12; // b21 = b12
	const compv_float64_t k31 = k11s * b13; // b31 = b13
	const compv_float64_t k32 = (1.0 / k22) * (b23 - (k31*k21)); // b32 = b23
	// Build Lt
	CompVMatPtr Lt;
	COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<compv_float64_t>(&Lt, 3, 3)));
	compv_float64_t* Lt1 = Lt->ptr<compv_float64_t>(0);
	compv_float64_t* Lt2 = Lt->ptr<compv_float64_t>(1);
	compv_float64_t* Lt3 = Lt->ptr<compv_float64_t>(2);
	Lt1[0] = k11, Lt1[1] = k21, Lt1[2] = k31;
	Lt2[0] = 0.0, Lt2[1] = k22, Lt2[2] = k32;
	Lt3[0] = 0.0, Lt3[1] = 0.0, Lt3[2] = k33;
	// Compute K* = Lt -> K = Lt*
	CompVMatPtr K;
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Inverse of upper triangular matrix should be obvious (do not call CompVMatrix::invA3x3)");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("K is upper triangular matrix with k33 = 1.0");
	COMPV_CHECK_CODE_RETURN(CompVMatrix::invA3x3(Lt, &K));

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "K = %f, %f, %f\n %f, %f, %f\n %f, %f, %f",
		*K->ptr<const compv_float64_t>(0, 0), *K->ptr<const compv_float64_t>(0, 1), *K->ptr<const compv_float64_t>(0, 2),
		*K->ptr<const compv_float64_t>(1, 0), *K->ptr<const compv_float64_t>(1, 1), *K->ptr<const compv_float64_t>(1, 2),
		*K->ptr<const compv_float64_t>(2, 0), *K->ptr<const compv_float64_t>(2, 1), *K->ptr<const compv_float64_t>(2, 2)
	);

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove");
	result_calib.K = K;

	/* Compute r and t: https://youtu.be/Ou9Uj75DJX0?t=16m14s */
	/* Compute radial distorsion(k1, k2): https://youtu.be/Ou9Uj75DJX0?t=25m34s */
	compv_float64_t r11, r12, r13, r21, r22, r23, r31, r32, r33, t11, t12, t13, norm, ha, hb, hc;
	const compv_float64_t fx = *K->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t fy = *K->ptr<const compv_float64_t>(1, 1);
	const compv_float64_t skew = *K->ptr<const compv_float64_t>(0, 1);
	const compv_float64_t cx = *K->ptr<const compv_float64_t>(0, 2);
	const compv_float64_t cy = *K->ptr<const compv_float64_t>(1, 2);

	CompVMatPtr D, d;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&D, (numPlanes * m_nPatternCornersTotal) << 1, 2));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&d, (numPlanes * m_nPatternCornersTotal) << 1, 1));
	const size_t DStrideTimes2 = D->stride() << 1;
	const size_t dStrideTimes2 = d->stride() << 1;
	compv_float64_t* DPtr0 = D->ptr<compv_float64_t>();
	compv_float64_t* DPtr1 = DPtr0 + D->stride();
	compv_float64_t* dPtr0 = d->ptr<compv_float64_t>();
	compv_float64_t* dPtr1 = dPtr0 + d->stride();
	for (it_planes = planes.begin(); it_planes < planes.end(); ++it_planes) {
		const CompVMatPtr& homography = it_planes->homography;
		// r1 = k*.h1 and ||r1|| = 1
		ha = *homography->ptr<const compv_float64_t>(0, 0);
		hb = *homography->ptr<const compv_float64_t>(1, 0);
		hc = *homography->ptr<const compv_float64_t>(2, 0);
		r11 = ((k11 * ha) + (k21 * hb) + (k31 * hc));
		r12 = ((k22 * hb) + (k32 * hc));
		r13 = (k33 * hc);
		norm = 1.0 / std::sqrt((r11 * r11) + (r12 * r12) + (r13 * r13)); // use same norm for r1, r2 and t to have same scaling
		r11 *= norm;
		r12 *= norm;
		r13 *= norm;

		// r2 = k*.h2 and ||r2|| = 1
		ha = *homography->ptr<const compv_float64_t>(0, 1);
		hb = *homography->ptr<const compv_float64_t>(1, 1);
		hc = *homography->ptr<const compv_float64_t>(2, 1);
		r21 = ((k11 * ha) + (k21 * hb) + (k31 * hc));
		r22 = ((k22 * hb) + (k32 * hc));
		r23 = (k33 * hc);
		//norm = 1.0 / std::sqrt((r21 * r21) + (r22 * r22) + (r23 * r23));
		r21 *= norm;
		r22 *= norm;
		r23 *= norm;

		// r3 = r1 x r2 (https://www.mathsisfun.com/algebra/vectors-cross-product.html)
		r31 = r12 * r23 - r13 * r22;
		r32 = r13 * r21 - r11 * r23;
		r33 = r11 * r22 - r12 * r21;
		//norm = 1.0 / std::sqrt((r31 * r31) + (r32 * r32) + (r33 * r33));
		//r31 *= norm;
		//r32 *= norm;
		//r33 *= norm;
		//mamadou = (r31 * r31) + (r32 * r32) + (r33 * r33);
		
		// r = pack(r1, r2, r3)
		//rptr = rt->ptr<compv_float64_t>(0), rptr[0] = r11, rptr[1] = r21, rptr[2] = r31;
		//rptr = rt->ptr<compv_float64_t>(1), rptr[0] = r12, rptr[1] = r22, rptr[2] = r32;
		//rptr = rt->ptr<compv_float64_t>(2), rptr[0] = r13, rptr[1] = r23, rptr[2] = r33;

		// t = k*.h3
		ha = *homography->ptr<const compv_float64_t>(0, 2);
		hb = *homography->ptr<const compv_float64_t>(1, 2);
		hc = *homography->ptr<const compv_float64_t>(2, 2);
		t11 = ((k11 * ha) + (k21 * hb) + (k31 * hc));
		t12 = ((k22 * hb) + (k32 * hc));
		t13 = (k33 * hc);
		//norm = 1.0 / std::sqrt((t11 * t11) + (t12 * t12) + (t13 * t13));
		t11 *= norm;
		t12 *= norm;
		t13 *= norm;

		// RT = pack(r1, r2, t)
		//r11  r21  t11
		//r12  r22  t12
		//r13  r23  t13

		/* Computing radial distorsion */
		const CompVPointFloat32Vector& intersections = it_planes->intersections;
		const size_t numPoints = intersections.size();
		CompVPointFloat32Vector::const_iterator it_intersection;
		size_t i;
		const compv_float64_t* patternX = it_planes->pattern->ptr<compv_float64_t>(0); //correctedPoints->ptr<compv_float64_t>(0);
		const compv_float64_t* patternY = it_planes->pattern->ptr<compv_float64_t>(1); // correctedPoints->ptr<compv_float64_t>(1);
		
		for (i = 0, it_intersection = intersections.begin(); i < numPoints; ++i, ++it_intersection) {
			// Rt.[X,Y,Z]
			// Rt = pack(r1, r2, t)
			//r11  r21  t11
			//r12  r22  t12
			//r13  r23  t13
			compv_float64_t z = (patternX[i] * r13) + (patternY[i] * r23) + t13;
			compv_float64_t scale = z ? (1.0 / z) : 1.0;
			compv_float64_t x = ((patternX[i] * r11) + (patternY[i] * r21) + t11) * scale;
			compv_float64_t y = ((patternX[i] * r12) + (patternY[i] * r22) + t12) * scale;
			compv_float64_t r2 = x * x + y * y;
			compv_float64_t r4 = r2 * r2;
			compv_float64_t u = static_cast<compv_float64_t>(it_intersection->x);
			compv_float64_t v = static_cast<compv_float64_t>(it_intersection->y);
			compv_float64_t uh = cx + fx * x + skew * y;
			compv_float64_t vh = cy + fy * y;
			DPtr0[0] = ((u - cx) * r2);
			DPtr0[1] = ((u - cx) * r4);
			DPtr1[0] = ((v - cy) * r2);
			DPtr1[1] = ((v - cy) * r4);
			dPtr0[0] = (uh - u);
			dPtr1[0] = (vh - v);

			DPtr0 += DStrideTimes2;
			DPtr1 += DStrideTimes2;
			dPtr0 += dStrideTimes2;
			dPtr1 += dStrideTimes2;
		}

		// Calibration matrice R (extrinsic)
		compv_float64_t *ptr;
		CompVMatPtr& R = it_planes->R;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&R, 3, 3));
		ptr = R->ptr<compv_float64_t>(0), ptr[0] = r11, ptr[1] = r21, ptr[2] = r31;
		ptr = R->ptr<compv_float64_t>(1), ptr[0] = r12, ptr[1] = r22, ptr[2] = r32;
		ptr = R->ptr<compv_float64_t>(2), ptr[0] = r13, ptr[1] = r23, ptr[2] = r33;

		// Calibration matrice t (extrinsic)
		CompVMatPtr& t = it_planes->t;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&t, 1, 3));
		ptr = t->ptr<compv_float64_t>(0), ptr[0] = t11, ptr[1] = t12, ptr[2] = t13;
	}

	/* Final radial distorsions (Least Square minimization) */
	CompVMatPtr DtD;
	// Dt.D
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(D, &DtD));
	const compv_float64_t aa = *DtD->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t bb = *DtD->ptr<const compv_float64_t>(0, 1);
	const compv_float64_t cc = *DtD->ptr<const compv_float64_t>(1, 0);
	const compv_float64_t dd = *DtD->ptr<const compv_float64_t>(1, 1);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "DtD = %f, %f\n %f, %f", aa, bb, cc, dd);
	// (2x2) matrix inverse: https://www.mathsisfun.com/algebra/matrix-inverse.html
	compv_float64_t det = (aa*dd) - (bb*cc);
	det = 1.0 / det;
	*DtD->ptr<compv_float64_t>(0, 0) = (dd * det);
	*DtD->ptr<compv_float64_t>(0, 1) = -(bb * det);
	*DtD->ptr<compv_float64_t>(1, 0) = -(cc * det);
	*DtD->ptr<compv_float64_t>(1, 1) = (aa * det);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "DtDinv = %f, %f\n %f, %f", 
		*DtD->ptr<const compv_float64_t>(0, 0), *DtD->ptr<const compv_float64_t>(0, 1), *DtD->ptr<const compv_float64_t>(1, 0), *DtD->ptr<const compv_float64_t>(1, 1));
	// (DtDinv.Dt).d
	CompVMatPtr kd;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulABt(DtD, D, &kd));
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(kd, d, &result_calib.k));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "k = %f, %f", *result_calib.k->ptr<const compv_float64_t>(0, 0), *result_calib.k->ptr<const compv_float64_t>(1, 0));

	// https://www.inf.ethz.ch/personal/pomarc/pubs/KumarCVPR08.pdf
	
	return COMPV_ERROR_CODE_S_OK;
}

// Project to 2D plan
static COMPV_ERROR_CODE proj(const CompVMatPtr& inPoints, const CompVMatPtr& K, const CompVMatPtr& k, const CompVMatPtr& p, const CompVMatPtr& R, const CompVMatPtr&t, CompVMatPtrPtr outPoints)
{
	COMPV_CHECK_EXP_RETURN(!inPoints || inPoints->isEmpty() || inPoints->rows() != 3 || !K || !k || !outPoints || !R || !t, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(K->rows() != 3 || K->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "K must be (3x3) matrix");
	COMPV_CHECK_EXP_RETURN(k->rows() < 2 || k->cols() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "k must be (2+x1) vector");
	COMPV_CHECK_EXP_RETURN(p && (p->rows() < 2 || p->cols() != 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "p must be (2+x1) vector");
	COMPV_CHECK_EXP_RETURN(R->rows() != 3 || R->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "R must be (3x3) matrix");
	COMPV_CHECK_EXP_RETURN(t->rows() != 1 || t->cols() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "R must be (1x3) vector");

	CompVPointFloat32Vector::const_iterator it_intersections;
	size_t index;
	const size_t numPoints = inPoints->cols();

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(outPoints, 3, numPoints));

	compv_float64_t* outPointsX = (*outPoints)->ptr<compv_float64_t>(0);
	compv_float64_t* outPointsY = (*outPoints)->ptr<compv_float64_t>(1);
	compv_float64_t* outPointsZ = (*outPoints)->ptr<compv_float64_t>(2);

	const compv_float64_t* inPointsX = inPoints->ptr<compv_float64_t>(0);
	const compv_float64_t* inPointsY = inPoints->ptr<compv_float64_t>(1);
	const compv_float64_t* inPointsZ = inPoints->ptr<compv_float64_t>(2);

	const compv_float64_t fx = *K->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t fy = *K->ptr<const compv_float64_t>(1, 1);
	const compv_float64_t cx = *K->ptr<const compv_float64_t>(0, 2);
	const compv_float64_t cy = *K->ptr<const compv_float64_t>(1, 2);

	const compv_float64_t k1 = *k->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t k2 = *k->ptr<const compv_float64_t>(1, 0);
	const compv_float64_t p1 = p ? *p->ptr<const compv_float64_t>(0, 0) : 0.0;
	const compv_float64_t p2 = p ? *p->ptr<const compv_float64_t>(1, 0) : 0.0;

	// https://youtu.be/Ou9Uj75DJX0?t=25m34s
	const compv_float64_t R0 = *R->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t R1 = *R->ptr<const compv_float64_t>(0, 1);
	const compv_float64_t R2 = *R->ptr<const compv_float64_t>(0, 2);
	const compv_float64_t R3 = *R->ptr<const compv_float64_t>(1, 0);
	const compv_float64_t R4 = *R->ptr<const compv_float64_t>(1, 1);
	const compv_float64_t R5 = *R->ptr<const compv_float64_t>(1, 2);
	const compv_float64_t R6 = *R->ptr<const compv_float64_t>(2, 0);
	const compv_float64_t R7 = *R->ptr<const compv_float64_t>(2, 1);
	const compv_float64_t R8 = *R->ptr<const compv_float64_t>(2, 2);

	const compv_float64_t tx = *t->ptr<const compv_float64_t>(0, 0);
	const compv_float64_t ty = *t->ptr<const compv_float64_t>(0, 1);
	const compv_float64_t tz = *t->ptr<const compv_float64_t>(0, 2);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	compv_float64_t xp, yp, zp;
	compv_float64_t x, y, z;
	compv_float64_t x2, y2, r2, r4, a1, a2, a3, radialdist;
	for (index = 0; index < numPoints; ++index) {
		xp = inPointsX[index];
		yp = inPointsY[index];
		zp = inPointsZ[index];
		// Apply R and t
		x = R0 * xp + R1 * yp + R2 * zp + tx;
		y = R3 * xp + R4 * yp + R5 * zp + ty;
		z = R6 * xp + R7 * yp + R8 * zp + tz;

		// https://youtu.be/Ou9Uj75DJX0?t=25m34s (1)
		z = z ? (1.0 / z) : 1.0;
		x *= z;
		y *= z;

		// https://youtu.be/Ou9Uj75DJX0?t=25m34s (2) or general form: https://en.wikipedia.org/wiki/Distortion_(optics)#Software_correction
		x2 = (x * x);
		y2 = (y * y);
		r2 = x2 + y2;
		r4 = r2 * r2;
		a1 = 2 * (x * y);
		a2 = r2 + (2 * x2);
		a3 = r2 + (2 * y2);
		radialdist = 1 + k1 * r2 + k2 * r4;
		// TODO(dmi): Add SIMD versions for (p1,p2) = (0,0) only
		x = x*radialdist + p1 * a1 + p2 * a2;
		y = y*radialdist + p1 * a3 + p2 * a1;

		// https://youtu.be/Ou9Uj75DJX0?t=25m34s (3)
		outPointsX[index] = x * fx + cx;
		outPointsY[index] = y * fy + cy;
		outPointsZ[index] = 1.0;
	}

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE projError(const CompVMatPtr& inPoints, const CompVMatPtr& outPoints, const CompVMatPtr& K, const CompVMatPtr& k, const CompVMatPtr& p, const CompVMatPtr& R, const CompVMatPtr&t, compv_float64_t& error)
{
	COMPV_CHECK_EXP_RETURN(!inPoints || !outPoints || !K || !k || !R || !t || inPoints->cols() != outPoints->cols() || inPoints->rows() != outPoints->rows() || inPoints->rows() != 3,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	error = 0.0;
	const size_t numPoints = outPoints->cols();
	compv_float64_t diffX, diffY;
	const compv_float64_t* inPointsX = inPoints->ptr<compv_float64_t>(0);
	const compv_float64_t* inPointsY = inPoints->ptr<compv_float64_t>(1);
	const compv_float64_t* outPointsX = outPoints->ptr<compv_float64_t>(0);
	const compv_float64_t* outPointsY = outPoints->ptr<compv_float64_t>(1);
	// MSE
	for (size_t i = 0; i < numPoints; ++i) {
		diffX = inPointsX[i] - outPointsX[i];
		diffY = inPointsY[i] - outPointsY[i];
		error += (diffX * diffX) + (diffY * diffY);
	}
	error = std::sqrt(error / static_cast<compv_float64_t>(numPoints));
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE projError(const CompVCalibCameraResult& result_calib, compv_float64_t& error)
{
	COMPV_CHECK_EXP_RETURN(!result_calib.k || !result_calib.K || result_calib.planes.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	error = 0;

	CompVMatPtr reprojected;
	compv_float64_t *intersectionsX, *intersectionsY;
	size_t i;
	CompVMatPtr intersections;
	CompVPointFloat32Vector::const_iterator it_intersections;
	compv_float64_t e;

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&intersections,
		result_calib.planes.begin()->pattern->rows(),
		result_calib.planes.begin()->pattern->cols(),
		result_calib.planes.begin()->pattern->stride()));
	intersectionsX = intersections->ptr<compv_float64_t>(0);
	intersectionsY = intersections->ptr<compv_float64_t>(1);
	const size_t numPointsPerPlan = intersections->cols();

	for (CompVCalibCameraPlanVector::const_iterator i_plans = result_calib.planes.begin(); i_plans < result_calib.planes.end(); ++i_plans) {
		COMPV_CHECK_CODE_RETURN(proj(i_plans->pattern, result_calib.K, result_calib.k, result_calib.p, i_plans->R, i_plans->t, &reprojected));

		COMPV_CHECK_EXP_RETURN(i_plans->intersections.size() != reprojected->cols() || intersections->cols() != reprojected->cols(), COMPV_ERROR_CODE_E_INVALID_STATE);
		for (i = 0, it_intersections = i_plans->intersections.begin(); i < numPointsPerPlan; ++i, ++it_intersections) {
			intersectionsX[i] = static_cast<compv_float64_t>(it_intersections->x);
			intersectionsY[i] = static_cast<compv_float64_t>(it_intersections->y);
		}
		
		COMPV_CHECK_CODE_RETURN(projError(reprojected, intersections, result_calib.K, result_calib.k, result_calib.p, i_plans->R, i_plans->t, e));
		error += e;
	}

	error /= static_cast<compv_float64_t>(result_calib.planes.size());
	
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibCamera::newObj(CompVCalibCameraPtrPtr calib)
{
	COMPV_CHECK_EXP_RETURN(!calib, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVCalibCameraPtr calib_ = new CompVCalibCamera();
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
