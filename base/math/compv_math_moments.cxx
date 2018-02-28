/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_moments.h"

#define COMPV_THIS_CLASSNAME	"CompVMathMoments"

COMPV_NAMESPACE_BEGIN()

template<typename T>
static void CompVMathMomentsCentralFirstOrder(const T* data, const size_t width, const size_t height, const size_t stride, double* m00, double* m01, double* m10)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("SUM per row and using int32 then add to double");
#if 0 // Up to the caller function to initialize values to zero
	*m00 = 0.0;
	*m01 = 0.0;
	*m10 = 0.0;
#endif
	double& m00_ = *m00;
	double& m01_ = *m01;
	double& m10_ = *m10;
	const int32_t width_ = static_cast<int32_t>(width);
	const int32_t height_ = static_cast<int32_t>(height);
	for (int32_t j = 0; j < height_; j++) {
		for (int32_t i = 0; i < width_; i++) {
			const double pixel = data[i];
			m00_ += pixel;
			m01_ += (j * pixel);
			m10_ += (i * pixel);
		}
		data += stride;
	}
}

COMPV_ERROR_CODE CompVMathMoments::centralFirstOrder(const CompVMatPtr& ptrIn, double(&moments)[3], bool binar COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!ptrIn || ptrIn->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	moments[0] = 0.0;
	moments[1] = 0.0;
	moments[2] = 0.0;
	for (int plane = 0; plane < ptrIn->planeCount(); ++plane) {
		CompVMathMomentsCentralFirstOrder(
			ptrIn->ptr<const uint8_t>(0, 0, plane),
			ptrIn->cols(plane), ptrIn->rows(plane), ptrIn->stride(plane),
			&moments[0], &moments[1], &moments[2]
		);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
