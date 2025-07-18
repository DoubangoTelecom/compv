/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_DISTANCE_INTRIN_FMA3_AVX_H_)
#define _COMPV_BASE_MATH_DISTANCE_INTRIN_FMA3_AVX_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathDistanceSquaredL2Row_32f_Intrin_FMA3_AVX(COMPV_ALIGNED(AVX) const float* dataset, COMPV_ALIGNED(AVX) const float* vectors, float* result1, const compv_uscalar_t& cols);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_DISTANCE_INTRIN_FMA3_AVX_H_ */
