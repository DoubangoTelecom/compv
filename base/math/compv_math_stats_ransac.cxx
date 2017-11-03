/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_stats_ransac.h"
#include "compv/base/math/compv_math_utils.h"

#include <random>

#define COMPV_THIS_CLASSNAME	"CompVMathStatsRansac"

COMPV_NAMESPACE_BEGIN()


//
//	CompVMathStatsRansacGeneric
//

class CompVMathStatsRansacGeneric
{
public:
	template <typename FloatType>
	static COMPV_ERROR_CODE process(
		const CompVMathStatsRansacControl<FloatType>* control, CompVMathStatsRansacStatus<FloatType>* status,
		COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject),
		COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelParamsFloatType& modelParams, FloatType* residualPtr, bool& userbreak)
	)
	{
		COMPV_CHECK_EXP_RETURN(!control || !status || !control->isValid() || !buildModelParams || !buildResiduals,
			COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		COMPV_DEBUG_INFO_CODE_TODO("Add TieBreaker"); // TieBreaker callback function: when number of inliers are equal then, compare variances
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");

		CompVMathStatsRansacModelParamsFloatType& modelParamsBest = status->modelParamsBest;
		modelParamsBest.clear();

		// Save control values to make sure won't change while we're doing ransac processing
		const FloatType threshold = control->threshold;
		const size_t maxIter = control->maxIter;
		const size_t minModelPoints = control->minModelPoints;
		const size_t totalPoints = control->totalPoints;
		const FloatType probInliersOnly = control->probInliersOnly;
		
		const size_t minInliersToStopIter = static_cast<size_t>(probInliersOnly * totalPoints); // minimum number of inliers to stop the iteration
		const FloatType totalPointsFloat = static_cast<FloatType>(totalPoints);
		const FloatType totalPointsFloatScale = 1 / totalPointsFloat;
		const FloatType minModelPointsFloat = static_cast<FloatType>(minModelPoints);
		size_t numIter = 0;
		size_t bestNumInliers = 0;
		size_t newMaxIter = maxIter;

		static const FloatType eps = std::numeric_limits<FloatType>::epsilon();
		const FloatType numerator = std::log(std::max(1 - probInliersOnly, eps));
		FloatType denominator, w;

		std::random_device rand_device;
		std::mt19937 prng{ rand_device() }; // TODO(dmi): one random device per thread
		std::uniform_int_distribution<size_t> unif_dist{ 0, static_cast<size_t>(totalPoints - 1) };
		size_t indice;
		CompVMathStatsRansacModelIndices indices;

		/* Create residual */
		CompVMatPtr residual;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(&residual, 1, totalPoints));
		FloatType* residualPtr = residual->ptr<FloatType>();

		do {
			/* Build random indices for the model */
			indices.clear();
			while (indices.size() < minModelPoints) {
				indice = unif_dist(prng);
				CompVMathStatsRansacModelIndices::const_iterator i;
				for (i = indices.begin(); i < indices.end(); ++i) {
					if (*i == indice) {
						break;
					}
				}
				if (i == indices.end()) {
					indices.push_back(indice);
				}
			}

			/* Build model params */
			bool userReject = false;
			CompVMathStatsRansacModelParamsFloatType params;
			COMPV_CHECK_CODE_RETURN(buildModelParams(
				control, indices, params, userReject
			));
			if (userReject) {
				// got while, increment(numIter) then try again
				continue;
			}

			/* Eval model params: build residual (must be >= 0) */
			bool userBreak = false;
			COMPV_CHECK_CODE_RETURN(buildResiduals(
				control, params, residualPtr, userBreak
			));

			/* Check residual */
			int numInliers = 0;
			for (size_t i = 0; i < totalPoints; ++i) {
				numInliers += (residualPtr[i] < threshold);
			}
			if (numInliers > bestNumInliers) {
				bestNumInliers = static_cast<size_t>(numInliers);
				modelParamsBest = params;
			}

			/* Update RANSAC loop control data */
			w = (totalPointsFloat - numInliers) * totalPointsFloatScale;
			denominator = 1 - std::pow((1 - w), minModelPointsFloat);
			if (denominator < eps) {
				// Happens when number of inliners close to number of points
				COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "Good news: denominator < eps");
				newMaxIter = 0;
			}
			else {
				denominator = std::log(denominator);
				if (denominator < 0 && numerator > (newMaxIter * denominator)) {
					newMaxIter = COMPV_MATH_ROUNDFU_2_NEAREST_INT(numerator / denominator, size_t);
				}
			}
		} while (numIter++ < newMaxIter && bestNumInliers < minInliersToStopIter);

		COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "RANSAC numIter/maxIter = %zu/%zu, bestNumInliers=%zu,", 
			numIter, maxIter,
			bestNumInliers
		);

		/* 
			Build inliers and make a final estimation of the parameters
			The condition "bestNumInliers >= minModelPoints" is always true because at least one set of random points will be matched as inliers
			unless the input data is incorrect (e.g. contains 'inf' or 'nan' values).
		*/
		if (bestNumInliers >= minModelPoints) { 
			/* Build residual using best params selected in ransac loop */
			bool userBreak = false;
			COMPV_CHECK_CODE_RETURN(buildResiduals(
				control, modelParamsBest, residualPtr, userBreak
			));
			/* Re-compute inliers indices using the residual from best params */
			CompVMathStatsRansacModelIndices indices(bestNumInliers);
			size_t i, k;
			for (i = 0, k = 0; i < totalPoints && k < bestNumInliers; ++i) {
				if (residualPtr[i] < threshold) {
					indices[k++] = i;
				}
			}
			COMPV_CHECK_EXP_RETURN(k != bestNumInliers, COMPV_ERROR_CODE_E_INVALID_CALL, "Second check for number of inliers returned different result");
			/* Build model params (will update modelParamsBest) using inliers only */
			bool userReject = false;
			COMPV_CHECK_CODE_RETURN(buildModelParams(
				control, indices, modelParamsBest, userReject
			));
		}

		return COMPV_ERROR_CODE_S_OK;
	}
};


//
//	CompVMathStatsRansac
//


template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const CompVMathStatsRansacControlFloat32* control, CompVMathStatsRansacStatusFloat32* status,
	COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControlFloat32* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat32& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControlFloat32* control, const CompVMathStatsRansacModelParamsFloat32& modelParams, compv_float32_t* residualPtr, bool& userbreak)
)
{
	COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process<compv_float32_t>(
		control, status,
		buildModelParams,
		buildResiduals
	));
	return COMPV_ERROR_CODE_S_OK;
}

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const CompVMathStatsRansacControlFloat64* control, CompVMathStatsRansacStatusFloat64* status,
	COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControlFloat64* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat64& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControlFloat64* control, const CompVMathStatsRansacModelParamsFloat64& modelParams, compv_float64_t* residualPtr, bool& userbreak)
)
{
	COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process<compv_float64_t>(
		control, status,
		buildModelParams,
		buildResiduals
		));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
