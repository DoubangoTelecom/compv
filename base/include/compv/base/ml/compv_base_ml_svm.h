/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MACHINE_LEARNING_SVM_H_)
#define _COMPV_BASE_MACHINE_LEARNING_SVM_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_lock.h"

#define COMPV_LIBSVM_KFOLDS_DEFAULT		5 // For cross-validation, use "4/5" of the train data for training and "1/5" for testing (a.k.a prediction)

struct svm_parameter;
struct svm_model;
struct svm_problem;

COMPV_NAMESPACE_BEGIN()

/* svm_type  */
COMPV_BASE_API extern const int kLibSVM_svm_type_C_SVC;
COMPV_BASE_API extern const int kLibSVM_svm_type_NU_SVC;
COMPV_BASE_API extern const int kLibSVM_svm_type_ONE_CLASS;
COMPV_BASE_API extern const int kLibSVM_svm_type_EPSILON_SVR;
COMPV_BASE_API extern const int kLibSVM_svm_type_NU_SVR;

/* kernel_type */
COMPV_BASE_API extern const int kLibSVM_kernel_type_LINEAR;
COMPV_BASE_API extern const int kLibSVM_kernel_type_POLY;
COMPV_BASE_API extern const int kLibSVM_kernel_type_RBF;
COMPV_BASE_API extern const int kLibSVM_kernel_type_SIGMOID;
COMPV_BASE_API extern const int kLibSVM_kernel_type_PRECOMPUTED;

#define COMPV_SVM_TYPE_C_SVC				kLibSVM_svm_type_C_SVC // multi-class classification
#define COMPV_SVM_TYPE_NU_SVC				kLibSVM_svm_type_NU_SVC // multi-class classification
#define COMPV_SVM_TYPE_ONE_CLASS			kLibSVM_svm_type_ONE_CLASS // one-class SVM
#define COMPV_SVM_TYPE_EPSILON_SVR			kLibSVM_svm_type_EPSILON_SVR // epsilon-SVR	(regression)
#define COMPV_SVM_TYPE_NU_SVR				kLibSVM_svm_type_NU_SVR // nu-SVR		(regression)

#define COMPV_SVM_KERNEL_TYPE_LINEAR		kLibSVM_kernel_type_LINEAR // linear: u'*v
#define COMPV_SVM_KERNEL_TYPE_POLY			kLibSVM_kernel_type_POLY // polynomial: (gamma*u'*v + coef0)^degree
#define COMPV_SVM_KERNEL_TYPE_RBF			kLibSVM_kernel_type_RBF // radial basis function: exp(-gamma*|u-v|^2)
#define COMPV_SVM_KERNEL_TYPE_SIGMOID		kLibSVM_kernel_type_SIGMOID // sigmoid: tanh(gamma*u'*v + coef0)
#define COMPV_SVM_KERNEL_TYPE_PRECOMPUTED	kLibSVM_kernel_type_PRECOMPUTED // precomputed kernel (kernel values in training_set_file)

struct CompVMachineLearningSVMParams {
	int svm_type = COMPV_SVM_TYPE_C_SVC;
	int kernel_type = COMPV_SVM_KERNEL_TYPE_RBF;
	int degree = 3; // set degree in kernel function (default 3)
	double gamma = 0.0; // set gamma in kernel function (default 1/num_features)
	double coef0 = 0.0; // set coef0 in kernel function (default 0)
	double C = 1.0; // cost, set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)
	double nu = 0.5; // set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)
	double eps = 1e-3; // epsilon, set tolerance of termination criterion (default 0.001)
	double P = 0.1; // set the epsilon in loss function of epsilon-SVR (default 0.1)
	int probability_estimates = 0; // whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)
};

struct CompVMachineLearningSVMCrossValidation {
	int kfold = COMPV_LIBSVM_KFOLDS_DEFAULT; // use "(k-1)/k" for training and "1/k" for testing. For k=5(default), use "4/5" for training and "1/5" for testing
	int log2c[3] = { -5,  15,  2 }; // C: begin, end, step. See "https://www.csie.ntu.edu.tw/~cjlin/papers/guide/guide.pdf" section 3.2 for range values.
	int log2g[3] = { 4, -15, -2 }; // Gamma: begin, end, step. See "https://www.csie.ntu.edu.tw/~cjlin/papers/guide/guide.pdf" section 3.2 for range values.

	COMPV_INLINE bool isValid() const {
		return kfold >= 2 &&
			(log2c[2] > 0 ? log2c[0] < log2c[1] : log2c[0] > log2c[1]) &&
			(log2g[2] > 0 ? log2g[0] < log2g[1] : log2g[0] > log2g[1]);
	}
};

COMPV_OBJECT_DECLARE_PTRS(MachineLearningSVM)

class COMPV_BASE_API CompVMachineLearningSVM : public CompVObj, public CompVLock {
protected:
	CompVMachineLearningSVM();
public:
	virtual ~CompVMachineLearningSVM();
	COMPV_OBJECT_GET_ID(CompVMachineLearningSVM);

	COMPV_INLINE bool isPredictProbEnabled()const {
		return m_bPredictProbEnabled;
	}
	COMPV_INLINE size_t vectorSize()const {
		return m_nVectorSize;
	}

	COMPV_ERROR_CODE predict(const CompVMatPtr& vector, int& label, double* distance = nullptr);
	COMPV_ERROR_CODE addVector(const int label, const CompVMatPtr& vector);
	COMPV_ERROR_CODE train(const std::vector<int>& trainLabels, const CompVMatPtrVector& trainVectors, const CompVMachineLearningSVMCrossValidation* crossValidation = nullptr);
	COMPV_ERROR_CODE train(const CompVMachineLearningSVMCrossValidation* crossValidation = nullptr);
	COMPV_ERROR_CODE save(const char* filePath, const bool releaseVectors_ = true);

	static COMPV_ERROR_CODE load(const char* filePath, CompVMachineLearningSVMPtrPtr mlSVM);
	static COMPV_ERROR_CODE newObj(CompVMachineLearningSVMPtrPtr mlSVM, const CompVMachineLearningSVMParams& params);

private:
	static COMPV_ERROR_CODE trainEx(const struct svm_problem* problem, struct svm_parameter* params, const CompVMachineLearningSVMCrossValidation* crossValidation, struct svm_model** model);
	static COMPV_ERROR_CODE crossValidation(const struct svm_problem* problem, const struct svm_parameter* params, const int _kfolds, double& accuracy);
	static COMPV_ERROR_CODE rawToNode(const CompVMatPtr& rawVector, CompVMatPtrPtr nodeVector);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	struct svm_parameter* m_ptrLibsvmParams;
	struct svm_model* m_ptrLibsvmModel;
	bool m_bPredictProbEnabled;
	size_t m_nVectorSize;
	CompVMatPtrVector m_vecTrainVectors;
	std::vector<int> m_vecTrainLabels;
	CompVMatPtrVector m_vecNodeVectors;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MACHINE_LEARNING_SVM_H_ */
