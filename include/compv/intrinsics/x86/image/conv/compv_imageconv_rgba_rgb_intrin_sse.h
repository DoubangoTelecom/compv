/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CONV_IMAGE_IMAGECONV_RGBA_RGB_INTRIN_SSE_H_)
#define _COMPV_CONV_IMAGE_IMAGECONV_RGBA_RGB_INTRIN_SSE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void rgbToRgbaKernel31_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb, COMPV_ALIGNED(SSE) uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);
void bgrToBgraKernel31_Intrin_Aligned_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* bgr, COMPV_ALIGNED(SSE) uint8_t* bgra, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride);

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */

#endif /* _COMPV_CONV_IMAGE_IMAGECONV_RGBA_RGB_INTRIN_SSE_H_ */
