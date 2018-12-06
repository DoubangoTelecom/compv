/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/cl/base/image/compv_cl_image_hysteresis.h"
#include "compv/cl/compv_cl.h"
#include "compv/cl/compv_cl_utils.h"
#include "compv/base/compv_mem.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/image/compv_image.h"

COMPV_NAMESPACE_BEGIN()

COMPV_EXTERNC COMPV_CL_API COMPV_ERROR_CODE clHysteresisProcess_8u8u(const CompVMatPtr& in, CompVMatPtrPtr out, const double& tLow, const double& tHigh)
{
	COMPV_CHECK_CODE_RETURN(CompVCL::init());
	COMPV_CHECK_CODE_RETURN(CompVCLHysteresis::process_8u8u(in, out, tLow, tHigh));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCLHysteresis::process_8u8u(const CompVMatPtr& in, CompVMatPtrPtr out, const double& tLow, const double& tHigh)
{
	COMPV_CHECK_EXP_RETURN(!in || !out || in->elmtInBytes() != sizeof(uint8_t) || in->planeCount() != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	cl_int clerr = CL_SUCCESS;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	const cl_device_id& device_id = CompVCL::clDeviceId();
	cl_context clContext = nullptr;
	cl_command_queue clCommandQueue = nullptr;
	cl_mem clMemIn = nullptr;
	cl_mem clMemOut = nullptr;
	cl_mem clMemNeighbors8 = nullptr;
	cl_mem clMemModifiedGlobal = nullptr;
	cl_program clProgram = nullptr;
	cl_kernel clkernelThresholding = nullptr;
	cl_kernel clkernelScanning = nullptr;
	cl_kernel clkernelRefining = nullptr;
	cl_event clEventThresholding = nullptr;
	cl_event clEventScanning = nullptr;
	cl_event clEventReadOut = nullptr;
	cl_event clEventReadModifiedGlobal = nullptr;
	cl_event clEventRefining = nullptr;
	size_t localWorkSize[2], globalWorkSize[2];
	const unsigned char clTLow = COMPV_MATH_ROUNDFU_2_NEAREST_INT(COMPV_MATH_CLIP3(0, 255.0, tLow), unsigned char);
	const unsigned char clThigh = COMPV_MATH_ROUNDFU_2_NEAREST_INT(COMPV_MATH_CLIP3(0, 255.0, tHigh), unsigned char);
	const cl_int clWidth = static_cast<cl_int>(in->cols());
	const cl_int clHeight = static_cast<cl_int>(in->rows());
	CompVMatPtr out_ = (*out == in) ? nullptr : *out;
	CompVMatPtr neighbors8;
	cl_int modified_global;
	size_t scanning_rounds = 0;
	double durationScanning = 0.;
	const cl_int NEIGHBORS8[8] = {
		-clWidth - 1, // LEFT_TOP
		-clWidth, // TOP
		-clWidth + 1, // RIGHT_TOP
		-1, // LEFT
		1, // RIGHT
		clWidth - 1, // LEFT_BOTTOM
		clWidth, // BOTTOM
		clWidth + 1, // RIGHT_BOTTOM
	};

	// Set local ang global work sizes
	localWorkSize[0] = 16;
	localWorkSize[1] = 16;
	globalWorkSize[0] = static_cast<size_t>((clWidth + 15) & -16);
	globalWorkSize[1] = static_cast<size_t>((clHeight + 15) & -16);

	// Create context
	clContext = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateContext failed");

	// Create a command queue
	clCommandQueue = clCreateCommandQueue(clContext, device_id, CL_QUEUE_PROFILING_ENABLE, &clerr); // TODO(dmi): make profiling only on request (disable on release)
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateCommandQueue failed");

	// Create buffers
	COMPV_CHECK_CODE_BAIL(err = CompVMat::newObjStrideless<uint8_t>(&out_, clHeight, clWidth));
	COMPV_CHECK_CODE_BAIL(err = CompVMat::newObjStrideless<cl_int>(&neighbors8, 1, 8));
	for (int i = 0; i < 8; ++i) *neighbors8->ptr<cl_int>(0, i) = NEIGHBORS8[i];
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(in, &clMemIn, CL_MEM_READ_ONLY, clContext, clCommandQueue));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(neighbors8, &clMemNeighbors8, CL_MEM_READ_ONLY, clContext, clCommandQueue));

	clMemModifiedGlobal = clCreateBuffer(clContext, CL_MEM_READ_WRITE, 1 * sizeof(cl_int), nullptr, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer(clMemModifiedGlobal) failed");

	clMemOut = clCreateBuffer(clContext, CL_MEM_READ_WRITE, out_->planeSizeInBytes(0), nullptr, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer(clMemOut) failed");

	// Zero border. Must not set the borders with zero but doing it here because this is what output from TMMS will provide
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("For TMMS no need for this as the input already have borders at zero which means the output will have zero borders after double thresholding");
		CompVMem::set(out_->ptr<uint8_t>(0), 255, out_->planeSizeInBytes());
		COMPV_CHECK_CODE_BAIL(err = CompVMem::zero(out_->ptr<uint8_t>(0), clWidth)); // top
		COMPV_CHECK_CODE_BAIL(err = CompVMem::zero(out_->ptr<uint8_t>(clHeight - 1), clWidth)); // bottom
		uint8_t*ptr0 = out_->ptr<uint8_t>(1); // left
		uint8_t*ptr1 = ptr0 + clWidth - 1; // right
		for (cl_int i = 0, j = 0; i < (clHeight - 2); i += 1, j += clWidth) {
			ptr0[j] = 0, ptr1[j] = 0;
		}
	}

	// Create and build program then, get kernels
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createProgramWithSource(&clProgram, clContext, "compv_cl_image_hysteresis.cl"));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::buildProgram(clProgram, CompVCL::clDeviceId()));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&clkernelThresholding, clProgram, "clHysteresisProcess_thresholding_8u8u"));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&clkernelScanning, clProgram, "clHysteresisProcess_scanning_8u8u"));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&clkernelRefining, clProgram, "clHysteresisProcess_refining_8u8u"));

	// Thresholding: Set kernel agruments and run
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelThresholding, 0, sizeof(clMemIn), (const void*)&clMemIn), "clSetKernelArg(clkernelThresholding, input, 0) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelThresholding, 1, sizeof(clMemOut), (const void*)&clMemOut), "clSetKernelArg(clkernelThresholding, output, 1) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelThresholding, 2, sizeof(clWidth), (const void*)&clWidth), "clSetKernelArg(clkernelThresholding, width, 2) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelThresholding, 3, sizeof(clHeight), (const void*)&clHeight), "clSetKernelArg(clkernelThresholding, height, 3) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelThresholding, 4, sizeof(clTLow), (const void*)&clTLow), "clSetKernelArg(clkernelThresholding, tLow, 4) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelThresholding, 5, sizeof(clThigh), (const void*)&clThigh), "clSetKernelArg(clkernelThresholding, tHigh, 5) failed");

	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelScanning, 0, sizeof(clMemOut), (const void*)&clMemOut), "clSetKernelArg(clkernelScanning, input, 0) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelScanning, 1, sizeof(clWidth), (const void*)&clWidth), "clSetKernelArg(clkernelScanning, width, 1) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelScanning, 2, sizeof(clHeight), (const void*)&clHeight), "clSetKernelArg(clkernelScanning, height, 2) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelScanning, 3, sizeof(clMemNeighbors8), (const void*)&clMemNeighbors8), "clSetKernelArg(clkernelScanning, neighbors8, 3) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelScanning, 4, sizeof(clMemModifiedGlobal), (const void*)&clMemModifiedGlobal), "clSetKernelArg(clkernelScanning, modified_global, 4) failed");

	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelRefining, 0, sizeof(clMemOut), (const void*)&clMemOut), "clSetKernelArg(clkernelRefining, input, 0) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelRefining, 1, sizeof(clWidth), (const void*)&clWidth), "clSetKernelArg(clkernelRefining, width, 1) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(clkernelRefining, 2, sizeof(clHeight), (const void*)&clHeight), "clSetKernelArg(clkernelRefining, height, 2) failed");

	// Run Thresholding
	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(clCommandQueue, clkernelThresholding, 2, nullptr, globalWorkSize, localWorkSize,
		0, nullptr, &clEventThresholding), "clEnqueueNDRangeKernel(clkernelThresholding) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clWaitForEvents(1, &clEventThresholding), "clWaitForEvents(clEventThresholding) failed");

	// Run Scanning
	do {
		modified_global = 0;
		double millis;
		COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueWriteBuffer(clCommandQueue, clMemModifiedGlobal, CL_TRUE, 0, sizeof(modified_global), &modified_global, 0, nullptr, nullptr), "clEnqueueWriteBuffer failed");
		COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(clCommandQueue, clkernelScanning, 2, nullptr, globalWorkSize, localWorkSize,
			0, nullptr, &clEventScanning), "clEnqueueNDRangeKernel(clkernelScanning) failed");
		COMPV_CHECK_CL_CODE_BAIL(clerr = clWaitForEvents(1, &clEventScanning), "clWaitForEvents(clEventScanning) failed");
		COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueReadBuffer(clCommandQueue, clMemModifiedGlobal, CL_TRUE, 0, sizeof(modified_global), &modified_global, 0, nullptr, &clEventReadModifiedGlobal)
			, "clEnqueueReadBuffer failed");
		COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::duration(clEventScanning, millis));
		durationScanning += millis;
		++scanning_rounds;
	} while (modified_global);

	// Refining
	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(clCommandQueue, clkernelRefining, 2, nullptr, globalWorkSize, localWorkSize,
		0, nullptr, &clEventRefining), "clEnqueueNDRangeKernel(clkernelRefining) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clWaitForEvents(1, &clEventRefining), "clWaitForEvents(clEventRefining) failed");

	// read result
	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueReadBuffer(clCommandQueue, clMemOut, CL_TRUE, 0, out_->planeSizeInBytes(0), out_->ptr<void>(), 0, nullptr, &clEventReadOut)
		, "clEnqueueReadBuffer failed");

	/* Profiling */
	{
		double durationTotal = durationScanning;
		cl_event clEventKernels[] = { clEventThresholding, clEventReadModifiedGlobal, clEventRefining, clEventReadOut };
		const char* clEventKernelsNames[] = { "Thresholding",  "ReadModifiedGlobal", "Refining", "ReadOut" };
		double millis;
		for (size_t i = 0; i < sizeof(clEventKernels) / sizeof(clEventKernels[0]); ++i) {
			if (!clEventKernels[i]) {
				continue;
			}
			COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::duration(clEventKernels[i], millis));
			COMPV_DEBUG_INFO_EX("clHysteresisProcess_8u8u", "%s: duration = %lf ms", clEventKernelsNames[i], millis);
			durationTotal += millis;
		}
		COMPV_DEBUG_INFO_EX("clHysteresisProcess_8u8u", "Scanning: duration = %lf ms, loops = %zu", durationScanning, scanning_rounds);
		COMPV_DEBUG_INFO_EX("clHysteresisProcess_8u8u", "Total: duration = %lf ms", durationTotal);
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("out must be strided and aligned. Otherwise, calling function won't be SIMD accelerated");
	*out = out_;

