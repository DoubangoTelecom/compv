/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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
#include "compv/base/compv_cpu.h"
#include "compv/base/jsoncpp-1.8.4/json.h"

/*
	Implemenation for Principal component analysis (PCA)
	- https://en.wikipedia.org/wiki/Principal_component_analysis
	- https://www.learnopencv.com/principal-component-analysis/
	- Computing covar matrix: https://www.itl.nist.gov/div898/handbook/pmc/section5/pmc541.htm
*/

COMPV_NAMESPACE_BEGIN()

template<typename T>
static COMPV_ERROR_CODE CompVMathPCAMean(const CompVMatPtr& input, CompVMatPtrPtr mean);

template<typename T>
static COMPV_ERROR_CODE CompVMathPCASubstractMean(CompVMatPtr& input, const CompVMatPtr& mean);

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

	// Transform input (cols = observations and rows = dimensions)
	CompVMatPtr input;
	if (rowBased) {
		// Change values to have each colum representing an observation
		COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(observations, &input));
	}
	else {
		COMPV_CHECK_CODE_RETURN(observations->clone(&input));
	}

	const COMPV_SUBTYPE inputSubType = input->subType();

	// Check input
	COMPV_CHECK_EXP_RETURN(input->rows() <= 1 || input->rows() <= maxDimensions, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(inputSubType != COMPV_SUBTYPE_RAW_FLOAT32 && inputSubType != COMPV_SUBTYPE_RAW_FLOAT32, COMPV_ERROR_CODE_E_INVALID_SUBTYPE);

	// Compute mean and substract
	CompVMatPtr mean;
	if (inputSubType == COMPV_SUBTYPE_RAW_FLOAT32) {
		COMPV_CHECK_CODE_RETURN((CompVMathPCAMean<compv_float32_t>(input, &mean)));
		COMPV_CHECK_CODE_RETURN((CompVMathPCASubstractMean<compv_float32_t>(input, mean)));
	}
	else {
		COMPV_CHECK_CODE_RETURN((CompVMathPCAMean<compv_float64_t>(input, &mean)));
		COMPV_CHECK_CODE_RETURN((CompVMathPCASubstractMean<compv_float64_t>(input, mean)));
	}

	// Compute covariance matrix
	CompVMatPtr covar;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulABt(input, input, &covar));
	input = nullptr; // Margaret Thatcher: "I want my memory back"

	// Transform covariance matrix from any type to double to improve acurracy when computing the eigen values/vectors
	if (inputSubType != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float32_t, compv_float64_t>(covar, &covar)));
	}

	// Compute Eigen values and Eigen vectors
	static const bool kSortEigenValuesVectors = true;
	static const bool kRowVectors = false; // EigenVectors (output for 'CompVMatrix::eigenS') as rows?
	static const bool kForceZerosInD = true;
	CompVMatPtr eigenValues, eigenVectors;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::eigenS(covar, &eigenValues, &eigenVectors, kSortEigenValuesVectors, kRowVectors, kForceZerosInD));

	COMPV_DEBUG_INFO_CODE_TODO("normalize the vectors??");
	COMPV_CHECK_CODE_RETURN(CompVMathStats::normMinmax(eigenVectors, &eigenVectors));

	// Create final eigen values/vectors
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD code could be found"); // eigenValues and eigenVectors vectors should be very small square matrix (dim x dim)
	CompVMatPtr ptr32fEigenVectors, ptr32fEigenValues;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&ptr32fEigenValues, 1, maxDimensions));
	const CompVRectFloat32 roi = {
		0.f,
		0.f,
		static_cast<compv_float32_t>(eigenVectors->cols()),
		static_cast<compv_float32_t>(maxDimensions - 1)
	};
	CompVMatPtr eigenVectorsBind;
	COMPV_CHECK_CODE_RETURN(eigenVectors->bind(&eigenVectorsBind, roi));
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float64_t, compv_float32_t>(eigenVectorsBind, &ptr32fEigenVectors)));
	compv_float32_t* ptr32fEigenValuesPtr = ptr32fEigenValues->ptr<compv_float32_t>();
	for (size_t i = 0; i < static_cast<size_t>(maxDimensions); ++i) {
		ptr32fEigenValuesPtr[i] = static_cast<compv_float32_t>(*eigenValues->ptr<const compv_float64_t>(i, i));
	}

	// Create final mean
	CompVMatPtr ptr32fMean;
	if (mean->subType() == COMPV_SUBTYPE_RAW_FLOAT32) {
		ptr32fMean = mean;
	}
	else {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float64_t, compv_float32_t>(mean, &ptr32fMean)));
	}

	// Set values
	m_ptr32fEigenValues = ptr32fEigenValues;
	m_ptr32fEigenVectors = ptr32fEigenVectors;
	m_ptr32fMean = ptr32fMean;
	m_nMaxDimensions = maxDimensions;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathPCA::read(const char* filePath, CompVMathPCAPtrPtr pca)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathPCA::write(const char* filePath, const CompVMathPCAPtr& pca)
{
	COMPV_CHECK_EXP_RETURN(!pca || !pca->isReady(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
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

template<typename T>
static COMPV_ERROR_CODE CompVMathPCASubstractMean(CompVMatPtr& input, const CompVMatPtr& mean)
{
	COMPV_ASSERT(input->subType() == mean->subType() && input->isRawTypeMatch<T>());

	const size_t cols = input->cols();
	const size_t rows = input->rows();
	const size_t stride = input->stride();

	const T* meanPtr = mean->ptr<const T>();
	T* inputPtr = input->ptr<T>();

	for (size_t j = 0; j < rows; ++j) {
		const T& mean_ = meanPtr[j];
		for (size_t i = 0; i < cols; ++i) {
			inputPtr[i] -= mean_;
		}
		inputPtr += stride;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
