/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/ml/compv_base_ml_svm_predict.h"
#include "compv/base/ml/libsvm-322/libsvm.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_fileutils.h"
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/time/compv_time.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_generic_invoke.h"

#define COMPV_THIS_CLASSNAME_PREDICT		"CompVMachineLearningSVMPredict"
#define COMPV_THIS_CLASSNAME_PREDICT_CPU1	"CompVMachineLearningSVMPredictBinaryRBF_CPU1"
#define COMPV_THIS_CLASSNAME_PREDICT_CPU2	"CompVMachineLearningSVMPredictBinaryRBF_CPU2"

COMPV_NAMESPACE_BEGIN()

//
//	CompVMachineLearningSVMPredictBinaryRBF_CPU1 (SIMD [AVX, SSE, NEON] accelerated but emulates something simular to LIBSVM implementation)
//
COMPV_OBJECT_DECLARE_PTRS(MachineLearningSVMPredictBinaryRBF_CPU1)

class CompVMachineLearningSVMPredictBinaryRBF_CPU1 : public CompVMachineLearningSVMPredict
{	
protected:
	CompVMachineLearningSVMPredictBinaryRBF_CPU1(const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t (&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
		: CompVMachineLearningSVMPredict(CompVMachineLearningSVMPredictTypeBinaryRBF)
	{
		COMPV_ASSERT(matSV != nullptr && (matSV->rows() == static_cast<size_t>(nr_sv[0] + nr_sv[1])));
		m_64fGamma = gamma;
		m_64fGammaMinus = -gamma;
		m_64fRho = rho;
		m_Labels[0] = labels[0]; m_Labels[1] = labels[1];
		m_nrSV[0] = nr_sv[0]; m_nrSV[1] = nr_sv[1];
		COMPV_CHECK_CODE_ASSERT(matSV->clone(&m_ptrMatSV));
		COMPV_CHECK_CODE_ASSERT(matCoeff->clone(&m_ptrMatCoeff));
		m_func_ptr.init();
	}
public:
	virtual ~CompVMachineLearningSVMPredictBinaryRBF_CPU1()
	{

	}
	COMPV_OBJECT_GET_ID(CompVMachineLearningSVMPredictBinaryRBF_CPU1);

	virtual bool isGPUAccelerated() const override { return false; }

	virtual COMPV_ERROR_CODE process(const CompVMatPtr& matVectors, CompVMatPtrPtr matResult) override
	{
		COMPV_CHECK_EXP_RETURN(!matVectors || !(matVectors->isRawTypeMatch<compv_float32_t>() || matVectors->isRawTypeMatch<compv_float64_t>()) || !matResult, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED_GPU("Should use CompVMachineLearningSVMPredictBinaryRBF_GPU::process which is GPGPU accelerated");
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("T-HOG version could be used with single precision float for the first part then, double precision for reduction part. See GPGPU implementation");

		// No need for "matVectors" to be aligned

		// Check vector size
		if (matVectors->cols() != m_ptrMatSV->cols()) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME_PREDICT_CPU1, "Vector size mismatch (%zu != %zu)", matVectors->cols(), m_ptrMatSV->cols());
			return COMPV_ERROR_CODE_E_INVALID_CALL;
		}

		// Convert to float64
		CompVMatPtr matVectorsFloat64;
		if (!matVectors->isRawTypeMatch<compv_float64_t>()) {
			COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float32_t, compv_float64_t>(matVectors, &matVectorsFloat64)));
		}
		else {
			matVectorsFloat64 = matVectors;
		}

		const size_t numvectors = matVectors->rows();

