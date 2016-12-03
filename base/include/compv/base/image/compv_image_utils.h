/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_UTILS_H_)
#define _COMPV_BASE_IMAGE_UTILS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVImageUtils
{
public:
    static COMPV_ERROR_CODE getBestStride(size_t stride, size_t *bestStride);
    static COMPV_ERROR_CODE getSizeForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t *size);
    static COMPV_ERROR_CODE getCompSizeForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t compId, size_t *size);
    static COMPV_ERROR_CODE getCompSizeForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t compId, size_t imgWidth, size_t imgHeight, size_t *compWidth, size_t *compHeight);
    static COMPV_ERROR_CODE getBitsCountForPixelFormat(COMPV_SUBTYPE ePixelFormat, size_t* bitsCount);
    static COMPV_ERROR_CODE getCompCount(COMPV_SUBTYPE ePixelFormat, size_t *compCount);
    static COMPV_ERROR_CODE getCompPacked(COMPV_SUBTYPE ePixelFormat, bool *packed);

private:
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_UTILS_H_ */
