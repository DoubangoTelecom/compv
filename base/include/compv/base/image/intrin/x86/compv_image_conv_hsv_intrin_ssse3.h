/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_HSV_INTRIN_SSSE3_H_)
#define _COMPV_BASE_IMAGE_CONV_HSV_INTRIN_SSSE3_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageConvRgba32ToHsv_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgba32Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride
	, const compv_float32_t(*scales43)[256], const compv_float32_t(*scales255)[256]);

void CompVImageConvRgb24ToHsv_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb24Ptr, COMPV_ALIGNED(SSE) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride
	, const compv_float32_t(*scales43)[256], const compv_float32_t(*scales255)[256]);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_CONV_HSV_INTRIN_SSSE3_H_ */
