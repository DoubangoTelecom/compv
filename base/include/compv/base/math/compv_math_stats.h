/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_STATS_H_)
#define _COMPV_BASE_MATH_STATS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathStats
{
public:
	template<typename T> static COMPV_ERROR_CODE normalize2D_hartley(const T* x, const T* y, size_t numPoints, T* tx1, T* ty1, T* s1);
	template<typename T> static COMPV_ERROR_CODE mse2D_homogeneous(CompVMatPtrPtr mse, const T* aX_h, const T* aY_h, const T* aZ_h, const T* bX, const T* bY, size_t numPoints);
	static COMPV_ERROR_CODE mse2D(const CompVMatPtr& aPoints, const CompVMatPtr& bPoints, double& error);
	template<typename T> static COMPV_ERROR_CODE variance(const T* data, size_t count, T mean, T* var1);
	template<typename T> static COMPV_ERROR_CODE stdev(const T* data, size_t count, T mean, T* std1);
	static COMPV_ERROR_CODE normL2(const CompVMatPtr& ptrIn, CompVMatPtrPtr ptrOut, const double maxVal = 1.0);
	static COMPV_ERROR_CODE normMinmax(const CompVMatPtr& ptrIn, CompVMatPtrPtr ptrOut, const double maxVal = 1.0);
	static COMPV_ERROR_CODE normZscore(const CompVMatPtr& ptrIn, CompVMatPtrPtr ptrOut, const double maxVal = 1.0);
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::normalize2D_hartley(const compv_float32_t* x, const compv_float32_t* y, size_t numPoints, compv_float32_t* tx1, compv_float32_t* ty1, compv_float32_t* s1);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::normalize2D_hartley(const compv_float64_t* x, const compv_float64_t* y, size_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::mse2D_homogeneous(CompVMatPtrPtr mse, const compv_float32_t* aX_h, const compv_float32_t* aY_h, const compv_float32_t* aZ_h, const compv_float32_t* bX, const compv_float32_t* bY, size_t numPoints);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::mse2D_homogeneous(CompVMatPtrPtr mse, const compv_float64_t* aX_h, const compv_float64_t* aY_h, const compv_float64_t* aZ_h, const compv_float64_t* bX, const compv_float64_t* bY, size_t numPoints);

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::variance(const compv_float32_t* data, size_t count, compv_float32_t mean, compv_float32_t* var1);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::variance(const compv_float64_t* data, size_t count, compv_float64_t mean, compv_float64_t* var1);

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::stdev(const compv_float32_t* data, size_t count, compv_float32_t mean, compv_float32_t* std1);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathStats::stdev(const compv_float64_t* data, size_t count, compv_float64_t mean, compv_float64_t* std1);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_STATS_H_ */

