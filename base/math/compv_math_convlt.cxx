/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_convlt.h"

COMPV_NAMESPACE_BEGIN()

// InputType = uint8_t, KernelType = int16_t, OutputType = uint8_t, FixedPoint = true
template<> COMPV_BASE_API void CompVMathConvlt::convlt1VtHz_private_fxp_true(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t stride, size_t pad, const int16_t* vthzKernPtr, size_t kernSize)
{
	CompVMathConvlt::convlt1VtHzFixedPoint_C(inPtr, outPtr, width, height, stride, pad, vthzKernPtr, kernSize);
}

// InputType = uint8_t, KernelType = compv_float32_t, OutputType = uint8_t, FixedPoint = false
template<> COMPV_BASE_API void CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t stride, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize)
{
	CompVMathConvlt::convlt1VtHzKernelFloat_C<uint8_t, compv_float32_t, uint8_t>(inPtr, outPtr, width, height, stride, pad, vthzKernPtr, kernSize);
}

COMPV_NAMESPACE_END()
