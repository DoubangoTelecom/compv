/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_CL_CONFIG_H_
#define _COMPV_CL_CONFIG_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"

#include <CL/opencl.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

// Windows's symbols export
#if COMPV_OS_WINDOWS
#	if defined(COMPV_CL_EXPORTS)
# 		define COMPV_CL_API		__declspec(dllexport)
#	else
# 		define COMPV_CL_API		__declspec(dllimport)
#	endif
#else
#	define COMPV_CL_API
#endif

#define COMPV_CHECK_CL_CODE_NOP(clerr, ...) do { cl_int __clerr__ = (clerr); if (__clerr__ != CL_SUCCESS) { COMPV_DEBUG_ERROR_EX("OpenCL", "OpenCL operation failed (%d -> %s) -> " ##__VA_ARGS__, __clerr__, COMPV_NAMESPACE::CompVCLUtils::errorString(__clerr__)); } } while(0)
#define COMPV_CHECK_CL_CODE_BAIL(clerr, ...) do { cl_int __clerr__ = (clerr); if (__clerr__ != CL_SUCCESS) { COMPV_DEBUG_ERROR_EX("OpenCL", "OpenCL operation failed (%d -> %s) -> " ##__VA_ARGS__, __clerr__, COMPV_NAMESPACE::CompVCLUtils::errorString(__clerr__)); goto bail; } } while(0)
#define COMPV_CHECK_CL_CODE_RETURN(clerr, ...) do { cl_int __clerr__ = (clerr); if (__clerr__ != CL_SUCCESS) { COMPV_DEBUG_ERROR_EX("OpenCL", "OpenCL operation failed (%d -> %s) -> " ##__VA_ARGS__, __clerr__, COMPV_NAMESPACE::CompVCLUtils::errorString(__clerr__)); return __clerr__; } } while(0)
#define COMPV_CHECK_CL_CODE_ASSERT(clerr, ...) do { cl_int __clerr__ = (clerr); if (__clerr__ != CL_SUCCESS) { COMPV_DEBUG_ERROR_EX("OpenCL", "OpenCL operation failed (%d -> %s) -> " ##__VA_ARGS__, __clerr__, COMPV_NAMESPACE::CompVCLUtils::errorString(__clerr__)); COMPV_ASSERT(false); } } while(0)
#define COMPV_CHECK_CL_EXP_NOP(exp, clerr, ...) do { if ((exp)) COMPV_CHECK_CL_CODE_NOP(hr,  ##__VA_ARGS__); } while(0)
#define COMPV_CHECK_CL_EXP_RETURN(exp, clerr, ...) do { if ((exp)) COMPV_CHECK_CL_CODE_RETURN(hr,  ##__VA_ARGS__); } while(0)
#define COMPV_CHECK_CL_EXP_BAIL(exp, clerr, ...) do { if ((exp)) COMPV_CHECK_CL_CODE_BAIL(hr,  ##__VA_ARGS__); } while(0)

#endif /* _COMPV_CL_CONFIG_H_ */
