/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#include "compv/cl/compv_cl_utils.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_fileutils.h"

#define COMPV_THIS_CLASSNAME "CompVCLUtils"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVCLUtils::createProgramWithSource(cl_program* program, cl_context context, const char* filename)
{
	COMPV_CHECK_EXP_RETURN(!program || !context || !filename, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBufferPtr source;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(CompVFileUtils::getFullPathFromFileName(filename).c_str(), &source));
	cl_int status = CL_SUCCESS;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	const char* kernelSource = reinterpret_cast<const char*>(source->ptr());
	*program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &status);
	COMPV_CHECK_CL_CODE_BAIL(status, "clCreateProgramWithSource failed");

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err) && status != CL_SUCCESS) {
		if (*program) {
			clReleaseProgram(*program);
			*program = NULL;
		}
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OPENCL);
	}
	return err;
}

COMPV_ERROR_CODE CompVCLUtils::buildProgram(cl_program program, cl_device_id deviceId)
{
	COMPV_CHECK_EXP_RETURN(!program, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	std::string options = ""; // FIXME: set to -Werror
#if DEBUG || _DEBUG
	options += std::string(" -g");
#endif
	cl_int status = clBuildProgram(program, 0, NULL, options.c_str(), NULL, NULL);
	if (status != CL_SUCCESS) {
		size_t siz;
		if (clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &siz) == CL_SUCCESS) {
			char* info = reinterpret_cast<char *>(CompVMem::malloc(siz));
			if (info) {
				if (clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, siz, info, NULL) == CL_SUCCESS) {
					COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Build OpenCL program (%p) failed: %s", program, info);
				}
				CompVMem::free(reinterpret_cast<void**>(&info));
			}
			COMPV_CHECK_CL_CODE_BAIL(status);
		}
	}
bail:
	COMPV_CHECK_EXP_RETURN((status != CL_SUCCESS), COMPV_ERROR_CODE_E_OPENCL);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCLUtils::createKernel(cl_kernel* kernel, cl_program program, const char* name)
{
	COMPV_CHECK_EXP_RETURN(!kernel || !program || !name, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	cl_int status = CL_SUCCESS;
	*kernel = clCreateKernel(program, name, &status);
	COMPV_CHECK_CL_CODE_BAIL(status);

bail:
	if (status != CL_SUCCESS) {
		if (*kernel) {
			clReleaseKernel(*kernel);
			*kernel = NULL;
		}
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OPENCL);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCLUtils::displayDevices(cl_platform_id platform, cl_device_type deviceType)
{
	char tmpString[1024];
	cl_uint deviceCount;
	cl_device_id* deviceIds = NULL;
	cl_int status = CL_SUCCESS;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	// Get platform name
	COMPV_CHECK_CL_CODE_BAIL(status = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(tmpString),
		tmpString, NULL), "clGetPlatformInfo failed");
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Selected platform vendor: %s", tmpString);
	
	// Get number of devices available
	deviceCount = 0;
	COMPV_CHECK_CL_CODE_BAIL(status = clGetDeviceIDs(platform, deviceType, 0, NULL, &deviceCount), "clGetDeviceIDs failed");
	deviceIds = reinterpret_cast<cl_device_id*>(CompVMem::malloc(sizeof(cl_device_id) *	deviceCount));
	COMPV_CHECK_EXP_BAIL(!deviceIds, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "failed to allocate device list");
	
	// Get device ids
	COMPV_CHECK_CL_CODE_BAIL(status = clGetDeviceIDs(platform, deviceType, deviceCount, deviceIds, NULL), "clGetDeviceIDs failed");
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "deviceCount=%d", deviceCount);
	
	// Print device index and device names
	for (cl_uint i = 0; i < deviceCount; ++i) {
		COMPV_CHECK_CL_CODE_BAIL(status = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, sizeof(tmpString),
			tmpString, NULL), "clGetDeviceInfo failed");
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Device -> name: %s, id: %p", tmpString, deviceIds[i]);
	}
	
bail:
	CompVMem::free(reinterpret_cast<void**>(&deviceIds));
	if (COMPV_ERROR_CODE_IS_NOK(err) && status != CL_SUCCESS) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OPENCL);
	}
	return err;
}

