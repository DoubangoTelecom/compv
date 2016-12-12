/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_H_)
#define _COMPV_BASE_IMAGE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVImage
{
public:
    static COMPV_ERROR_CODE newObj8u(CompVMatPtrPtr mat, size_t width, size_t height, COMPV_SUBTYPE pixelFormat);
    static COMPV_ERROR_CODE newObj16u(CompVMatPtrPtr mat, size_t width, size_t height, COMPV_SUBTYPE pixelFormat);
	static COMPV_ERROR_CODE readPixels(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t stride, const char* filePath, CompVMatPtrPtr mat);
	static COMPV_ERROR_CODE wrap(COMPV_SUBTYPE ePixelFormat, const void* dataPtr, size_t width, size_t height, size_t stride, CompVMatPtrPtr mat);
private:
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_H_ */
