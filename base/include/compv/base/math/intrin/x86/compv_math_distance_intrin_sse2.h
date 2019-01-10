/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_DISTANCE_INTRIN_SSE2_H_)
#define _COMPV_BASE_MATH_DISTANCE_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathDistanceLine_32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* xPtr, COMPV_ALIGNED(SSE) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(SSE) compv_float32_t* distPtr, const compv_uscalar_t count);
void CompVMathDistanceParabola_32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* xPtr, COMPV_ALIGNED(SSE) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(SSE) compv_float32_t* distPtr, const compv_uscalar_t count);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_DISTANCE_INTRIN_SSE2_H_ */
