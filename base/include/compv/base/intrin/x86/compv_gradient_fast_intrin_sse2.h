/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_GRADIENT_FAST_INTRIN_SSE2_H_)
#define _COMPV_BASE_GRADIENT_FAST_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVGradientFastGradX_8u16s_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVGradientFastGradX_8u32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVGradientFastGradX_32f32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVGradientFastGradY_8u16s_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVGradientFastGradY_8u32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVGradientFastGradY_32f32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_GRADIENT_FAST_INTRIN_SSE2_H_ */
