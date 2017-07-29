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
#include "compv/base/time/compv_time.h"

#include <iterator> /* std::back_inserter */

#define PATTERN_ROW_CORNERS_NUM				10 // Number of corners per row
#define PATTERN_COL_CORNERS_NUM				8  // Number of corners per column
#define PATTERN_CORNERS_NUM					(PATTERN_ROW_CORNERS_NUM * PATTERN_COL_CORNERS_NUM) // Total number of corners
#define PATTERN_GROUP_MAXLINES				10 // Maximum number of lines per group (errors)
#define PATTERN_BLOCK_SIZE_PIXEL			40					

#define HOUGH_ID							COMPV_HOUGHSHT_ID
#define HOUGH_RHO							0.5f // "rho-delta" (half-pixel)
#define HOUGH_THETA							0.5f // "theta-delta" (half-radian)
#define HOUGH_SHT_THRESHOLD_FACT			0.10416667
#define HOUGH_SHT_THRESHOLD_MAX				120
#define HOUGH_KHT_THRESHOLD_FACT			1.0 // GS
#define HOUGH_SHT_THRESHOLD					50
#define HOUGH_KHT_THRESHOLD					1 // filter later when GS is known	
#define HOUGH_KHT_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGH_KHT_CLUSTER_MIN_SIZE			10
#define HOUGH_KHT_KERNEL_MIN_HEIGTH			0.002f // must be within [0, 1]

#define CANNY_LOW							1.33f
#define CANNY_HIGH							CANNY_LOW*2.f
#define CANNY_KERNEL_SIZE					3

#define COMPV_THIS_CLASSNAME	"CompVCalibCamera"

// Implementation based on video course "Photogrammetry I - 16b - DLT & Camera Calibration (2015)": https://www.youtube.com/watch?v=Ou9Uj75DJX0
// TODO(dmi): use "ceres-solver" http://ceres-solver.org/index.html
// TODO(dmi): add support for lmfit (FreeBSD): http://apps.jcns.fz-juelich.de/doku/sc/lmfit

COMPV_NAMESPACE_BEGIN()

static const float kSmallRhoFactVt = 0.015f; /* small = (rho * fact) */
static const float kSmallRhoFactHz = 0.0075f; /* small = (rho * fact) */

struct CompVCabLineGroup {
	const CompVLineFloat32* pivot_cartesian;
	const CompVHoughLine* pivot_hough;
	compv_float32_t pivot_distance; // distance(pivot, origin)
	CompVCabLines lines;
};
typedef std::vector<CompVCabLineGroup, CompVAllocatorNoDefaultConstruct<CompVCabLineGroup> > CompVCabLineGroupVector;

struct CompVCabLineFloat32 {
	CompVLineFloat32 line;
	size_t strength;
};
typedef std::vector<CompVCabLineFloat32, CompVAllocatorNoDefaultConstruct<CompVCabLineFloat32> > CompVCabLineFloat32Vector;

CompVCalibCamera::CompVCalibCamera()
	: m_nPatternCornersNumRow(PATTERN_ROW_CORNERS_NUM)
	, m_nPatternCornersNumCol(PATTERN_COL_CORNERS_NUM)
	, m_nPatternLinesHz(PATTERN_ROW_CORNERS_NUM)
	, m_nPatternLinesVt(PATTERN_COL_CORNERS_NUM)
	, m_nPatternBlockSizePixel(PATTERN_BLOCK_SIZE_PIXEL)
{
	m_nPatternCornersTotal = m_nPatternCornersNumRow * m_nPatternCornersNumCol;
	m_nPatternLinesTotal = (m_nPatternCornersNumRow + m_nPatternCornersNumCol);
}

CompVCalibCamera::~CompVCalibCamera()
{

}

// FIXME(dmi): remove (Code from StackOverflow)
// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines 
// intersect the intersection point may be stored in the floats i_x and i_y.
template<typename T = compv_float32_t>
static int get_line_segments_intersection(T p0_x, T p0_y, T p1_x, T p1_y,
	compv_float32_t p2_x, compv_float32_t p2_y, T p3_x, T p3_y, T *i_x, T *i_y)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Re-write code from StackOverflow");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Intersections with x-axis and y-axis can be speeded");
#if 0
	T s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

	T s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
		// Collision detected
		*i_x = p0_x + (t * s1_x);
		*i_y = p0_y + (t * s1_y);
		return 1;
	}
	return 0; // No collision
