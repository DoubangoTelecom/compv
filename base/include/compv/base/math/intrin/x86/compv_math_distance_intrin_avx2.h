/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_DISTANCE_INTRIN_AVX2_H_)
#define _COMPV_BASE_MATH_DISTANCE_INTRIN_AVX2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathDistanceHamming32_Intrin_POPCNT_AVX2(COMPV_ALIGNED(AVX) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride, COMPV_ALIGNED(AVX) const uint8_t* patch1xnPtr, COMPV_ALIGNED(AVX) int32_t* distPtr);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_DISTANCE_INTRIN_AVX2_H_ */
