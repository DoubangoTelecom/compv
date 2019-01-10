/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_GAUSS_H_ */
