/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CL_MACHINE_LEARNING_SVM_PREDICT_H_)
#define _COMPV_CL_MACHINE_LEARNING_SVM_PREDICT_H_

#include "compv/cl/compv_cl_config.h"
#include "compv/base/ml/compv_base_ml_svm_predict.h"

#include <CL/opencl.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(MachineLearningSVMPredictBinaryRBF_GPU);

class CompVMachineLearningSVMPredictBinaryRBF_GPU : public CompVMachineLearningSVMPredict
{
protected:
	CompVMachineLearningSVMPredictBinaryRBF_GPU(const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff);
public:
	virtual ~CompVMachineLearningSVMPredictBinaryRBF_GPU();
	COMPV_OBJECT_GET_ID(CompVMachineLearningSVMPredictBinaryRBF_GPU);

	COMPV_INLINE bool isValid() const {
		return m_bValid;
	}
	COMPV_INLINE bool isInitialized() const {
		return m_bInitialized;
	}

	virtual bool isGPUAccelerated() const override { return true; }

	virtual COMPV_ERROR_CODE process(const CompVMatPtr& matVectors, CompVMatPtrPtr matResult) override;

	static COMPV_ERROR_CODE newObj(CompVMachineLearningSVMPredictPtrPtr mlSVM, const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], const CompVMatPtr& matSV, const CompVMatPtr& matCoeff);

private:
	COMPV_ERROR_CODE init(const CompVMatPtr& matSV, const CompVMatPtr& matCoeff);
	COMPV_ERROR_CODE deInit();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bValid;
	bool m_bInitialized;
	cl_int m_nMatSVs_cols;
	cl_int m_nMatSVs_rows;
	compv_float64_t m_64fGammaMinus;
	compv_float64_t m_64fRho;
	cl_context m_clContext;
	cl_command_queue m_clCommandQueue;
	cl_mem m_clMemMatSV;
	cl_mem m_clMemMatCoeff;
	cl_program m_clProgram;
	cl_kernel m_clkernelPart1;
	cl_kernel m_clkernelPart2;
	int32_t m_labels[2];
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CL_MACHINE_LEARNING_SVM_PREDICT_H_ */
