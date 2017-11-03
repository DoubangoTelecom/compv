/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_stats_fit.h"
#include "compv/base/math/compv_math_stats_ransac.h"
#include "compv/base/math/compv_math_utils.h"


#define COMPV_THIS_CLASSNAME	"CompVMathStatsFit"

// [2] A tutorial on the total least squares method for fitting a straight line and a plane : https://www.researchgate.net/publication/272179120_A_tutorial_on_the_total_least_squares_method_for_fitting_a_straight_line_and_a_plane

COMPV_NAMESPACE_BEGIN()

static const size_t kNumParamsLine = 3; // #3 params (A, B, C): "Ax + By + C = 0"

//
//	CompVMathStatsFitGeneric
//

template<class FloatType>
class CompVMathStatsFitGeneric
{
	struct CompVMathStatsFitGenericOpaque  {
	public:
		const CompVMatPtr& points;
		CompVMathStatsFitGenericOpaque(const CompVMatPtr& points_) 
			: points(points_) {}
	};
public:
	// Output: 3 parameters A, B and C used in standard line equation: Ax + By + C = 0
	static COMPV_ERROR_CODE line(const CompVMatPtr& points, const FloatType threshold, CompVMatPtrPtr params)
	{
		// Internal function, no need to check input parameters

		static const size_t kMinModelPoints = 2;
		
		CompVMathStatsFitGenericOpaque opaque(points);
		CompVMathStatsRansacControl<FloatType> control(threshold, points->cols(), kMinModelPoints, &opaque);
		control.maxIter = COMPV_MATH_MAX_3(control.maxIter, points->cols(), 1000);
		CompVMathStatsRansacStatus<FloatType> status;
		COMPV_CHECK_CODE_RETURN(CompVMathStatsRansac::process(
			&control, &status,
			buildModelParamsLine, buildResidualsLine
		));
		COMPV_CHECK_EXP_RETURN(status.modelParamsBest.size() != kNumParamsLine, COMPV_ERROR_CODE_E_INVALID_STATE, "Ransac for line fitting must return exactly #3 params (A, B, C)");
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(params, 1, kNumParamsLine));
		*(*params)->ptr<FloatType>(0, 0) = status.modelParamsBest[0];
		*(*params)->ptr<FloatType>(0, 1) = status.modelParamsBest[1];
		*(*params)->ptr<FloatType>(0, 2) = status.modelParamsBest[2];
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE parabola(const CompVMatPtr& points, CompVMatPtrPtr params, COMPV_MATH_PARABOLA_TYPE type)
	{
		// Internal function, no need to check input parameters
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	static COMPV_ERROR_CODE buildModelParamsLine(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject)
	{
		COMPV_CHECK_EXP_RETURN(modelIndices.size() < 2, COMPV_ERROR_CODE_E_INVALID_CALL, "A model for a line requires at least #2 points");

		modelParams.resize(kNumParamsLine);
		userReject = false;

		const CompVMathStatsFitGenericOpaque* opaque_ = reinterpret_cast<const CompVMathStatsFitGenericOpaque*>(control->opaque);
		const CompVMatPtr& points = opaque_->points;
		const size_t count = modelIndices.size();
		const FloatType* pointsXPtr = points->ptr<const FloatType>(0);
		const FloatType* pointsYPtr = points->ptr<const FloatType>(1);

		if (count == 2) { /* Ransac calling on probing #2 random points */
			// A, B and C params for standard equation Ax + By + C = 0 
			const FloatType& x0 = pointsXPtr[modelIndices[0]];
			const FloatType& y0 = pointsYPtr[modelIndices[0]];
			const FloatType& x1 = pointsXPtr[modelIndices[1]];
			const FloatType& y1 = pointsYPtr[modelIndices[1]];
			if (x0 == y0 && x1 == y1) {
				COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "Same point, reject");
				userReject = true;
				return COMPV_ERROR_CODE_S_OK;
			}
			// http://daniel.microdor.com/LineEquations.html
			FloatType A = (y1 - y0);
			FloatType B = (x0 - x1);
			FloatType C = (x1 * y0) - (x0 * y1);

			// Normalization (not required)
			// When distance is computed (https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_an_equation) we divide by 
			// std::sqrt((A*A) + (B*B)) which means we can do it now and avoid it alter
			if (A != 0 && B != 0) {
				const FloatType scale = 1 / std::sqrt((A*A) + (B*B));
				A *= scale;
				B *= scale;
				C *= scale;
			}
			modelParams[0] = A;
			modelParams[1] = B;
			modelParams[2] = C;
		}
		else { /* Ransac calling on probing #n>2 inliers */
			// To avoid issues with vertical and horizontal lines as explained here at [1]
			// we use Total Least Squares as per [2]
			// [1]: https://stackoverflow.com/questions/10982387/vertical-line-fit-using-polyfit
			// [2]: https://www.researchgate.net/publication/272179120_A_tutorial_on_the_total_least_squares_method_for_fitting_a_straight_line_and_a_plane

			// Computing mean(x) and mean(y) as per [2] Eq (11)
			FloatType mean_x = 0, mean_y = 0;
			for (CompVMathStatsRansacModelIndices::const_iterator i = modelIndices.begin(); i < modelIndices.end(); ++i) {
				mean_x += pointsXPtr[*i];
				mean_y += pointsYPtr[*i];
			}
			const FloatType scale = 1 / static_cast<FloatType>(count);
			mean_x *= scale;
			mean_y *= scale;

			// Computing Phi as per [2] Eq (16)
			FloatType a, b;
			FloatType sum_num = 0.f, sum_den = 0.f;
			for (CompVMathStatsRansacModelIndices::const_iterator i = modelIndices.begin(); i < modelIndices.end(); ++i) {
				a = (pointsXPtr[*i] - mean_x);
				b = (pointsYPtr[*i] - mean_y);
				sum_num += (a * b);
				sum_den += ((b * b) - (a * a));
			}
			sum_num *= -2;
			// https://en.wikipedia.org/wiki/Atan2#Definition_and_computation
			// artan2(something, 0) is defined but atan2(0, 0) isn't defined
			const bool atan2IsUndefined = (sum_num == 0 && sum_den == 0);
			const FloatType phi = atan2IsUndefined 
				? 0
				: static_cast<FloatType>(0.5) * std::atan2(sum_num, sum_den);

			// Computing r as per [2] Eq (12)
			const FloatType r = (mean_x * std::cos(phi)) + (mean_y * std::sin(phi));

			// Computing A, B, C as per [2] Eq (3)
			modelParams[0] = std::cos(phi);
			modelParams[1] = std::sin(phi);
			modelParams[2] = -r;
			
			COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "buildModelParamsLine(): atan2IsUndefined = %s, A/B/C=(%f, %f, %f))",
				atan2IsUndefined ? "true" : "false",
				modelParams[0], modelParams[1], modelParams[2]);
		}
		
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE buildResidualsLine(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelParamsFloatType& modelParams, FloatType* residualPtr, bool& userbreak)
	{
		COMPV_CHECK_EXP_RETURN(modelParams.size() != kNumParamsLine, COMPV_ERROR_CODE_E_INVALID_CALL, "A model for a line requires #3 params (A, B, C)");

		const CompVMathStatsFitGenericOpaque* opaque_ = reinterpret_cast<const CompVMathStatsFitGenericOpaque*>(control->opaque);
		const CompVMatPtr& points = opaque_->points;
		const FloatType* pointsXPtr = points->ptr<const FloatType>(0);
		const FloatType* pointsYPtr = points->ptr<const FloatType>(1);
		const FloatType A = modelParams[0];
		const FloatType B = modelParams[1];
		const FloatType C = modelParams[2];
		const size_t count = points->cols();

		/* Distance from a point to a line: https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_an_equation */

		if (A == 0 && B != 0) {
			// Horizontal line. No need for SIMD, not common case.
			COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "buildResidualsLine(): Residual for horizontal line");
			const FloatType scale = 1 / std::abs(B);
			for (size_t i = 0; i < count; ++i) {
				residualPtr[i] = std::abs((B * pointsYPtr[i]) + C) * scale;
			}
		}
		else if (B == 0) {
			// Vertical line. No need for SIMD, not common case.
			COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "buildResidualsLine(): Residual for vertical line");
			const FloatType scale = 1 / std::abs(A);
			for (size_t i = 0; i < count; ++i) {
				residualPtr[i] = std::abs((A * pointsXPtr[i]) + C) * scale;
			}
		}
		else {
			// All other cases
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
			// No need for scaling (1/sqrt((a*a) + (b*b))), already done on A, B and C params in buildModelParamsLine
			for (size_t i = 0; i < count; ++i) {
				residualPtr[i] = std::abs((A * pointsXPtr[i]) + (B * pointsYPtr[i]) + C);
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}
};

