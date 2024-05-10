/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gpu/compv_gpu.h"
#include "compv/gpu/compv_gpu_common.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_fileutils.h"

#include "compv/gpu/base/math/compv_gpu_math_convlt.h"

#include "compv/gpu/core/features/fast/compv_gpu_feature_fast_dete.h"

#define COMPV_THIS_CLASSNAME "CompVGpu"

#if COMPV_OS_WINDOWS
#	define COMPV_GPU_OPENCL_SHAREDLIB_NAME "CompVPluginOpenCL.dll"
#else
#	define COMPV_GPU_OPENCL_SHAREDLIB_NAME "libCompVPluginOpenCL.so"
#endif

// Zero-copy rules: https://software.intel.com/en-us/articles/getting-the-most-from-opencl-12-how-to-increase-performance-by-minimizing-buffer-copies-on-intel-processor-graphics

COMPV_NAMESPACE_BEGIN()

bool CompVGpu::s_bInitialized = false;
bool CompVGpu::s_bActive = false;
bool CompVGpu::s_bEnabled = true;
bool CompVGpu::s_bCudaSupported = false;
CompVSharedLibPtr CompVGpu::s_ptrImpl = nullptr;

static bool CompVGpu_isCudaSupported();

COMPV_ERROR_CODE CompVGpu::init()
{
	if (isInitialized()) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	bool openclIntialized = false;
	bool cudaIntialized = false;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing [gpu] module (v %s)...", COMPV_VERSION_STRING);

	COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

	// Load OpenCL shared library
	// See ultimateBase for GPGPU functions: https://github.com/DoubangoTelecom/ultimateBase

	s_bCudaSupported = CompVGpu_isCudaSupported();
	s_bActive = s_ptrImpl && (openclIntialized || cudaIntialized);
	s_bInitialized = true;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(CompVGpu::deInit());
	}
	return err;
}

COMPV_ERROR_CODE CompVGpu::deInit()
{
	// Release the newObj pointers then, close the shared library
	CompVGpuCornerDeteFAST::s_ptrNewObj = NULL;
	s_ptrImpl = nullptr;

	COMPV_CHECK_CODE_NOP(CompVBase::deInit());

	s_bInitialized = false;
	s_bActive = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGpu::setEnabled(bool enabled)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "GPU enabled: %s", enabled ? "true" : "false");
	s_bEnabled = enabled;
	return COMPV_ERROR_CODE_S_OK;
}

static bool CompVGpu_isCudaSupported()
{
	static bool __checked = false;
	static bool __supported = false;

	if (__checked) {
		return __supported;
	}
	__checked = true;

	static const char* kCUDA_dll_name =
#if COMPV_OS_WINDOWS
		"nvcuda.dll";
#else
		"libcuda.so";
#endif
	CompVSharedLibPtr sharedlib;
	COMPV_ERROR_CODE compv_err = CompVSharedLib::newObj(&sharedlib, kCUDA_dll_name, true);
	if (!sharedlib || COMPV_ERROR_CODE_IS_NOK(compv_err)) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Failed to open CUDA lib");
		return (__supported = false);
	}

	int(*cuInit_pt)(unsigned int Flags) = reinterpret_cast<decltype(cuInit_pt)>(sharedlib->sym("cuInit"));
	if (!cuInit_pt) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find cuInit");
		return (__supported = false);
	}
	int cuda_err = cuInit_pt(0);
	if (cuda_err) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "cuInit failed with error code %d", cuda_err);
		return (__supported = false);
	}

	int(*cuDeviceGetCount_pt)(int *count) = reinterpret_cast<decltype(cuDeviceGetCount_pt)>(sharedlib->sym("cuDeviceGetCount"));
	if (!cuDeviceGetCount_pt) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find cuDeviceGetCount");
		return (__supported = false);
	}
	int numGPUs = 0;
	cuda_err = cuDeviceGetCount_pt(&numGPUs);
	if (cuda_err) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "cuDeviceGetCount failed with error code %d", cuda_err);
		return (__supported = false);
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "=== Number of CUDA devices: %d ===", numGPUs);
	return (__supported = (numGPUs > 0));
}

COMPV_NAMESPACE_END()
