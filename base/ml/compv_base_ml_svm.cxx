/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/ml/compv_base_ml_svm.h"
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_fileutils.h"

#include "compv/base/ml/libsvm-322/libsvm.h"

// Libsvm guide: https://www.csie.ntu.edu.tw/~cjlin/papers/guide/guide.pdf

#define COMPV_THIS_CLASSNAME	"CompVMachineLearningSVM"

COMPV_NAMESPACE_BEGIN()

/* svm_type  */
COMPV_BASE_API const int kLibSVM_svm_type_C_SVC = C_SVC;
COMPV_BASE_API const int kLibSVM_svm_type_NU_SVC = NU_SVC;
COMPV_BASE_API const int kLibSVM_svm_type_ONE_CLASS = ONE_CLASS;
COMPV_BASE_API const int kLibSVM_svm_type_EPSILON_SVR = EPSILON_SVR;
COMPV_BASE_API const int kLibSVM_svm_type_NU_SVR = NU_SVR;

/* kernel_type */
COMPV_BASE_API const int kLibSVM_kernel_type_LINEAR = LINEAR;
COMPV_BASE_API const int kLibSVM_kernel_type_POLY = POLY;
COMPV_BASE_API const int kLibSVM_kernel_type_RBF = RBF;
COMPV_BASE_API const int kLibSVM_kernel_type_SIGMOID = SIGMOID;
COMPV_BASE_API const int kLibSVM_kernel_type_PRECOMPUTED = PRECOMPUTED;

CompVMachineLearningSVM::CompVMachineLearningSVM()
	: m_ptrLibsvmParams(nullptr)
	, m_ptrLibsvmModel(nullptr)
	, m_bPredictProbEnabled(false)
	, m_nVectorSize(0)
{

}

CompVMachineLearningSVM::~CompVMachineLearningSVM()
{
	if (m_ptrLibsvmModel) {
		svm_free_and_destroy_model(&m_ptrLibsvmModel);
	}
	if (m_ptrLibsvmParams) {
		svm_destroy_param(m_ptrLibsvmParams);
		CompVMem::free(reinterpret_cast<void**>(&m_ptrLibsvmParams));
	}
}

// Must be MT-friendly
COMPV_ERROR_CODE CompVMachineLearningSVM::predict(const CompVMatPtr& vector, int& label)
{
	// Must not lock, function will be called from different threads
	// For example, when called by ultimateText, the MT-decision is per Char

	COMPV_CHECK_EXP_RETURN(!vector || vector->rows() != 1 ||
		(vector->subType() != COMPV_SUBTYPE_RAW_FLOAT64 && vector->subType() != COMPV_SUBTYPE_RAW_FLOAT32),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!m_ptrLibsvmModel, COMPV_ERROR_CODE_E_INVALID_STATE, "No model associated to this object. Are you calling CompVMachineLearningSVM::load to create the SVM object?");
	
	// Make sure the vector size is correct
	if (vector->cols() != vectorSize()) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid vector size: %zu # %zu", vector->cols(), vectorSize());
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}

	// Create a node containing the vector data
	CompVMatPtr nodeVector;
	COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::rawToNode(vector, &nodeVector));
	label = static_cast<int>(svm_predict(m_ptrLibsvmModel, nodeVector->ptr<const svm_node>()));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::addVector(const int label, const CompVMatPtr& vector)
{
	COMPV_CHECK_EXP_RETURN(!vector || vector->rows() != 1 ||
		(vector->subType() != COMPV_SUBTYPE_RAW_FLOAT64 && vector->subType() != COMPV_SUBTYPE_RAW_FLOAT32),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_AUTOLOCK_THIS(CompVMachineLearningSVM);
	const size_t vecSize = m_vecTrainVectors.empty() ? vector->cols() : m_vecTrainVectors.back()->cols();
	// Make sure the vector size is correct
	if (vector->cols() != vecSize) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid vector size: %zu # %zu", vector->cols(), vecSize);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	CompVMatPtr vector_;
	if (vector->subType() == COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN(vector->clone(&vector_));
	}
	else {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<float, double>(vector, &vector_)));
	}
	m_vecTrainVectors.push_back(vector_);
	m_vecTrainLabels.push_back(label);

	m_nVectorSize = vecSize;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::train(const std::vector<int>& trainLabels, const CompVMatPtrVector& trainVectors, const bool _crossValidation COMPV_DEFAULT(false), const int _nfolds COMPV_DEFAULT(COMPV_LIBSVM_NFOLDS_DEFAULT))
{
	COMPV_CHECK_EXP_RETURN(trainLabels.empty() || trainLabels.size() != trainVectors.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(_crossValidation && _nfolds < 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "n-folds must be >= 2");
	COMPV_AUTOLOCK_THIS(CompVMachineLearningSVM);

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	svm_problem problem = { 0 };
	svm_model* model = nullptr;
	const char* error_msg = nullptr;
	std::function<COMPV_ERROR_CODE(const size_t ystart, const size_t yend)> funcPtr;
	m_vecNodeVectors.resize(trainVectors.size(), nullptr); // member variable because model use it as reference pointer

	problem.l = static_cast<int>(trainVectors.size());
	problem.y = reinterpret_cast<double*>(CompVMem::malloc(problem.l * sizeof(double)));
	COMPV_CHECK_EXP_BAIL(!problem.y, err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	problem.x = reinterpret_cast<svm_node**>(CompVMem::malloc(problem.l * sizeof(svm_node*)));
	COMPV_CHECK_EXP_BAIL(!problem.x, err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	// Set Problem's values
	funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		for (size_t l = start; l < end; l++) {
			COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::rawToNode(trainVectors[l], &m_vecNodeVectors[l]));
			problem.x[l] = m_vecNodeVectors[l]->ptr<svm_node>();
			problem.y[l] = trainLabels[l];
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		1,
		static_cast<size_t>(problem.l),
		1
	));

	// Check params
	error_msg = svm_check_parameter(&problem, m_ptrLibsvmParams);
	if (error_msg) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "LibSVM error=%s", error_msg);
		COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_LIBSVM);
	}

	// Train and build model
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Training started...");
	model = svm_train(&problem, m_ptrLibsvmParams);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Training finished!!");
	COMPV_CHECK_EXP_BAIL(!model, err = COMPV_ERROR_CODE_E_LIBSVM, "svm_train failed");
	if (m_ptrLibsvmModel) {
		svm_free_and_destroy_model(&m_ptrLibsvmModel);
	}
	m_ptrLibsvmModel = model;

