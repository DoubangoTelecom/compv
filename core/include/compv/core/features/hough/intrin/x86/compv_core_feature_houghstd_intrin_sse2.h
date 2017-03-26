/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURE_HOUGHSTD_INTRIN_SSE2_H_)
#define _COMPV_CORE_FEATURE_HOUGHSTD_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVHoughStdNmsGatherRow_4mpd_Intrin_SSE2(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, compv_uscalar_t nThreshold, compv_uscalar_t colStart, compv_uscalar_t maxCols);

void CompVHoughStdNmsApplyRow_Intrin_SSE2(COMPV_ALIGNED(SSE) int32_t* pACC, COMPV_ALIGNED(SSE) uint8_t* pNMS, size_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t colStart, size_t maxCols, CompVHoughLineVector& lines);

void CompVHoughStdRowTimesSinRho_Intrin_SSE2(COMPV_ALIGNED(SSE) const int32_t* pSinRho, COMPV_ALIGNED(SSE) compv_uscalar_t row, COMPV_ALIGNED(SSE) int32_t* rowTimesSinRhoPtr, compv_uscalar_t count);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURE_HOUGHSTD_INTRIN_SSE2_H_ */
