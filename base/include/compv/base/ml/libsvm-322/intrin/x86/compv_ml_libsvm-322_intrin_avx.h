/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_ML_LIBSVM322_INTRIN_AVX_H_)
#define _COMPV_BASE_ML_LIBSVM322_INTRIN_AVX_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVLibSVM322KernelRbf0Out_64f64f_AVX(const double& gamma, const double* xSquarePtr, const double* dotMatPtr, double* outPtr, const size_t count);
void CompVLibSVM322KernelRbf1Out_Step1_64f64f_AVX(const double& gamma, const double& x_squarei, const double* xSquarejPtr, const double* dotMatPtr, double* outPtr, const size_t count);
void CompVLibSVM322KernelRbf1Out_Step2_64f32f_AVX(const double& yi, const double* yjPtr, const double* outStep1Ptr, float* outPtr, const size_t count);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_ML_LIBSVM322_INTRIN_AVX_H_ */