bail:
	CompVMem::free(reinterpret_cast<void**>(&problem.y));
	CompVMem::free(reinterpret_cast<void**>(&problem.x));
	return err;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::train(const bool _crossValidation COMPV_DEFAULT(false), const int _nfolds COMPV_DEFAULT(COMPV_LIBSVM_NFOLDS_DEFAULT))
{
	COMPV_AUTOLOCK_THIS(CompVMachineLearningSVM);
	// Train using local labels and vectors
	COMPV_CHECK_CODE_RETURN(train(m_vecTrainLabels, m_vecTrainVectors, _crossValidation, _nfolds));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::save(const char* filePath, const bool releaseVectors_ COMPV_DEFAULT(true))
{
	COMPV_AUTOLOCK_THIS(CompVMachineLearningSVM);
	COMPV_CHECK_EXP_RETURN(!m_ptrLibsvmModel, COMPV_ERROR_CODE_E_INVALID_CALL, "Nothing to save, model is null");
	COMPV_CHECK_EXP_RETURN(svm_save_model(filePath, m_ptrLibsvmModel) != 0, COMPV_ERROR_CODE_E_LIBSVM, "svm_save_model failed");
	// Margaret Thatcher: "I want my memory back"
	if (releaseVectors_) {
		m_vecTrainVectors.clear();
		m_vecTrainLabels.clear();
		m_vecNodeVectors.clear();
		m_vecTrainVectors.shrink_to_fit();
		m_vecTrainLabels.shrink_to_fit();
		m_vecNodeVectors.shrink_to_fit();
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::crossValidation(const struct svm_problem* problem, const struct svm_parameter* params, const int _nfolds)
{
	// Private function, do not check input parameters

	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = reinterpret_cast<double*>(CompVMem::malloc(sizeof(double) * problem->l));
	COMPV_CHECK_EXP_RETURN(!target, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	svm_cross_validation(problem, params, _nfolds, target);
	if (params->svm_type == EPSILON_SVR || params->svm_type == NU_SVR) {
		for (int i = 0; i< problem->l; i++) {
			double y = problem->y[i];
			double v = target[i];
			total_error += (v - y)*(v - y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Cross Validation Mean squared error = %g", total_error / problem->l);
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Cross Validation Squared correlation coefficient = %g",
			((problem->l*sumvy - sumv*sumy)*(problem->l*sumvy - sumv*sumy)) /
			((problem->l*sumvv - sumv*sumv)*(problem->l*sumyy - sumy*sumy))
		);
	}
	else {
		for (int i = 0; i < problem->l; i++) {
			if (target[i] == problem->y[i]) {
				++total_correct;
			}
		}
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Cross Validation Accuracy = %g%%", 100.0*total_correct / problem->l);
	}
	CompVMem::free(reinterpret_cast<void**>(&target));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::rawToNode(const CompVMatPtr& rawVector, CompVMatPtrPtr nodeVector)
{
	// Private function, do not check parameters

	CompVMatPtr rawVector64f = rawVector;
	if (rawVector->subType() != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float32_t, compv_float64_t>(rawVector, &rawVector64f)));
	}
	const int count = static_cast<int>(rawVector64f->cols());
	CompVMatPtr nodeVector_ = (*nodeVector == rawVector && rawVector64f == rawVector) ? nullptr : *nodeVector;
	COMPV_CHECK_CODE_RETURN((CompVMat::newObjStrideless<svm_node, COMPV_MAT_TYPE_STRUCT>(&nodeVector_, 1, count + 1)));
	svm_node* xnodePtr = nodeVector_->ptr<svm_node>();
	const double* rawVector64fPtr = rawVector64f->ptr<const double>();

	for (int i = 0; i < count; ++i) {
		xnodePtr[i].index = (i + 1); // index must start at #1 and finish at #-1
		xnodePtr[i].value = rawVector64fPtr[i];
	}
	xnodePtr[count].index = -1; // term indication

	*nodeVector = nodeVector_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::load(const char* filePath, CompVMachineLearningSVMPtrPtr mlSVM)
{
	COMPV_CHECK_EXP_RETURN(!filePath || !mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (!CompVFileUtils::exists(filePath)) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "File doesn't exist (%s)", filePath);
		return COMPV_ERROR_CODE_E_FILE_NOT_FOUND;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	svm_model* model = nullptr;
	CompVMachineLearningSVMPtr mlSVM_;

	// Load model
	COMPV_CHECK_EXP_BAIL((!(model = svm_load_model(filePath))), err = COMPV_ERROR_CODE_E_LIBSVM, "svm_load_model failed");
	COMPV_CHECK_EXP_BAIL(model->param.svm_type != COMPV_SVM_TYPE_C_SVC, err = COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Only classification is supported in this beta version");
	
	// Create SVM object
	mlSVM_ = new CompVMachineLearningSVM();
	COMPV_CHECK_EXP_BAIL(!mlSVM_, err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	mlSVM_->m_ptrLibsvmModel = model;
	mlSVM_->m_bPredictProbEnabled = (svm_check_probability_model(model) != 0);
	mlSVM_->m_ptrLibsvmParams = reinterpret_cast<struct svm_parameter*>(CompVMem::malloc(sizeof(struct svm_parameter)));
	COMPV_CHECK_EXP_BAIL(!mlSVM_->m_ptrLibsvmParams, err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_BAIL(err = CompVMem::copy(mlSVM_->m_ptrLibsvmParams, &model->param, sizeof(struct svm_parameter)));
	mlSVM_->m_ptrLibsvmParams->weight_label = nullptr, mlSVM_->m_ptrLibsvmParams->weight = nullptr; // Do not copy the pointers addresses (otherwise, we'll try to delete them when params are freed)

	// Compute vector size (will be used when the user try to predict something)
	{
		size_t& m_nVectorSize = mlSVM_->m_nVectorSize;
		const svm_node *SV0 = model->SV[0];
		while (SV0++->index != -1) ++m_nVectorSize;
	}

	// Final, set
	*mlSVM = mlSVM_;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		svm_free_and_destroy_model(&model);
	}
	return err;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::newObj(CompVMachineLearningSVMPtrPtr mlSVM, const MachineLearningSVMParams& params)
{
	COMPV_CHECK_EXP_RETURN(!mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(params.svm_type != COMPV_SVM_TYPE_C_SVC, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Only classification is supported in this beta version");

	CompVMachineLearningSVMPtr mlSVM_ = new CompVMachineLearningSVM();
	COMPV_CHECK_EXP_RETURN(!mlSVM_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	struct svm_parameter*& ptrLibsvmParams = mlSVM_->m_ptrLibsvmParams;

	ptrLibsvmParams = reinterpret_cast<struct svm_parameter*>(CompVMem::calloc(1, sizeof(struct svm_parameter)));
	COMPV_CHECK_EXP_RETURN(!ptrLibsvmParams, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	// Default values
	ptrLibsvmParams->svm_type = params.svm_type;
	ptrLibsvmParams->kernel_type = params.kernel_type;
	ptrLibsvmParams->degree = params.degree;
	ptrLibsvmParams->gamma = params.gamma;	// 1/num_features
	ptrLibsvmParams->coef0 = params.coef0;
	ptrLibsvmParams->nu = params.nu;
	ptrLibsvmParams->cache_size = 100;
	ptrLibsvmParams->C = params.C;
	ptrLibsvmParams->eps = params.eps;
	ptrLibsvmParams->p = params.P;
	ptrLibsvmParams->shrinking = 1;
	ptrLibsvmParams->probability = params.probability_estimates;
	ptrLibsvmParams->nr_weight = 0;
	ptrLibsvmParams->weight_label = nullptr;
	ptrLibsvmParams->weight = nullptr;

	*mlSVM = mlSVM_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
