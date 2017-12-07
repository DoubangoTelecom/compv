/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURE_HOUGHSHT_INTRIN_AVX2_H_)
#define _COMPV_CORE_FEATURE_HOUGHSHT_INTRIN_AVX2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVHoughShtAccGatherRow_8mpd_Intrin_AVX2(COMPV_ALIGNED(AVX) const int32_t* pCosRho, COMPV_ALIGNED(AVX) const int32_t* pRowTimesSinRho, compv_uscalar_t col, int32_t* pACC, compv_uscalar_t accStride, compv_uscalar_t maxTheta);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURE_HOUGHSHT_INTRIN_AVX2_H_ */
