/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_SCALE_IMAGESCALE_BILINEAR_H_)
#define _COMPV_IMAGE_SCALE_IMAGESCALE_BILINEAR_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVImageScaleBilinear
{
public:
    static COMPV_ERROR_CODE process(const CompVPtr<CompVImage* >& inImage, CompVPtr<CompVImage* >& outImage);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_SCALE_IMAGESCALE_BILINEAR_H_ */
