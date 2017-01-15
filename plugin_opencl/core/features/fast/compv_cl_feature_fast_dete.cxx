/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/cl/core/features/fast/compv_cl_feature_fast_dete.h"
#include "compv/cl/compv_cl.h"
#include "compv/cl/compv_cl_utils.h"
#include "compv/base/compv_mem.h"

COMPV_NAMESPACE_BEGIN()

CompVCLCornerDeteFAST::CompVCLCornerDeteFAST()
	: m_bInitialized(false)
	, m_clProgram(NULL)
	, m_clKernel(NULL)
	, m_clMemIP(NULL)
	, m_clMemStrengths(NULL)
	, m_nWidth(NULL)
	, m_nHeight(NULL)
	, m_nStride(NULL)
{

}

CompVCLCornerDeteFAST::~CompVCLCornerDeteFAST()
{
	if (m_clProgram) {
		clReleaseProgram(m_clProgram);
	}
	if (m_clKernel) {
		clReleaseKernel(m_clKernel);
	}
	COMPV_CHECK_CODE_NOP(deInit());
}

COMPV_ERROR_CODE CompVCLCornerDeteFAST::deInit()
{
	if (m_clMemIP) {
		clReleaseMemObject(m_clMemIP);
		m_clMemIP = NULL;
	}
	if (m_clMemStrengths) {
		clReleaseMemObject(m_clMemStrengths);
		m_clMemStrengths = NULL;
	}
	if (m_clMemPixels16) {
		clReleaseMemObject(m_clMemPixels16);
		m_clMemPixels16 = NULL;
	}
	m_bInitialized = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCLCornerDeteFAST::init(const uint8_t* IP, uint8_t* strengths, unsigned int width, unsigned int height, unsigned int stride, unsigned char N, unsigned char threshold)
{
	if (m_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	cl_int clerr = CL_SUCCESS;
	cl_mem_flags addFlags = 0;

	const int strideSigned = static_cast<int>(stride);
	m_arrayPixels16[0] = -(strideSigned * 3) + 0;
	m_arrayPixels16[1] = -(strideSigned * 3) + 1;
	m_arrayPixels16[2] = -(strideSigned * 2) + 2;
	m_arrayPixels16[3] = -(strideSigned * 1) + 3;
	m_arrayPixels16[4] = +(strideSigned * 0) + 3;
	m_arrayPixels16[5] = +(strideSigned * 1) + 3;
	m_arrayPixels16[6] = +(strideSigned * 2) + 2;
	m_arrayPixels16[7] = +(strideSigned * 3) + 1;
	m_arrayPixels16[8] = +(strideSigned * 3) + 0;
	m_arrayPixels16[9] = +(strideSigned * 3) - 1;
	m_arrayPixels16[10] = +(strideSigned * 2) - 2;
	m_arrayPixels16[11] = +(strideSigned * 1) - 3;
	m_arrayPixels16[12] = +(strideSigned * 0) - 3;
	m_arrayPixels16[13] = -(strideSigned * 1) - 3;
	m_arrayPixels16[14] = -(strideSigned * 2) - 2;
	m_arrayPixels16[15] = -(strideSigned * 3) - 1;

#if defined(CL_MEM_USE_PERSISTENT_MEM_AMD) && 0 // FIXME: must also check it's AMD device at runtime
	addFlags |= CL_MEM_USE_PERSISTENT_MEM_AMD;
#endif

	// FIXME: using 'CL_MEM_USE_HOST_PTR' means m_clMemIP and m_clMemStrengths no longer valid when host memory is reallocated -> deInit if change

	m_clMemPixels16 = clCreateBuffer(CompVCL::clContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | addFlags, sizeof(m_arrayPixels16), reinterpret_cast<void*>(m_arrayPixels16), &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer failed");
	m_clMemIP = clCreateBuffer(CompVCL::clContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | addFlags, static_cast<size_t>((stride * height) * sizeof(uint8_t)), (void*)IP, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer failed");
	m_clMemStrengths = clCreateBuffer(CompVCL::clContext(), CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR | addFlags, static_cast<size_t>((stride * height) * sizeof(uint8_t)), (void*)strengths, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer failed");

	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 0, sizeof(cl_mem), &m_clMemIP));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 1, sizeof(unsigned int), &width));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 2, sizeof(unsigned int), &height));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 3, sizeof(unsigned int), &stride));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 4, sizeof(cl_mem), &m_clMemPixels16));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 5, sizeof(unsigned char), &N));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 6, sizeof(unsigned char), &threshold));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clKernel, 7, sizeof(cl_mem), &m_clMemStrengths));

	m_nWidth = width;
	m_nHeight = height;
	m_nStride = stride;
	m_bInitialized = true;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err) || (clerr != CL_SUCCESS)) {
		COMPV_CHECK_CODE_NOP(deInit());
		COMPV_CHECK_CODE_RETURN(err);
		COMPV_CHECK_EXP_RETURN((clerr != CL_SUCCESS), COMPV_ERROR_CODE_E_OPENCL);
	}
	return err;
}

