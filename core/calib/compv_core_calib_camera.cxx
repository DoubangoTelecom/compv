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

#define PATTERN_ROW_CORNERS_NUM				10 // Number of corners per row
#define PATTERN_COL_CORNERS_NUM				8  // Number of corners per column
#define PATTERN_CORNERS_NUM					(PATTERN_ROW_CORNERS_NUM * PATTERN_COL_CORNERS_NUM) // Total number of corners
#define PATTERN_GROUP_MAXLINES				10 // Maximum number of lines per group (errors)

#define HOUGH_ID							COMPV_HOUGHSHT_ID
#define HOUGH_RHO							0.5f // "rho-delta" (half-pixel)
#define HOUGH_THETA							0.5f // "theta-delta" (half-radian)
#define HOUGH_SHT_THRESHOLD_FACT			0.00016276
#define HOUGH_SHT_THRESHOLD_MAX				50
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

static const float kSmallAngle = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(10.0 * HOUGH_THETA);
static const float kAngle45 = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(45.0);
static const float kTinyAngle = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(5.0);
static const float kSmallRhoFactRound1 = 0.015f; /* small = (rho * fact) */
static const float kSmallRhoFactRound2 = 0.00375f; /* small = (rho * fact) */

struct CompVHoughLineGroup {
	const CompVHoughLine* pivot = nullptr;
	CompVHoughLineVector lines;
};
typedef std::vector<CompVHoughLineGroup, CompVAllocatorNoDefaultConstruct<CompVHoughLineGroup> > CompVHoughLineGroupVector;

struct CompVCabLineGroup {
	const CompVLineFloat32* pivot_cartesian = nullptr;
	const CompVHoughLine* pivot_hough = nullptr;
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
			std::min(HOUGH_SHT_THRESHOLD_MAX, static_cast<int>(static_cast<double>(image->cols() * image->rows()) * HOUGH_SHT_THRESHOLD_FACT) + 1))
		);
	}
	// Process
	COMPV_CHECK_CODE_RETURN(m_ptrHough->process(result.edges, result.raw_hough_lines));

	/* Remove weak lines using global scale (GS) */
	if (m_ptrHough->id() == COMPV_HOUGHKHT_ID) {
		compv_float64_t gs;
		COMPV_CHECK_CODE_RETURN(m_ptrHough->getFloat64(COMPV_HOUGHKHT_GET_FLT64_GS, &gs));
		const size_t min_strength = static_cast<size_t>(gs * HOUGH_KHT_THRESHOLD_FACT);
		auto fncShortLines = std::remove_if(result.raw_hough_lines.begin(), result.raw_hough_lines.end(), [&](const CompVHoughLine& line) {
			return line.strength < min_strength;
		});
		result.raw_hough_lines.erase(fncShortLines, result.raw_hough_lines.end());
	}

	/* Return if no enough points */
	if (result.raw_hough_lines.size() < m_nPatternLinesTotal) {
		result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		return COMPV_ERROR_CODE_S_OK;
	}

	// FIXME(dmi): remove
	//COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(image_width, image_height, result.raw_hough_lines, result.grouped_cartesian_lines));
	//return COMPV_ERROR_CODE_S_OK;

	/* Line grouping (round1)
	This will roughly group the lines to decrease the number of lines and speedup the next calculation
	*/
	//COMPV_CHECK_CODE_RETURN(groupingRound1(result));
	//COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "After groupingRound1: raw_lines=%zu, gouped_lines=%zu", result.raw_hough_lines.size(), result.grouped_hough_lines.size());

	// FIXME(dmi): remove
	result.grouped_hough_lines = result.raw_hough_lines;

	/* Lines subdivision
	*/
	CompVCabLines lines_hz, lines_vt;
	COMPV_CHECK_CODE_RETURN(groupingSubdivision(result, lines_hz, lines_vt));
	//result.grouped_cartesian_lines.assign(lines_hz.lines_cartesian.begin(), lines_hz.lines_cartesian.end());
	if (lines_hz.lines_cartesian.size() < m_nPatternLinesHz || lines_vt.lines_cartesian.size() < m_nPatternLinesVt) { // FIXME(dmi)
		result.grouped_cartesian_lines.assign(lines_hz.lines_cartesian.begin(), lines_hz.lines_cartesian.end()); // FIXME(dmi): remove
		result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Line grouping (round2) and sorting */
	CompVLineFloat32Vector lines_hz_grouped, lines_vt_grouped;
	/*if (lines_hz.lines_cartesian.size() > m_nPatternLinesHz) {
		COMPV_CHECK_CODE_RETURN(groupingRound2(image_width, image_height, lines_hz, lines_hz_grouped));
		if (lines_hz_grouped.size() < m_nPatternLinesHz) {
			//--lines_hz_grouped = lines_hz.lines_cartesian; // FIXME(dmi): uncomment
		}
	}
	else {
		lines_hz_grouped = lines_hz.lines_cartesian;
	}*/
	
	if (lines_vt.lines_cartesian.size() > m_nPatternLinesVt) {
		COMPV_CHECK_CODE_RETURN(groupingRound2(image_width, image_height, lines_vt, lines_vt_grouped));
		if (lines_vt_grouped.size() < m_nPatternLinesVt) {
			//--lines_vt_grouped = lines_vt.lines_cartesian; // FIXME(dmi): uncomment
		}
	}
	else {
		lines_vt_grouped = lines_vt.lines_cartesian;
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "lines_hz_grouped.size=%zu, lines_vt_grouped.size=%zu", lines_hz_grouped.size(), lines_vt_grouped.size());
	//lines_hz_grouped.resize(m_nPatternLinesHz);
	//lines_vt_grouped.resize(m_nPatternLinesVt);
	lines_vt_grouped.reserve(lines_hz_grouped.size() + lines_vt_grouped.size());
	result.grouped_cartesian_lines.assign(lines_hz_grouped.begin(), lines_hz_grouped.end());
	result.grouped_cartesian_lines.insert(result.grouped_cartesian_lines.end(), lines_vt_grouped.begin(), lines_vt_grouped.end());

	return COMPV_ERROR_CODE_S_OK;
}

