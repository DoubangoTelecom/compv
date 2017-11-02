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
		const void* opaque, const size_t dataCount, const size_t modelDataCount,
		COMPV_ERROR_CODE(*buildModelParams)(const void* opaque, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject),
		COMPV_ERROR_CODE(*evalModelParams)(const void* opaque, const CompVMathStatsRansacModelParamsFloatType& modelParams, FloatType* residualPtr, bool& userbreak),
		const CompVMathStatsRansacControl* control, CompVMathStatsRansacStatus<FloatType>* status
	)
	{
		COMPV_CHECK_EXP_RETURN(!dataCount || !modelDataCount || dataCount < modelDataCount || !control || !status || !buildModelParams || !evalModelParams,
			COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		CompVMathStatsRansacModelParamsFloatType& modelParamsBest = status->modelParamsBest;
		modelParamsBest.clear();
		
		FloatType residualMin = std::numeric_limits<FloatType>().max();
		int maxTries = 1000;
		int numTries = 0;

		/* Create residual */
		CompVMatPtr residual;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<FloatType>(&residual, 1, dataCount));
		FloatType* residualPtr = residual->ptr<FloatType>();

		do {
			/* Build random indices for the model */
			CompVMathStatsRansacModelIndices indices;
			size_t indice;
			std::random_device rand_device;
			std::mt19937 prng{ rand_device() }; // TODO(dmi): one random device per thread
			std::uniform_int_distribution<size_t> unif_dist{ 0, static_cast<size_t>(dataCount - 1) };
			while (indices.size() < modelDataCount) {
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
				opaque, indices, params, userReject
			));
			if (userReject) {
				// got while, increment(numTries) then try again
				continue;
			}

			/* Eval model params */
			bool userBreak = false;
			COMPV_CHECK_CODE_RETURN(evalModelParams(
				opaque, params, residualPtr, userBreak
			));

			/* Check residual */
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation for CompVMatUtils::sumSquare");
			FloatType residual_sum = 0.f;
			for (size_t i = 0; i < dataCount; ++i) {
				// FIXME(dmi): require residual to be positive
				residual_sum += (residualPtr[i] * residualPtr[i]); // square to make sure residual will be positive
			}
			if (residual_sum < residualMin) {
				residualMin = residual_sum;
				modelParamsBest = params;
			}

		} while (numTries++ < maxTries);

		return COMPV_ERROR_CODE_S_OK;
	}
};


//
//	CompVMathStatsRansac
//


template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const void* opaque, const size_t dataCount, const size_t modelDataCount,
	COMPV_ERROR_CODE(*buildModelParams)(const void* opaque, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat32& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*evalModelParams)(const void* opaque, const CompVMathStatsRansacModelParamsFloat32& modelParams, compv_float32_t* residualPtr, bool& userbreak),
	const CompVMathStatsRansacControl* control, CompVMathStatsRansacStatusFloat32* status
)
{
	COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process<compv_float32_t>(
		opaque, dataCount, modelDataCount,
		buildModelParams,
		evalModelParams,
		control, status
	));
	return COMPV_ERROR_CODE_S_OK;
}

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const void* opaque, const size_t dataCount, const size_t modelDataCount,
	COMPV_ERROR_CODE(*buildModelParams)(const void* opaque, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat64& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*evalModelParams)(const void* opaque, const CompVMathStatsRansacModelParamsFloat64& modelParams, compv_float64_t* residualPtr, bool& userbreak),
	const CompVMathStatsRansacControl* control, CompVMathStatsRansacStatusFloat64* status
)
{
	COMPV_CHECK_CODE_RETURN(CompVMathStatsRansacGeneric::process<compv_float64_t>(
		opaque, dataCount, modelDataCount,
		buildModelParams,
		evalModelParams,
		control, status
		));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
