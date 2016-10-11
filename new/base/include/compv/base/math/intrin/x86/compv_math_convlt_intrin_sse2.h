/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_CONVLT_INTRIN_SSE2_H_)
#define _COMPV_BASE_MATH_CONVLT_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void MathConvlt1VertHz_8u16i16i_Intrin_SSE2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize);
void MathConvlt1VertHz_16i16i16i_Intrin_SSE2(COMPV_ALIGNED(SSE) const int16_t* inPtr, COMPV_ALIGNED(SSE) int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_CONVLT_INTRIN_SSE2_H_ */
