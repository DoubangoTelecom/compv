/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_MATRIX_INTRIN_AVX2_H_)
#define _COMPV_MATH_MATRIX_INTRIN_AVX2_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void MatrixIsEqual_64f_Intrin_AVX2(const COMPV_ALIGNED(AVX2) compv_float64_t* A, const COMPV_ALIGNED(AVX2) compv_float64_t* B, compv_uscalar_t rows, compv_uscalar_t cols, compv_uscalar_t strideInBytes, compv_scalar_t *equal);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_MATH_MATRIX_INTRIN_AVX2_H_ */
