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

#define TYP_DOUBLE 0


// Must not change
#if TYP_DOUBLE
#	define TYP double
#else
#	define TYP float
#endif

COMPV_NAMESPACE_BEGIN()

CompVMachineLearningSVMPredictBinaryRBF_GPU::CompVMachineLearningSVMPredictBinaryRBF_GPU(const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
	: CompVMachineLearningSVMPredict(CompVMachineLearningSVMPredictTypeBinaryRBF)
	, m_bValid(false)
	, m_bInitialized(false)
	, m_64fGammaMinus(-gamma)
	, m_64fRho(rho)
	, m_clContext(nullptr)
	, m_clCommandQueue(nullptr)
	, m_clMemMatSV(nullptr)
	, m_clMemMatCoeff(nullptr)
	, m_clProgram(nullptr)
	, m_clkernelPart1(nullptr)
	, m_clkernelPart2(nullptr)
	, m_clkernelReduction(nullptr)
{
	// Input parameters already checked by caller, do light checking
	COMPV_ASSERT(matSV->isRawTypeMatch<compv_float64_t>() && matCoeff->isRawTypeMatch<compv_float64_t>());

	m_labels[0] = labels[0], m_labels[1] = labels[1];
	// "nr_sv" is useless, needed by the CPU version only

	m_nMatSVs_cols = static_cast<cl_int>(matSV->cols());
	m_nMatSVs_rows = static_cast<cl_int>(matSV->rows());

	CompVMatPtr matSV_, matCoeff_;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
#if TYP_DOUBLE
	matSV_ = matSV;
	matCoeff_ = matCoeff;
#else
	COMPV_CHECK_CODE_BAIL((err = CompVMathCast::process_static<double, float>(matSV, &matSV_)));
	COMPV_CHECK_CODE_BAIL((err = CompVMathCast::process_static<double, float>(matCoeff, &matCoeff_)));
#endif
	COMPV_CHECK_CODE_BAIL(err = init(matSV_, matCoeff_));
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
	const TYP gammaMinus = static_cast<TYP>(m_64fGammaMinus);

#if COMPV_CL_PROFILING_ENABLED
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Profiling enabled");
	qprop |= CL_QUEUE_PROFILING_ENABLE;
#endif
	// Create context
	m_clContext = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateContext failed");

	// Create a command queue
	m_clCommandQueue = clCreateCommandQueue(m_clContext, device_id, qprop, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateCommandQueue failed");

	// Create buffers
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(matSV, &m_clMemMatSV, CL_MEM_READ_ONLY, m_clContext, m_clCommandQueue));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(matCoeff, &m_clMemMatCoeff, CL_MEM_READ_ONLY, m_clContext, m_clCommandQueue));

	// Create and build program then, get kernels
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createProgramWithSource(&m_clProgram, m_clContext, "compv_cl_ml_svm_predict.cl"));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::buildProgram(m_clProgram, CompVCL::clDeviceId()));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&m_clkernelPart1, m_clProgram, "clCompVMachineLearningSVMPredictBinaryRBF_Part1"));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&m_clkernelPart2, m_clProgram, "clCompVMachineLearningSVMPredictBinaryRBF_Part2"));
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createKernel(&m_clkernelReduction, m_clProgram, "clCompVMachineLearningSVMPredictBinaryRBF_Reduction"));

	// Set Kernel (Part #1) Params
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 1, sizeof(m_clMemMatSV), (const void*)&m_clMemMatSV), "clSetKernelArg(m_clkernelPart1, matSVs, 1) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 2, sizeof(m_clMemMatCoeff), (const void*)&m_clMemMatCoeff), "clSetKernelArg(m_clkernelPart1, matCoeffs, 2) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 4, sizeof(gammaMinus), (const void*)&gammaMinus), "clSetKernelArg(m_clkernelPart1, gammaMinus, 4) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 5, sizeof(m_nMatSVs_cols), (const void*)&m_nMatSVs_cols), "clSetKernelArg(m_clkernelPart1, matSVs_cols, 5) failed");

	// Set Kernel (Part #2) Params
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart2, 5, sizeof(m_64fRho), (const void*)&m_64fRho), "clSetKernelArg(m_clkernelPart2, rho, 5) failed");

	// Set Kernel (reduction) Params
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelReduction, 5, sizeof(m_64fRho), (const void*)&m_64fRho), "clSetKernelArg(m_clkernelReduction, rho, 5) failed");

	// Process
	COMPV_CHECK_CL_CODE_BAIL(clerr = clFinish(m_clCommandQueue), "clFinish failed");

	m_bInitialized = true;