#else
	// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	T s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
	s10_x = p1_x - p0_x;
	s10_y = p1_y - p0_y;
	s32_x = p3_x - p2_x; // 0.f for y-axis instersection
	s32_y = p3_y - p2_y; // 0.f for x-axis intersection

	denom = s10_x * s32_y - s32_x * s10_y;
	if (denom == 0)
		return 0; // Collinear
	bool denomPositive = denom > 0;

	s02_x = p0_x - p2_x;
	s02_y = p0_y - p2_y;
	s_numer = s10_x * s02_y - s10_y * s02_x;
	if ((s_numer < 0) == denomPositive)
		return 0; // No collision

	t_numer = s32_x * s02_y - s32_y * s02_x;
	if ((t_numer < 0) == denomPositive)
		return 0; // No collision

	if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
		return 0; // No collision
				  // Collision detected
	t = t_numer / denom;

	*i_x = p0_x + (t * s10_x);
	*i_y = p0_y + (t * s10_y);

	return 1;
#endif
}

COMPV_ERROR_CODE CompVCalibCamera::process(const CompVMatPtr& image, CompVCalibCameraResult& result_calib)
{
	COMPV_CHECK_EXP_RETURN(!image || image->elmtInBytes() != sizeof(uint8_t) || image->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");

	// Reset the previous result
	result_calib.reset();

	const size_t image_width = image->cols();
	const size_t image_height = image->rows();
	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	int intersect;
	compv_float32_t intersect_x, intersect_y;
	static const compv_float32_t intersect_z = 1.f;


	/* Canny edge detection */
	COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(image, &result_calib.edges));

	/* Hough lines */
	// For SHT, set the threshold before processing. But for KHT, we need the global scale (GS) which is defined only *after* processing
	if (m_ptrHough->id() == COMPV_HOUGHSHT_ID) {
		COMPV_CHECK_CODE_RETURN(m_ptrHough->setInt(COMPV_HOUGH_SET_INT_THRESHOLD, 
			std::min(HOUGH_SHT_THRESHOLD_MAX, static_cast<int>(static_cast<double>(std::min(image->cols(), image->rows())) * HOUGH_SHT_THRESHOLD_FACT) + 1))
		);
	}

	// Process
	COMPV_CHECK_CODE_RETURN(m_ptrHough->process(result_calib.edges, result_calib.lines_raw.lines_hough));

	/* Remove weak lines using global scale (GS) */
	if (m_ptrHough->id() == COMPV_HOUGHKHT_ID) {
		compv_float64_t gs;
		COMPV_CHECK_CODE_RETURN(m_ptrHough->getFloat64(COMPV_HOUGHKHT_GET_FLT64_GS, &gs));
		const size_t min_strength = static_cast<size_t>(gs * HOUGH_KHT_THRESHOLD_FACT);
		auto fncShortLines = std::remove_if(result_calib.lines_raw.lines_hough.begin(), result_calib.lines_raw.lines_hough.end(), [&](const CompVHoughLine& line) {
			return line.strength < min_strength;
		});
		result_calib.lines_raw.lines_hough.erase(fncShortLines, result_calib.lines_raw.lines_hough.end());
	}

	/* Return if no enough points */
	if (result_calib.lines_raw.lines_hough.size() < m_nPatternLinesTotal) {
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Convert from polar to cartesian coordinates */
	COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(image_width, image_height, result_calib.lines_raw.lines_hough, result_calib.lines_raw.lines_cartesian));

	/* Lines subdivision */
	CompVCabLines lines_hz, lines_vt;
	COMPV_CHECK_CODE_RETURN(subdivision(image_width, image_height, result_calib.lines_raw, lines_hz, lines_vt));
	if (lines_hz.lines_cartesian.size() < m_nPatternLinesHz || lines_vt.lines_cartesian.size() < m_nPatternLinesVt) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After subdivision we got no enough hz or vt lines");
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Line grouping (round2) and sorting */
	CompVLineFloat32Vector lines_hz_grouped, lines_vt_grouped;

	// Hz
	if (lines_hz.lines_cartesian.size() > m_nPatternLinesHz) {
		COMPV_CHECK_CODE_RETURN(grouping(image_width, image_height, lines_hz, kSmallRhoFactHz, lines_hz_grouped));
		if (lines_hz_grouped.size() < m_nPatternLinesHz) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [hz] grouping we got less lines than what is requires, not a good news at all");
			lines_hz_grouped = lines_hz.lines_cartesian;
		}
	}
	else {
		lines_hz_grouped = lines_hz.lines_cartesian;
	}
	if (lines_hz_grouped.size() != m_nPatternLinesHz) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [hz] grouping we don't have exactly %zu lines but more (%zu). Maybe our grouping function missed some orphans", m_nPatternLinesHz, lines_hz_grouped.size());
	}
	
	// Vt
	if (lines_vt.lines_cartesian.size() > m_nPatternLinesVt) {
		COMPV_CHECK_CODE_RETURN(grouping(image_width, image_height, lines_vt, kSmallRhoFactVt, lines_vt_grouped));
		if (lines_vt_grouped.size() < m_nPatternLinesVt) {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [vt] grouping we got less lines than what is requires. Not a good news at all");
			lines_vt_grouped = lines_vt.lines_cartesian;
		}
	}
	else {
		lines_vt_grouped = lines_vt.lines_cartesian;
	}
	if (lines_vt_grouped.size() != m_nPatternLinesVt) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After [vt] grouping we don't have exactly %zu lines but more (%zu). Maybe our grouping function missed some orphans", m_nPatternLinesVt, lines_vt_grouped.size());
	}

	/* Keep best lines only (already sorted by strength in grouping function) */
	lines_hz_grouped.resize(m_nPatternLinesHz);
	lines_vt_grouped.resize(m_nPatternLinesVt);

	/* Sorting the lines */
	// Sort top-bottom
	std::sort(lines_hz_grouped.begin(), lines_hz_grouped.end(), [](const CompVLineFloat32 &line1, const CompVLineFloat32 &line2) {
		return std::tie(line1.a.y, line1.b.y) < std::tie(line2.a.y, line2.b.y); // coorect only because x-components are constant when using CompV's hough implementations
	});
	// Sort left-right: sort the intersection with x-axis
	std::vector<std::pair<compv_float32_t, CompVLineFloat32> > intersections;
	const compv_float32_t image_widthF_minus = -image_widthF;
	intersections.reserve(lines_vt_grouped.size());
	for (CompVLineFloat32Vector::const_iterator i = lines_vt_grouped.begin(); i < lines_vt_grouped.end(); ++i) {
		intersect = get_line_segments_intersection(i->a.x, i->a.y, i->b.x, i->b.y,
			image_widthF_minus, 0.f, image_widthF, 0.f, &intersect_x, &intersect_y);
		if (intersect) { // no need to check if it's in the image domain (for sure it is)
			intersections.push_back(std::make_pair(intersect_x, *i));
		}
		else {
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Vertical line doesn't intersect with x-axis. Stop processing");
			result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS;
			return COMPV_ERROR_CODE_S_OK;
		}
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
			intersect = get_line_segments_intersection(i->a.x, i->a.y, i->b.x, i->b.y,
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
			result_calib.points_intersections.push_back(CompVPointFloat32(intersect_x, intersect_y, intersect_z)); // z is fake and contain distance to origine (to avoid computing distance several times)
		}
	}

	/* Compute homography */
	uint64_t timeStart = CompVTime::nowMillis();
	CompVHomographyResult result_homography;
	COMPV_CHECK_CODE_RETURN(homography(result_calib, result_homography));
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Homography Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	if (result_homography.inlinersCount < (m_nPatternCornersTotal >> 1)) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No enough inliners after homography computation (%zu / %zu)", result_homography.inlinersCount, m_nPatternCornersTotal);
		result_calib.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INLIERS;
		return COMPV_ERROR_CODE_S_OK;
	}

	//static size_t count = 0;
	//COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "\n\nCOOOOOOL %zu\n\n", ++count);

	result_calib.code = COMPV_CALIB_CAMERA_RESULT_OK;

	return COMPV_ERROR_CODE_S_OK;
}

