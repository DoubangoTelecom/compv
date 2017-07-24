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

#include <iterator> /* std::back_inserter */

#define PATTERN_ROW_CORNERS_NUM				10 // Number of corners per row
#define PATTERN_COL_CORNERS_NUM				8  // Number of corners per column
#define PATTERN_CORNERS_NUM					(PATTERN_ROW_CORNERS_NUM * PATTERN_COL_CORNERS_NUM) // Total number of corners
#define PATTERN_GROUP_MAXLINES				10 // Maximum number of lines per group (errors)

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

#define CANNY_LOW							1.83f
#define CANNY_HIGH							CANNY_LOW*2.f
#define CANNY_KERNEL_SIZE					3

#define COMPV_THIS_CLASSNAME	"CompVCalibCamera"

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
static int get_line_intersection(compv_float32_t p0_x, compv_float32_t p0_y, compv_float32_t p1_x, compv_float32_t p1_y,
	compv_float32_t p2_x, compv_float32_t p2_y, compv_float32_t p3_x, compv_float32_t p3_y, compv_float32_t *i_x, compv_float32_t *i_y)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Re-write code from StackOverflow");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	compv_float32_t s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

	compv_float32_t s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
		// Collision detected
		*i_x = p0_x + (t * s1_x);
		*i_y = p0_y + (t * s1_y);
		return 1;
	}

	return 0; // No collision
}

COMPV_ERROR_CODE CompVCalibCamera::process(const CompVMatPtr& image, CompVCalibCameraResult& result)
{
	COMPV_CHECK_EXP_RETURN(!image || image->elmtInBytes() != sizeof(uint8_t) || image->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");

	// Reset the previous result
	result.reset();

	const size_t image_width = image->cols();
	const size_t image_height = image->rows();

	/* Canny edge detection */
	COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(image, &result.edges));

	/* Hough lines */
	// For SHT, set the threshold before processing. But for KHT, we need the global scale (GS) which is defined only *after* processing
	if (m_ptrHough->id() == COMPV_HOUGHSHT_ID) {
		COMPV_CHECK_CODE_RETURN(m_ptrHough->setInt(COMPV_HOUGH_SET_INT_THRESHOLD, 
			std::min(HOUGH_SHT_THRESHOLD_MAX, static_cast<int>(static_cast<double>(std::min(image->cols(), image->rows())) * HOUGH_SHT_THRESHOLD_FACT) + 1))
		);
	}
	// Process
	COMPV_CHECK_CODE_RETURN(m_ptrHough->process(result.edges, result.lines_raw.lines_hough));

	/* Remove weak lines using global scale (GS) */
	if (m_ptrHough->id() == COMPV_HOUGHKHT_ID) {
		compv_float64_t gs;
		COMPV_CHECK_CODE_RETURN(m_ptrHough->getFloat64(COMPV_HOUGHKHT_GET_FLT64_GS, &gs));
		const size_t min_strength = static_cast<size_t>(gs * HOUGH_KHT_THRESHOLD_FACT);
		auto fncShortLines = std::remove_if(result.lines_raw.lines_hough.begin(), result.lines_raw.lines_hough.end(), [&](const CompVHoughLine& line) {
			return line.strength < min_strength;
		});
		result.lines_raw.lines_hough.erase(fncShortLines, result.lines_raw.lines_hough.end());
	}

	/* Return if no enough points */
	if (result.lines_raw.lines_hough.size() < m_nPatternLinesTotal) {
		result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Convert from polar to cartesian coordinates */
	COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(image_width, image_height, result.lines_raw.lines_hough, result.lines_raw.lines_cartesian));

	/* Lines subdivision */
	CompVCabLines lines_hz, lines_vt;
	COMPV_CHECK_CODE_RETURN(subdivision(image_width, image_height, result.lines_raw, lines_hz, lines_vt));
	if (lines_hz.lines_cartesian.size() < m_nPatternLinesHz || lines_vt.lines_cartesian.size() < m_nPatternLinesVt) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After subdivision we got no enough hz or vt lines");
		result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
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
	// The line equation could be defined as 'y = mx + b', with 'm' the slope and 'b' the 'intercept'.
	// m = (b.y - a.y) / (b.x - a.x), '(b.x - a.x)' being a contant when using CompV's hough lines then we can consider using 
	// m = (b.y - a.y).
	// b = y - mx, 'x' being a contant when using CompV's hough lines then we can consider using 
	// b = y - m.
	// The intersection with x-axis is defined by "y = 0" -> "0 = mx + b" -> "x = -b/m"
	std::vector<std::pair<compv_float32_t, CompVLineFloat32> > intersections;
	intersections.reserve(lines_vt_grouped.size());
	std::for_each(lines_vt_grouped.begin(), lines_vt_grouped.end(), [&intersections](CompVLineFloat32 &line) {
		const compv_float32_t slope = (line.b.y - line.a.y);
		const compv_float32_t intercept = (line.a.y - slope);
		intersections.push_back(std::make_pair((intercept / slope), line));
	});
	std::sort(intersections.begin(), intersections.end(), [](const std::pair<compv_float32_t, const CompVLineFloat32> &pair1, const std::pair<compv_float32_t, const CompVLineFloat32> &pair2) {
		return pair1.first > pair2.first;
	});
	lines_vt_grouped.clear();
	std::transform(intersections.begin(), intersections.end(), std::back_inserter(lines_vt_grouped), [](const std::pair<compv_float32_t, CompVLineFloat32>& p) { 
		return p.second; 
	});

	/* Push grouped lines */
	lines_vt_grouped.reserve(lines_hz_grouped.size() + lines_vt_grouped.size());
	result.lines_grouped.lines_cartesian.assign(lines_hz_grouped.begin(), lines_hz_grouped.end());
	result.lines_grouped.lines_cartesian.insert(result.lines_grouped.lines_cartesian.end(), lines_vt_grouped.begin(), lines_vt_grouped.end());

	/* Compute intersections */
	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);
	compv_float32_t intersect_x, intersect_y;
	static const compv_float32_t intersect_z = 1.f;
	for (CompVLineFloat32Vector::const_iterator i = lines_hz_grouped.begin(); i < lines_hz_grouped.end(); ++i) {
		for (CompVLineFloat32Vector::const_iterator j = lines_vt_grouped.begin(); j < lines_vt_grouped.end(); ++j) {
			int intersect = get_line_intersection(i->a.x, i->a.y, i->b.x, i->b.y,
				j->a.x, j->a.y, j->b.x, j->b.y, &intersect_x, &intersect_y);
			if (!intersect) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No intersection between the lines. Stop processing");
				result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS;
				return COMPV_ERROR_CODE_S_OK;
			}
			if (intersect_x < 0.f || intersect_y < 0.f || intersect_x >= image_widthF || intersect_y >= image_heightF) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Intersection outside the image domain. Stop processing");
				result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_INTERSECTIONS;
				return COMPV_ERROR_CODE_S_OK;
			}
			result.points_intersections.push_back(CompVPointFloat32(intersect_x, intersect_y, intersect_z)); // z is fake and contain distance to origine (to avoid computing distance several times)
		}
	}

	static size_t count = 0;
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "\n\nCOOOOOOL %zu\n\n", ++count);

	return COMPV_ERROR_CODE_S_OK;
}

