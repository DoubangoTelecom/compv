/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_SCALE_BICUBIC_H_)
#define _COMPV_BASE_IMAGE_SCALE_BICUBIC_H_

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

struct CompVImageScaleBicubicProcessor {
public:
	void (*bicubic_8u32f)(
		uint8_t* outPtr,
		const uint8_t* inPtr,
		const int32_t* xint1,
		const compv_float32_t* xfract1,
		const int32_t* yint1,
		const compv_float32_t* yfract1,
		const compv_uscalar_t inWidth,
		const compv_uscalar_t inHeight,
		const compv_uscalar_t inStride
	) = nullptr;
	COMPV_ERROR_CODE init();
};

class CompVImageScaleBicubic
{
public:
	static COMPV_ERROR_CODE process(const CompVMatPtr& imageIn, CompVMatPtr& imageOut);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_SCALE_BICUBIC_H_ */
