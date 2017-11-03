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

		COMPV_DEBUG_INFO_CODE_TODO("Add TieBreaker");

		CompVMathStatsRansacModelParamsFloatType& modelParamsBest = status->modelParamsBest;
		modelParamsBest.clear();

		// Save control values to make sure won't change while we're doing ransac processing
		const FloatType threshold = control->threshold;
		const size_t maxIter = control->maxIter;
		const size_t minModelPoints = control->minModelPoints;
		const size_t totalPoints = control->totalPoints;

		size_t numIter = 0;
		int bestNumInliers = 0;

		/* Create residual */
		CompVMatPtr residual;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(&residual, 1, totalPoints));
		FloatType* residualPtr = residual->ptr<FloatType>();

		do {
			/* Build random indices for the model */
			CompVMathStatsRansacModelIndices indices;
			size_t indice;
			std::random_device rand_device;
			std::mt19937 prng{ rand_device() }; // TODO(dmi): one random device per thread
			std::uniform_int_distribution<size_t> unif_dist{ 0, static_cast<size_t>(totalPoints - 1) };
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
				bestNumInliers = numInliers;
				modelParamsBest = params;
			}

		} while (numIter++ < maxIter);

		/* Build inliers and make a final estimation of the parameters */
		const size_t szBestNumInliers = static_cast<size_t>(bestNumInliers);
		if (szBestNumInliers >= minModelPoints) { // always true because at least one set of random points will be matched as inliers
			/* Build residual */
			bool userBreak = false;
			COMPV_CHECK_CODE_RETURN(buildResiduals(
				control, modelParamsBest, residualPtr, userBreak
			));
			/* Re-compute inliers indices */
			CompVMathStatsRansacModelIndices indices(bestNumInliers);
			size_t i, k;
			for (i = 0, k = 0; i < totalPoints && k < szBestNumInliers; ++i) {
				if (residualPtr[i] < threshold) {
					indices[k++] = i;
				}
			}
			COMPV_CHECK_EXP_RETURN(k != szBestNumInliers, COMPV_ERROR_CODE_E_INVALID_CALL, "Second check for number of inliers returned different result");
			/* Build model params */
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
