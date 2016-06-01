/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PATCH_INTRIN_SSE_H_)
#define _COMPV_PATCH_INTRIN_SSE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void Moments0110_Intrin_SSE41(COMPV_ALIGNED(SSE) const uint8_t* top, COMPV_ALIGNED(SSE)const uint8_t* bottom, COMPV_ALIGNED(SSE)const int16_t* x, COMPV_ALIGNED(SSE) const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_PATCH_INTRIN_SSE_H_ */