bail:
	if (clerr != CL_SUCCESS) {
		err = COMPV_ERROR_CODE_E_OPENCL;
	}
	if (clMemMatSV) {
		clReleaseMemObject(clMemMatSV), clMemMatSV = nullptr;
	}
	if (clMemMatCoeff) {
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
	if (m_clkernelReduction) {
		clReleaseKernel(m_clkernelReduction), m_clkernelReduction = nullptr;
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

// MUST be thread-safe
COMPV_ERROR_CODE CompVMachineLearningSVMPredictBinaryRBF_GPU::process(const CompVMatPtr& matVectors, CompVMatPtrPtr matResult)
{
	COMPV_CHECK_EXP_RETURN(!isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(!matVectors || !(matVectors->isRawTypeMatch<compv_float32_t>() || matVectors->isRawTypeMatch<compv_float64_t>()) || !matResult, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_TODO("Make thread-safe (use context-based for all 'm_xxx' modified here)");

	// Check vector size
	if (matVectors->cols() != static_cast<size_t>(m_nMatSVs_cols)) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Vector size mismatch (%zu != %d)", matVectors->cols(), m_nMatSVs_cols);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}

	// Convert to float64
	CompVMatPtr matVectorsFloat64;
	if (!TYP_DOUBLE && matVectors->isRawTypeMatch<compv_float64_t>()) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float64_t, compv_float32_t>(matVectors, &matVectorsFloat64))); // multithreaded
	}
	else {
		matVectorsFloat64 = matVectors;
	}

	cl_int clerr = CL_SUCCESS;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	cl_event clEventKernel1 = nullptr, clEventKernel2 = nullptr, clEventKernelReduction = nullptr, clEventReadBuffer = nullptr;
	CompVMatPtr matResultSVMValues, matResult_ = *matResult;
	const size_t numvectors = matVectorsFloat64->rows();
	cl_mem clMemMatVectors = nullptr;
	cl_mem clMemMatResult1 = nullptr;
	cl_mem clMemMatResult2 = nullptr;
	size_t localWorkSize[2], globalWorkSize[2];

	const size_t matResult1_rows = matVectorsFloat64->rows();
	const size_t matResult1_cols = static_cast<size_t>(m_nMatSVs_rows);
	const cl_int matResult1_colsInt = static_cast<cl_int>(matResult1_cols);
	const cl_int matResult1_rowsInt = static_cast<cl_int>(matResult1_rows);
	const cl_int matVectors_colsInt = static_cast<cl_int>(matVectorsFloat64->cols());
	const cl_int matVectors_rowsInt = static_cast<cl_int>(matVectorsFloat64->rows());
	cl_int matResult2_colsInt, matResult2_rowsInt = matResult1_rowsInt;
	cl_int reductionIdx = 0;

	// Create matVectors memory
	COMPV_CHECK_CODE_BAIL(err = CompVCLUtils::createDataStrideless(matVectorsFloat64, &clMemMatVectors, CL_MEM_READ_ONLY, m_clContext, m_clCommandQueue));

	// Create matResult1 memory
	clMemMatResult1 = clCreateBuffer(m_clContext, CL_MEM_READ_WRITE, (matResult1_rows * matResult1_cols) * sizeof(TYP), nullptr, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer(clMemMatResult1) failed");

	// Set kernel arguments (Part #1)
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 0, sizeof(clMemMatVectors), (const void*)&clMemMatVectors), "clSetKernelArg(m_clkernelPart1, matVectors, 0) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 3, sizeof(clMemMatResult1), (const void*)&clMemMatResult1), "clSetKernelArg(m_clkernelPart1, matResult, 3) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 6, sizeof(matResult1_colsInt), (const void*)&matResult1_colsInt), "clSetKernelArg(m_clkernelPart1, matResult_cols, 6) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart1, 7, sizeof(matVectors_rowsInt), (const void*)&matVectors_rowsInt), "clSetKernelArg(m_clkernelPart1, matVectors_rows, 7) failed");
	
	// Execute
#define SVS 1 // must be same number as what is defined in CL script (IMPORTANT: WHY "7" is good by "8" is BAD)?
	localWorkSize[0] = 16;
	localWorkSize[1] = 16;
	globalWorkSize[0] = (((matResult1_cols + (SVS - 1)) / SVS) + 15) & -16; // matResult1_cols is number of support vectors (should be large, e.g 56958)
	globalWorkSize[1] = ((matResult1_rows + 15) & -16); // number of inputs (features, should be small) - variable [1 - INFINIT]

	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(m_clCommandQueue, m_clkernelPart1, 2, nullptr, globalWorkSize, localWorkSize,
		0, nullptr, &clEventKernel1), "clEnqueueNDRangeKernel(m_clkernelPart1) failed");

	// Create clMemMatResult2 for kernel #2 while kernel #1 is running
	matResult2_colsInt = static_cast<cl_int>(((globalWorkSize[0] + 255) / 256));
	clMemMatResult2 = clCreateBuffer(m_clContext, CL_MEM_READ_WRITE, (matResult2_rowsInt * matResult2_colsInt) * sizeof(compv_float64_t), nullptr, &clerr);
	COMPV_CHECK_CL_CODE_BAIL(clerr, "clCreateBuffer(clMemMatResult2) failed");

	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart2, 0, sizeof(clMemMatResult1), (const void*)&clMemMatResult1), "clSetKernelArg(m_clkernelPart2, matResult1, 0) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart2, 1, sizeof(clMemMatResult2), (const void*)&clMemMatResult2), "clSetKernelArg(m_clkernelPart2, matResult2, 1) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart2, 2, sizeof(matResult1_colsInt), (const void*)&matResult1_colsInt), "clSetKernelArg(m_clkernelPart2, matResult1_cols, 2) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart2, 3, sizeof(matResult1_rowsInt), (const void*)&matResult1_rowsInt), "clSetKernelArg(m_clkernelPart2, matResult1_rows, 3) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelPart2, 4, sizeof(matResult2_colsInt), (const void*)&matResult2_colsInt), "clSetKernelArg(m_clkernelPart2, matResult2_cols, 4) failed");

	COMPV_DEBUG_INFO_CODE_TODO("Do we need to wait ?? before running kernel 2?");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clWaitForEvents(1, &clEventKernel1), "clWaitForEvents(clEventKernel1) failed");

	// Kernel #2
	localWorkSize[0] = 256;
	localWorkSize[1] = 1;
	globalWorkSize[0] = (globalWorkSize[0] + 255) & -256;
	globalWorkSize[1] = matResult1_rowsInt;

	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(m_clCommandQueue, m_clkernelPart2, 2, nullptr, globalWorkSize, localWorkSize,
		0, nullptr, &clEventKernel2), "clEnqueueNDRangeKernel(m_clkernelPart2) failed");
	COMPV_CHECK_CL_CODE_BAIL(clerr = clWaitForEvents(1, &clEventKernel2), "clWaitForEvents(clEventKernel2) failed");

	if (globalWorkSize[0] > (localWorkSize[0] * localWorkSize[0])) {
		COMPV_DEBUG_INFO_CODE_NOT_TESTED("Looping more than once not fully tested yet (need SVM with such large number of support vectors)");
	}

	if ((globalWorkSize[0] = static_cast<size_t>(matResult2_colsInt)) > 1) {
		const void* inPtrs[2] = { (const void*)&clMemMatResult2, (const void*)&clMemMatResult1 };
		const void* outPtrs[2] = { (const void*)&clMemMatResult1, (const void*)&clMemMatResult2 };
		cl_int inCols[2] = { matResult2_colsInt, 0 }; // inCols[1] have to be updated after permutation
		cl_int inRows[2] = { matResult2_rowsInt, matResult1_rowsInt }; // inRows[1] is constant
		cl_int outCols[2] = { ((inCols[0] + 255) / 256), 0 }; // outCols[1] have to be updated after permutation

		// TODO(dmi): use localsize[0] = 8 when globalSize is now <= 256 ?

		globalWorkSize[0] = (globalWorkSize[0] + 255) & -256;

		while (globalWorkSize[0] > 1) {
			cl_int reductionIdx_prev;
			COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelReduction, 0, sizeof(cl_mem), inPtrs[reductionIdx]), "clSetKernelArg(m_clkernelReduction, inPtr, 0) failed");
			COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelReduction, 1, sizeof(cl_mem), outPtrs[reductionIdx]), "clSetKernelArg(m_clkernelReduction, outPtr, 1) failed");
			COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelReduction, 2, sizeof(inCols[reductionIdx]), (const void*)&inCols[reductionIdx]), "clSetKernelArg(m_clkernelReduction, inCols, 2) failed");
			COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelReduction, 3, sizeof(inRows[reductionIdx]), (const void*)&inRows[reductionIdx]), "clSetKernelArg(m_clkernelReduction, inRows, 3) failed");
			COMPV_CHECK_CL_CODE_BAIL(clerr = clSetKernelArg(m_clkernelReduction, 4, sizeof(outCols[reductionIdx]), (const void*)&outCols[reductionIdx]), "clSetKernelArg(m_clkernelReduction, outCols, 4) failed");

			COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueNDRangeKernel(m_clCommandQueue, m_clkernelReduction, 2, nullptr, globalWorkSize, localWorkSize,
				0, nullptr, &clEventKernelReduction), "clEnqueueNDRangeKernel(m_clkernelReduction) failed");
			COMPV_CHECK_CL_CODE_BAIL(clerr = clWaitForEvents(1, &clEventKernelReduction), "clWaitForEvents(clEventKernelReduction) failed");

			globalWorkSize[0] /= localWorkSize[0];
			reductionIdx_prev = reductionIdx; // save previous permutation index
			reductionIdx = (reductionIdx + 1) & 1; // update permutation index to next one
			inCols[reductionIdx] = outCols[reductionIdx_prev];
			outCols[reductionIdx] = inCols[reductionIdx] / cl_int(localWorkSize[0]); // no need to align forward alread the case at arrays init
		}
	}

	// Read Data from CPU
	COMPV_CHECK_CODE_RETURN(err = CompVMat::newObjStrideless<double>(&matResultSVMValues, 1, numvectors)); // very small buffer after reduction
	COMPV_CHECK_CL_CODE_BAIL(clerr = clEnqueueReadBuffer(m_clCommandQueue, ((reductionIdx == 1) ? clMemMatResult1 : clMemMatResult2), CL_TRUE, 0, matResultSVMValues->planeSizeInBytes(0), matResultSVMValues->ptr<void>(), 0, nullptr, &clEventReadBuffer)
		, "clEnqueueReadBuffer failed");

