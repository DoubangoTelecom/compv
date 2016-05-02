/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/compv_gauss.h"
#include "compv/compv_math_utils.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

template class CompVGaussKern<double >;
template class CompVGaussKern<float >;

template<class T>
COMPV_ERROR_CODE CompVGaussKern<T>::buildKern2(CompVPtr<CompVArray<T>* >* kern, int size, T sigma)
{
    COMPV_CHECK_EXP_RETURN(!kern || !(size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    COMPV_CHECK_CODE_BAIL(err_ = CompVArray<T>::newObj(kern, 2, COMPV_ARRAY_DIM_SIZE2(size, size)));

    const T sigma2_times2 = 2 * (sigma * sigma); // 2*(sigma^2)
    const int size_div2 = (size >> 1);
    int x, y, kx, ky;
    T sum = 0, x2_plus_y2, y2, k;
    const T one_over_pi_times_sigma2_times2 = (1 / ((T)COMPV_MATH_PI * sigma2_times2)); // 1 / (2 * pi * sigma^2)
    T* kernel = (T*)(*kern)->getDataPtr();

#define kernelAt(_y_, _x_) *(kernel + ((_y_) * size) + (_x_))

    // Formula: https://en.wikipedia.org/wiki/Gaussian_blur
    // Ignore negative x and y as we'll be using x^2 and y^2 then, complete the kernel (symetric)
    for (ky = size_div2, y = 0; y <= size_div2; ++y, ++ky) {
        y2 = (T)(y * y);
        for (kx = size_div2, x = 0; x <= size_div2; ++x, ++kx) {
            x2_plus_y2 = (x * x) + y2;
            k = one_over_pi_times_sigma2_times2 * exp(-(x2_plus_y2 / sigma2_times2)); // x>=0 and y>=0
            kernelAt(ky, kx) = k;
            if (y != 0 || x != 0) {
                kernelAt(size_div2 - y, kx) = k;
                kernelAt(ky, size_div2 - x) = k;
                kernelAt(size_div2 - y, size_div2 - x) = k;
            }
        }
    }

    // Compute sum
    for (ky = 0; ky < size; ++ky) {
        for (kx = 0; kx < size; ++kx) {
            sum += kernelAt(ky, kx);
        }
    }

    // Normalize
	sum = 1 / sum;
    for (y = 0; y < size; ++y) {
        for (x = 0; x < size; ++x) {
            kernelAt(y, x) *= sum;
        }
    }

#undef kernelAt

bail:
    return err_;
}

template<class T>
COMPV_ERROR_CODE CompVGaussKern<T>::buildKern1(CompVPtr<CompVArray<T>* >* kern, int size, T sigma)
{
    COMPV_CHECK_EXP_RETURN(!kern || !(size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    COMPV_CHECK_CODE_BAIL(err_ = CompVArray<T>::newObj(kern, 1, COMPV_ARRAY_DIM_SIZE1(size)));

    const int size_div2 = (size >> 1);
	const T sigma2_times2 = 2 * (sigma * sigma); // 2*(sigma^2)
	const T one_over_sqrt_pi_times_sigma2_times2 = (1 / sqrt((T)COMPV_MATH_PI * sigma2_times2)); // 1 / sqrt(2 * pi * sigma^2)
	T sum, k;
    int x;
	T* kernel = (T*)(*kern)->getDataPtr();

    // for x = 0
    kernel[0 + size_div2] = one_over_sqrt_pi_times_sigma2_times2;
    sum = one_over_sqrt_pi_times_sigma2_times2;
    // for x = 1...
    for (x = 1; x <= size_div2; ++x) {
        k = one_over_sqrt_pi_times_sigma2_times2 * exp(-((x * x) / sigma2_times2));
        kernel[x + size_div2] = k;
        kernel[size_div2 - x] = k;
        sum += (k + k);
    }

    // Normalize
	sum = 1 / sum;
    for (x = 0; x < size; ++x) {
        kernel[x] *= sum;
    }

bail:
    return err_;
}

COMPV_NAMESPACE_END()
