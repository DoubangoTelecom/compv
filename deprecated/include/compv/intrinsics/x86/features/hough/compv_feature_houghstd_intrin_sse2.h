/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_HOUGHSTD_INTRIN_SSE2_H_)
#define _COMPV_FEATURES_HOUGHSTD_INTRIN_SSE2_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"
#include "compv/compv_box.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void HoughStdNmsGatherRow_Intrin_SSE2(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, int32_t nThreshold, compv_uscalar_t width);
void HoughStdNmsApplyRow_Intrin_SSE2(COMPV_ALIGNED(SSE) int32_t* pACC, COMPV_ALIGNED(SSE) uint8_t* pNMS, int32_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t maxCols, CompVPtrBox(CompVCoordPolar2f)& coords);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_FEATURES_HOUGHSTD_INTRIN_SSE2_H_ */
