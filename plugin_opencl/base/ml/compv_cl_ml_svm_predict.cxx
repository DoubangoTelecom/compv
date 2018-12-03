/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/cl/base/ml/compv_cl_ml_svm_predict.h"
#include "compv/cl/compv_cl.h"
#include "compv/cl/compv_cl_utils.h"
#include "compv/base/math/compv_math_cast.h"

#define COMPV_THIS_CLASSNAME	"CompVMachineLearningSVMPredictBinaryRBF_GPU"

#define COMPV_CL_PROFILING_ENABLED	1

COMPV_NAMESPACE_BEGIN()

CompVMachineLearningSVMPredictBinaryRBF_GPU::CompVMachineLearningSVMPredictBinaryRBF_GPU(const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
	: CompVMachineLearningSVMPredict(CompVMachineLearningSVMPredictTypeBinaryRBF)
	, m_bValid(false)
	, m_bInitialized(false)
	, m_nMatSVs_cols(static_cast<cl_int>(matSV->cols()))
	, m_nMatSVs_rows(static_cast<cl_int>(matSV->rows()))
	, m_64fGammaMinus(-gamma)
	, m_64fRho(rho)
	, m_clContext(nullptr)
	, m_clCommandQueue(nullptr)
	, m_clMemMatSV(nullptr)
	, m_clMemMatCoeff(nullptr)
	, m_clProgram(nullptr)
	, m_clkernelPart1(nullptr)
	, m_clkernelPart2(nullptr)
{
	// Input parameters already checked by caller, do light checking
	COMPV_ASSERT(matSV->isRawTypeMatch<compv_float64_t>() && matCoeff->isRawTypeMatch<compv_float64_t>());

	m_labels[0] = labels[0], m_labels[1] = labels[1];
	// "nr_sv" is useless, needed by the CPU version only

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = init(matSV, matCoeff));
	m_bValid = isInitialized();

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(deInit());
	}
}

COMPV_ERROR_CODE CompVMachineLearningSVMPredictBinaryRBF_GPU::init(const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
{
	if (isInitialized()) {
		return COMPV_ERROR_CODE_S_OK;
	}

	cl_int clerr = CL_SUCCESS;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	const cl_device_id& device_id = CompVCL::clDeviceId();
	cl_mem clMemMatSV = nullptr, clMemMatCoeff = nullptr;
	cl_command_queue_properties qprop = 0;
#if COMPV_CL_PROFILING_ENABLED
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Profiling enabled");
	qprop |= CL_QUEUE_PROFILING_ENABLE;
#endif
	// Create context
	m_clContext = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateContext failed");

	// Create a command commands
	m_clCommandQueue = clCreateCommandQueue(m_clContext, device_id, qprop, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateCommandQueue failed");

	// Create buffers
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(matSV, &m_clMemMatSV, CL_MEM_READ_ONLY, m_clContext, m_clCommandQueue));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(matCoeff, &m_clMemMatCoeff, CL_MEM_READ_ONLY, m_clContext, m_clCommandQueue));

	// Create and build program then, get kernels
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createProgramWithSource(&m_clProgram, m_clContext, "compv_cl_ml_svm_predict.cl"));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::buildProgram(m_clProgram, CompVCL::clDeviceId()));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&m_clkernelPart1, m_clProgram, "clCompVMachineLearningSVMPredictBinaryRBF_Part1"));

	// Set Kernel (Part #1) Params
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 1, sizeof(m_clMemMatSV), (const void*)&m_clMemMatSV), "clSetKernelArg(m_clkernelPart1, matSVs, 1) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 2, sizeof(m_clMemMatCoeff), (const void*)&m_clMemMatCoeff), "clSetKernelArg(m_clkernelPart1, matCoeffs, 2) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 4, sizeof(m_64fGammaMinus), (const void*)&m_64fGammaMinus), "clSetKernelArg(m_clkernelPart1, gammaMinus, 4) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 6, sizeof(m_nMatSVs_cols), (const void*)&m_nMatSVs_cols), "clSetKernelArg(m_clkernelPart1, matSVs_cols, 6) failed");

	// Process
	COMPV_CHECK_CL_CODE_BAIL(clerr = clFinish(m_clCommandQueue), "clFinish failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clFlush(m_clCommandQueue), "clFlush failed");

	m_bInitialized = true;

bail:
	if (clerr != CL_SUCCESS) {
		err = COMPV_ERROR_CODE_E_OPENCL;
	}
	if (clMemMatSV != nullptr) {
		clReleaseMemObject(clMemMatSV), clMemMatSV = nullptr;
	}
	if (clMemMatCoeff != nullptr) {
		clReleaseMemObject(clMemMatCoeff), clMemMatCoeff = nullptr;
	}
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(deInit());
	}

	return err;
}

