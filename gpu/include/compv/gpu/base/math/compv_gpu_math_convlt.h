/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GPU_BASE_MATH_CONVLT_H_)
#define _COMPV_GPU_BASE_MATH_CONVLT_H_

#include "compv/gpu/compv_gpu_config.h"
#include "compv/gpu/compv_gpu.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

typedef COMPV_ERROR_CODE(*gpu_convlt1VtHz_8u8u32f)(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize);

class COMPV_GPU_API CompVGpuMathConvlt
{
public:
	static gpu_convlt1VtHz_8u8u32f convlt1VtHz_8u8u32f;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_GPU_BASE_MATH_CONVLT_H_ */
