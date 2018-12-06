/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_CL_UTILS_H_
#define _COMPV_CL_UTILS_H_

#include "compv/cl/compv_cl_config.h"
#include "compv/cl/compv_cl_common.h"
#include "compv/base/compv_mat.h"

#include <CL/opencl.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVCLUtils
{
public:
	static COMPV_ERROR_CODE createProgramWithSource(cl_program* program, cl_context context, const char* filename);
	static COMPV_ERROR_CODE buildProgram(cl_program program, cl_device_id deviceId);
	static COMPV_ERROR_CODE createKernel(cl_kernel* kernel, cl_program program, const char* name);
	static COMPV_ERROR_CODE createDataStrideless(const CompVMatPtr& hostdata, cl_mem* devdata, const cl_mem_flags& devdataFlags, cl_context clContext, cl_command_queue clCommand);
	static COMPV_ERROR_CODE duration(cl_event evt, double& millis);

	static COMPV_ERROR_CODE displayDevices(cl_platform_id platform, cl_device_type deviceType);
	static const char* errorString(cl_int err);

private:
	
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CL_UTILS_H_ */
