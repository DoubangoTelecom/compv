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

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURE_HOUGHSTD_INTRIN_SSE2_H_ */
