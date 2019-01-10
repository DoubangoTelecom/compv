/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CL_FEATURE_FAST_DETE_H_)
#define _COMPV_CL_FEATURE_FAST_DETE_H_

#include "compv/cl/compv_cl_config.h"
#include "compv/gpu/base/math/compv_gpu_math_convlt.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"

#include <CL/opencl.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_EXTERNC COMPV_CL_API gpu_convlt1VtHz_8u8u32f cl_convlt1VtHz_8u8u32f;

COMPV_NAMESPACE_END()

#endif /* _COMPV_CL_FEATURE_FAST_DETE_H_ */
