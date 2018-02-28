/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MACHINE_LEARNING_KNN_H_)
#define _COMPV_BASE_MACHINE_LEARNING_KNN_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(MachineLearningKNN)

class CompVMachineLearningKNNGenericInterface {
public:
	virtual ~CompVMachineLearningKNNGenericInterface() {  }
	virtual bool isValid() const = 0;
	virtual COMPV_ERROR_CODE addVector(const CompVMatPtr& vector, const size_t name) = 0;
};

class CompVMachineLearningKNN : public CompVObj {
protected:
	CompVMachineLearningKNN(const size_t vectorLength, const COMPV_SUBTYPE vectorType = COMPV_SUBTYPE_RAW_FLOAT32, const COMPV_DISTANCE_TYPE distanceType = COMPV_DISTANCE_TYPE_EUCLIDEAN);
public:
	virtual ~CompVMachineLearningKNN();
	COMPV_OBJECT_GET_ID(CompVMachineLearningKNN);

	bool isValid() const;

	COMPV_ERROR_CODE addVector(const CompVMatPtr& vector, const size_t name);

	static COMPV_ERROR_CODE newObj(CompVMachineLearningKNNPtrPtr knn, const size_t vectorLength, const COMPV_SUBTYPE vectorType = COMPV_SUBTYPE_RAW_FLOAT32, const COMPV_DISTANCE_TYPE distanceType = COMPV_DISTANCE_TYPE_EUCLIDEAN);

private:
	CompVMachineLearningKNNGenericInterface* m_ptrGeneric;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MACHINE_LEARNING_KNN_H_ */
