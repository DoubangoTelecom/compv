/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/ml/compv_base_ml_svm.h"
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/math/compv_math.h"
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
COMPV_ERROR_CODE CompVMachineLearningSVM::predict(const CompVMatPtr& vector, int& label, double* distance COMPV_DEFAULT(nullptr))
{
	// Must not lock, function will be called from different threads
	// For example, when called by ultimateText, the MT-decision is per Char

	COMPV_CHECK_EXP_RETURN(!vector || vector->rows() != 1 ||
		(vector->subType() != COMPV_SUBTYPE_RAW_FLOAT64 && vector->subType() != COMPV_SUBTYPE_RAW_FLOAT32),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!m_ptrLibsvmModel, COMPV_ERROR_CODE_E_INVALID_CALL, "No model associated to this object. Are you calling CompVMachineLearningSVM::load to create the SVM object?");
	COMPV_CHECK_EXP_RETURN(distance && !isPredictProbEnabled(), COMPV_ERROR_CODE_E_INVALID_CALL, "Model not trained with probabilities enabled. No way to get distances in this case");

	// Make sure the vector size is correct
	if (vector->cols() != vectorSize()) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid vector size: %zu # %zu", vector->cols(), vectorSize());
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}

	// Create a node containing the vector data
	CompVMatPtr nodeVector, vector64f = vector;
	if (vector->subType() != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float32_t, compv_float64_t>(vector, &vector64f)));
	}
	const bool SIMDFriendly = !!svm_check_SIMDFriendly_model(m_ptrLibsvmModel);
	COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::rawToNode(SIMDFriendly, vector64f, &nodeVector));
	const svm_node_base node(
		SIMDFriendly ? NODE_TYPE_MAT : NODE_TYPE_INDEXED,
		SIMDFriendly ? reinterpret_cast<const void*>(&nodeVector) : nodeVector->ptr<const void>()
	);
	if (distance) {
#if 1
		label = static_cast<int>(svm_predict_distance(m_ptrLibsvmModel, &node, distance));
#else
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Computing probabilities instead of distances");
		const int nr_class = m_ptrLibsvmModel->nr_class;
		double *prob_estimates = reinterpret_cast<double*>(CompVMem::malloc(sizeof(double) * nr_class));
		COMPV_CHECK_EXP_RETURN(!prob_estimates, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		label = static_cast<int>(svm_predict_probability(m_ptrLibsvmModel, &node, prob_estimates));
		for (int i = 0; i < nr_class; i++) {
			if (m_ptrLibsvmModel->label[i] == label) {
				*distance = 1.0 - prob_estimates[i];
				break;
			}
		}
		CompVMem::free(reinterpret_cast<void**>(&prob_estimates));
#endif
	}
	else {
		label = static_cast<int>(svm_predict(m_ptrLibsvmModel, &node));
	}

	return COMPV_ERROR_CODE_S_OK;
}

