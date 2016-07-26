/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_HOUGHSTD_INTRIN_SSE41_H_)
#define _COMPV_FEATURES_HOUGHSTD_INTRIN_SSE41_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"
#include "compv/compv_box.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void HoughStdAccGatherRow_Intrin_SSE41(int32_t* pACC, int32_t accStride, const uint8_t* pixels, int32_t maxCols, int32_t maxThetaCount, int32_t row, COMPV_ALIGNED(SSE) const int32_t* pCosRho, COMPV_ALIGNED(SSE) const int32_t* pSinRho);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_FEATURES_HOUGHSTD_INTRIN_SSE41_H_ */
