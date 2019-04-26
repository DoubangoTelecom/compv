/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_pca.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_eigen.h"
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/math/compv_math_stats.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_json.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_fileutils.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/jsoncpp-1.8.4/json.h"

/*
	Implemenation for Principal component analysis (PCA)
	- https://en.wikipedia.org/wiki/Principal_component_analysis
	- https://www.learnopencv.com/principal-component-analysis/
	- Computing covar matrix: https://www.itl.nist.gov/div898/handbook/pmc/section5/pmc541.htm
*/

#define COMPV_THIS_CLASSNAME "CompVMathPCA"

#define COMPV_MATH_PCA_SUB_SAMPLES_PER_THREAD (50 * 50)

COMPV_NAMESPACE_BEGIN()

template<typename T>
static COMPV_ERROR_CODE CompVMathPCAMean(const CompVMatPtr& input, CompVMatPtrPtr mean);

template<typename T>
static COMPV_ERROR_CODE CompVMathPCACovariance(const CompVMatPtr& input, const CompVMatPtr& mean, CompVMatPtrPtr covar);

CompVMathPCA::CompVMathPCA()
	: m_nMaxDimensions(0)
{

}

CompVMathPCA:: ~CompVMathPCA()
{

}

// rowBased = true means each row represent an observation. If there are N observations with K dimension then,
// vectors will be and (N x K) matrix.
COMPV_ERROR_CODE CompVMathPCA::compute(const CompVMatPtr& observations, const int maxDimensions, const bool rowBased)
{
	COMPV_CHECK_EXP_RETURN(!observations, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(observations->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && observations->subType() != COMPV_SUBTYPE_RAW_FLOAT64, COMPV_ERROR_CODE_E_INVALID_SUBTYPE);

	// Transform input (cols = observations and rows = dimensions)
	CompVMatPtr input;
	if (rowBased) {
		// Change values to have each colum representing an observation
		COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(observations, &input));
	}
	else {
		COMPV_CHECK_CODE_RETURN(observations->clone(&input));
	}

	// Check input (must have at least #2 rows/observations)
	COMPV_CHECK_EXP_RETURN(input->rows() <= 1 || input->rows() <= static_cast<size_t>(maxDimensions), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Compute mean and covar
	CompVMatPtr mean, covar;
	if (input->subType() == COMPV_SUBTYPE_RAW_FLOAT32) {
		COMPV_CHECK_CODE_RETURN((CompVMathPCAMean<compv_float32_t>(input, &mean)));
		COMPV_CHECK_CODE_RETURN(CompVMathPCACovariance<compv_float32_t>(input, mean, &covar));
	}
	else {
		COMPV_CHECK_CODE_RETURN((CompVMathPCAMean<compv_float64_t>(input, &mean)));
		COMPV_CHECK_CODE_RETURN(CompVMathPCACovariance<compv_float64_t>(input, mean, &covar));
	}

	// Transform covariance matrix from any type to double to improve acurracy when computing the eigen values/vectors
	if (covar->subType() != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float32_t, compv_float64_t>(covar, &covar)));
	}

	// Compute Eigen values and Eigen vectors
	static const bool kSortEigenValuesVectors = true;
	static const bool kRowVectors = true; // EigenVectors (output for 'CompVMatrix::eigenS') as rows?
	static const bool kForceZerosInD = true;
	CompVMatPtr eigenValues, eigenVectors;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::eigenS(covar, &eigenValues, &eigenVectors, kSortEigenValuesVectors, kRowVectors, kForceZerosInD));

	// Norm eigen vectors
#if 0 // MUST NOT
	COMPV_ASSERT(false);
	COMPV_CHECK_CODE_RETURN(CompVMathStats::normMinmax(eigenVectors, &eigenVectors));