// trainLabels : (1 x n) - each col represent a label (must be 32s)
// trainVectors: (n x m) - each row reprsent a vector (must be 32f or 64f)
COMPV_ERROR_CODE CompVMachineLearningSVM::train(const CompVMatPtr& trainLabels, const CompVMatPtr& trainVectors, const CompVMachineLearningSVMCrossValidation* crossValidation COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_EXP_RETURN(!trainLabels || !trainVectors || trainLabels->rows() != 1 && trainLabels->cols() != trainVectors->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(trainLabels->subType() != COMPV_SUBTYPE_RAW_INT32, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "trainLabels must be 32s");
	COMPV_CHECK_EXP_RETURN(trainVectors->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && trainVectors->subType() != COMPV_SUBTYPE_RAW_FLOAT64, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "trainVectors must be 32f or 64f");
	COMPV_CHECK_EXP_RETURN(crossValidation && !crossValidation->isValid(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "crossValidation params are incorrect");
	COMPV_AUTOLOCK_THIS(CompVMachineLearningSVM);

	// Convert "trainVectors" to double
	CompVMatPtr vectors;
	if (trainVectors->subType() != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<compv_float32_t, compv_float64_t>(trainVectors, &vectors)));
	}
	else {
		vectors = trainVectors;
	}
	const size_t vectorLength = vectors->cols();
	const int32_t* trainLabelsPtr = trainLabels->ptr<int32_t>();

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	svm_problem problem = { 0 };
	const char* error_msg = nullptr;
	std::function<COMPV_ERROR_CODE(const size_t ystart, const size_t yend)> funcPtr;
	m_vecNodeVectors.resize(vectors->rows(), nullptr); // member variable because model use it as reference pointer

	problem.l = static_cast<int>(vectors->rows());
	problem.y = reinterpret_cast<double*>(CompVMem::malloc(problem.l * sizeof(double)));
	COMPV_CHECK_EXP_BAIL(!problem.y, err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	problem.x = reinterpret_cast<svm_node**>(CompVMem::malloc(problem.l * sizeof(svm_node*)));
	COMPV_CHECK_EXP_BAIL(!problem.x, err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	// Set Problem's values
	funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		for (size_t l = start; l < end; l++) {
			COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::rawToNodeIndexed(vectors->ptr<const double>(l), vectorLength, &m_vecNodeVectors[l]));
			problem.x[l] = m_vecNodeVectors[l]->ptr<svm_node>();
			problem.y[l] = trainLabelsPtr[l];
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
	COMPV_CHECK_CODE_BAIL(err = CompVMachineLearningSVM::trainEx(&problem, m_ptrLibsvmParams, crossValidation, &m_ptrLibsvmModel));
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Training finished!!");
	COMPV_CHECK_EXP_BAIL(!m_ptrLibsvmModel, err = COMPV_ERROR_CODE_E_LIBSVM, "svm_train failed");

bail:
	CompVMem::free(reinterpret_cast<void**>(&problem.y));
	CompVMem::free(reinterpret_cast<void**>(&problem.x));
	return err;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::save(const char* filePath, const bool releaseVectors_ COMPV_DEFAULT(true))
{
	COMPV_AUTOLOCK_THIS(CompVMachineLearningSVM);
	COMPV_CHECK_EXP_RETURN(!m_ptrLibsvmModel, COMPV_ERROR_CODE_E_INVALID_CALL, "Nothing to save, model is null");
	COMPV_CHECK_EXP_RETURN(svm_save_model(filePath, m_ptrLibsvmModel) != 0, COMPV_ERROR_CODE_E_LIBSVM, "svm_save_model failed");
	// Margaret Thatcher: "I want my memory back"
	if (releaseVectors_) {
		m_vecNodeVectors.clear();
		m_vecNodeVectors.shrink_to_fit();
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::trainEx(const struct svm_problem* problem, struct svm_parameter* params, const CompVMachineLearningSVMCrossValidation* crossValidation, struct svm_model** model)
{
	struct CompVMachineLearningSVMCrossValidationRound {
		double accuracy = 0.0;
		double gamma;
		double C;
		CompVMachineLearningSVMCrossValidationRound(const double gamma_, const double C_) :
			gamma(gamma_), C(C_) { }
	};

	// Private function, do not check input parameters
	struct svm_model*& model_ = *model;
	if (model_) {
		svm_free_and_destroy_model(&model_);
	}
	
	if (!crossValidation) {
		model_ = svm_train(problem, params);
		COMPV_CHECK_EXP_RETURN(!model_, COMPV_ERROR_CODE_E_LIBSVM, "svm_train failed");
		return COMPV_ERROR_CODE_S_OK;
	}

	// Create cross-validation grid
	static const size_t kMaxgrid = 10000; // To avoid endless loops
	std::vector<CompVMachineLearningSVMCrossValidationRound> rounds;
	const bool log2c0LessThan1 = (crossValidation->log2c[0] < crossValidation->log2c[1]);
	const bool log2g0LessThan1 = (crossValidation->log2g[0] < crossValidation->log2g[1]);
	for (int log2c = crossValidation->log2c[0]; (log2c0LessThan1 ? log2c <= crossValidation->log2c[1] : log2c >= crossValidation->log2c[1]); log2c += crossValidation->log2c[2]) {
		for (int log2g = crossValidation->log2g[0]; (log2g0LessThan1 ? log2g <= crossValidation->log2g[1] : log2g >= crossValidation->log2g[1]); log2g += crossValidation->log2g[2]) {
			rounds.push_back(CompVMachineLearningSVMCrossValidationRound(std::pow(2.0, log2g), std::pow(2.0, log2c)));
		}
		COMPV_CHECK_EXP_RETURN(rounds.size() >= kMaxgrid, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	}
	const size_t count = rounds.size();
	COMPV_CHECK_EXP_RETURN(!count, COMPV_ERROR_CODE_E_INVALID_CALL);
	
	// Run cross validation on the grid
	unsigned progress = 0;
	const float progress_scale = 100.f / float(count);
	auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		for (size_t i = start; i < end; i++) {
			CompVMachineLearningSVMCrossValidationRound& round = rounds[i];
			svm_parameter mt_params = *params;
			mt_params.C = round.C;
			mt_params.gamma = round.gamma;
			COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::crossValidation(problem, &mt_params, crossValidation->kfold, round.accuracy));
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Cross Validation progress: %f%%", compv_atomic_inc(&progress) * progress_scale);
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? threadDisp->threadsCount() : 1;
	const size_t threadsCount = maxThreadsCount > 1 ? COMPV_MATH_MIN(count, (maxThreadsCount - 1)) : 1; // Do not use all threads -> your PC will freeze ("I neva freeze" <- BLACK PANTHER)
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		count,
		threadsCount,
		threadDisp
	));

	// Sort rounds
	std::sort(rounds.begin(), rounds.end(), [](const CompVMachineLearningSVMCrossValidationRound &a, const CompVMachineLearningSVMCrossValidationRound &b) { 
		return a.accuracy > b.accuracy; 
	});

	// Train now using best params and all data
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Cross Validation Best: C=%g, gamma=%g, accuracy = %g%%", rounds[0].C, rounds[0].gamma, rounds[0].accuracy);
	params->C = rounds[0].C;
	params->gamma = rounds[0].gamma;
	model_ = svm_train(problem, params);
	COMPV_CHECK_EXP_RETURN(!model_, COMPV_ERROR_CODE_E_LIBSVM, "svm_train failed");

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::crossValidation(const struct svm_problem* problem, const struct svm_parameter* params, const int _kfolds, double& accuracy)
{
	// Private function, do not check input parameters

	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = reinterpret_cast<double*>(CompVMem::malloc(sizeof(double) * problem->l));
	COMPV_CHECK_EXP_RETURN(!target, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	svm_cross_validation(problem, params, _kfolds, target);
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
		accuracy = total_error / problem->l;
	}
	else {
		for (int i = 0; i < problem->l; i++) {
			if (target[i] == problem->y[i]) {
				++total_correct;
			}
		}
		accuracy = (100.0* (total_correct / double(problem->l)));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Cross Validation Accuracy (C=%g, gamma=%g) = %g%%", params->C, params->gamma, accuracy);
	}
	CompVMem::free(reinterpret_cast<void**>(&target));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::rawToNode(const bool SIMDFriendly, const CompVMatPtr& rawVector, CompVMatPtrPtr node)
{
	// Private function, do not check parameters
#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0))
	COMPV_ASSERT(rawVector->isRawTypeMatch<double>());
#endif
	if (SIMDFriendly) {
		*node = rawVector;
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::rawToNodeIndexed(rawVector->ptr<const double>(), rawVector->cols(), node));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::rawToNodeIndexed(const double* rawVector, const size_t count, CompVMatPtrPtr node)
{
	// Private function, do not check parameters
	COMPV_CHECK_CODE_RETURN((CompVMat::newObjStrideless<svm_node, COMPV_MAT_TYPE_STRUCT>(node, 1, count + 1)));
	svm_node* xnodePtr = (*node)->ptr<svm_node>();
	for (int i = 0; i < count; ++i) {
		xnodePtr[i].index = (i + 1); // index must start at #1 and finish at #-1
		xnodePtr[i].value = rawVector[i];
	}
	xnodePtr[count].index = -1; // term indication
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
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "SVs size = %zu", m_nVectorSize);
	}

	// Build SIMD-friendly support vectors
#if COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY
	if (mlSVM_->m_ptrLibsvmParams->kernel_type == RBF) {
		COMPV_CHECK_CODE_BAIL(err = svm_makeSVs_SIMD_frienly(model, mlSVM_->m_nVectorSize));
	}
	else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("SIMD implemented for RBF kernel type only");
	}
#else
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("SVM loaded without making the support vectors SIMD-friendly");
#endif /* COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY */

	// Final, set
	*mlSVM = mlSVM_;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		svm_free_and_destroy_model(&model);
	}
	return err;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::rbf(const CompVMatPtr& x, const CompVMatPtr& yy, const size_t count, const double& gamma, CompVMatPtr& kvalues, const struct svm_model *model COMPV_DEFAULT(nullptr))
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Public function used for perf testing");
	svm_simd_func_ptrs simd_func_ptrs;
	if (model) {
		simd_func_ptrs = model->simd_func_ptrs;
	}
	COMPV_CHECK_CODE_RETURN(svm_k_function_rbf(x, yy, count, gamma, kvalues, &simd_func_ptrs));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVM::newObj(CompVMachineLearningSVMPtrPtr mlSVM, const CompVMachineLearningSVMParams& params)
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
	ptrLibsvmParams->cache_size = params.cache_size;
	ptrLibsvmParams->C = params.C;
	ptrLibsvmParams->eps = params.eps;
	ptrLibsvmParams->p = params.P;
	ptrLibsvmParams->shrinking = params.shrinking;
	ptrLibsvmParams->probability = params.probability_estimates;
	ptrLibsvmParams->nr_weight = 0;
	ptrLibsvmParams->weight_label = nullptr;
	ptrLibsvmParams->weight = nullptr;

	*mlSVM = mlSVM_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