// Raw grouping to shorten the number of lines and speedup claculation
// FIXME(dmi): remove?
COMPV_ERROR_CODE CompVCalibCamera::groupingRound1(CompVCalibCameraResult& result)
{
	COMPV_CHECK_EXP_RETURN(result.raw_hough_lines.size() < m_nPatternLinesTotal, COMPV_ERROR_CODE_E_INVALID_STATE, "No enought points");
	CompVHoughLineGroupVector groups;
	const size_t image_width = result.edges->cols();
	const size_t image_height = result.edges->rows();
	const compv_float32_t r = std::sqrt(static_cast<compv_float32_t>((image_width * image_width) + (image_height * image_height)));
	compv_float32_t rsmall = kSmallRhoFactRound1 * r;
	compv_float32_t asmall = kSmallAngle;
	const compv_float32_t rsmallMax = rsmall * 2.0f;
	compv_float32_t adiff;
	bool grouped;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove");

	// Build groups using angles and rho (will be increased)
	do {
		groups.clear();
		const CompVHoughLineVector& raw_hough_lines = result.grouped_hough_lines.empty() ? result.raw_hough_lines : result.grouped_hough_lines;
		for (CompVHoughLineVector::const_iterator i = raw_hough_lines.begin(); i < raw_hough_lines.end(); ++i) {
			CompVHoughLineGroup* group = nullptr;
			for (CompVHoughLineGroupVector::iterator g = groups.begin(); g < groups.end(); ++g) {
				adiff = std::abs(g->pivot->theta - i->theta);
				if (adiff < asmall && std::abs(g->pivot->rho - i->rho) <= rsmall) {
					group = &(*g);
					break;
				}
			}
			if (!group) {
				CompVHoughLineGroup g;
				g.pivot = &(*i);
				groups.push_back(g);
				group = &groups[groups.size() - 1];
			}
			group->lines.push_back(*i);
		}

		// Return if no enough groups
		if (groups.size() < m_nPatternLinesTotal) {
			if (result.grouped_hough_lines.size() >= m_nPatternLinesTotal) {
				break; // we have enough lines from previous try
			}
			result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
			return COMPV_ERROR_CODE_S_OK;
		}

		// Increase rsmall and decrease asmall for next loop
		rsmall *= 2.f;
		asmall /= 2.f;

		// Update 'rho' and 'theta' for the groups using strenght
		result.grouped_hough_lines.clear();
		result.grouped_hough_lines.reserve(groups.size());
		grouped = false;
		for (CompVHoughLineGroupVector::const_iterator g = groups.begin(); g < groups.end(); ++g) {
			if (g->lines.size() > 1) {
				size_t strength_sum = 0;
				for (CompVHoughLineVector::const_iterator i = g->lines.begin(); i < g->lines.end(); ++i) {
					strength_sum += i->strength;
				}
				const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(strength_sum);
				compv_float32_t rho = 0;
				compv_float32_t theta = 0;
				for (CompVHoughLineVector::const_iterator i = g->lines.begin(); i < g->lines.end(); ++i) {
					// FIXME(dmi): pre-compute strength_scale = (i->strength) * scale)
					rho += (((i->rho) * i->strength) * scale);
					theta += ((i->theta) * i->strength) * scale;
				}

				result.grouped_hough_lines.push_back(CompVHoughLine(
					rho, theta, strength_sum
				));
				grouped = true;
			}
			else {
				result.grouped_hough_lines.push_back(*g->lines.begin());
			}
		}
	} while (grouped && (rsmall <= rsmallMax));

	return COMPV_ERROR_CODE_S_OK;
}

