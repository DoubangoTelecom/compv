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

COMPV_NAMESPACE_BEGIN()

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
	static COMPV_ERROR_CODE line(const CompVMatPtr& points, CompVMatPtrPtr params)
	{
		// Internal function, no need to check input parameters
		
		CompVMathStatsFitGenericOpaque opaque(points);
		static const size_t minModelPoints = 2;
		CompVMathStatsRansacControl<FloatType> control(4, points->cols(), minModelPoints, &opaque);
		CompVMathStatsRansacStatus<FloatType> status;
		COMPV_CHECK_CODE_RETURN(CompVMathStatsRansac::process(
			&control, &status,
			buildModelParamsLine, buildResidualsLine
		));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(params, 1, 2));
		*(*params)->ptr<FloatType>(0, 0) = status.modelParamsBest[0];
		*(*params)->ptr<FloatType>(0, 1) = status.modelParamsBest[1];
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

		modelParams.resize(2);
		const CompVMathStatsFitGenericOpaque* opaque_ = reinterpret_cast<const CompVMathStatsFitGenericOpaque*>(control->opaque);
		const CompVMatPtr& points = opaque_->points;
		const FloatType* pointsXPtr = points->ptr<const FloatType>(0);
		const FloatType* pointsYPtr = points->ptr<const FloatType>(1);

		if (modelIndices.size() == 2) { // Ransac calling on probing #2 random points
			COMPV_DEBUG_INFO_CODE_TODO("Not handling vertical lines");
			const FloatType& x0 = pointsXPtr[modelIndices[0]];
			const FloatType& y0 = pointsYPtr[modelIndices[0]];
			const FloatType& x1 = pointsXPtr[modelIndices[1]];
			const FloatType& y1 = pointsYPtr[modelIndices[1]];
#if 0
			if (x0 == x1) {
				// Vertical line
				modelParams[0] = 0; // slope
				modelParams[1] = x0; // intercept
			}
			else
#endif
			{
				modelParams[0] = (y1 - y0) / (x1 - x0); // slope
				modelParams[1] = y0 - (modelParams[0] * x0); // intercept
			}
		}
		else { // Ransac calling on probing #n inliers
			COMPV_DEBUG_INFO_CODE_TODO("Not handling vertical lines");
			// line best fit: https://www.varsitytutors.com/hotmath/hotmath_help/topics/line-of-best-fit
			FloatType mean_x = 0, mean_y = 0;
			FloatType a, b;
			const size_t count = modelIndices.size();
			for (CompVMathStatsRansacModelIndices::const_iterator i = modelIndices.begin(); i < modelIndices.end(); ++i) {
				mean_x += pointsXPtr[*i];
				mean_y += pointsYPtr[*i];
			}
			const FloatType scale = 1 / static_cast<FloatType>(count);
			mean_x *= scale;
			mean_y *= scale;
			FloatType sum_num = 0.f, sum_den = 0.f;
			for (CompVMathStatsRansacModelIndices::const_iterator i = modelIndices.begin(); i < modelIndices.end(); ++i) {
				a = (pointsXPtr[*i] - mean_x);
				b = (pointsYPtr[*i] - mean_y);
				sum_num += (a * b);
				sum_den += (a * a);
			}
			modelParams[0] = sum_den ? (sum_num / sum_den) : 0.f;
			modelParams[1] = mean_y - (modelParams[0] * mean_x);
		}
		
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE buildResidualsLine(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelParamsFloatType& modelParams, FloatType* residualPtr, bool& userbreak)
	{
		COMPV_CHECK_EXP_RETURN(modelParams.size() != 2, COMPV_ERROR_CODE_E_INVALID_CALL, "A model for a line requires #2 params (slope and intercept)");

		const CompVMathStatsFitGenericOpaque* opaque_ = reinterpret_cast<const CompVMathStatsFitGenericOpaque*>(control->opaque);
		const CompVMatPtr& points = opaque_->points;
		const FloatType* pointsXPtr = points->ptr<const FloatType>(0);
		const FloatType* pointsYPtr = points->ptr<const FloatType>(1);
		const FloatType slope = modelParams[0];
		const FloatType scale = 1 / std::sqrt(1 + (slope * slope));
		const FloatType intercept = modelParams[1];
		const size_t count = points->cols();

		for (size_t i = 0; i < count; ++i) {
			// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Another_formula
			residualPtr[i] = std::abs(intercept + ((slope * pointsXPtr[i]) - pointsYPtr[i])) * scale;
		}

		return COMPV_ERROR_CODE_S_OK;
	}
};

//
//	CompVMathStatsFit
//

COMPV_ERROR_CODE CompVMathStatsFit::line(const CompVMatPtr& points, CompVMatPtrPtr params)
{
	COMPV_CHECK_EXP_RETURN(!points || points->isEmpty() || points->rows() < 2 || !params, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (points->subType()) {
		case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVMathStatsFitGeneric<compv_float32_t>::line(points, params))); break;
		case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVMathStatsFitGeneric<compv_float64_t>::line(points, params))); break;
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
