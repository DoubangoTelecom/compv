/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_STATS_H_)
#define _COMPV_MATH_STATS_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/math/compv_math.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

template <class T>
class COMPV_API CompVMathStats
{
public:
    static COMPV_ERROR_CODE normalize2D_hartley(const T* x, const T* y, size_t numPoints, T* tx1, T* ty1, T* s1);
    static COMPV_ERROR_CODE mse2D_homogeneous(const T* aX_h, const T* aY_h, const T* aZ_h, const T* bX, const T* bY, CompVPtrArray(T)& mse, size_t numPoints);
    static COMPV_ERROR_CODE variance(const T* data, size_t count, const T* mean1, T* var1);
    static COMPV_ERROR_CODE stdev(const T* data, size_t count, const T* mean1, T* std1);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_STATS_H_ */