// Grouping using cartesian distances
// "lines_hough_parallel" must contains lines almost parallel so that the distances are meaningful
COMPV_ERROR_CODE CompVCalibCamera::groupingRound2(const size_t image_width, const size_t image_height, const CompVCabLines& lines_parallel, CompVLineFloat32Vector& lines_parallel_grouped)
{
	lines_parallel_grouped.clear();

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Converting to cartesian again while already done");

	// Group using distance to the origine point (x0,y0) = (0, 0)
	// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
	CompVCabLineGroupVector groups;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Using distance approximation: sqrt (c**2 + d**2) = abs(c + d)");

	const compv_float32_t image_widthF = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightF = static_cast<compv_float32_t>(image_height);

	// Compute distance
	std::vector<compv_float32_t> distances;
	compv_float32_t distance, c, d, distance_sum = 0;
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("remove distance_sum");

	distances.reserve(lines_parallel.lines_cartesian.size());
	for (CompVLineFloat32Vector::const_iterator i = lines_parallel.lines_cartesian.begin(); i < lines_parallel.lines_cartesian.end(); ++i) {
		// x1 = a.x, y1 = a.y
		// x2 = b.x, y2 = b.y
		c = (i->b.y - i->a.y);
		d = (i->b.x - i->a.x);
		distance = std::abs(((i->b.x * i->a.y) - (i->b.y * i->a.x))
			/ std::sqrt((c * c) + (d * d)));
		if (std::isnan(distance)) {
			int kaka = 0;
		}
		distance_sum += distance;
		distances.push_back(distance);
	}

#if 1
	const compv_float32_t r = std::sqrt(static_cast<compv_float32_t>((image_width * image_width) + (image_height * image_height)));
	const compv_float32_t rsmall = 5.f;// kSmallRhoFactRound1 * r;
#else
	const compv_float32_t rsmall = (distance_sum / distances.size()) / 10.f;
#endif
	
	// Grouping
	CompVHoughLineVector::const_iterator it_hough = lines_parallel.lines_hough.begin();
	std::vector<compv_float32_t>::const_iterator it_distance = distances.begin();
	for (CompVLineFloat32Vector::const_iterator i = lines_parallel.lines_cartesian.begin(); i < lines_parallel.lines_cartesian.end(); ++i, ++it_hough, ++it_distance) {
		CompVCabLineGroup* group = nullptr;
		for (CompVCabLineGroupVector::iterator g = groups.begin(); g < groups.end(); ++g) {
			c = (g->pivot_cartesian->b.y - g->pivot_cartesian->a.y);
			d = (g->pivot_cartesian->b.x - g->pivot_cartesian->a.x);
			distance = std::abs(((g->pivot_cartesian->b.x * g->pivot_cartesian->a.y) - (g->pivot_cartesian->b.y * g->pivot_cartesian->a.x))
				/ std::sqrt((c * c) + (d * d)));
			//if (std::abs(distance - *it_distance) < rsmall) {
				//group = &(*g);
				//break;
			//}
			compv_float32_t i_x, i_y;
			int intersect = get_line_intersection(g->pivot_cartesian->a.x, g->pivot_cartesian->a.y, g->pivot_cartesian->b.x, g->pivot_cartesian->b.y,
				i->a.x, i->a.y, i->b.x, i->b.y, &i_x, &i_y);
			COMPV_DEBUG_INFO_CODE_FOR_TESTING("Also check intersection angle");
			if (intersect && i_x >= 0.f && i_y >= 0.f && i_x < image_widthF && i_y < image_heightF) { // FIXME(dmi): also check intersection angle
				group = &(*g);
				break;
			}
		}
		if (!group) {
			CompVCabLineGroup g;
			g.pivot_cartesian = &(*i);
			g.pivot_hough = &(*it_hough);
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

// Subdivide the lines in two groups: those parallel to the strongest lines and those vertical
COMPV_ERROR_CODE CompVCalibCamera::groupingSubdivision(CompVCalibCameraResult& result, CompVCabLines& lines_hz, CompVCabLines& lines_vt)
{
	COMPV_CHECK_EXP_RETURN(result.grouped_hough_lines.size() < m_nPatternLinesTotal, COMPV_ERROR_CODE_E_INVALID_STATE, "No enought points");
	
#if 1
	const size_t image_width = result.edges->cols();
	const size_t image_height = result.edges->rows();
	const compv_float32_t image_widthf = static_cast<compv_float32_t>(image_width);
	const compv_float32_t image_heightf = static_cast<compv_float32_t>(image_height);

	// Convert from Polar to cartesian
	CompVLineFloat32Vector lines_cartesian;
	COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(image_width, image_height, result.grouped_hough_lines, lines_cartesian));

	// Find the strongest line
	size_t lines_cartesian_strongestIndex = 0, strongestStrength = 0, index;
	index = 0;
	for (CompVHoughLineVector::const_iterator i = result.grouped_hough_lines.begin(); i < result.grouped_hough_lines.end(); ++i, ++index) {
		if (i->strength > strongestStrength) {
			strongestStrength = i->strength;
			lines_cartesian_strongestIndex = index;
		}
	}
	const CompVLineFloat32& lines_cartesian_strongest = lines_cartesian[lines_cartesian_strongestIndex];
	CompVCabLines *p_lines_hz, *p_lines_vt;

	// FIXME(dmi): use cross-product instead of slopes https://math.stackexchange.com/questions/1858274/how-to-determine-if-two-lines-are-parallel-almost-parallel
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Use cross-product which is faster -> no arctan or sin comparisons");

	// Find the angle between the strongest line and other lines
	// When a line is vertical, (a.x == b.x) and slope is undefined (infinine which means atan(slope) is pi/2)
	CompVHoughLineVector::const_iterator it_hough = result.grouped_hough_lines.begin();
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
	for (CompVLineFloat32Vector::const_iterator i = lines_cartesian.begin(); i < lines_cartesian.end(); ++i, ++it_hough) {
		angle = std::atan2((i->b.y - i->a.y), (i->b.x - i->a.x)); // inclinaison angle, within [-pi/2, pi/2]
		//COMPV_DEBUG_INFO("angle=%f, %f", COMPV_MATH_RADIAN_TO_DEGREE_FLOAT(st_angle), COMPV_MATH_RADIAN_TO_DEGREE_FLOAT(angle));
		angle_diff = std::abs(st_angle - angle); // within [0, pi]
		angle_sin = std::sinf(angle_diff); // within [0, 1]
		if (angle_sin > 0.5f) {
			// almost parallel
			//COMPV_DEBUG_INFO("****angle_diff=%f, angle_sin=%f", COMPV_MATH_RADIAN_TO_DEGREE_FLOAT(angle_diff), angle_sin);
			p_lines_hz->lines_hough.push_back(*it_hough);
			p_lines_hz->lines_cartesian.push_back(*i);
		}
		else {
			p_lines_vt->lines_hough.push_back(*it_hough);
			p_lines_vt->lines_cartesian.push_back(*i);
		}
	}
#else
	for (CompVHoughLineVector::const_iterator i = result.grouped_hough_lines.begin(); i < result.grouped_hough_lines.end(); ++i) {
		if (std::sinf(i->theta) > 0.5f) { // theta is within [0,pi] -> sin(theta) within [0,1]
			lines_hz.push_back(*i);
		}
		else {
			lines_vt.push_back(*i);
		}
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCalibCamera::lineBestFit(const CompVLineFloat32Vector& points_cartesian, const CompVHoughLineVector& points_hough, CompVLineFloat32& line)
{
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() < 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Need at least #2 points");
	COMPV_CHECK_EXP_RETURN(points_cartesian.size() != points_hough.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Must have same number of points for polar and cartesian points");
	
#if 0
	const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(points_cartesian.size() * 2);

	// Step 1: Calculate the mean of the x-values and the mean of the y-values.
	compv_float32_t mean_x = 0.f, mean_y = 0.f;
	for (CompVLineFloat32Vector::const_iterator i = points_cartesian.begin(); i < points_cartesian.end(); ++i) {
		mean_x += i->a.x + i->b.x;
		mean_y += i->a.y + i->b.y;
	}
	mean_x *= scale;
	mean_y *= scale;

	// Step 2: Calculate the slope
	compv_float32_t t0 = 0.f, t1 = 0.f, t2_a, t2_b;
	for (CompVLineFloat32Vector::const_iterator i = points_cartesian.begin(); i < points_cartesian.end(); ++i) {
		t2_a = (i->a.x - mean_x);
		t2_b = (i->b.x - mean_x);
		t0 += (t2_a * (i->a.y - mean_y)) + (t2_b * (i->b.y - mean_y));
		t1 += (t2_a * t2_a) + (t2_b * t2_b);
	}

	const compv_float32_t m = (t1 == 0.f) ? 0.f : (t0 / t1);

	// Step 3: Compute the y-intercept
	const compv_float32_t intercept = mean_y - (m * mean_x);

	// Step 4: compute the best line using equation 'y = mx + b'
	const CompVLineFloat32& line_ref = points_cartesian[0];
	line.a.x = line_ref.a.x;
	line.a.z = line_ref.a.z;
	line.b.x = line_ref.b.x;
	line.b.z = line_ref.b.z;
	line.a.y = (m * line.a.x) + intercept;
	line.b.y = (m * line.b.x) + intercept;
#elif 0
	const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(points_cartesian.size() * 2);

	compv_float32_t mean_y = 0.f;
	for (CompVLineFloat32Vector::const_iterator i = points_cartesian.begin(); i < points_cartesian.end(); ++i) {
		mean_y += i->a.y + i->b.y;
	}
	mean_y *= scale;
	
	compv_float32_t t0 = 0.f;
	for (CompVLineFloat32Vector::const_iterator i = points_cartesian.begin(); i < points_cartesian.end(); ++i) {
		t0 += ((i->a.y - mean_y)) + ((i->b.y - mean_y));
	}
	const compv_float32_t intercept = (t0 * mean_y);

	line = points_cartesian[0];
	line.a.y += intercept;
	line.b.y += intercept;
#elif 1
	// Implementing "Least Square Method" (https://www.varsitytutors.com/hotmath/hotmath_help/topics/line-of-best-fit)
	// while ignoring the x-component for the simple reason that they are always constant
	// when using CompV's KHT and SHT implementations (a.x = 0 and b.x = image_width)
	const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(points_cartesian.size() * 2);

	compv_float32_t mean_y = 0.f;
	for (CompVLineFloat32Vector::const_iterator i = points_cartesian.begin(); i < points_cartesian.end(); ++i) {
		mean_y += i->a.y + i->b.y;
	}
	mean_y *= scale;

	compv_float32_t t0 = 0.f;
	for (CompVLineFloat32Vector::const_iterator i = points_cartesian.begin(); i < points_cartesian.end(); ++i) {
		t0 += ((i->a.y - mean_y)) + ((i->b.y - mean_y));
	}
	line = points_cartesian[0];
	line.a.y += (line.a.y * t0);
	line.b.y += (line.b.y * t0);
#elif 1
	CompVLineFloat32Vector::const_iterator i;
	CompVHoughLineVector::const_iterator j;

	compv_float32_t sum_strengths = 0.f;
	for (j = points_hough.begin(); j < points_hough.end(); ++j) {
		sum_strengths += j->strength;
	}
	const compv_float32_t scale_strengths = (1.f / sum_strengths);

	compv_float32_t mean_ya = 0.f, mean_yb = 0.f;
	compv_float32_t scale_strength;
	for (i = points_cartesian.begin(), j = points_hough.begin(); i < points_cartesian.end(); ++i, ++j) {
		scale_strength = (j->strength * scale_strengths);
		mean_ya += (i->a.y * scale_strength);
		mean_yb += (i->b.y * scale_strength);
	}

	line = points_cartesian[0];
	line.a.y = mean_ya;
	line.b.y = mean_yb;
#endif
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
