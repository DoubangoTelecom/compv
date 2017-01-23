/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_CONV_IMAGECONV_GRAYSCALE_H_)
#define _COMPV_IMAGE_CONV_IMAGECONV_GRAYSCALE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVImageConvGrayscale
{
public:
    static COMPV_ERROR_CODE rgbToGrayscale(const CompVPtr<CompVImage* >& rgb, CompVPtr<CompVImage* >& grayscale);
    static COMPV_ERROR_CODE bgrToGrayscale(const CompVPtr<CompVImage* >& bgr, CompVPtr<CompVImage* >& grayscale);

    static COMPV_ERROR_CODE rgbaToGrayscale(const CompVPtr<CompVImage* >& rgba, CompVPtr<CompVImage* >& grayscale);
    static COMPV_ERROR_CODE argbToGrayscale(const CompVPtr<CompVImage* >& argb, CompVPtr<CompVImage* >& grayscale);
    static COMPV_ERROR_CODE bgraToGrayscale(const CompVPtr<CompVImage* >& bgra, CompVPtr<CompVImage* >& grayscale);
    static COMPV_ERROR_CODE abgrToGrayscale(const CompVPtr<CompVImage* >& abgr, CompVPtr<CompVImage* >& grayscale);

    static COMPV_ERROR_CODE i420ToGrayscale(const CompVPtr<CompVImage* >& i420, CompVPtr<CompVImage* >& grayscale);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_CONV_IMAGECONV_GRAYSCALE_H_ */