#if COMPV_CL_PROFILING_ENABLED
	{
		cl_event clEventKernels[] = { clEventKernel1, clEventKernel2, clEventKernelReduction, clEventReadBuffer };
		const char* clEventKernelsNames[] = { "Part #1", "Part #2", "Reduction", "ReadBuffer" };
		for (size_t i = 0; i < sizeof(clEventKernels) / sizeof(clEventKernels[0]); ++i) {
			if (!clEventKernels[i]) {
				continue;
			}
			cl_ulong startTime, endTime;
			cl_ulong nanoSeconds;

			COMPV_CHECK_CL_CODE_BAIL(clerr = clGetEventProfilingInfo(clEventKernels[i], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, 0), "clGetEventProfilingInfo(CL_PROFILING_COMMAND_START) failed");
			COMPV_CHECK_CL_CODE_BAIL(clerr = clGetEventProfilingInfo(clEventKernels[i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, 0), "clGetEventProfilingInfo(CL_PROFILING_COMMAND_END) failed");

			nanoSeconds = (endTime - startTime); // GPGPU timer resolution = 10e-9 (nanoseconds)
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s: duration = %lf ms", clEventKernelsNames[i], (nanoSeconds * 1e-6));
		}
	}
#endif /* COMPV_CL_PROFILING_ENABLED */

	{
		// Building labels
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&matResult_, 1, numvectors));
		const double* matValuesPtr = matResultSVMValues->ptr<const double>();
		int32_t* matResultPtr = matResult_->ptr<int32_t>();
		for (size_t i = 0; i < numvectors; ++i) {
			matResultPtr[i] = m_labels[(matValuesPtr[i] < 0)];
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
	if (clEventKernel2) {
		clReleaseEvent(clEventKernel2), clEventKernel2 = nullptr;
	}
	if (clEventKernelReduction) {
		clReleaseEvent(clEventKernelReduction), clEventKernelReduction = nullptr;
	}
	if (clEventReadBuffer) {
		clReleaseEvent(clEventReadBuffer), clEventReadBuffer = nullptr;
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
