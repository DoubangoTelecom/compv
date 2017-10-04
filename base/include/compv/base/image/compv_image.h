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
    static COMPV_ERROR_CODE newObj8u(CompVMatPtrPtr image, COMPV_SUBTYPE pixelFormat, size_t width, size_t height, size_t stride = 0);
    static COMPV_ERROR_CODE newObj16u(CompVMatPtrPtr image, COMPV_SUBTYPE pixelFormat, size_t width, size_t height, size_t stride = 0);
	static COMPV_ERROR_CODE newObj16s(CompVMatPtrPtr image, COMPV_SUBTYPE pixelFormat, size_t width, size_t height, size_t stride = 0);
	static COMPV_ERROR_CODE readPixels(COMPV_SUBTYPE ePixelFormat, size_t width, size_t height, size_t stride, const char* filePath, CompVMatPtrPtr image);
	static COMPV_ERROR_CODE wrap(COMPV_SUBTYPE ePixelFormat, const void* dataPtr, size_t width, size_t height, size_t stride, CompVMatPtrPtr image);
	static COMPV_ERROR_CODE clone(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut);
	static COMPV_ERROR_CODE crop(const CompVMatPtr& imageIn, const CompVRectFloat32& roi, CompVMatPtrPtr imageOut);

	static COMPV_ERROR_CODE convert(const CompVMatPtr& imageIn, COMPV_SUBTYPE pixelFormatOut, CompVMatPtrPtr imageOut);
	static COMPV_ERROR_CODE convertGrayscale(const CompVMatPtr& imageIn, CompVMatPtrPtr imageGray);
	static COMPV_ERROR_CODE convertGrayscaleFast(CompVMatPtr& imageInOut);

	static COMPV_ERROR_CODE scale(const CompVMatPtr& imageIn, CompVMatPtrPtr imageOut, size_t widthOut, size_t heightOut, COMPV_INTERPOLATION_TYPE scaleType = COMPV_INTERPOLATION_TYPE_BILINEAR);
private:
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_H_ */