COMPV_ERROR_CODE CompVCLCornerDeteFAST::processData(
	const uint8_t* IP,
	compv_uscalar_t width,
	compv_uscalar_t height,
	compv_uscalar_t stride,
	compv_uscalar_t N,
	compv_uscalar_t threshold,
	uint8_t* strengths) /*override */
{
	if (m_nWidth != width || m_nHeight != height || m_nStride != stride) {
		COMPV_CHECK_CODE_RETURN(deInit());
	}

	// FIXME: check N, threshold change and call clSetKernelArg
	// FIXME: use local queue
	// FIXME: check opencl version and make sure the kernel uses the supported builtin functions
	// FIXME: IP and strengghts must be CompVMem::isGpuFriendly

	COMPV_CHECK_CODE_RETURN(init(
		IP,
		strengths,
		static_cast<unsigned int>(width),
		static_cast<unsigned int>(height),
		static_cast<unsigned int>(stride),
		static_cast<unsigned char>(N),
		static_cast<unsigned char>(threshold)
	));
	
	cl_int clerr = CL_SUCCESS;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	void* mappedBuff = NULL;

	size_t global[2] = { 
		static_cast<size_t>(CompVMem::alignForward(stride, COMPV_ALIGNV_GPU_LINE)), // FIXME: should already be aligned
		height
	};
	size_t local[2] = { // FIXME: max = 256 for AMD, must be retrieve at runtime. This means (local[0] * local[1]) must be <= 256
		COMPV_ALIGNV_GPU_LINE,
		1
	};

	// Write
	mappedBuff = clEnqueueMapBuffer(CompVCL::clCommandQueue(), m_clMemIP, CL_TRUE,
		CL_MAP_WRITE, 0, static_cast<size_t>((stride * height) * sizeof(uint8_t)), 0, NULL, NULL, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clEnqueueMapBuffer failed");
	// FIXME: if (IP == mappedBuff) do  nothing
	if (mappedBuff != IP) {
		CompVMem::copy(mappedBuff, IP, static_cast<size_t>((stride * height) * sizeof(uint8_t))); // FIXME: copying for than what is needed (copy width only instead of stride)
	}
	clerr = clEnqueueUnmapMemObject(CompVCL::clCommandQueue(), m_clMemIP, mappedBuff, 0, NULL, NULL);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clEnqueueUnmapMemObject failed");
	
	// Execute
	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(CompVCL::clCommandQueue(), m_clKernel, 2, NULL, global, local, 0, NULL, NULL),
		"clEnqueueNDRangeKernel failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clFinish(CompVCL::clCommandQueue()),
		"clFinish failed");

	// Read
	mappedBuff = clEnqueueMapBuffer(CompVCL::clCommandQueue(), m_clMemStrengths, CL_TRUE,
		CL_MAP_READ, 0, static_cast<size_t>((stride * height) * sizeof(uint8_t)), 0, NULL, NULL, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clEnqueueMapBuffer failed");
	// FIXME: if (strengths == mappedBuff) do  nothing
	if (mappedBuff != strengths) {
		CompVMem::copy(strengths, mappedBuff, static_cast<size_t>((stride * height) * sizeof(uint8_t))); // FIXME: copying for than what is needed (copy width only instead of stride)
	}
	clerr = clEnqueueUnmapMemObject(CompVCL::clCommandQueue(), m_clMemStrengths, mappedBuff, 0, NULL, NULL);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clEnqueueUnmapMemObject failed");

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err) || (clerr != CL_SUCCESS)) {
		COMPV_CHECK_CODE_RETURN(err);
		COMPV_CHECK_EXP_RETURN((clerr != CL_SUCCESS), COMPV_ERROR_CODE_E_OPENCL);
	}
	return err;
}

COMPV_ERROR_CODE CompVCLCornerDeteFAST::newObj(CompVCLCornerDeteFASTPtrPtr fast)
{
	COMPV_CHECK_CODE_RETURN(CompVCL::init());
	COMPV_CHECK_EXP_RETURN(!fast, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVCLCornerDeteFASTPtr fast_ = new CompVCLCornerDeteFAST();
	COMPV_CHECK_EXP_RETURN(!fast_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVCLUtils::createProgramWithSource(&fast_->m_clProgram, CompVCL::clContext(), "compv_cl_feature_fast_dete.cl"));
	COMPV_CHECK_CODE_RETURN(CompVCLUtils::buildProgram(fast_->m_clProgram, CompVCL::clDeviceId()));
	COMPV_CHECK_CODE_RETURN(CompVCLUtils::createKernel(&fast_->m_clKernel, fast_->m_clProgram, "clFAST"));
	
	*fast = fast_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
