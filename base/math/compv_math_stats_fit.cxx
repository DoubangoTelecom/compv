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
		CompVMathStatsRansacControl control;
		CompVMathStatsRansacStatus<FloatType> status;
		COMPV_CHECK_CODE_RETURN(CompVMathStatsRansac::process(
			&opaque, points->cols(), 2, 
			buildModelParamsLine, evalModelParamsLine,
			&control, &status
		));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(params, 1, 2));
		*(*params)->ptr<FloatType>(0, 0) = status.modelParamsBest[0];
		*(*params)->ptr<FloatType>(0, 1) = status.modelParamsBest[0];
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE parabola(const CompVMatPtr& points, CompVMatPtrPtr params, COMPV_MATH_PARABOLA_TYPE type)
	{
		// Internal function, no need to check input parameters
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	static COMPV_ERROR_CODE buildModelParamsLine(const void* opaque, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject)
	{
		COMPV_CHECK_EXP_RETURN(modelIndices.size() != 2, COMPV_ERROR_CODE_E_INVALID_CALL, "A model for a line requires #2 points");

		modelParams.resize(2);
		const CompVMathStatsFitGenericOpaque* opaque_ = reinterpret_cast<const CompVMathStatsFitGenericOpaque*>(opaque);
		const CompVMatPtr& points = opaque_->points;
		const FloatType* pointsXPtr = points->ptr<const FloatType>(0);
		const FloatType* pointsYPtr = points->ptr<const FloatType>(1);
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

		COMPV_DEBUG_INFO_CODE_TODO("Not handling vertical lines");
		
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE evalModelParamsLine(const void* opaque, const CompVMathStatsRansacModelParamsFloatType& modelParams, FloatType* residualPtr, bool& userbreak)
	{
		COMPV_CHECK_EXP_RETURN(modelParams.size() != 2, COMPV_ERROR_CODE_E_INVALID_CALL, "A model for a line requires #2 params (slope and intercept)");

		const CompVMathStatsFitGenericOpaque* opaque_ = reinterpret_cast<const CompVMathStatsFitGenericOpaque*>(opaque);
		const CompVMatPtr& points = opaque_->points;
		const FloatType* pointsXPtr = points->ptr<const FloatType>(0);
		const FloatType* pointsYPtr = points->ptr<const FloatType>(1);
		const FloatType slope = modelParams[0];
		const FloatType intercept = modelParams[1];
		const size_t count = points->cols();

		for (size_t i = 0; i < count; ++i) {
			// y = f(x) = slope*x + intercept
			// residual = (y - f(x)) = y - 
			residualPtr[i] = (pointsYPtr[i] - ((slope * pointsXPtr[i]) + intercept));
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
