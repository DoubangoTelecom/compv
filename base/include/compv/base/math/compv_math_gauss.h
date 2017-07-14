/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_GAUSS_H_)
#define _COMPV_BASE_MATH_GAUSS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_mem.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/math/compv_math_convlt.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathGauss
{ 
public:
	template <typename T = compv_float32_t,
		typename = std::enable_if<std::is_floating_point<T>::value>>
	static COMPV_ERROR_CODE kernelDim2(CompVMatPtrPtr kernel, size_t size, float sigma)
	{
		COMPV_CHECK_EXP_RETURN(!kernel || !(size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		size_t stride = static_cast<size_t>(CompVMem::alignForward(size));
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<T>(kernel, size, size, stride)));

		const T sigma2_times2 = static_cast<T>(2 * (sigma * sigma)); // 2*(sigma^2)
		const size_t size_div2 = (size >> 1);
		size_t x, y, kx, ky;
		T sum = 0, x2_plus_y2, y2, k;
		const T one_over_pi_times_sigma2_times2 = static_cast<T>(1 / (COMPV_MATH_PI * sigma2_times2)); // 1 / (2 * pi * sigma^2)
		CompVMatPtr kernel_ = *kernel;

#define compv_kernelAt(_y_, _x_) *kernel_->ptr<T>(_y_, _x_)

		// Formula: https://en.wikipedia.org/wiki/Gaussian_blur
		// Ignore negative x and y as we'll be using x^2 and y^2 then, complete the kernel (symetric)
		for (ky = size_div2, y = 0; y <= size_div2; ++y, ++ky) {
			y2 = static_cast<T>(y * y);
			for (kx = size_div2, x = 0; x <= size_div2; ++x, ++kx) {
				x2_plus_y2 = (x * x) + y2;
				k = static_cast<T>(one_over_pi_times_sigma2_times2 * exp(-static_cast<double>(x2_plus_y2 / sigma2_times2))); // x>=0 and y>=0
				compv_kernelAt(ky, kx) = k;
				if (y != 0 || x != 0) {
					compv_kernelAt(size_div2 - y, kx) = k;
					compv_kernelAt(ky, size_div2 - x) = k;
					compv_kernelAt(size_div2 - y, size_div2 - x) = k;
				}
			}
		}

		// Compute sum
		for (ky = 0; ky < size; ++ky) {
			for (kx = 0; kx < size; ++kx) {
				sum += compv_kernelAt(ky, kx);
			}
		}

		// Normalize
		sum = 1 / sum;
		for (y = 0; y < size; ++y) {
			for (x = 0; x < size; ++x) {
				compv_kernelAt(y, x) *= sum;
			}
		}

#undef compv_kernelAt

		return COMPV_ERROR_CODE_S_OK;
	}
	template <typename T = compv_float32_t,
		typename = std::enable_if<std::is_floating_point<T>::value>>
	static COMPV_ERROR_CODE kernelDim1(CompVMatPtrPtr kernel, size_t size, float sigma)
	{
		COMPV_CHECK_EXP_RETURN(!kernel || !(size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		size_t stride = static_cast<size_t>(CompVMem::alignForward(size));
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<T>(kernel, 1, size, stride)));

		const size_t size_div2 = (size >> 1);
		const T sigma2_times2 = static_cast<T>(2 * (sigma * sigma)); // 2*(sigma^2)
		const T one_over_sqrt_pi_times_sigma2_times2 = static_cast<T>(1 / sqrt(COMPV_MATH_PI * sigma2_times2)); // 1 / sqrt(2 * pi * sigma^2)
		T sum, k;
		size_t x;
		T* kernel_ = (*kernel)->ptr<T>();

		// for x = 0
		kernel_[0 + size_div2] = one_over_sqrt_pi_times_sigma2_times2;
		sum = one_over_sqrt_pi_times_sigma2_times2;
		// for x = 1...
		for (x = 1; x <= size_div2; ++x) {
			k = static_cast<T>(one_over_sqrt_pi_times_sigma2_times2 * exp(-static_cast<double>((x * x) / sigma2_times2)));
			kernel_[x + size_div2] = k;
			kernel_[size_div2 - x] = k;
			sum += (k + k);
		}

		// Normalize
		sum = 1 / sum;
		for (x = 0; x < size; ++x) {
			kernel_[x] *= sum;
		}

		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE kernelDim1FixedPoint(CompVMatPtrPtr fixedPointKernel, size_t size, float sigma);
	static COMPV_ERROR_CODE kernelDim2FixedPoint(CompVMatPtrPtr fixedPointKernel, size_t size, float sigma);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_GAUSS_H_ */
