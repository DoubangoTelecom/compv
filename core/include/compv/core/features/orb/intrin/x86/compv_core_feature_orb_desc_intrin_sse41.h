/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_ORB_DESC_INTRIN_SSE41_H_)
#define _COMPV_CORE_FEATURES_ORB_DESC_INTRIN_SSE41_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVOrbBrief256_31_32f_Intrin_SSE41(
	const uint8_t* img_center, compv_uscalar_t img_stride,
	const float* cos1, const float* sin1,
	const float* kBrief256Pattern31AX, const float* kBrief256Pattern31AY,
	const float* kBrief256Pattern31BX, const float* kBrief256Pattern31BY,
	void* out
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURES_ORB_DESC_INTRIN_SSE41_H_ */