const char* CompVCLUtils::errorString(cl_int err)
{
	switch (err)
	{
	case CL_DEVICE_NOT_FOUND:
		return "CL_DEVICE_NOT_FOUND";
	case CL_DEVICE_NOT_AVAILABLE:
		return "CL_DEVICE_NOT_AVAILABLE";
	case CL_COMPILER_NOT_AVAILABLE:
		return "CL_COMPILER_NOT_AVAILABLE";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:
		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case CL_OUT_OF_RESOURCES:
		return "CL_OUT_OF_RESOURCES";
	case CL_OUT_OF_HOST_MEMORY:
		return "CL_OUT_OF_HOST_MEMORY";
	case CL_PROFILING_INFO_NOT_AVAILABLE:
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case CL_MEM_COPY_OVERLAP:
		return "CL_MEM_COPY_OVERLAP";
	case CL_IMAGE_FORMAT_MISMATCH:
		return "CL_IMAGE_FORMAT_MISMATCH";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case CL_BUILD_PROGRAM_FAILURE:
		return "CL_BUILD_PROGRAM_FAILURE";
	case CL_MAP_FAILURE:
		return "CL_MAP_FAILURE";
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:
		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case CL_INVALID_VALUE:
		return "CL_INVALID_VALUE";
	case CL_INVALID_DEVICE_TYPE:
		return "CL_INVALID_DEVICE_TYPE";
	case CL_INVALID_PLATFORM:
		return "CL_INVALID_PLATFORM";
	case CL_INVALID_DEVICE:
		return "CL_INVALID_DEVICE";
	case CL_INVALID_CONTEXT:
		return "CL_INVALID_CONTEXT";
	case CL_INVALID_QUEUE_PROPERTIES:
		return "CL_INVALID_QUEUE_PROPERTIES";
	case CL_INVALID_COMMAND_QUEUE:
		return "CL_INVALID_COMMAND_QUEUE";
	case CL_INVALID_HOST_PTR:
		return "CL_INVALID_HOST_PTR";
	case CL_INVALID_MEM_OBJECT:
		return "CL_INVALID_MEM_OBJECT";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case CL_INVALID_IMAGE_SIZE:
		return "CL_INVALID_IMAGE_SIZE";
	case CL_INVALID_SAMPLER:
		return "CL_INVALID_SAMPLER";
	case CL_INVALID_BINARY:
		return "CL_INVALID_BINARY";
	case CL_INVALID_BUILD_OPTIONS:
		return "CL_INVALID_BUILD_OPTIONS";
	case CL_INVALID_PROGRAM:
		return "CL_INVALID_PROGRAM";
	case CL_INVALID_PROGRAM_EXECUTABLE:
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case CL_INVALID_KERNEL_NAME:
		return "CL_INVALID_KERNEL_NAME";
	case CL_INVALID_KERNEL_DEFINITION:
		return "CL_INVALID_KERNEL_DEFINITION";
	case CL_INVALID_KERNEL:
		return "CL_INVALID_KERNEL";
	case CL_INVALID_ARG_INDEX:
		return "CL_INVALID_ARG_INDEX";
	case CL_INVALID_ARG_VALUE:
		return "CL_INVALID_ARG_VALUE";
	case CL_INVALID_ARG_SIZE:
		return "CL_INVALID_ARG_SIZE";
	case CL_INVALID_KERNEL_ARGS:
		return "CL_INVALID_KERNEL_ARGS";
	case CL_INVALID_WORK_DIMENSION:
		return "CL_INVALID_WORK_DIMENSION";
	case CL_INVALID_WORK_GROUP_SIZE:
		return "CL_INVALID_WORK_GROUP_SIZE";
	case CL_INVALID_WORK_ITEM_SIZE:
		return "CL_INVALID_WORK_ITEM_SIZE";
	case CL_INVALID_GLOBAL_OFFSET:
		return "CL_INVALID_GLOBAL_OFFSET";
	case CL_INVALID_EVENT_WAIT_LIST:
		return "CL_INVALID_EVENT_WAIT_LIST";
	case CL_INVALID_EVENT:
		return "CL_INVALID_EVENT";
	case CL_INVALID_OPERATION:
		return "CL_INVALID_OPERATION";
	case CL_INVALID_GL_OBJECT:
		return "CL_INVALID_GL_OBJECT";
	case CL_INVALID_BUFFER_SIZE:
		return "CL_INVALID_BUFFER_SIZE";
	case CL_INVALID_MIP_LEVEL:
		return "CL_INVALID_MIP_LEVEL";
	case CL_INVALID_GLOBAL_WORK_SIZE:
		return "CL_INVALID_GLOBAL_WORK_SIZE";
#if defined(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR)
	case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
		return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
#endif
#if defined(CL_PLATFORM_NOT_FOUND_KHR)
	case CL_PLATFORM_NOT_FOUND_KHR:
		return "CL_PLATFORM_NOT_FOUND_KHR";
#endif
#if defined(CL_DEVICE_PARTITION_FAILED_EXT)
	case CL_DEVICE_PARTITION_FAILED_EXT:
		return "CL_DEVICE_PARTITION_FAILED_EXT";
#endif
#if defined(CL_INVALID_PARTITION_COUNT_EXT)
	case CL_INVALID_PARTITION_COUNT_EXT:
		return "CL_INVALID_PARTITION_COUNT_EXT";
#endif
	default:
		return "unknown error code";
	}
}

COMPV_NAMESPACE_END()