bail:
	if (clerr != CL_SUCCESS) {
		err = COMPV_ERROR_CODE_E_OPENCL;
	}
	if (clMemIn) {
		clReleaseMemObject(clMemIn), clMemIn = nullptr;
	}
	if (clMemOut) {
		clReleaseMemObject(clMemOut), clMemOut = nullptr;
	}
	if (clMemNeighbors8) {
		clReleaseMemObject(clMemNeighbors8), clMemNeighbors8 = nullptr;
	}
	if (clMemModifiedGlobal) {
		clReleaseMemObject(clMemModifiedGlobal), clMemModifiedGlobal = nullptr;
	}
	if (clProgram) {
		clReleaseProgram(clProgram), clProgram = nullptr;
	}
	if (clkernelThresholding) {
		clReleaseKernel(clkernelThresholding), clkernelThresholding = nullptr;
	}
	if (clkernelScanning) {
		clReleaseKernel(clkernelScanning), clkernelScanning = nullptr;
	}
	if (clkernelRefining) {
		clReleaseKernel(clkernelRefining), clkernelRefining = nullptr;
	}
	if (clEventThresholding) {
		clReleaseEvent(clEventThresholding), clEventThresholding = nullptr;
	}
	if (clEventScanning) {
		clReleaseEvent(clEventScanning), clEventScanning = nullptr;
	}
	if (clEventReadOut) {
		clReleaseEvent(clEventReadOut), clEventReadOut = nullptr;
	}
	if (clEventReadModifiedGlobal) {
		clReleaseEvent(clEventReadModifiedGlobal), clEventReadModifiedGlobal = nullptr;
	}
	if (clEventRefining) {
		clReleaseEvent(clEventRefining), clEventRefining = nullptr;
	}
	if (clCommandQueue) {
		clReleaseCommandQueue(clCommandQueue), clCommandQueue = nullptr;
	}
	if (clContext) {
		clReleaseContext(clContext), clContext = nullptr;
	}
	return err;
}

COMPV_NAMESPACE_END()
