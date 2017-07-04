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

#define PATTERN_ROW_CORNERS_NUM			10 // Number of corners per row
#define PATTERN_COL_CORNERS_NUM			8  // Number of corners per column
#define PATTERN_CORNERS_NUM				(PATTERN_ROW_CORNERS_NUM * PATTERN_COL_CORNERS_NUM) // Total number of corners
#define PATTERN_GROUP_MAXLINES			5 // Maximum number of lines per group (errors)

#define HOUGH_RHO						(1.0f * 0.5f) // "rho-delta" (half-pixel)
#define HOUGH_THETA						(kfMathTrigPiOver180 * 0.5f) // "theta-delta" (half-radian)
#define HOUGH_THRESHOLD					1 //150
#define HOUGH_THRESHOLD_FACT			0.0003828					
#define HOUGH_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGH_CLUSTER_MIN_SIZE			10
#define HOUGH_KERNEL_MIN_HEIGTH			0.002f // must be within [0, 1]

#define CANNY_LOW						2.41f
#define CANNY_HIGH						CANNY_LOW*2.f
#define CANNY_KERNEL_SIZE				3

COMPV_NAMESPACE_BEGIN()

struct CompVHoughLineGroup {
	const CompVHoughLine* pivot = nullptr;
	CompVHoughLineVector lines;
};
typedef std::vector<CompVHoughLineGroup> CompVHoughLineGroupVector;

struct CalibLine {
	CompVLineFloat32 line;
	compv_float32_t slope;
	size_t strength;
};
typedef std::vector<CalibLine> CalibLineVector;

struct CalibLineGroup {
	CalibLine pivot;
	CalibLineVector lines;
};
typedef std::vector<CalibLineGroup> CalibLineGroupVector;

static COMPV_ERROR_CODE __linesFromPolarToCartesian(const size_t sceneWidth, const size_t sceneHeight, const CompVHoughLineVector& polar, CalibLineVector& cartesian);

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

// FIXME(dmi): remove (Code from StackOverflow)
// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines 
// intersect the intersection point may be stored in the floats i_x and i_y.
int get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
	float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
	float s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

	float s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		// Collision detected
		if (i_x != NULL)
			*i_x = p0_x + (t * s1_x);
		if (i_y != NULL)
			*i_y = p0_y + (t * s1_y);
		return 1;
	}

	return 0; // No collision
}

