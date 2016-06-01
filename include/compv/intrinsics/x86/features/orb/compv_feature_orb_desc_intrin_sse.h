/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURE_ORB_DESC_INTRIN_SSE_H_)
#define _COMPV_FEATURE_ORB_DESC_INTRIN_SSE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void Brief256_31_Intrin_SSE2(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
void Brief256_31_Fxpq16_Intrin_SSE2(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(SSE) void* out);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_FEATURE_ORB_DESC_INTRIN_SSE_H_ */
