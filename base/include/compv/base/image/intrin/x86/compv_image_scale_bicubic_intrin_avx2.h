/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_AVX2_H_)
#define _COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_AVX2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageScaleBicubicHermite_32f32s_Intrin_AVX2(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	const int32_t* xint1,
	const compv_float32_t* xfract1,
	const int32_t* yint1,
	const compv_float32_t* yfract1,
	const compv_uscalar_t inWidthMinus1,
	const compv_uscalar_t inHeightMinus1,
	const compv_uscalar_t inStride
);

void CompVImageScaleBicubicPostProcessRow_32f32s_Intrin_AVX2(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	COMPV_ALIGNED(SSE) const int32_t* xint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* xfract4,
	COMPV_ALIGNED(SSE) const int32_t* yint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* yfract4,
	const compv_uscalar_t rowCount
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_AVX2_H_ */
