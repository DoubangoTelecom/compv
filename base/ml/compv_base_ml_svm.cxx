/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/ml/compv_base_ml_svm.h"

#include "compv/base/ml/libsvm-322/libsvm.h"

// Libsvm guide: https://www.csie.ntu.edu.tw/~cjlin/papers/guide/guide.pdf

COMPV_NAMESPACE_BEGIN()

/* svm_type  */
const int kLibSVM_svm_type_C_SVC = C_SVC;
const int kLibSVM_svm_type_NU_SVC = NU_SVC;
const int kLibSVM_svm_type_ONE_CLASS = ONE_CLASS;
const int kLibSVM_svm_type_EPSILON_SVR = EPSILON_SVR;
const int kLibSVM_svm_type_NU_SVR = NU_SVR;

/* kernel_type */
const int kLibSVM_kernel_type_LINEAR = LINEAR;
const int kLibSVM_kernel_type_POLY = POLY;
const int kLibSVM_kernel_type_RBF = RBF;
const int kLibSVM_kernel_type_SIGMOID = SIGMOID;
const int kLibSVM_kernel_type_PRECOMPUTED = PRECOMPUTED;

CompVMachineLearningSVM::CompVMachineLearningSVM()
	: m_ptrLibsvmParams(nullptr)
{

}

CompVMachineLearningSVM::~CompVMachineLearningSVM()
{
	if (m_ptrLibsvmParams) {
		svm_destroy_param(m_ptrLibsvmParams);
		CompVMem::free(reinterpret_cast<void**>(&m_ptrLibsvmParams));
	}
}

COMPV_ERROR_CODE CompVMachineLearningSVM::newObj(CompVMachineLearningSVMPtrPtr mlSVM, const MachineLearningSVMParams& params)
{
	COMPV_CHECK_EXP_RETURN(!mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVMachineLearningSVMPtr mlSVM_ = new CompVMachineLearningSVM();
	COMPV_CHECK_EXP_RETURN(!mlSVM_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	MachineLearningSVMParams& CompVParams_ = mlSVM_->m_CompVParams;
	struct svm_parameter*& ptrLibsvmParams = mlSVM_->m_ptrLibsvmParams;

	ptrLibsvmParams = reinterpret_cast<struct svm_parameter*>(CompVMem::calloc(1, sizeof(struct svm_parameter)));
	COMPV_CHECK_EXP_RETURN(!ptrLibsvmParams, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	// Default values
	ptrLibsvmParams->svm_type = static_cast<int>(params.svm_type);
	ptrLibsvmParams->kernel_type = static_cast<int>(params.kernel_type);
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

	CompVParams_ = params;

	*mlSVM = mlSVM_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
