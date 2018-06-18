/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_SSE41_H_)
#define _COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_SSE41_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageScaleBicubicHermite_32f32s_Intrin_SSE41(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	const int32_t* xint1,
	const compv_float32_t* xfract1,
	const int32_t* yint1,
	const compv_float32_t* yfract1,
	const compv_uscalar_t inWidth,
	const compv_uscalar_t inHeight,
	const compv_uscalar_t inStride
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_SSE41_H_ */