#endif

	// Create final eigen values/vectors
	CompVMatPtr ptr32fEigenVectors, ptr32fEigenValues, ptr32fMean;
	const CompVRectFloat32 roiVectors = {
		0.f,
		0.f,
		static_cast<compv_float32_t>(eigenVectors->cols()),
		static_cast<compv_float32_t>(maxDimensions - 1)
	};
	CompVMatPtr eigenVectorsBind;
	COMPV_CHECK_CODE_RETURN(eigenVectors->bind(&eigenVectorsBind, roiVectors));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&ptr32fEigenValues, 1, maxDimensions));
	compv_float32_t* ptr32fEigenValuesPtr = ptr32fEigenValues->ptr<compv_float32_t>();

	if (eigenVectors->subType() == COMPV_SUBTYPE_RAW_FLOAT64) {
		// Vectors
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float64_t, compv_float32_t>(eigenVectorsBind, &ptr32fEigenVectors)));
		// Values
		for (size_t i = 0; i < static_cast<size_t>(maxDimensions); ++i) {
			ptr32fEigenValuesPtr[i] = static_cast<compv_float32_t>(*eigenValues->ptr<const compv_float64_t>(i, i));
		}
	}
	else {
		// MUST clone because 'eigenVectorsBind' lifetime is tied to 'eigenVectors' which is a local variable
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&ptr32fEigenVectors, eigenVectorsBind));
		COMPV_CHECK_CODE_RETURN(CompVMem::copy(ptr32fEigenVectors->ptr<void>(), eigenVectorsBind->ptr<const void>(), ptr32fEigenVectors->dataSizeInBytes()));
		for (size_t i = 0; i < static_cast<size_t>(maxDimensions); ++i) {
			ptr32fEigenValuesPtr[i] = *eigenValues->ptr<const compv_float32_t>(i, i);
		}
	}

	// Mean
	if (mean->subType() == COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float64_t, compv_float32_t>(mean, &ptr32fMean)));
	}
	else {
		ptr32fMean = mean;
	}

	// Set values
	m_ptr32fEigenValues = ptr32fEigenValues;
	m_ptr32fEigenVectors = ptr32fEigenVectors;
	m_ptr32fMean = ptr32fMean;
	m_nMaxDimensions = maxDimensions;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathPCA::read(CompVMathPCAPtrPtr pca, const char* filePath)
{
	COMPV_CHECK_EXP_RETURN(!pca || !filePath, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBufferPtr content;
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
	Json::Reader reader;
	COMPV_GCC_DISABLE_WARNINGS_END()
	Json::Value root;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(filePath, &content));	
	COMPV_CHECK_EXP_RETURN(!reader.parse(reinterpret_cast<const char*>(content->ptr()), reinterpret_cast<const char*>(content->ptr()) + content->size(), root, false)
		, COMPV_ERROR_CODE_E_JSON_CPP, "JSON parsing failed");

	CompVMatPtr ptr32fMean, ptr32fEigenVectors, ptr32fEigenValues;
	COMPV_CHECK_CODE_RETURN(CompVJSON::read(&root, "mean", &ptr32fMean));
	COMPV_CHECK_CODE_RETURN(CompVJSON::read(&root, "vectors", &ptr32fEigenVectors));
	COMPV_CHECK_CODE_RETURN(CompVJSON::read(&root, "values", &ptr32fEigenValues));

	COMPV_ASSERT(ptr32fMean->rows() == 1);
	COMPV_ASSERT(ptr32fEigenValues->rows() == 1);
	COMPV_ASSERT(ptr32fEigenVectors->rows() == ptr32fEigenValues->cols());
	COMPV_ASSERT(ptr32fEigenVectors->cols() == ptr32fMean->cols());

	COMPV_CHECK_CODE_RETURN(CompVMathPCA::newObj(pca));
	(*pca)->m_nMaxDimensions = static_cast<int>(ptr32fEigenVectors->rows());
	(*pca)->m_ptr32fMean = ptr32fMean;
	(*pca)->m_ptr32fEigenVectors = ptr32fEigenVectors;
	(*pca)->m_ptr32fEigenValues = ptr32fEigenValues;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathPCA::write(const char* filePath) const
{
	COMPV_CHECK_EXP_RETURN(!filePath, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!isReady(), COMPV_ERROR_CODE_E_INVALID_STATE, "Not ready. You should call compute first");

	Json::Value root;
	COMPV_CHECK_CODE_RETURN(CompVJSON::write(&root, "vectors", m_ptr32fEigenVectors));
	COMPV_CHECK_CODE_RETURN(CompVJSON::write(&root, "values", m_ptr32fEigenValues));
	COMPV_CHECK_CODE_RETURN(CompVJSON::write(&root, "mean", m_ptr32fMean));
	
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
	Json::FastWriter writer;
	COMPV_GCC_DISABLE_WARNINGS_END()
	const std::string root_string = writer.write(root);
	COMPV_CHECK_EXP_RETURN(root_string.empty(), COMPV_ERROR_CODE_E_JSON_CPP, "Writting json object failed");
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::write(filePath, root_string.c_str(), root_string.size()));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathPCA::project(const CompVMatPtr& observations, CompVMatPtrPtr vectors) const
{
	COMPV_CHECK_EXP_RETURN(!observations || observations->isEmpty() || !vectors, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!isReady(), COMPV_ERROR_CODE_E_INVALID_STATE, "Not ready. You should call compute first");
	if (observations->cols() != m_ptr32fEigenVectors->cols()) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalize dimension. Found %zu, expected %zu", observations->cols(), m_ptr32fEigenVectors->cols());
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	const COMPV_SUBTYPE subtype = m_ptr32fEigenVectors->subType();
	if (observations->subType() != subtype) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalize subtype. Found %s, expected %s", CompVGetSubtypeString(observations->subType()), CompVGetSubtypeString(subtype));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}

	// Substract mean (kind of normaliz -zero-mean centered-)
	CompVMatPtr input;
	if (m_ptr32fMean) {
		COMPV_ASSERT(m_ptr32fMean->cols() == observations->cols());
		const size_t rows = observations->rows();
		if (rows == 1) {
			COMPV_CHECK_CODE_RETURN(CompVMath::sub(observations, m_ptr32fMean, &input));
		}
		else {
			COMPV_CHECK_CODE_RETURN(observations->clone(&input));
			auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
				for (size_t j = ystart; j < yend; ++j) {
					// Sub(A, B, R) expect A.rows == B.rows. In our case, Mean.rows == 1 -> bind row by row
					const CompVRectFloat32 roi = {
						0.f, // left
						static_cast<compv_float32_t>(j), // top
						static_cast<compv_float32_t>(input->cols()), // right
						static_cast<compv_float32_t>(j) // bottom
					};
					CompVMatPtr inputBind;
					COMPV_CHECK_CODE_RETURN(input->bind(&inputBind, roi));
					COMPV_CHECK_CODE_RETURN(CompVMath::sub(inputBind, m_ptr32fMean, &inputBind)); // Sub(A, B, R) will NOT override R if [R==A] or [R==B]
				}
				return COMPV_ERROR_CODE_S_OK;
			};
			COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
				funcPtr,
				input->cols(),
				input->rows(),
				COMPV_MATH_PCA_SUB_SAMPLES_PER_THREAD
			));
		}
	}
	else {
		input = observations;
	}

	// Project
	CompVMatPtr vectors_ = (*vectors == input) ? nullptr : *vectors;
	COMPV_CHECK_CODE_RETURN(CompVMath::mulABt(input, m_ptr32fEigenVectors, &vectors_));

	// Set result and return
	*vectors = vectors_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathPCA::newObj(CompVMathPCAPtrPtr pca)
{
	COMPV_CHECK_EXP_RETURN(!pca, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMathPCAPtr pca_ = new CompVMathPCA();
	COMPV_CHECK_EXP_RETURN(!pca_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*pca = pca_;
	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static COMPV_ERROR_CODE CompVMathPCAMean(const CompVMatPtr& input, CompVMatPtrPtr mean)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_ASSERT(input->isRawTypeMatch<T>());

	// Each column in input represent an observation -> each row represent the same component

	const size_t cols = input->cols();
	const size_t rows = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr mean_ = (input == *mean) ? nullptr : *mean;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&mean_, 1, rows));

	T* meanPtr = mean_->ptr<T>();
	const T* inputPtr = input->ptr<const T>();
	const double scale = 1.0 / static_cast<double>(cols);

	for (size_t j = 0; j < rows; ++j) {
		double sum = 0;
		for (size_t i = 0; i < cols; ++i) {
			sum += inputPtr[i];
		}
		meanPtr[j] = static_cast<T>(sum * scale);
		inputPtr += stride;
	}

	*mean = mean_;
	return COMPV_ERROR_CODE_S_OK;
}