COMPV_ERROR_CODE CompVCalibCamera::process(const CompVMatPtr& image, CompVCalibCameraResult& result)
{
	COMPV_CHECK_EXP_RETURN(!image || image->subType() != COMPV_SUBTYPE_PIXELS_Y, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input image is null or not in grayscale format");

	// Reset the previous result
	result.reset();

	// Canny edge detection
	COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(image, &result.edges));

	// Hough lines
	//COMPV_CHECK_CODE_RETURN(m_ptrHough->setInt(COMPV_HOUGH_SET_INT_THRESHOLD, static_cast<int>(static_cast<double>(image->cols() * image->rows()) * HOUGH_THRESHOLD_FACT) + 1));
	COMPV_CHECK_CODE_RETURN(m_ptrHough->process(result.edges, result.hough_lines));
	if (result.hough_lines.size() < m_nPatternLinesTotal) {
		result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		return COMPV_ERROR_CODE_S_OK;
	}

	// Convert from polar to cartesian coordinates
	CalibLineVector cartesian;
	COMPV_CHECK_CODE_RETURN(__linesFromPolarToCartesian(image->cols(), image->rows(), result.hough_lines, cartesian));

#if 0
	result.grouped_lines = result.raw_lines;
#elif 1
	CalibLineGroupVector groupes;
	static const compv_float32_t small_angle = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(15); // FIXME(dmi): make parameter
	const compv_float32_t imageWidthF = static_cast<compv_float32_t>(image->cols());
	const compv_float32_t imageHeightF = static_cast<compv_float32_t>(image->rows());
	for (CalibLineVector::const_iterator i = cartesian.begin(); i < cartesian.end(); ++i) {
		// compute slope: https://en.wikipedia.org/wiki/Slope
		CalibLineGroupVector::iterator group = groupes.end();
		for (CalibLineGroupVector::iterator g = groupes.begin(); g < groupes.end(); ++g) {
			compv_float32_t intersect_angle = std::atan2f((i->slope - g->pivot.slope), (1 - (i->slope*g->pivot.slope)));
			if (intersect_angle < 0) intersect_angle += kfMathTrigPi;
			const compv_float32_t intersect_angle_degree = COMPV_MATH_RADIAN_TO_DEGREE_FLOAT(intersect_angle);
			if (intersect_angle < small_angle) {
				float ix, iy;
				int intersect = get_line_intersection(
					i->line.a.x, i->line.a.y, i->line.b.x, i->line.b.y,
					g->pivot.line.a.x, g->pivot.line.a.y, g->pivot.line.b.x, g->pivot.line.b.y,
					&ix, &iy);
				if (intersect && ix >=0 && ix < imageWidthF && iy >= 0 && iy < imageHeightF) {
					group = g;
					break;
				}
				else {
					int kaka = 0;
				}
			}
		}
		if (group == groupes.end()) {
			CalibLineGroup g;
			g.pivot = *i;
			groupes.push_back(g);
			group = groupes.begin() + (groupes.size() - 1);
		}
		
		group->lines.push_back(*i);
	}

#	if 1
	CalibLineVector clines;
	for (CalibLineGroupVector::iterator g = groupes.begin(); g < groupes.end(); ++g) {
		size_t strength_sum = 0;

		for (CalibLineVector::const_iterator i = g->lines.begin(); i < g->lines.end(); ++i) {
			strength_sum += i->strength;
		}
		const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(strength_sum);
		CalibLine cline;
		for (CalibLineVector::const_iterator i = g->lines.begin(); i < g->lines.end(); ++i) {
			// FIXME(dmi): pre-compute (i->strength * scale)
			cline.line.a.x += (((i->line.a.x) * i->strength) * scale);
			cline.line.a.y += (((i->line.a.y) * i->strength) * scale);
			cline.line.b.x += (((i->line.b.x) * i->strength) * scale);
			cline.line.b.y += (((i->line.b.y) * i->strength) * scale);
		}
		cline.strength = strength_sum;
		clines.push_back(cline);
	}
	std::sort(clines.begin(), clines.end(), [](const CalibLine &line1, const CalibLine &line2) {
		return (line1.strength > line2.strength);
	});
	for (CalibLineVector::const_iterator i = clines.begin(); i < clines.end(); ++i) {
		result.grouped_lines.push_back(i->line);
	}
	//result.grouped_lines.resize(m_nPatternLinesTotal);
#	else

	// FIXME
	size_t groupes_size = groupes.size();
	for (CalibLineGroupVector::const_iterator i = groupes.begin(); i < groupes.end(); ++i) {
		result.grouped_lines.push_back(i->lines[0].line);
	}

	for (CalibLineVector::const_iterator i = cartesian.begin(); i < cartesian.end(); ++i) {
		//result.grouped_lines.push_back(i->line);
	}
#	endif

#elif 1
	CompVHoughLineGroupVector groups;

	static const float k10DegreeInRad = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(10.0);
	const double r = std::sqrt(static_cast<double>((image->cols() * image->cols()) + (image->rows() * image->rows())));
	const double rhalf = 0.5 * r;
	const double rsmall = r * 0.01875;

	// 'rho' values are within [-r/2 - r/2] -> change the system coord. to [0-r] by adding r/2 to rho
	// 'theta' values are within [0 - 180]

	for (CompVHoughLineVector::const_iterator i = result.raw_lines.begin(); i < result.raw_lines.end(); ++i) {
		const double i_rho = (i->rho + rhalf);
		CompVHoughLineGroup* group = nullptr;
		for (CompVHoughLineGroupVector::iterator g = groups.begin(); g < groups.end(); ++g) {
			const double j_rho = (g->pivot->rho + rhalf);
			if (std::abs(g->pivot->theta - i->theta) < k10DegreeInRad && std::abs(j_rho - i_rho) < rsmall) {
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

	if (groups.size() < m_nPatternLinesTotal) {
		result.code = COMPV_CALIB_CAMERA_RESULT_NO_ENOUGH_POINTS;
		//return COMPV_ERROR_CODE_S_OK;
	}

	for (CompVHoughLineGroupVector::const_iterator g = groups.begin(); g < groups.end(); ++g) {
		size_t strength_sum = 0;
		for (CompVHoughLineVector::const_iterator i = g->lines.begin(); i < g->lines.end(); ++i) {
			strength_sum += i->strength;
		}
		const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(strength_sum);
		compv_float32_t rho = 0;
		compv_float32_t theta = 0;
		for (CompVHoughLineVector::const_iterator i = g->lines.begin(); i < g->lines.end(); ++i) {
			rho += (((i->rho) * i->strength) * scale);
			theta += ((i->theta) * i->strength) * scale;
		}

		result.grouped_lines.push_back(CompVHoughLine(
			rho, theta, strength_sum 
		));
	}

	std::sort(result.grouped_lines.begin(), result.grouped_lines.end(), [](const CompVHoughLine &line1, const CompVHoughLine &line2) {
		return (line1.strength > line2.strength);
	});

	const double r_scale = 1.0 / r;
	const double theta_scale = (1.0 / COMPV_MATH_PI) * (r / COMPV_MATH_PI);

	if (result.grouped_lines.size() > m_nPatternLinesTotal) {
		for (CompVHoughLineVector::const_iterator g_orphans = (result.grouped_lines.begin() + m_nPatternLinesTotal); g_orphans < result.grouped_lines.end(); ++g_orphans) {
			const double g_orphans_rho = (g_orphans->rho + rhalf);
			// Find the closest group
			double min_score = DBL_MAX;
			CompVHoughLineVector::iterator min_g1;
			for (CompVHoughLineVector::iterator g1 = result.grouped_lines.begin(); g1 < (result.grouped_lines.begin() + m_nPatternLinesTotal); ++g1) {
				const double g1_rho = (g1->rho + rhalf);
				const double diff_rho = std::abs(g_orphans_rho - g1_rho);
				const double diff_theta = std::abs(g_orphans->theta - g1->theta);
				const double diff_score = (diff_rho * r_scale) + (diff_theta * theta_scale); // should be within [0, 1]
				if (diff_score < min_score) {
					min_score = diff_score;
					min_g1 = g1;
				}
			}
#if 0
			// Merge to the closest group
			const size_t strength_sum = g_orphans->strength + min_g1->strength;
			const compv_float32_t scale = 1.f / static_cast<compv_float32_t>(strength_sum);
			min_g1->rho = (((min_g1->rho) * min_g1->strength) * scale) + (((g_orphans->rho) * g_orphans->strength) * scale);
			min_g1->theta = (((min_g1->theta) * min_g1->strength) * scale) + (((g_orphans->theta) * g_orphans->strength) * scale);
			min_g1->strength += g_orphans->strength;
#endif
		}
	}

	result.grouped_lines.resize(m_nPatternLinesTotal);

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

static COMPV_ERROR_CODE __linesFromPolarToCartesian(const size_t sceneWidth, const size_t sceneHeight, const CompVHoughLineVector& polar, CalibLineVector& cartesian)
{
	cartesian.clear();
#if 1
	const compv_float32_t widthF = static_cast<compv_float32_t>(sceneWidth);
	const compv_float32_t heightF = static_cast<compv_float32_t>(sceneHeight);
	const compv_float32_t half_widthF = widthF * 0.5f;
	const compv_float32_t half_heightF = heightF * 0.5f;
	for (CompVHoughLineVector::const_iterator i = polar.begin(); i < polar.end(); ++i) {
		const compv_float32_t rho = i->rho;
		const compv_float32_t theta = i->theta;
		const compv_float32_t a = std::cos(theta), b = 1.f / std::sin(theta);
		CalibLine cline;
		cline.line.a.x = 0;
		cline.line.a.y = ((rho + (half_widthF * a)) * b) + half_heightF;
		cline.line.b.x = widthF;
		cline.line.b.y = ((rho - (half_widthF * a)) * b) + half_heightF;
		// slope: https://en.wikipedia.org/wiki/Slope
		cline.slope = (cline.line.b.y - cline.line.a.y) / (cline.line.b.x - cline.line.a.x);
		cartesian.push_back(cline);
		cline.strength = i->strength;
	}
#else
	const compv_float32_t rhalf = std::sqrt(static_cast<compv_float32_t>((sceneWidth * sceneWidth) + (sceneHeight * sceneHeight))) * 0.5f;
	const compv_float32_t widthF = static_cast<compv_float32_t>(sceneWidth);
	for (CompVHoughLineVector::const_iterator i = polar.begin(); i < polar.end(); ++i) {
		const compv_float32_t rho = i->rho + rhalf;
		const compv_float32_t theta = i->theta;
		const compv_float32_t a = std::cos(theta), b = 1.f / std::sin(theta);
		CalibLine cline;
		cline.line.a.x = 0;
		cline.line.a.y = ((rho + (cline.line.a.x * a)) * b);
		cline.line.b.x = widthF;
		cline.line.b.y = ((rho - (cline.line.b.x * a)) * b);
		// slope: https://en.wikipedia.org/wiki/Slope
		cline.slope = (cline.line.b.y - cline.line.a.y) / (cline.line.b.x - cline.line.a.x);
		cartesian.push_back(cline);
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
