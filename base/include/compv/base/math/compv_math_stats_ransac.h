/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_STATS_RANSAC_H_)
#define _COMPV_BASE_MATH_STATS_RANSAC_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

#define CompVMathStatsRansacModelParamsFloatType std::vector<FloatType>
typedef std::vector<compv_float32_t> CompVMathStatsRansacModelParamsFloat32;
typedef std::vector<compv_float64_t> CompVMathStatsRansacModelParamsFloat64;

typedef std::vector<size_t> CompVMathStatsRansacModelIndices;

template <typename FloatType>
struct CompVMathStatsRansacControl {
	// userdata - could be anything
	const void* opaque = nullptr;
	// nd - total number of data points
	size_t totalPoints;
	// t - threshold value to determine when a data point fits a model
	FloatType threshold;
	// k – maximum number of iterations allowed in the algorithm
	size_t maxIter = 2000;
	// n – minimum number of data points required to fit the model. #2 for line, #3 for plane, #4 for homography, #8 for essential matrix...
	size_t minModelPoints;
	// p – the probability that the RANSAC algorithm in some iteration selects only inliers from the input data set when it chooses the n points from which the model parameters are estimated.
	FloatType probInliersOnly = static_cast<FloatType>(0.95);
	CompVMathStatsRansacControl(const FloatType threshold_, const size_t totalPoints_, size_t minModelPoints_, const void* opaque_ = nullptr) {
		threshold = threshold_;
		totalPoints = totalPoints_;
		minModelPoints = minModelPoints_;
		opaque = opaque_;
	}
	COMPV_INLINE const bool isValid()const {
		return minModelPoints && totalPoints && maxIter &&
			(minModelPoints <= totalPoints) &&
			(probInliersOnly > 0 && probInliersOnly <= 1);
	}
};
typedef CompVMathStatsRansacControl<compv_float32_t> CompVMathStatsRansacControlFloat32;
typedef CompVMathStatsRansacControl<compv_float64_t> CompVMathStatsRansacControlFloat64;

template <typename FloatType>
struct CompVMathStatsRansacStatus {
	CompVMathStatsRansacModelParamsFloatType modelParamsBest;
	size_t numInliers = 0;
	size_t maxIter = 0;
	size_t numIter = 0;

	void reset() {
		modelParamsBest.clear();
		numInliers = 0;
		maxIter = 0;
		numIter = 0;
	}
};
typedef CompVMathStatsRansacStatus<compv_float32_t> CompVMathStatsRansacStatusFloat32;
typedef CompVMathStatsRansacStatus<compv_float64_t> CompVMathStatsRansacStatusFloat64;

class COMPV_BASE_API CompVMathStatsRansac
{
public:
	template <typename FloatType = compv_float32_t>
	static COMPV_ERROR_CODE process(
		const CompVMathStatsRansacControl<FloatType>* control, CompVMathStatsRansacStatus<FloatType>* status,
		COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject),
		COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControl<FloatType>* control, const CompVMathStatsRansacModelParamsFloatType& modelParams, CompVMatPtr residual, bool& userbreak)
	);
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const CompVMathStatsRansacControlFloat32* control, CompVMathStatsRansacStatusFloat32* status,
	COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControlFloat32* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat32& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControlFloat32* control, const CompVMathStatsRansacModelParamsFloat32& modelParams, CompVMatPtr residual, bool& userbreak)
);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const CompVMathStatsRansacControlFloat64* control, CompVMathStatsRansacStatusFloat64* status,
	COMPV_ERROR_CODE(*buildModelParams)(const CompVMathStatsRansacControlFloat64* control, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat64& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*buildResiduals)(const CompVMathStatsRansacControlFloat64* control, const CompVMathStatsRansacModelParamsFloat64& modelParams, CompVMatPtr residual, bool& userbreak)
);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_STATS_RANSAC_H_ */

