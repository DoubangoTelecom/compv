/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_INTEGRAL_INTRIN_SSSE3_H_)
#define _COMPV_BASE_IMAGE_INTEGRAL_INTRIN_SSSE3_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageIntegralProcess_8u64f_Intrin_SSSE3(const uint8_t* in, compv_float64_t* sum, compv_float64_t* sumsq, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t in_stride, const compv_uscalar_t sum_stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_INTEGRAL_INTRIN_SSSE3_H_ */
