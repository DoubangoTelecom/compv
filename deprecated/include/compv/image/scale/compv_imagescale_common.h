/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_SCALE_IMAGESCALE_COMMON_H_)
#define _COMPV_IMAGE_SCALE_IMAGESCALE_COMMON_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

typedef void(*scaleBilinear)(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t inHeight, compv_uscalar_t inWidth, compv_uscalar_t inStride, compv_uscalar_t outHeight, compv_uscalar_t outWidth, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y);

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_SCALE_IMAGESCALE_COMMON_H_ */