		// Create result (labels)
		CompVMatPtr matResult_ = *matResult;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&matResult_, 1, numvectors));
		int32_t* matResultPtr = matResult_->ptr<int32_t>();

		const size_t total_sv = m_ptrMatSV->rows();
		
		auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
			CompVMatPtr kvalueMat;
			COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<compv_float64_t>(&kvalueMat, 1, total_sv));
			compv_float64_t* kvalueMatPtr = kvalueMat->ptr<compv_float64_t>();
			compv_float64_t sum;
			for (size_t i = start; i < end; ++i) {
				COMPV_CHECK_CODE_ASSERT(rbf(matVectorsFloat64->ptr<const compv_float64_t>(i), kvalueMatPtr)); // multithreaded
				m_func_ptr.dot_64f64f(m_ptrMatCoeff->data<const compv_float64_t>(), kvalueMatPtr, total_sv, 1, 0, 0, &sum); // multithreaded
				matResultPtr[i] = m_Labels[(((sum - m_64fRho)) < 0)];
			}
			return COMPV_ERROR_CODE_S_OK;
		};

		// Muti-threading decision
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		const size_t maxThreads = static_cast<size_t>(threadDisp ? threadDisp->threadsCount() : 1); // minus 1 to avoid freezing the PC
		const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
			? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(1, numvectors, maxThreads, 1)
			: 1;
		if (threadsCount > numvectors) {
			// MT based on the support vectors (numvectors is small compared to the number of avail CPUs)
			COMPV_CHECK_CODE_RETURN(funcPtr(0, numvectors));
		}
		else {
			// MT based on the input (numvectors is large compared to the number of avail CPUs)
			COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
				funcPtr,
				1,
				numvectors,
				1
			));
		}

		*matResult = matResult_;
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE newObj(CompVMachineLearningSVMPredictPtrPtr mlSVM, const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
	{
		COMPV_CHECK_EXP_RETURN(!mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVMachineLearningSVMPredictBinaryRBF_CPU1Ptr mlSVM_;
		mlSVM_ = new CompVMachineLearningSVMPredictBinaryRBF_CPU1(gamma, rho, labels, nr_sv, matSV, matCoeff);

		COMPV_CHECK_EXP_RETURN(!mlSVM_, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
		*mlSVM = *mlSVM_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	COMPV_ERROR_CODE rbf(const compv_float64_t* inPtr, compv_float64_t* kvalues)
	{
		// Private function, no need to check for input parameters

		const size_t total_sv = m_ptrMatSV->rows();
		const size_t sv_length = m_ptrMatSV->cols();

		// inPtr size = (1 x sv_length)		
		auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
			for (size_t j = start; j < end; ++j) {
				m_func_ptr.dotSub_64f64f(inPtr, m_ptrMatSV->ptr<const compv_float64_t>(j), sv_length, 1, 0, 0, &kvalues[j]); // no memalign required
			}
			m_func_ptr.scale_64f64f(&kvalues[start], &kvalues[start], (end - start), 1, 0, &m_64fGammaMinus); // no memalign required
			m_func_ptr.expo(&kvalues[start], &kvalues[start], (end - start)); // no memalign required

			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			1,
			total_sv,
			1
		));

		return COMPV_ERROR_CODE_S_OK;
	}

private:
	compv_float64_t m_64fGamma;
	compv_float64_t m_64fGammaMinus;
	compv_float64_t m_64fRho;
	int32_t m_Labels[2];
	int32_t m_nrSV[2];
	CompVMatPtr m_ptrMatSV;
	CompVMatPtr m_ptrMatCoeff;
	svm_simd_func_ptrs m_func_ptr;
};



//
//	CompVMachineLearningSVMPredictBinaryRBF_CPU2 (SIMD [AVX, SSE, NEON] accelerated but emulates something simular to GPGPU implementation)
//
COMPV_OBJECT_DECLARE_PTRS(MachineLearningSVMPredictBinaryRBF_CPU2)

class CompVMachineLearningSVMPredictBinaryRBF_CPU2 : public CompVMachineLearningSVMPredict
{
protected:
	CompVMachineLearningSVMPredictBinaryRBF_CPU2(const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
		: CompVMachineLearningSVMPredict(CompVMachineLearningSVMPredictTypeBinaryRBF)
	{
		COMPV_ASSERT(matSV != nullptr && (matSV->rows() == static_cast<size_t>(nr_sv[0] + nr_sv[1])));
		m_64fGamma = gamma;
		m_64fGammaMinus = -gamma;
		m_64fRho = rho;
		m_Labels[0] = labels[0]; m_Labels[1] = labels[1];
		m_nrSV[0] = nr_sv[0]; m_nrSV[1] = nr_sv[1];
		COMPV_CHECK_CODE_ASSERT(matSV->clone(&m_ptrMatSV));
		COMPV_CHECK_CODE_ASSERT(matCoeff->clone(&m_ptrMatCoeff));
	}
public:
	virtual ~CompVMachineLearningSVMPredictBinaryRBF_CPU2()
	{

	}
	COMPV_OBJECT_GET_ID(CompVMachineLearningSVMPredictBinaryRBF_CPU2);

	virtual bool isGPUAccelerated() const override { return false; }

	virtual COMPV_ERROR_CODE process(const CompVMatPtr& matVectors, CompVMatPtrPtr matResult) override
	{
		COMPV_CHECK_EXP_RETURN(!matVectors || !(matVectors->isRawTypeMatch<compv_float32_t>() || matVectors->isRawTypeMatch<compv_float64_t>()) || !matResult, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED_GPU("Should use CompVMachineLearningSVMPredictBinaryRBF_GPU::process which is GPGPU accelerated");
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("This code is for testing only, pleale use GPU implemention or CPU1 version"); // reference code for GPU

		// No need for "matVectors" to be aligned

		// Check vector size
		if (matVectors->cols() != m_ptrMatSV->cols()) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME_PREDICT_CPU1, "Vector size mismatch (%zu != %zu)", matVectors->cols(), m_ptrMatSV->cols());
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

		const size_t numvectors = matVectors->rows();

		CompVMatPtr matValues;
		COMPV_CHECK_CODE_RETURN(process_64f64f(matVectorsFloat64, &matValues));
		COMPV_ASSERT(matValues->cols() == m_ptrMatSV->rows() && matValues->rows() == numvectors);

		CompVMatPtr matResult_ = *matResult;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&matResult_, 1, numvectors));
		const compv_float64_t* matValuesPtr = matValues->ptr<const compv_float64_t>();
		const size_t matValuesStride = matValues->stride();
		int32_t* matResultPtr = matResult_->ptr<int32_t>();
		for (size_t i = 0; i < numvectors; ++i) {
			matResultPtr[i] = m_Labels[(matValuesPtr[0] < 0)];
			matValuesPtr += matValuesStride;
		}
		
		*matResult = matResult_;
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE newObj(CompVMachineLearningSVMPredictPtrPtr mlSVM, const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
	{
		COMPV_CHECK_EXP_RETURN(!mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVMachineLearningSVMPredictBinaryRBF_CPU2Ptr mlSVM_;
		mlSVM_ = new CompVMachineLearningSVMPredictBinaryRBF_CPU2(gamma, rho, labels, nr_sv, matSV, matCoeff);

		COMPV_CHECK_EXP_RETURN(!mlSVM_, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
		*mlSVM = *mlSVM_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	COMPV_ERROR_CODE process_64f64f(const CompVMatPtr& vectors, CompVMatPtrPtr R)
	{
		COMPV_CHECK_EXP_RETURN(!vectors || !R || vectors->subType() != m_ptrMatSV->subType() || vectors->cols() != m_ptrMatSV->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_ASSERT(vectors->isRawTypeMatch<compv_float64_t>() && m_ptrMatSV->isRawTypeMatch<compv_float64_t>());

		CompVMatPtr C;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&C, vectors->rows(), m_ptrMatSV->rows()));

		// A = vectors
		// B = m_ptrMatSV

		const compv_float64_t* Aptr = vectors->ptr<const compv_float64_t>();
		const compv_float64_t* CoeffPtr = m_ptrMatCoeff->ptr<const compv_float64_t>();
		compv_float64_t* Cptr = C->ptr<compv_float64_t>();
		const size_t Arows = vectors->rows(); // 408
		const size_t Astride = vectors->stride();
		const size_t Cstride = C->stride();

		// Matrix multiplication options and cache optiz: http://web.cecs.pdx.edu/~jrb/cs201/lectures/cache.friendly.code.pdf

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found, see CompVMathOpMul::mulABt");
		// Next code is matrix multiplication-like except we are doing "c=(a-b)*(a-b)" instead of "c=(a*b)"
		// -> like(C = mulABt(vectors, SV))
#if 1 // mulABt
		const compv_float64_t* Bptr = m_ptrMatSV->ptr<const compv_float64_t>();
		const size_t Bcols = m_ptrMatSV->cols();
		const size_t Brows = m_ptrMatSV->rows();
		const size_t Bstride = m_ptrMatSV->stride();
		size_t k;
		for (size_t i = 0; i < Arows; ++i) {
			for (size_t j = 0; j < Brows; ++j) {
				compv_float64_t sum = 0; 
				for (k = 0; k < Bcols; k += 1) {
					const compv_float64_t diff = Aptr[(i * Astride) + k] - Bptr[(j * Bstride) + k];
					sum += (diff * diff);
				}
				Cptr[(i * Cstride) + j] = std::exp(sum * m_64fGammaMinus) * CoeffPtr[j];
			}
		}

		// GPGPU reduction (horizontal sum)
		compv_float64_t* Cptr0 = Cptr;
		const size_t Crows = C->rows();
		const size_t Ccols = C->cols();
		for (size_t j = 0; j < Crows; ++j) {
			compv_float64_t& sum = Cptr0[0];
			for (size_t i = 1; i < Ccols; ++i) {
				sum += Cptr0[i];
			}
			sum -= m_64fRho;
			Cptr0 += Cstride;
		}

#else // mulAB
			CompVMatPtr ptrMatSV_T;
			COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(m_ptrMatSV, &ptrMatSV_T));
			COMPV_CHECK_CODE_RETURN(C->zero_all());
			const compv_float64_t* Bptr = ptrMatSV_T->ptr<const compv_float64_t>();
			const size_t Bcols = ptrMatSV_T->cols(); // 56958
			const size_t Brows = ptrMatSV_T->rows(); // 63
			const size_t Bstride = ptrMatSV_T->stride();
			size_t k;
			for (k = 0; k < Brows; k += 1) {
				for (size_t i = 0; i < Arows; ++i) {
					const compv_float64_t& r = Aptr[(i * Astride) + k];
					for (size_t j = 0; j < Bcols; ++j) {
						const compv_float64_t diff = r - Bptr[(k * Bstride) + j];
						Cptr[(i * Cstride) + j] += (diff * diff);
					}
				}
			}

			// GPGPU reduction (horizontal sum)
			for (size_t i = 0; i < Arows; ++i) {
				double sum = 0;
				for (size_t j = 0; j < Bcols; ++j) {
					sum += std::exp(Cptr[(i * Cstride) + j] * m_64fGammaMinus) * CoeffPtr[j];
				}
				Cptr[(i * Cstride) + 0] = (sum - m_64fRho);
			}
#endif

		*R = C;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	compv_float64_t m_64fGamma;
	compv_float64_t m_64fGammaMinus;
	compv_float64_t m_64fRho;
	int32_t m_Labels[2];
	int32_t m_nrSV[2];
	CompVMatPtr m_ptrMatSV;
	CompVMatPtr m_ptrMatCoeff;
};


//
//	CompVMachineLearningSVMPredict
//

newObjBinaryRBFMachineLearningSVMPredict CompVMachineLearningSVMPredict::s_ptrNewObjBinaryRBF_CPU = CompVMachineLearningSVMPredictBinaryRBF_CPU1::newObj; // must be "CPU1", "CPU2" is for testing only 
newObjBinaryRBFMachineLearningSVMPredict CompVMachineLearningSVMPredict::s_ptrNewObjBinaryRBF_GPU = nullptr;

CompVMachineLearningSVMPredict::CompVMachineLearningSVMPredict(CompVMachineLearningSVMPredictType eType)
	: m_eType(eType)
{

}

CompVMachineLearningSVMPredict::~CompVMachineLearningSVMPredict()
{

}

// gpuActive -> use CompVGpu::isActiveAndEnabled
COMPV_ERROR_CODE CompVMachineLearningSVMPredict::newObjBinaryRBF(CompVMachineLearningSVMPredictPtrPtr mlSVM, const char* pathToFlatModel, bool gpuActiveAndEnabled COMPV_DEFAULT(true))
{
	COMPV_CHECK_EXP_RETURN(!CompVMachineLearningSVMPredict::s_ptrNewObjBinaryRBF_CPU, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(!mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const uint64_t timeStart = CompVTime::nowMillis();

	// Patch path and check
	const std::string path_to_model = CompVFileUtils::patchFullPath(pathToFlatModel);
	if (!CompVFileUtils::exists(path_to_model.c_str())) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME_PREDICT, "File with flat model doesn't exist: %s", path_to_model.c_str());
		return COMPV_ERROR_CODE_E_FILE_NOT_FOUND;
	}

	// Load model
	const size_t fileSize = CompVFileUtils::getSize(path_to_model.c_str());
	/* Read flat file
		gamma: float64
		rho: float64
		xsize: int32
		label: int32, int32
		nr_sv: int32, int32
		sv: ...float64....
		sv_coef: ...float64....
	*/
	static const size_t fileHeaderSize =
		(1 * sizeof(compv_float64_t)) + // gamma
		(1 * sizeof(compv_float64_t)) + // rho
		(1 * sizeof(int32_t)) + // xsize
		(2 * sizeof(int32_t)) +  // labels
		(2 * sizeof(int32_t));  // nr_sv
	static const size_t fileMinSize =
		fileHeaderSize +
		(2 * sizeof(compv_float64_t)) + // sv (at least #2)
		(2 * sizeof(compv_float64_t)); // sv_coef (at least #2)
	if (fileSize < fileMinSize) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME_PREDICT, "File size too short (%zu < %zu)", fileSize, fileMinSize);
	}
	CompVBufferPtr buffer;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(path_to_model.c_str(), &buffer)); // Do not use "fopen()+fread()" -> won't work on Android (requires using Assets)
	const compv_float64_t gamma = reinterpret_cast<const compv_float64_t*>(buffer->ptr())[0];
	const compv_float64_t rho = reinterpret_cast<const compv_float64_t*>(buffer->ptr())[1];
	const int32_t xsize = reinterpret_cast<const int32_t*>(buffer->ptr())[4];
	const int32_t labels[2] = {
		reinterpret_cast<const int32_t*>(buffer->ptr())[5],
		reinterpret_cast<const int32_t*>(buffer->ptr())[6]
	};
	const int32_t nr_sv[2] = {
		reinterpret_cast<const int32_t*>(buffer->ptr())[7],
		reinterpret_cast<const int32_t*>(buffer->ptr())[8]
	};
	COMPV_CHECK_EXP_RETURN(xsize <= 0, COMPV_ERROR_CODE_E_INVALID_CALL, "xsize must be > 0");
	const size_t total_sv = static_cast<size_t>(nr_sv[0] + nr_sv[1]);
	const size_t sv_total_size_in_uint8 = buffer->size() - fileHeaderSize; // "sv" + "sv_coef"
	const size_t sv_total_size_in_float64 = (sv_total_size_in_uint8 >> 3); // "sv" + "sv_coef"
	const size_t sv_size_in_float64 = sv_total_size_in_float64 - total_sv; // "sv" only
	COMPV_CHECK_EXP_RETURN((sv_size_in_float64 % xsize), COMPV_ERROR_CODE_E_INVALID_CALL, "(sv_size_in_float64 %% xsize) != 0");
	const size_t sv_rows_in_float64 = (sv_size_in_float64 / xsize);
	if (static_cast<int32_t>(sv_rows_in_float64) != total_sv) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME_PREDICT, "Invalid file size: sv_size_in_float64 = %zu, nr_sv=[%d, %d]", sv_size_in_float64, nr_sv[0], nr_sv[1]);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	CompVMatPtr matSV;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&matSV, total_sv, xsize));
	const compv_float64_t* buffetSVPtr = reinterpret_cast<const compv_float64_t*>((reinterpret_cast<const uint8_t *>(buffer->ptr()) + fileHeaderSize));
	COMPV_ASSERT((buffetSVPtr + sv_total_size_in_float64) == reinterpret_cast<const compv_float64_t*>((reinterpret_cast<const uint8_t *>(buffer->ptr()) + buffer->size())));
	auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		const size_t rowInBytes = matSV->rowInBytes();
		for (size_t i = start; i < end; ++i) {
			COMPV_CHECK_CODE_RETURN(CompVMem::copy(matSV->ptr<compv_float64_t>(i), &buffetSVPtr[i * xsize], rowInBytes));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		1,
		matSV->rows(),
		1000
	));

	CompVMatPtr matCoeff;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&matCoeff, 1, total_sv));
	COMPV_CHECK_CODE_RETURN(CompVMem::copy(matCoeff->ptr<compv_float64_t>(), &buffetSVPtr[total_sv * xsize], (total_sv * sizeof(compv_float64_t))));

	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME_PREDICT, "Elapsed time (loading model) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME_PREDICT, "SVM model: gamma=%lf, rho=%lf, xsize=%d, labels=[%d, %d], nr_sv=[%d, %d], total_sv=%zu",
		gamma,
		rho,
		xsize,
		labels[0], labels[1],
		nr_sv[0], nr_sv[1],
		matSV->rows());

	CompVMachineLearningSVMPredictPtr mlSVM_;
	if (gpuActiveAndEnabled && CompVMachineLearningSVMPredict::s_ptrNewObjBinaryRBF_GPU) {
		COMPV_ERROR_CODE err = CompVMachineLearningSVMPredict::s_ptrNewObjBinaryRBF_GPU(&mlSVM_, gamma, rho, labels, nr_sv, matSV, matCoeff); 
		if (COMPV_ERROR_CODE_IS_ERROR(err)) { // do not exit if fails -> CPU fallback
			COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME_PREDICT, "GPGPU is enabled but failed to create an instance. Using CPU fallback. Sad!");
		}
	}
	if (!mlSVM_) { // Create CPU implementation if GPU one failed or not active
		COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVMPredict::s_ptrNewObjBinaryRBF_CPU(&mlSVM_, gamma, rho, labels, nr_sv, matSV, matCoeff));
	}

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME_PREDICT, "Newly created SVM RBF binary predictor is GPGPU accelerated: %s", mlSVM_->isGPUAccelerated() ? "true" : "false");
	
	*mlSVM = mlSVM_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