// https://www.itl.nist.gov/div898/handbook/pmc/section5/pmc541.htm
template<typename T>
static COMPV_ERROR_CODE CompVMathPCACovariance(const CompVMatPtr& input, const CompVMatPtr& mean, CompVMatPtrPtr covar)
{
	// For now this function is called on training phase only
	// -> do not waste your time writing SIMD/GPGPU code

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	COMPV_ASSERT(input->subType() == mean->subType() && input->isRawTypeMatch<T>());
	COMPV_ASSERT(input->rows() == mean->cols() && mean->rows() == 1);
	const size_t cols = input->cols();
	const size_t rows = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr inputSubMean;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&inputSubMean, rows, cols, stride));
	
	// Substract mean
	// Row-major, different than the SubMean compute in CompVMathPCASubstractMean
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found - SubstractMean");
	const T* meanPtr = mean->ptr<const T>();
	const T* inputPtr = input->ptr<const T>();
	T* inputSubMeanPtr = inputSubMean->ptr<T>();
	for (size_t j = 0; j < rows; ++j) {
		const T mean_ = meanPtr[j];
		for (size_t i = 0; i < cols; ++i) {
			inputSubMeanPtr[i] = (inputPtr[i] - mean_);
		}
		inputPtr += stride;
		inputSubMeanPtr += stride;
	}

	// Covar = M * Mt
	COMPV_CHECK_CODE_RETURN(CompVMath::mulABt(inputSubMean, inputSubMean, covar));

	// Scale
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found - Scale");
	if (cols > 1) {
		const T scale = 1 / static_cast<T>(cols - 1);
		const size_t cols = (*covar)->cols();
		const size_t rows = (*covar)->rows();
		const size_t stride = (*covar)->stride();
		T* covarPtr = (*covar)->ptr<T>();
		for (size_t j = 0; j < rows; ++j) {
			for (size_t i = 0; i < cols; ++i) {
				covarPtr[i] *= scale;
			}
			covarPtr += stride;
		}
	}
	
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