COMPV_ERROR_CODE CompVMachineLearningSVMPredictBinaryRBF_GPU::deInit()
{
	if (m_clMemMatSV) {
		clReleaseMemObject(m_clMemMatSV), m_clMemMatSV = nullptr;
	}
	if (m_clMemMatCoeff) {
		clReleaseMemObject(m_clMemMatCoeff), m_clMemMatCoeff = nullptr;
	}
	if (m_clProgram) {
		clReleaseProgram(m_clProgram), m_clProgram = nullptr;
	}
	if (m_clkernelPart1) {
		clReleaseKernel(m_clkernelPart1), m_clkernelPart1 = nullptr;
	}
	if (m_clkernelPart2) {
		clReleaseKernel(m_clkernelPart2), m_clkernelPart2 = nullptr;
	}
	if (m_clCommandQueue) {
		clReleaseCommandQueue(m_clCommandQueue), m_clCommandQueue = nullptr;
	}
	if (m_clContext) {
		clReleaseContext(m_clContext), m_clContext = nullptr;
	}

	m_bInitialized = false;

	return COMPV_ERROR_CODE_S_OK;
}

CompVMachineLearningSVMPredictBinaryRBF_GPU::~CompVMachineLearningSVMPredictBinaryRBF_GPU()
{
	COMPV_CHECK_CODE_NOP(deInit());
}

COMPV_ERROR_CODE CompVMachineLearningSVMPredictBinaryRBF_GPU::process(const CompVMatPtr& matVectors, CompVMatPtrPtr matResult)
{
	COMPV_CHECK_EXP_RETURN(!isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(!matVectors || !(matVectors->isRawTypeMatch<compv_float32_t>() || matVectors->isRawTypeMatch<compv_float64_t>()) || !matResult, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Check vector size
	if (matVectors->cols() != static_cast<size_t>(m_nMatSVs_cols)) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Vector size mismatch (%zu != %d)", matVectors->cols(), m_nMatSVs_cols);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}

	// Convert to float64
	CompVMatPtr matVectorsFloat64;
	if (!matVectors->isRawTypeMatch<compv_float64_t>()) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float32_t, compv_float64_t>(matVectors, &matVectorsFloat64))); // multithreaded
	}
	else {
		matVectorsFloat64 = matVectors;
	}

	cl_int clerr = CL_SUCCESS;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	cl_event clEventKernel1 = nullptr;
	CompVMatPtr matResult1, matResult_ = *matResult;
	const size_t numvectors = matVectors->rows();
	cl_mem clMemMatVectors = nullptr;
	cl_mem clMemMatResult1 = nullptr;
	cl_mem clMemMatResult2 = nullptr;
	size_t localWorkSize1[2], globalWorkSize1[2];

	const size_t matResult1_rows = matVectors->rows();
	const size_t matResult1_cols = static_cast<size_t>(m_nMatSVs_rows);
	const cl_int matResult1_colsInt = static_cast<cl_int>(matResult1_cols);
	const cl_int matVectors_colsInt = static_cast<cl_int>(matVectors->cols());

	// Create matVectors memory
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(matVectors, &clMemMatVectors, CL_MEM_READ_ONLY, m_clContext, m_clCommandQueue));

	// Create matResult1 memory
	clMemMatResult1 = clCreateBuffer(m_clContext, CL_MEM_READ_WRITE, (matResult1_rows * matResult1_cols) * sizeof(compv_float64_t), nullptr,
		&clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer failed");

	// Set kernel arguments (Part #1)
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 0, sizeof(clMemMatVectors), (const void*)&clMemMatVectors), "clSetKernelArg(m_clkernelPart1, matVectors, 0) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 3, sizeof(clMemMatResult1), (const void*)&clMemMatResult1), "clSetKernelArg(m_clkernelPart1, matResult, 3) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 5, sizeof(matVectors_colsInt), (const void*)&matVectors_colsInt), "clSetKernelArg(m_clkernelPart1, matVectors_cols, 5) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 7, sizeof(matResult1_colsInt), (const void*)&matResult1_colsInt), "clSetKernelArg(m_clkernelPart1, matResult_cols, 7) failed");

	// Execute
#define SVS 7 // must be same number as what is defined in CL script (IMPORTANT: WHY "7" is good by "8" is BAD)?
	localWorkSize1[0] = 16;
	localWorkSize1[1] = 16;
	globalWorkSize1[0] = ((matResult1_cols + (SVS - 1)) / SVS); // matResult1_cols is number of support vectors (should be large, e.g 56958)
	globalWorkSize1[1] = matResult1_rows; // number of inputs (features, should be small) - variable [1 - INFINIT]
	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(m_clCommandQueue, m_clkernelPart1, 2, nullptr, globalWorkSize1, localWorkSize1,
		0, nullptr, &clEventKernel1), "clEnqueueNDRangeKernel failed");

