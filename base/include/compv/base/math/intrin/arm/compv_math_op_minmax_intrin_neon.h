/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_OP_MINMAX_INTRIN_NEON_H_)
#define _COMPV_BASE_MATH_OP_MINMAX_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_ARM && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathOpMinMax_32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, compv_float32_t* min1, compv_float32_t* max1);

void CompVMathOpMin_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint8_t* min1);

void CompVMathOpMax_32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* APtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, const compv_float32_t* b1, COMPV_ALIGNED(NEON) compv_float32_t* RPtr);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_OP_MINMAX_INTRIN_NEON_H_ */