// Subdivide the lines in two groups: those parallel to the strongest lines and those vertical
COMPV_ERROR_CODE CompVCalibCamera::subdivision(const size_t image_width, const size_t image_height, const CompVCabLines& lines, CompVCabLines& lines_hz, CompVCabLines& lines_vt)
{
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < m_nPatternLinesTotal, COMPV_ERROR_CODE_E_INVALID_STATE, "No enought points");
	COMPV_CHECK_EXP_RETURN(lines.lines_cartesian.size() < lines.lines_hough.size(), COMPV_ERROR_CODE_E_INVALID_STATE, "Must have same number of cartesian and polar lines");

	const compv_float32_t image_widthf = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightf = static_cast<compv_float32_t>(image_height);

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
	if (std::sinf(std::abs(st_angle)) > 0.5f) {
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
		angle_sin = std::sinf(angle_diff); // within [0, 1]
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

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Converting to cartesian again while already done");

	// Group using distance to the origine point (x0,y0) = (0, 0)
	// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
	CompVCabLineGroupVector groups;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Using distance approximation: sqrt (c**2 + d**2) = abs(c + d)");

	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	const compv_float32_t r = std::sqrt(static_cast<compv_float32_t>((image_width * image_width) + (image_height * image_height)));
	const compv_float32_t rsmall = smallRhoFact * r;
	const compv_float32_t rmedium = rsmall * 2.f;
	
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
			else /*if (distance_diff < rmedium)*/ {
				// If the distance isn't small be reasonably close (medium) then, check
				// if the lines intersect in the image domain
				compv_float32_t i_x, i_y;
				int intersect = get_line_intersection(g->pivot_cartesian->a.x, g->pivot_cartesian->a.y, g->pivot_cartesian->b.x, g->pivot_cartesian->b.y,
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

	CompVCabLineFloat32Vector lines_cab;
	CompVHoughLineVector lines_hough_orphans;
	CompVLineFloat32Vector lines_cartesian_orphans;
	lines_cab.reserve(groups.size());
	CompVHoughLineVector::const_iterator i;
	CompVLineFloat32Vector::const_iterator j;
	for (CompVCabLineGroupVector::const_iterator g = groups.begin(); g < groups.end(); ++g) {
		if (g->lines.lines_hough.size() > 1) {
			size_t strength_sum = 0, max_strength_index = 0, index = 0, max_strength = 0;
			for (i = g->lines.lines_hough.begin(); i < g->lines.lines_hough.end(); ++i, ++index) {
				strength_sum += i->strength;
				if (max_strength < i->strength) {
					max_strength = i->strength;
					max_strength_index = index;
				}
			}
			const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(strength_sum);
			CompVCabLineFloat32 line_cab_cartesian;
			CompVLineFloat32& line_cartesian = line_cab_cartesian.line;
			line_cartesian.a.x = line_cartesian.a.y = line_cartesian.b.x = line_cartesian.b.y = 0.f;
			j = g->lines.lines_cartesian.begin();
#if 0
			for (i = g->lines.lines_hough.begin(); i < g->lines.lines_hough.end(); ++i, ++j) {
				// FIXME(dmi): pre-compute strength_scale = (i->strength) * scale)
				line_cartesian.a.x += (j->a.x * i->strength * scale);
				line_cartesian.a.y += (j->a.y * i->strength * scale);
				line_cartesian.b.x += (j->b.x * i->strength * scale);
				line_cartesian.b.y += (j->b.y * i->strength * scale);
			}
#elif 1
			COMPV_CHECK_CODE_RETURN(lineBestFit(g->lines.lines_cartesian, g->lines.lines_hough, line_cab_cartesian.line));
#else
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do something to a and b, maybe avg?");
			line_cab_cartesian.line = g->lines.lines_cartesian[max_strength_index];
#endif
			line_cab_cartesian.strength = strength_sum;
			lines_cab.push_back(line_cab_cartesian);
		}
		else {
			lines_hough_orphans.push_back(*g->lines.lines_hough.begin());
		}
	}

	// Convert orphans to cartesian and push to cab lines
	COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(image_width, image_height, lines_hough_orphans, lines_cartesian_orphans));
	j = lines_cartesian_orphans.begin();
	for (i = lines_hough_orphans.begin(); i < lines_hough_orphans.end(); ++i, ++j) {
		CompVCabLineFloat32 line_cab_cartesian;
		line_cab_cartesian.strength = i->strength;
		line_cab_cartesian.line = *j;
		lines_cab.push_back(line_cab_cartesian);
	}

	// Sort and push
	std::sort(lines_cab.begin(), lines_cab.end(), [](const CompVCabLineFloat32 &line1, const CompVCabLineFloat32 &line2) {
		return (line1.strength > line2.strength);
	});
	lines_parallel_grouped.reserve(lines_cab.size());
	for (CompVCabLineFloat32Vector::const_iterator i = lines_cab.begin(); i < lines_cab.end(); ++i) {
		lines_parallel_grouped.push_back(i->line);
	}

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

	// Compute the sum of the strengths and the global scaling factor
	compv_float32_t sum_strengths = 0.f;
	for (j = points_hough.begin(); j < points_hough.end(); ++j) {
		sum_strengths += j->strength;
	}
	sum_strengths *= 2.f; // times #2 because we have #2 points (a & b) for each step.
	const compv_float32_t scale_strengths = (1.f / sum_strengths);

	// Compute the strengths (for each point)
	std::vector<compv_float32_t> strengths(points_hough.size());
	size_t index;
	for (j = points_hough.begin(), index = 0; j < points_hough.end(); ++j, ++index) {
		strengths[index] = (j->strength * scale_strengths);
	}

	// Compute mean(y)
	compv_float32_t mean_y = 0.f;
	compv_float32_t scale_strength;
	for (i = points_cartesian.begin(), j = points_hough.begin(), k = strengths.begin(); i < points_cartesian.end(); ++i, ++j, ++k) {
		mean_y += (i->a.y + i->b.y) * (*k);
	}

	// Compute t0
	compv_float32_t t0 = 0.f;
	for (i = points_cartesian.begin(), j = points_hough.begin(), k = strengths.begin(); i < points_cartesian.end(); ++i, ++j, ++k) {
		scale_strength = (j->strength * scale_strengths);
		t0 += (((i->a.y - mean_y)) + ((i->b.y - mean_y))) * (*k);
	}

	// Set the result
	line = points_cartesian[0]; // set x, y, z
	line.a.y += (line.a.y * t0);
	line.b.y += (line.b.y * t0);
	
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
