/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_STATS_INTRIN_NEON64_H_)
#define _COMPV_BASE_MATH_STATS_INTRIN_NEON64_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_ARM64 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathStatsNormalize2DHartley_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
void CompVMathStatsNormalize2DHartley_4_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* x, const COMPV_ALIGNED(NEON) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1);
void CompVMathStatsMSE2DHomogeneous_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints);
void CompVMathStatsMSE2DHomogeneous_4_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* aX_h, const COMPV_ALIGNED(NEON) compv_float64_t* aY_h, const COMPV_ALIGNED(NEON) compv_float64_t* aZ_h, const COMPV_ALIGNED(NEON) compv_float64_t* bX, const COMPV_ALIGNED(NEON) compv_float64_t* bY, COMPV_ALIGNED(NEON) compv_float64_t* mse, compv_uscalar_t numPoints);
void CompVMathStatsVariance_64f_Intrin_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* data, compv_uscalar_t count, const compv_float64_t* mean1, compv_float64_t* var1);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM64 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_STATS_INTRIN_NEON64_H_ */
