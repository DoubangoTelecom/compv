/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_TO_GRAYSCALE_H_)
#define _COMPV_BASE_IMAGE_CONV_TO_GRAYSCALE_H_

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class CompVImageConvToGrayscale
{
public:
	static COMPV_ERROR_CODE process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageGray, const bool enforceSingleThread = false);

private:
	static COMPV_ERROR_CODE yuv422family(const CompVMatPtr& imageYUV422family, CompVMatPtrPtr imageGray, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE rgbfamily(const CompVMatPtr& imageRGBfamily, CompVMatPtrPtr imageGray, const bool enforceSingleThread = false);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_CONV_TO_GRAYSCALE_H_ */
