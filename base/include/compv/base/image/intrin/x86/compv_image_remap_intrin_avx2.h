/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_REMAP_INTRIN_AVX2_H_)
#define _COMPV_BASE_IMAGE_REMAP_INTRIN_AVX2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageRemapBilinear_8u32f_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const compv_float32_t* mapXPtr, COMPV_ALIGNED(AVX) const compv_float32_t* mapYPtr,
	const uint8_t* inputPtr, compv_float32_t* outputPtr,
	COMPV_ALIGNED(AVX) const compv_float32_t* roi, COMPV_ALIGNED(AVX) const int32_t* size,
	const compv_float32_t* defaultPixelValue1,
	COMPV_ALIGNED(AVX) const compv_uscalar_t count
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_REMAP_INTRIN_AVX2_H_ */
