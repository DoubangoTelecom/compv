/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include <compv/cl/base/ml/compv_cl_ml_svm_predict.h>

COMPV_NAMESPACE_BEGIN()

CompVMachineLearningSVMPredictBinaryRBF_GPU::CompVMachineLearningSVMPredictBinaryRBF_GPU(const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
	: CompVMachineLearningSVMPredict(CompVMachineLearningSVMPredictTypeBinaryRBF)
{
	// TODO(dmi): do not clone "matSV" and "matCoeff", move to GPU global memory
	// "gamma" and "rho" must be "constant __local"
	// save "labels" to host memory
	// "nr_sv" is useless (required by CPU implemenation but )
}

CompVMachineLearningSVMPredictBinaryRBF_GPU::~CompVMachineLearningSVMPredictBinaryRBF_GPU()
{

}

COMPV_ERROR_CODE CompVMachineLearningSVMPredictBinaryRBF_GPU::process(const CompVMatPtr& matVectors, CompVMatPtrPtr matResult)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningSVMPredictBinaryRBF_GPU::newObj(CompVMachineLearningSVMPredictPtrPtr mlSVM, const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff)
{
	COMPV_CHECK_EXP_RETURN(!mlSVM, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMachineLearningSVMPredictBinaryRBF_GPUPtr mlSVM_;
	mlSVM_ = new CompVMachineLearningSVMPredictBinaryRBF_GPU(gamma, rho, labels, nr_sv, matSV, matCoeff);

	COMPV_CHECK_EXP_RETURN(!mlSVM_, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	*mlSVM = *mlSVM_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
