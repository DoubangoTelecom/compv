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
	void (*NOT_OPTIMIZ_hermite_32f32s)(
		compv_float32_t* outPtr,
		const compv_float32_t* inPtr,
		const int32_t* xint1,
		const compv_float32_t* xfract1,
		const int32_t* yint1,
		const compv_float32_t* yfract1,
		const compv_uscalar_t inWidthMinus1,
		const compv_uscalar_t inHeightMinus1,
		const compv_uscalar_t inStride
	) = nullptr;
	void (*postprocessrow_32f32s)(
		compv_float32_t* outPtr,
		const compv_float32_t* inPtr,
		const int32_t* xint4,
		const compv_float32_t* xfract4,
		const int32_t* yint4,
		const compv_float32_t* yfract4,
		const compv_uscalar_t rowCount
	) = nullptr;
	void (*preprocess_32s32f)(
		int32_t* intergral,
		compv_float32_t* fraction,
		const compv_float32_t* sv1,
		const compv_uscalar_t outSize,
		const compv_scalar_t intergralMax,
		const compv_scalar_t intergralStride
	) = nullptr;
	COMPV_ERROR_CODE init();
};

class CompVImageScaleBicubic
{
public:
	static COMPV_ERROR_CODE process(const CompVMatPtr& imageIn, CompVMatPtr& imageOut, const COMPV_INTERPOLATION_TYPE bicubicType = COMPV_INTERPOLATION_TYPE_BICUBIC);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_SCALE_BICUBIC_H_ */
