/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathHistogram
{
public:
	static COMPV_ERROR_CODE build(const CompVMatPtr& dataIn, CompVMatPtrPtr histogram);
	static COMPV_ERROR_CODE equaliz(const CompVMatPtr& dataIn, CompVMatPtrPtr dataOut);
	static COMPV_ERROR_CODE equaliz(const CompVMatPtr& dataIn, const CompVMatPtr& histogram, CompVMatPtrPtr dataOut);

private:
	static COMPV_ERROR_CODE process_8u32u(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, uint32_t* histogramPtr);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_HISTOGRAM_H_ */