// Subdivide the lines in two groups: those parallel to the strongest lines and those vertical
COMPV_ERROR_CODE CompVCalibCamera::subdivision(const size_t image_width, const size_t image_height, const CompVCabLines& lines, CompVCabLines& lines_hz, CompVCabLines& lines_vt)
{
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < m_nPatternLinesTotal, COMPV_ERROR_CODE_E_INVALID_STATE, "No enought points");
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < lines.lines_hough.size(), COMPV_ERROR_CODE_E_INVALID_STATE, "Must have same number of cartesian and polar lines");

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
COMPV_ERROR_CODE CompVCalibCamera::grouping(const size_t image_width, const size_t image_height, const CompVCabLines& lines_parallel, const compv_float32_t smallRhoFact, CompVLineFloat32Vector& lines_parallel_grouped)
{
	lines_parallel_grouped.clear();

	// Group using distance to the origine point (x0,y0) = (0, 0)
	// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
	CompVCabLineGroupVector groups;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Using distance approximation: sqrt (c**2 + d**2) = abs(c + d)");

	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	const compv_float32_t r = std::sqrt(static_cast<compv_float32_t>((image_width * image_width) + (image_height * image_height)));
	const compv_float32_t rsmall = smallRhoFact * r;
	const compv_float32_t rmedium = rsmall * 8.f;
	
	compv_float32_t distance, c, d, distance_diff;

	// Grouping
	CompVHoughLineVector::const_iterator it_hough = lines_parallel.lines_hough.begin();
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
				// If the distance isn't small be reasonably close (medium) then, check
				// if the lines intersect in the image domain
				compv_float32_t i_x, i_y;
				int intersect = get_line_segments_intersection(g->pivot_cartesian->a.x, g->pivot_cartesian->a.y, g->pivot_cartesian->b.x, g->pivot_cartesian->b.y,
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

	CompVCabLineFloat32Vector lines_cab_gouped;
	lines_cab_gouped.reserve(groups.size());
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
			lines_cab_gouped.push_back(line_cab_cartesian);
		}
		else {
			CompVCabLineFloat32 line_cab_cartesian;
			line_cab_cartesian.line = *g->lines.lines_cartesian.begin();
			line_cab_cartesian.strength = g->lines.lines_hough.begin()->strength;
			lines_cab_gouped.push_back(line_cab_cartesian);
		}
	}

	// Sort
	std::sort(lines_cab_gouped.begin(), lines_cab_gouped.end(), [](const CompVCabLineFloat32 &line1, const CompVCabLineFloat32 &line2) {
		return (line1.strength > line2.strength);
	});

	// Push
	lines_parallel_grouped.reserve(lines_cab_gouped.size());
	std::transform(lines_cab_gouped.begin(), lines_cab_gouped.end(), std::back_inserter(lines_parallel_grouped), [](const CompVCabLineFloat32& line_cab_cartesian) {
		return line_cab_cartesian.line;
	});

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibCamera::lineBestFit(const CompVLineFloat32Vector& points_cartesian, const CompVHoughLineVector& points_hough, CompVLineFloat32& line)
{
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() < 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Need at least #2 points");
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() != points_hough.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Must have same number of points for polar and cartesian points");
	
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
COMPV_ERROR_CODE CompVCalibCamera::buildPatternCorners()
{
	if (!m_ptrPatternCorners || m_ptrPatternCorners->cols() != m_nPatternCornersTotal) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Building pattern corners");
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&m_ptrPatternCorners, 3, m_nPatternCornersTotal));
		compv_float64_t* trainX = m_ptrPatternCorners->ptr<compv_float64_t>(0);
		compv_float64_t* trainY = m_ptrPatternCorners->ptr<compv_float64_t>(1);
		COMPV_CHECK_CODE_RETURN(m_ptrPatternCorners->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
		COMPV_CHECK_EXP_RETURN(m_nPatternCornersTotal != (m_nPatternCornersNumRow * m_nPatternCornersNumCol), COMPV_ERROR_CODE_E_INVALID_STATE, "Pattern is invalid");
		const compv_float64_t patternBlockSizePixel = static_cast<compv_float64_t>(m_nPatternBlockSizePixel);
		compv_float64_t x, y;
		size_t i, j, k;
		for (j = 0, y = 0.0, k = 0; j < m_nPatternCornersNumRow; ++j, y += patternBlockSizePixel) {
			for (i = 0, x = 0.0; i < m_nPatternCornersNumCol; ++i, x += patternBlockSizePixel, ++k) {
				trainX[k] = x;
				trainY[k] = y;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Compute homography
COMPV_ERROR_CODE CompVCalibCamera::homography(CompVCalibCameraResult& result_calib, CompVHomographyResult& result_homography)
{
	COMPV_CHECK_EXP_RETURN(result_calib.points_intersections.size() != m_nPatternCornersTotal, COMPV_ERROR_CODE_E_INVALID_CALL, "Invalid number of corners");

	// Build pattern's corners
	COMPV_CHECK_CODE_RETURN(buildPatternCorners());

	// Convert the intersections from float32 to float64 for homagraphy
	CompVMatPtr query;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&query, 3, m_nPatternCornersTotal));
	compv_float64_t* queryX = query->ptr<compv_float64_t>(0);
	compv_float64_t* queryY = query->ptr<compv_float64_t>(1);
	COMPV_CHECK_CODE_RETURN(query->one_row<compv_float64_t>(2)); // homogeneous coord. with Z = 1
	size_t index = 0;
	for (CompVPointFloat32Vector::const_iterator i = result_calib.points_intersections.begin(); i < result_calib.points_intersections.end(); ++i, ++index) {
		queryX[index] = static_cast<compv_float64_t>(i->x);
		queryY[index] = static_cast<compv_float64_t>(i->y);
	}

	// Find homography
	COMPV_CHECK_CODE_RETURN(CompVHomography<compv_float64_t>::find(m_ptrPatternCorners, query, &result_calib.homography, &result_homography));

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
