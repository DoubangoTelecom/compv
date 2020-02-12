/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_HISTOGRAM_H_)
#define _COMPV_BASE_MATH_HISTOGRAM_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVMathHistogram
{
public:
	static COMPV_ERROR_CODE build(const CompVMatPtr& dataIn, CompVMatPtrPtr histogram);
	static COMPV_ERROR_CODE buildProjectionY(const CompVMatPtr& dataIn, CompVMatPtrPtr ptr32sProjection);
	static COMPV_ERROR_CODE buildProjectionX(const CompVMatPtr& dataIn, CompVMatPtrPtr ptr32sProjection);
	static COMPV_ERROR_CODE equaliz(const CompVMatPtr& dataIn, CompVMatPtrPtr dataOut);
	static COMPV_ERROR_CODE equaliz(const CompVMatPtr& dataIn, const CompVMatPtr& histogram, CompVMatPtrPtr dataOut);

private:
	static COMPV_ERROR_CODE process_8u32u(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, uint32_t* histogramPtr);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_HISTOGRAM_H_ */
