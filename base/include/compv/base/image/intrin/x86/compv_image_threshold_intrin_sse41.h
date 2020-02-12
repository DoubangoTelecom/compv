/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_THRESHOLD_INTRIN_SSE41_H_)
#define _COMPV_BASE_IMAGE_THRESHOLD_INTRIN_SSE41_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageThresholdOtsuSum_32s32s_Intrin_SSE41(COMPV_ALIGNED(SSE) const uint32_t* ptr32uHistogram, COMPV_ALIGNED(SSE) uint32_t* sumA256, uint32_t* sumB1);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_THRESHOLD_INTRIN_SSE41_H_ */