#if COMPV_CL_PROFILING_ENABLED
	{
		cl_ulong startTime, endTime;
		cl_int eventStatus = CL_QUEUED;
		cl_ulong nanoSeconds;

		COMPV_CHECK_CL_CODE_BAIL(clerr = clWaitForEvents(1, &clEventKernel1), "clWaitForEvents(clEventKernel1) failed");
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetEventProfilingInfo(clEventKernel1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, 0), "clGetEventProfilingInfo(clEventKernel1, CL_PROFILING_COMMAND_START) failed");
		COMPV_CHECK_CL_CODE_BAIL(clerr = clGetEventProfilingInfo(clEventKernel1, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, 0), "clGetEventProfilingInfo(clEventKernel1, CL_PROFILING_COMMAND_END) failed");
		
		nanoSeconds = (endTime - startTime); // GPGPU timer resolution = 10e-9 (nanoseconds)
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Kernel1 duration = %lf ms", (nanoSeconds * 1e-6));
	}
#endif /* COMPV_CL_PROFILING_ENABLED */

	// Get Data using async read
	COMPV_CHECK_CODE_RETURN(err = CompVMat::newObjStrideless<compv_float64_t>(&matResult1, matResult1_rows, matResult1_cols));
	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueReadBuffer(m_clCommandQueue, clMemMatResult1, CL_FALSE, 0, matResult1->planeSizeInBytes(0), matResult1->ptr<void>(), 0, nullptr, nullptr)
		, "clEnqueueReadBuffer failed");

	// Wait for completion
	COMPV_CHECK_CL_CODE_BAIL(clerr = clFinish(m_clCommandQueue), "clEnqueueNDRangeKernel failed");

	if (1){
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("This is for testing only mus move code to gpu");
		// GPGPU reduction (horizontal sum)
		compv_float64_t* Cptr0 = matResult1->ptr<compv_float64_t>();
		const size_t Crows = matResult1->rows();
		const size_t Ccols = matResult1->cols();
		const size_t Cstride = matResult1->stride();
		for (size_t j = 0; j < Crows; ++j) {
			compv_float64_t& sum = Cptr0[0];
			for (size_t i = 1; i < Ccols; ++i) {
				sum += Cptr0[i];
			}
			sum -= m_64fRho;
			Cptr0 += Cstride;
		}
	}

	{
		// Building labels
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&matResult_, 1, numvectors));
		const compv_float64_t* matValuesPtr = matResult1->ptr<const compv_float64_t>();
		const size_t matValuesStride = matResult1->stride();
		int32_t* matResultPtr = matResult_->ptr<int32_t>();
		for (size_t i = 0; i < numvectors; ++i) {
			matResultPtr[i] = m_labels[(matValuesPtr[0] < 0)];
			matValuesPtr += matValuesStride;
		}

		*matResult = matResult_;
	}

bail:
	if (clerr != CL_SUCCESS) {
		err = COMPV_ERROR_CODE_E_OPENCL;
	}
	if (clMemMatVectors) {
		clReleaseMemObject(clMemMatVectors), clMemMatVectors = nullptr;
	}
	if (clMemMatResult1) {
		clReleaseMemObject(clMemMatResult1), clMemMatResult1 = nullptr;
	}
	if (clMemMatResult2) {
		clReleaseMemObject(clMemMatResult2), clMemMatResult2 = nullptr;
	}
	if (clEventKernel1) {
		clReleaseEvent(clEventKernel1), clEventKernel1 = nullptr;
	}
	if (COMPV_ERROR_CODE_IS_OK(err) && clerr != CL_SUCCESS) {
		err = COMPV_ERROR_CODE_E_OPENCL;
	}
	return err;
}

COMPV_ERROR_CODE CompVMachineLearningSVMPredictBinaryRBF_GPU::newObj(CompVMachineLearningSVMPredictPtrPtr mlSVM, const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
{
	COMPV_CHECK_EXP_RETURN(!mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVCL::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	CompVMachineLearningSVMPredictBinaryRBF_GPUPtr mlSVM_;
	mlSVM_ = new CompVMachineLearningSVMPredictBinaryRBF_GPU(gamma, rho, labels, nr_sv, matSV, matCoeff);

	COMPV_CHECK_EXP_RETURN(!mlSVM_, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	*mlSVM = *mlSVM_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
