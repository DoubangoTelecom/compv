/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MACHINE_LEARNING_SVM_PREDICT_H_)
#define _COMPV_BASE_MACHINE_LEARNING_SVM_PREDICT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_lock.h"


COMPV_NAMESPACE_BEGIN()

enum CompVMachineLearningSVMPredictType {
	CompVMachineLearningSVMPredictTypeBinaryRBF // type=C_SVC, kernel_type=RBF
};

COMPV_OBJECT_DECLARE_PTRS(MachineLearningSVMPredict)

typedef COMPV_ERROR_CODE(*newObjBinaryRBFMachineLearningSVMPredict)(CompVMachineLearningSVMPredictPtrPtr mlSVM, const compv_float64_t& gamma, const compv_float64_t& rho, const int32_t(&labels)[2], const int32_t(&nr_sv)[2], CompVMatPtr& matSV, CompVMatPtr& matCoeff);

//
//	CompVMachineLearningSVMPredict
//
class COMPV_BASE_API CompVMachineLearningSVMPredict : public CompVObj, public CompVLock {
protected:
	CompVMachineLearningSVMPredict(CompVMachineLearningSVMPredictType eType);
public:
	virtual ~CompVMachineLearningSVMPredict();
	COMPV_OBJECT_GET_ID(CompVMachineLearningSVMPredict);
	COMPV_INLINE CompVMachineLearningSVMPredictType type() const {
		return m_eType;
	}
	
	virtual bool isGPUAccelerated() const = 0;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& matVectors, CompVMatPtrPtr matResult) = 0;

	static COMPV_ERROR_CODE newObjBinaryRBF(CompVMachineLearningSVMPredictPtrPtr mlSVM, const char* pathToFlatModel, bool gpuActiveAndEnabled = true);

public:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static newObjBinaryRBFMachineLearningSVMPredict s_ptrNewObjBinaryRBF_CPU;
	static newObjBinaryRBFMachineLearningSVMPredict s_ptrNewObjBinaryRBF_GPU;
	COMPV_VS_DISABLE_WARNINGS_END()

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVMachineLearningSVMPredictType m_eType;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MACHINE_LEARNING_SVM_PREDICT_H_ */
