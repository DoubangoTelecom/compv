/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/ml/compv_base_ml_knn.h"
#include "compv/base/compv_debug.h"

#include "compv/base/ml/annoy-1.11.4/annoylib.h"
#include "compv/base/ml/annoy-1.11.4/kissrandom.h"

#define COMPV_THIS_CLASSNAME	"CompVMachineLearningKNN"

COMPV_NAMESPACE_BEGIN()

typedef AnnoyIndexInterface<size_t, compv_float64_t> CompVAnnoyIndexInterface64f;
typedef AnnoyIndexInterface<size_t, compv_float32_t> CompVAnnoyIndexInterface32f;

typedef AnnoyIndex<size_t, compv_float64_t, Euclidean, Kiss32Random> CompVAnnoyIndexEuclidian64f;
typedef AnnoyIndex<size_t, compv_float64_t, Angular, Kiss32Random> CompVAnnoyIndexAngular64f;
typedef AnnoyIndex<size_t, compv_float64_t, Manhattan, Kiss32Random> CompVAnnoyIndexManhattan64f;
typedef AnnoyIndex<size_t, compv_float64_t, Hamming, Kiss32Random> CompVAnnoyIndexHamming64f;

typedef AnnoyIndex<size_t, compv_float32_t, Euclidean, Kiss32Random> CompVAnnoyIndexEuclidian32f;
typedef AnnoyIndex<size_t, compv_float32_t, Angular, Kiss32Random> CompVAnnoyIndexAngular32f;
typedef AnnoyIndex<size_t, compv_float32_t, Manhattan, Kiss32Random> CompVAnnoyIndexManhattan32f;
typedef AnnoyIndex<size_t, compv_float32_t, Hamming, Kiss32Random> CompVAnnoyIndexHamming32f;

//
//	CompVMachineLearningKNNGeneric
//

template<class T>
class CompVMachineLearningKNNGeneric : public CompVMachineLearningKNNGenericInterface {
public:
	CompVMachineLearningKNNGeneric(const size_t vectorLength, const COMPV_SUBTYPE vectorType = COMPV_SUBTYPE_RAW_FLOAT32, const COMPV_DISTANCE_TYPE distanceType = COMPV_DISTANCE_TYPE_EUCLIDEAN)
		: m_ptrAnnoy(nullptr)
	{
		COMPV_ASSERT((vectorType == COMPV_SUBTYPE_RAW_FLOAT32 && std::is_same<T, compv_float32_t>::value) || (vectorType == COMPV_SUBTYPE_RAW_FLOAT64 && std::is_same<T, compv_float64_t>::value));
		switch (distanceType) {
			case COMPV_DISTANCE_TYPE_ANGULAR: {
				m_ptrAnnoy = new AnnoyIndex<size_t, T, Angular, Kiss32Random>(static_cast<int>(vectorLength));
				break;
			}
			case COMPV_DISTANCE_TYPE_MANHATTAN: {
				m_ptrAnnoy = new AnnoyIndex<size_t, T, Manhattan, Kiss32Random>(static_cast<int>(vectorLength));
				break;
			}
			default:
			case COMPV_DISTANCE_TYPE_EUCLIDEAN: {
				m_ptrAnnoy = new AnnoyIndex<size_t, T, Euclidean, Kiss32Random>(static_cast<int>(vectorLength));
				break;
			}
		}
	}

	virtual ~CompVMachineLearningKNNGeneric()
	{
		if (m_ptrAnnoy) {
			delete[] m_ptrAnnoy;
			m_ptrAnnoy = nullptr;
		}
	}

	COMPV_INLINE size_t vectorLength() const {
		return m_nVectorLength;
	}
	COMPV_INLINE COMPV_SUBTYPE vectorType() const {
		return m_eVectorType;
	}
	COMPV_INLINE COMPV_DISTANCE_TYPE distanceType() const {
		return m_eDistanceType;
	}
	COMPV_INLINE bool isLoaded()const {
		return m_bLoaded;
	}

	virtual bool isValid() const override {
		return (m_ptrAnnoy != nullptr);
	}

	virtual COMPV_ERROR_CODE addVector(const CompVMatPtr& vector, const size_t name) override
	{
		COMPV_CHECK_EXP_RETURN(!vector || !vector->isRawTypeMatch<T>() || vector->rows() != 1 || vector->cols() != vectorLength(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_ptrAnnoy->add_item(m_vecNames.size(), vector->ptr<const T>());
		m_vecNames.push_back(name);
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	size_t m_nVectorLength;
	COMPV_SUBTYPE m_eVectorType;
	COMPV_DISTANCE_TYPE m_eDistanceType;
	bool m_bLoaded;
	std::vector<size_t> m_vecNames;
	AnnoyIndexInterface<size_t, T>* m_ptrAnnoy;
}; 

//
//	CompVMachineLearningKNN
//

CompVMachineLearningKNN::CompVMachineLearningKNN(const size_t vectorLength, const COMPV_SUBTYPE vectorType COMPV_DEFAULT(COMPV_SUBTYPE_RAW_FLOAT32), const COMPV_DISTANCE_TYPE distanceType COMPV_DEFAULT(COMPV_DISTANCE_TYPE_EUCLIDEAN))
	: m_ptrGeneric(nullptr)
{
	switch (vectorType) {
	case COMPV_SUBTYPE_RAW_FLOAT64:
		m_ptrGeneric = new CompVMachineLearningKNNGeneric<compv_float64_t>(vectorLength, vectorType, distanceType);
		break;
	default:
	case COMPV_SUBTYPE_RAW_FLOAT32:
		m_ptrGeneric = new CompVMachineLearningKNNGeneric<compv_float32_t>(vectorLength, vectorType, distanceType);
		break;
	}
	if (m_ptrGeneric && !m_ptrGeneric->isValid()) {
		delete[] m_ptrGeneric;
		m_ptrGeneric = nullptr;
	}
}

CompVMachineLearningKNN::~CompVMachineLearningKNN()
{
	if (m_ptrGeneric) {
		delete[] m_ptrGeneric;
		m_ptrGeneric = nullptr;
	}
}

// Function called in learning phase.
// This function isn't thread-safe.
COMPV_ERROR_CODE CompVMachineLearningKNN::addVector(const CompVMatPtr& vector, const size_t name)
{
	COMPV_CHECK_CODE_RETURN(m_ptrGeneric->addVector(vector, name));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningKNN::newObj(CompVMachineLearningKNNPtrPtr knn, const size_t vectorLength, const COMPV_SUBTYPE vectorType COMPV_DEFAULT(COMPV_SUBTYPE_RAW_FLOAT32), const COMPV_DISTANCE_TYPE distanceType COMPV_DEFAULT(COMPV_DISTANCE_TYPE_EUCLIDEAN))
{
	COMPV_CHECK_EXP_RETURN(!knn || !vectorLength 
		|| (vectorType != COMPV_SUBTYPE_RAW_FLOAT32 && vectorType != COMPV_SUBTYPE_RAW_FLOAT64)
		|| (distanceType != COMPV_DISTANCE_TYPE_EUCLIDEAN && distanceType != COMPV_DISTANCE_TYPE_ANGULAR && distanceType != COMPV_DISTANCE_TYPE_MANHATTAN)
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMachineLearningKNNPtr knn_ = new CompVMachineLearningKNN(vectorLength, vectorType, distanceType);
	COMPV_CHECK_EXP_RETURN(!knn_ || !knn_->m_ptrGeneric, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*knn = knn_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
