/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/cl/compv_cl.h"
#include "compv/cl/compv_cl_common.h"
#include "compv/cl/compv_cl_utils.h"
#include "compv/base/compv_base.h"

#define COMPV_THIS_CLASSNAME "CompVCL"

COMPV_NAMESPACE_BEGIN()

bool CompVCL::s_bInitialized = false;
cl_platform_id CompVCL::s_clPlatformId = NULL;
cl_device_id CompVCL::s_clDeviceId = NULL;
cl_context CompVCL::s_clContext = NULL;
cl_command_queue CompVCL::s_clQueue = NULL;

COMPV_ERROR_CODE CompVCL::init()
{
	if (s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	cl_int clerr = CL_SUCCESS;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing [opencl] module (v %s)...", COMPV_VERSION_STRING);

	COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

	COMPV_CHECK_CL_CODE_BAIL(clerr = clGetPlatformIDs(1, &s_clPlatformId, NULL), "clGetPlatformIDs failed");
	COMPV_CHECK_CODE_NOP(CompVCLUtils::displayDevices(s_clPlatformId, CL_DEVICE_TYPE_GPU));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceIDs(s_clPlatformId, CL_DEVICE_TYPE_GPU, 1, &s_clDeviceId, NULL), "clGetDeviceIDs(CL_DEVICE_TYPE_GPU) failed");	
	s_clContext = clCreateContext(0, 1, &s_clDeviceId, NULL, NULL, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateContext failed");	
	s_clQueue = clCreateCommandQueue(s_clContext, s_clDeviceId, 0, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateCommandQueue failed");

	s_bInitialized = true;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err) || (clerr != CL_SUCCESS)) {
		COMPV_CHECK_CODE_NOP(CompVCL::deInit());
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OPENCL);
	}
	return err;
}

COMPV_ERROR_CODE CompVCL::deInit()
{
	COMPV_CHECK_CODE_ASSERT(CompVBase::deInit());

	if (s_clQueue) {
		clReleaseCommandQueue(s_clQueue);
		s_clQueue = NULL;
	}
	if (s_clContext) {
		clReleaseContext(s_clContext);
		s_clContext = NULL;
	}

	s_bInitialized = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
