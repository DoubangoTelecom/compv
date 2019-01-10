/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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
bool CompVCL::s_cl_khr_fp64_supported = false;
cl_platform_id CompVCL::s_clPlatformId = nullptr;
cl_device_id CompVCL::s_clDeviceId = nullptr;

COMPV_ERROR_CODE CompVCL::init()
{
	if (isInitialized()) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	cl_int clerr = CL_SUCCESS;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing [opencl] module (v %s)...", COMPV_VERSION_STRING);

	// Init CompVbase if not already done
	COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

	// Get a platform
	COMPV_CHECK_CL_CODE_BAIL(clerr = clGetPlatformIDs(1, &s_clPlatformId, NULL), "clGetPlatformIDs failed");
	
	// Display devices and choose GPU one
	COMPV_CHECK_CODE_NOP(CompVCLUtils::displayDevices(s_clPlatformId, CL_DEVICE_TYPE_GPU));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceIDs(s_clPlatformId, CL_DEVICE_TYPE_GPU, 1, &s_clDeviceId, NULL), "clGetDeviceIDs(CL_DEVICE_TYPE_GPU) failed");

	// Print some useful info
	{
		cl_uint uint_val;
		cl_ulong ulong_val;
		size_t size_val;
		char buffer[1024];
		std::vector<size_t> vecMaXWorkItemDimensions;
		 
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(uint_val), &uint_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT=%u", uint_val);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(uint_val), &uint_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE=%u", uint_val);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(uint_val), &uint_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_MAX_COMPUTE_UNITS=%u", uint_val);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(uint_val), &uint_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS=%u", uint_val);
		vecMaXWorkItemDimensions.resize(uint_val);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * vecMaXWorkItemDimensions.size(), vecMaXWorkItemDimensions.data(), nullptr));
		{
			std::string sizes;
			for (std::vector<size_t>::const_iterator i = vecMaXWorkItemDimensions.begin(); i != vecMaXWorkItemDimensions.end(); ++i) {
				sizes += CompVBase::to_string(*i) + std::string(", ");
			}
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_MAX_WORK_ITEM_SIZES=%s", sizes.c_str());
		}
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_val), &size_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_MAX_WORK_GROUP_SIZE=%zu", size_val);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(uint_val), &uint_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_MAX_CLOCK_FREQUENCY=%u MHz", uint_val);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(uint_val), &uint_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE=%u B", uint_val);		
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ulong_val), &ulong_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_GLOBAL_MEM_SIZE=%llu B (%llu MB)", ulong_val, ulong_val >> 20);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(ulong_val), &ulong_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_LOCAL_MEM_SIZE=%llu B (%llu KB)", ulong_val, ulong_val >> 10);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(ulong_val), &ulong_val, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_MAX_MEM_ALLOC_SIZE=%llu MB", ulong_val >> 20);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetPlatformInfo(s_clPlatformId, CL_PLATFORM_VERSION, sizeof(buffer), buffer, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_PLATFORM_VERSION=%s", buffer);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_VERSION, sizeof(buffer), buffer, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_VERSION=%s", buffer);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DRIVER_VERSION, sizeof(buffer), buffer, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DRIVER_VERSION=%s", buffer);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_OPENCL_C_VERSION, sizeof(buffer), buffer, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_OPENCL_C_VERSION=%s", buffer);
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetDeviceInfo(s_clDeviceId, CL_DEVICE_EXTENSIONS, sizeof(buffer), buffer, nullptr));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "CL_DEVICE_EXTENSIONS=%s", buffer);
		s_cl_khr_fp64_supported = (strstr(buffer, "cl_khr_fp64") != nullptr);
	}

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
#if 0 // MUST not deInit() CompVBase, OpenCL can safely fail loading or initializing
	COMPV_CHECK_CODE_ASSERT(CompVBase::deInit());
#endif

	s_bInitialized = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