//
//	CompVMathStatsFit
//

// Output: 3 parameters A, B and C used in standard line equation: Ax + By + C = 0. Not using slope/intercept lines to avoid
// issues with vertical and horizontal lines.
// Using RANSAC and Total Least Squares (http://en.wikipedia.org/wiki/Total_least_squares)
COMPV_ERROR_CODE CompVMathStatsFit::line(const CompVMatPtr& points, const double threshold, CompVMatPtrPtr params)
{
	COMPV_CHECK_EXP_RETURN(!points || points->isEmpty() || points->rows() < 2 || threshold < 0 || !params, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (points->subType()) {
		case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVMathStatsFitGeneric<compv_float32_t>::line(points, static_cast<compv_float32_t>(threshold), params))); break;
		case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVMathStatsFitGeneric<compv_float64_t>::line(points, static_cast<compv_float64_t>(threshold), params))); break;
		default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "points must constain float32 or float64 indices");  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathStatsFit::parabola(const CompVMatPtr& points, CompVMatPtrPtr params, COMPV_MATH_PARABOLA_TYPE type COMPV_DEFAULT(COMPV_MATH_PARABOLA_TYPE_REGULAR))
{
	COMPV_CHECK_EXP_RETURN(!points || points->isEmpty() || points->rows() < 3 || !params, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (points->subType()) {
		case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVMathStatsFitGeneric<compv_float32_t>::parabola(points, params, type))); break;
		case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVMathStatsFitGeneric<compv_float64_t>::parabola(points, params, type))); break;
		default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "points must constain float32 or float64 indices");  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
