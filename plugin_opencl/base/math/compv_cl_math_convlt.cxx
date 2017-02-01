/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/cl/base/math/compv_cl_math_convlt.h"
#include "compv/cl/compv_cl.h"
#include "compv/cl/compv_cl_utils.h"
#include "compv/base/compv_mem.h"

COMPV_NAMESPACE_BEGIN()

// FIXME: kernel and program not released

static cl_program __cl_convlt1VtHz_8u8u32f_program = NULL;
static cl_kernel __cl_convlt1VtHz_8u8u32f_kernel = NULL;
static COMPV_ERROR_CODE __cl_convlt1VtHz_8u8u32f(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize)
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	size_t trueStride = (width + pad); // stride passed in arg is the process step
	cl_int clerr = CL_SUCCESS;
	cl_mem cl_inPtr = NULL, cl_outPtr = NULL, cl_vthzKernPtr = NULL;
	unsigned int width_ = static_cast<unsigned int>(width)
		, height_ = static_cast<unsigned int>(height)
		, step_ = static_cast<unsigned int>(step)
		, pad_ = static_cast<unsigned int>(pad)
		, kernSize_ = static_cast<unsigned int>(kernSize);
	void* mappedBuff = NULL;
	size_t global[2] = {
		static_cast<size_t>(CompVMem::alignForward(trueStride, COMPV_ALIGNV_GPU_LINE)),
		height
	};
	size_t local[2] = { // FIXME: max = 256 for AMD, must be retrieve at runtime. This means (local[0] * local[1]) must be <= 256
		COMPV_ALIGNV_GPU_LINE,
		1
	};

	if (!__cl_convlt1VtHz_8u8u32f_program || !__cl_convlt1VtHz_8u8u32f_kernel) {
		COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createProgramWithSource(&__cl_convlt1VtHz_8u8u32f_program, CompVCL::clContext(), "compv_cl_math_convlt.cl"));
		COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::buildProgram(__cl_convlt1VtHz_8u8u32f_program, CompVCL::clDeviceId()));
		COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&__cl_convlt1VtHz_8u8u32f_kernel, __cl_convlt1VtHz_8u8u32f_program, "clConvlt1VtHz_8u8u32f"));
	}

	if (!outPtr) {
		outPtr = reinterpret_cast<uint8_t*>(CompVMem::malloc((trueStride * height) * sizeof(uint8_t)));
		COMPV_CHECK_EXP_BAIL(!outPtr, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to allocate out memory");
	}

	cl_inPtr = clCreateBuffer(CompVCL::clContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, static_cast<size_t>((trueStride * height) * sizeof(uint8_t)), (void*)inPtr, &clerr);
	COMPV_CHECK_EXP_BAIL((clerr != CL_SUCCESS), (err = COMPV_ERROR_CODE_E_OPENCL), "clCreateBuffer failed");
	cl_outPtr = clCreateBuffer(CompVCL::clContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, static_cast<size_t>((trueStride * height) * sizeof(uint8_t)), (void*)outPtr, &clerr);
	COMPV_CHECK_EXP_BAIL((clerr != CL_SUCCESS), (err = COMPV_ERROR_CODE_E_OPENCL), "clCreateBuffer failed");
	cl_vthzKernPtr = clCreateBuffer(CompVCL::clContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, static_cast<size_t>(kernSize * sizeof(compv_float32_t)), (void*)vthzKernPtr, &clerr);
	COMPV_CHECK_EXP_BAIL((clerr != CL_SUCCESS), (err = COMPV_ERROR_CODE_E_OPENCL), "clCreateBuffer failed");

	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 0, sizeof(cl_mem), &cl_inPtr)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");
	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 1, sizeof(cl_mem), &cl_outPtr)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");
	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 2, sizeof(unsigned int), &width_)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");
	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 3, sizeof(unsigned int), &height_)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");
	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 4, sizeof(unsigned int), &step_)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");
	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 5, sizeof(unsigned int), &pad_)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");
	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 6, sizeof(cl_mem), &cl_vthzKernPtr)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");
	COMPV_CHECK_EXP_BAIL((clerr = clSetKernelArg(__cl_convlt1VtHz_8u8u32f_kernel, 7, sizeof(unsigned int), &kernSize_)) != CL_SUCCESS, (err = COMPV_ERROR_CODE_E_OPENCL), "clSetKernelArg failed");

	/*  Write */
	mappedBuff = clEnqueueMapBuffer(CompVCL::clCommandQueue(), cl_inPtr, CL_TRUE,
		CL_MAP_WRITE, 0, static_cast<size_t>((trueStride * height) * sizeof(uint8_t)), 0, NULL, NULL, &clerr);
	COMPV_CHECK_EXP_BAIL((clerr != CL_SUCCESS), (err = COMPV_ERROR_CODE_E_OPENCL), "clEnqueueMapBuffer failed");
	// FIXME: if (IP == mappedBuff) do  nothing
	if (mappedBuff != inPtr) {
		CompVMem::copy(mappedBuff, inPtr, static_cast<size_t>((trueStride * height) * sizeof(uint8_t))); // FIXME: copying for than what is needed (copy width only instead of stride)
	}
	COMPV_CHECK_EXP_BAIL((clerr = clEnqueueUnmapMemObject(CompVCL::clCommandQueue(), cl_inPtr, mappedBuff, 0, NULL, NULL)) != CL_SUCCESS,
		(err = COMPV_ERROR_CODE_E_OPENCL), "clEnqueueUnmapMemObject failed"); // FIXME: do async

	mappedBuff = clEnqueueMapBuffer(CompVCL::clCommandQueue(), cl_vthzKernPtr, CL_TRUE,
		CL_MAP_WRITE, 0, static_cast<size_t>(kernSize * sizeof(compv_float32_t)), 0, NULL, NULL, &clerr);
	COMPV_CHECK_EXP_BAIL((clerr != CL_SUCCESS), (err = COMPV_ERROR_CODE_E_OPENCL), "clEnqueueMapBuffer failed");
	// FIXME: if (IP == mappedBuff) do  nothing
	if (mappedBuff != cl_vthzKernPtr) {
		CompVMem::copy(mappedBuff, cl_vthzKernPtr, static_cast<size_t>(kernSize * sizeof(compv_float32_t))); // FIXME: copying for than what is needed (copy width only instead of stride)
	}
	COMPV_CHECK_EXP_BAIL((clerr = clEnqueueUnmapMemObject(CompVCL::clCommandQueue(), cl_vthzKernPtr, mappedBuff, 0, NULL, NULL)) != CL_SUCCESS,
		(err = COMPV_ERROR_CODE_E_OPENCL), "clEnqueueUnmapMemObject failed"); // FIXME: do async

	/* Execute */
	COMPV_CHECK_EXP_BAIL((clerr = clEnqueueNDRangeKernel(CompVCL::clCommandQueue(), __cl_convlt1VtHz_8u8u32f_kernel, 2, NULL, global, local, 0, NULL, NULL)) != CL_SUCCESS,
		(err = COMPV_ERROR_CODE_E_OPENCL), "clEnqueueNDRangeKernel failed");
	COMPV_CHECK_EXP_BAIL((clerr = clFinish(CompVCL::clCommandQueue())) != CL_SUCCESS,
		(err = COMPV_ERROR_CODE_E_OPENCL), "clFinish failed");

	// Read
	mappedBuff = clEnqueueMapBuffer(CompVCL::clCommandQueue(), cl_outPtr, CL_TRUE,
		CL_MAP_READ, 0, static_cast<size_t>((trueStride * height) * sizeof(uint8_t)), 0, NULL, NULL, &clerr);
	COMPV_CHECK_EXP_BAIL((clerr != CL_SUCCESS), (err = COMPV_ERROR_CODE_E_OPENCL), "clEnqueueMapBuffer failed");
	// FIXME: if (strengths == mappedBuff) do  nothing
	if (mappedBuff != outPtr) {
		CompVMem::copy(outPtr, mappedBuff, static_cast<size_t>((trueStride * height) * sizeof(uint8_t))); // FIXME: copying for than what is needed (copy width only instead of stride)
	}
	COMPV_CHECK_EXP_BAIL((clerr = clEnqueueUnmapMemObject(CompVCL::clCommandQueue(), cl_outPtr, mappedBuff, 0, NULL, NULL)) != CL_SUCCESS,
		(err = COMPV_ERROR_CODE_E_OPENCL), "clEnqueueUnmapMemObject failed"); // FIXME: do async

bail:
	if (cl_inPtr) {
		clReleaseMemObject(cl_inPtr);
	}
	if (cl_outPtr) {
		clReleaseMemObject(cl_outPtr);
	}
	if (cl_vthzKernPtr) {
		clReleaseMemObject(cl_vthzKernPtr);
	}
	
	return err;
}

COMPV_EXTERNC COMPV_CL_API gpu_convlt1VtHz_8u8u32f cl_convlt1VtHz_8u8u32f = __cl_convlt1VtHz_8u8u32f;

COMPV_NAMESPACE_END()