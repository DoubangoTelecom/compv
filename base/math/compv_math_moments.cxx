/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_moments.h"

#define COMPV_THIS_CLASSNAME	"CompVMathMoments"

COMPV_NAMESPACE_BEGIN()

// Up to the caller function to initialize values to zero
template<typename T>
static void CompVMathMomentsRawFirstOrder(const T* data, const size_t width, const size_t height, const size_t stride, double* m00, double* m01, double* m10)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("SUM per row and using int32 then add to double");

	double& m00_ = *m00;
	double& m01_ = *m01;
	double& m10_ = *m10;
	const int32_t width_ = static_cast<int32_t>(width);
	const int32_t height_ = static_cast<int32_t>(height);
	for (int32_t j = 0; j < height_; j++) {
		for (int32_t i = 0; i < width_; i++) {
			if (data[i]) {
				const double pixel = data[i];
				m00_ += pixel;
				m01_ += (j * pixel);
				m10_ += (i * pixel);
			}
		}
		data += stride;
	}
}

// Up to the caller function to initialize values to zero
template<typename T>
static void CompVMathMomentsRawSecondOrder(const T* data, const size_t width, const size_t height, const size_t stride, double* m00, double* m01, double* m10, double* m11, double* m02, double* m20)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("SUM per row and using int32 then add to double");

	double& m00_ = *m00;
	double& m01_ = *m01;
	double& m10_ = *m10;
	double& m11_ = *m11;
	double& m02_ = *m02;
	double& m20_ = *m20;
	const int32_t width_ = static_cast<int32_t>(width);
	const int32_t height_ = static_cast<int32_t>(height);
	for (int32_t j = 0; j < height_; j++) {
		for (int32_t i = 0; i < width_; i++) {
			if (data[i]) {
				const double pixel = data[i];
				const double k = (j * pixel);
				const double p = (i * pixel);
				m00_ += pixel;
				m01_ += k; /* = (j * pixel) */
				m10_ += p; /* sum(x) */
				m11_ += (k * i); /* sum(x*y) = ((i * j) * pixel) = ((j * pixel) * i) = (k * i) */
				m02_ += (k * j); /* = ((j * j) * pixel) */
				m20_ += (p * i);/* = ((i * i) * pixel) = ((i * pixel) * i) = (p * i) */
			}
		}
		data += stride;
	}
}

COMPV_ERROR_CODE CompVMathMoments::rawFirstOrder(const CompVMatPtr& ptrIn, double(&moments)[3], bool binar COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!ptrIn || ptrIn->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	moments[0] = 0.0; // M00(area)
	moments[1] = 0.0; // M01(y)
	moments[2] = 0.0; // M10(x)
	for (int plane = 0; plane < ptrIn->planeCount(); ++plane) {
		CompVMathMomentsRawFirstOrder(
			ptrIn->ptr<const uint8_t>(0, 0, plane),
			ptrIn->cols(plane), ptrIn->rows(plane), ptrIn->stride(plane),
			&moments[0], &moments[1], &moments[2]
		);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMoments::rawSecondOrder(const CompVMatPtr& ptrIn, double(&moments)[6], bool binar COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!ptrIn || ptrIn->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	moments[0] = 0.0; // M00(area)
	moments[1] = 0.0; // M01(y)
	moments[2] = 0.0; // M10(x)
	moments[3] = 0.0; // M11(x*y)
	moments[4] = 0.0; // M02(y*y)
	moments[5] = 0.0; // M20(x*x)
	for (int plane = 0; plane < ptrIn->planeCount(); ++plane) {
		CompVMathMomentsRawSecondOrder(
			ptrIn->ptr<const uint8_t>(0, 0, plane),
			ptrIn->cols(plane), ptrIn->rows(plane), ptrIn->stride(plane),
			&moments[0], &moments[1], &moments[2], &moments[3], &moments[4], &moments[5]
		);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMoments::skewness(const CompVMatPtr& ptrIn, double& skew, bool binar COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!ptrIn || ptrIn->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	double m00 = 0, m01 = 0, m10 = 0, m11 = 0, m02 = 0, m20 = 0;
	for (int plane = 0; plane < ptrIn->planeCount(); ++plane) {
		CompVMathMomentsRawSecondOrder(
			ptrIn->ptr<const uint8_t>(0, 0, plane),
			ptrIn->cols(plane), ptrIn->rows(plane), ptrIn->stride(plane),
			&m00, &m01, &m10, &m11, &m02, &m20
		);
	}
	if (m00 != 0) {
		const double scale = 1. / m00;
		const double ybar = m01 * scale;
		const double xbar = m10 * scale;
		const double u02 = m02 - (ybar * m01);
		const double u11 = m11 - (xbar * m01);
		skew = (std::abs(u02) > DBL_EPSILON)
			? u11 / u02
			: 0.0;
	}
	else {
		skew = 0.0;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathMoments::orientation(const CompVMatPtr& ptrIn, double& theta, bool binar COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!ptrIn || ptrIn->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	double m00 = 0, m01 = 0, m10 = 0, m11 = 0, m02 = 0, m20 = 0;
	for (int plane = 0; plane < ptrIn->planeCount(); ++plane) {
		CompVMathMomentsRawSecondOrder(
			ptrIn->ptr<const uint8_t>(0, 0, plane),
			ptrIn->cols(plane), ptrIn->rows(plane), ptrIn->stride(plane),
			&m00, &m01, &m10, &m11, &m02, &m20
		);
	}
	if (m00 != 0) {
		const double scale = 1. / m00;
		const double ybar = m01 * scale;
		const double xbar = m10 * scale;
		const double u20 = (m20 * scale) - (xbar * xbar);
		const double u02 = (m02 * scale) - (ybar * ybar);
		const double u11 = (m11 * scale) - (xbar * ybar);
		const double denom = (u20 - u02);
		theta = (std::abs(denom) > DBL_EPSILON)
			? 0.5 * std::atan2(2 * u11, denom)
			: 0.0;
	}
	else {
		theta = 0.0;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
