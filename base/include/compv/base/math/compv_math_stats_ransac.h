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

struct CompVMathStatsRansacControl {

};

template <typename FloatType>
struct CompVMathStatsRansacStatus {
	CompVMathStatsRansacModelParamsFloatType modelParamsBest;
};
typedef CompVMathStatsRansacStatus<compv_float32_t> CompVMathStatsRansacStatusFloat32;
typedef CompVMathStatsRansacStatus<compv_float64_t> CompVMathStatsRansacStatusFloat64;

class COMPV_BASE_API CompVMathStatsRansac
{
public:
	template <typename FloatType = compv_float32_t>
	static COMPV_ERROR_CODE process(
		const void* opaque, const size_t dataCount, const size_t modelDataCount,
		COMPV_ERROR_CODE(*buildModelParams)(const void* opaque, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloatType& modelParams, bool& userReject),
		COMPV_ERROR_CODE(*evalModelParams)(const void* opaque, const CompVMathStatsRansacModelParamsFloatType& modelParams, FloatType* residualPtr, bool& userbreak),
		const CompVMathStatsRansacControl* control, CompVMathStatsRansacStatus<FloatType>* status
	);
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const void* opaque, const size_t dataCount, const size_t modelDataCount,
	COMPV_ERROR_CODE(*buildModelParams)(const void* opaque, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat32& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*evalModelParams)(const void* opaque, const CompVMathStatsRansacModelParamsFloat32& modelParams, compv_float32_t* residualPtr, bool& userbreak),
	const CompVMathStatsRansacControl* control, CompVMathStatsRansacStatusFloat32* status
);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStatsRansac::process(
	const void* opaque, const size_t dataCount, const size_t modelDataCount,
	COMPV_ERROR_CODE(*buildModelParams)(const void* opaque, const CompVMathStatsRansacModelIndices& modelIndices, CompVMathStatsRansacModelParamsFloat64& modelParams, bool& userReject),
	COMPV_ERROR_CODE(*evalModelParams)(const void* opaque, const CompVMathStatsRansacModelParamsFloat64& modelParams, compv_float64_t* residualPtr, bool& userbreak),
	const CompVMathStatsRansacControl* control, CompVMathStatsRansacStatusFloat64* status
);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_STATS_RANSAC_H_ */

