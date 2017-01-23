/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_H_)
#define _COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVImageConvRgbaI420
{
public:
    static COMPV_ERROR_CODE rgbToI420(const CompVPtr<CompVImage* >& rgb, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE bgrToI420(const CompVPtr<CompVImage* >& bgr, CompVPtr<CompVImage* >& i420);

    static COMPV_ERROR_CODE rgbaToI420(const CompVPtr<CompVImage* >& rgba, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE argbToI420(const CompVPtr<CompVImage* >& argb, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE bgraToI420(const CompVPtr<CompVImage* >& bgra, CompVPtr<CompVImage* >& i420);
    static COMPV_ERROR_CODE abgrToI420(const CompVPtr<CompVImage* >& abgr, CompVPtr<CompVImage* >& i420);

    static COMPV_ERROR_CODE i420ToRgba(const CompVPtr<CompVImage* >& i420, CompVPtr<CompVImage* >& rgba);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_CONV_IMAGECONV_RGBA_I420_H_ */