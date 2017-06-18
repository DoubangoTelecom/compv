/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_TO_RGBA32_H_)
#define _COMPV_BASE_IMAGE_CONV_TO_RGBA32_H_

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class CompVImageConvToRGBA32
{
public:
	static COMPV_ERROR_CODE process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageRGBA32);

private:
	static COMPV_ERROR_CODE yuvPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBA32);
	static COMPV_ERROR_CODE yuvSemiPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBA32);
	static COMPV_ERROR_CODE yuvPacked(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBA32);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_CONV_TO_RGBA32_H_ */
