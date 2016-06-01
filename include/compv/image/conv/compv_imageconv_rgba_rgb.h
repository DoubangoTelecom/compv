/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_CONV_IMAGECONV_RGBA_RGB_H_)
#define _COMPV_IMAGE_CONV_IMAGECONV_RGBA_RGB_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVImageConvRgbaRgb
{
public:
    static COMPV_ERROR_CODE rgbToRgba(const CompVPtr<CompVImage* >& rgb, CompVPtr<CompVImage* >& rgba);
    static COMPV_ERROR_CODE bgrToBgra(const CompVPtr<CompVImage* >& bgr, CompVPtr<CompVImage* >& bgra);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_CONV_IMAGECONV_RGBA_RGB_H_ */
