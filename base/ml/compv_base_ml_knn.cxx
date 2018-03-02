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
		: m_nVectorLength(vectorLength)
		, m_eVectorType(vectorType)
		, m_ptrAnnoy(nullptr)
		, m_eDistanceType(distanceType)
		, m_bLoaded(false)
		, m_bBuilt(false)
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

	virtual bool isValid() const override {
		return (m_ptrAnnoy != nullptr);
	}
	virtual bool isLoaded() const override {
		return m_bLoaded;
	}
	virtual bool isBuilt() const override {
		return m_bBuilt;
	}

	// Not thread-safe
	virtual COMPV_ERROR_CODE addVector(const CompVMatPtr& vector, const size_t name) override
	{
		COMPV_CHECK_EXP_RETURN(!vector || !vector->isRawTypeMatch<T>() || vector->rows() != 1 || vector->cols() != vectorLength() || m_vecNames.size() >= INT32_MAX, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_ptrAnnoy->add_item(m_vecNames.size(), vector->ptr<const T>());
		m_vecNames.push_back(name);
		return COMPV_ERROR_CODE_S_OK;
	}

	// Not thread-safe
	virtual COMPV_ERROR_CODE build(const int n_trees) override
	{
		if (!isBuilt()) {
			m_ptrAnnoy->build(n_trees);
			m_bBuilt = true;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// Not thread-safe
	virtual COMPV_ERROR_CODE save(const char* path) override
	{
		if (!m_ptrAnnoy->save(path)) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to save to %s", path);
			return COMPV_ERROR_CODE_E_FAILED_TO_OPEN_FILE;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// Not thread-safe
	virtual COMPV_ERROR_CODE load(const char* path) override
	{
		if (!isLoaded()) {
			if (!m_ptrAnnoy->load(path)) {
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to load from %s", path);
				return COMPV_ERROR_CODE_E_FAILED_TO_OPEN_FILE;
			}
			m_bLoaded = true;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	virtual COMPV_ERROR_CODE search(const CompVMatPtr& vector, std::vector<size_t>& result, const size_t k, std::vector<double>* distances) override
	{
		std::vector<T> distances_;
		COMPV_CHECK_EXP_RETURN(!vector || !vector->isRawTypeMatch<T>() || vector->rows() != 1 || vector->cols() != vectorLength(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		result.clear();
		m_ptrAnnoy->get_nns_by_vector(vector->ptr<const T>(), k, (size_t)-1, &result, (distances ? &distances_ : nullptr));
		if (distances) {
			if (std::is_same<T, double>::value) {
				*distances = *reinterpret_cast<std::vector<double>*>(&distances_);
			}
			else {
				const size_t count = distances_.size();
				*distances = std::vector<double>(count);
				for (size_t i = 0; i < count; ++i) {
					(*distances)[i] = static_cast<double>(distances_[i]);
				}
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	size_t m_nVectorLength;
	COMPV_SUBTYPE m_eVectorType;
	COMPV_DISTANCE_TYPE m_eDistanceType;
	bool m_bLoaded;
	bool m_bBuilt;
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

// Function called in training phase.
COMPV_ERROR_CODE CompVMachineLearningKNN::addVector(const CompVMatPtr& vector, const size_t name)
{
	CompVAutoLock<CompVMachineLearningKNN>(this); // Adding vector not thread-safe -> lock
	COMPV_CHECK_EXP_RETURN(m_ptrGeneric->isBuilt(), COMPV_ERROR_CODE_E_INVALID_CALL, "No vector could be added after building the tree");
	COMPV_CHECK_CODE_RETURN(m_ptrGeneric->addVector(vector, name));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningKNN::save(const char* path, const int n_trees COMPV_DEFAULT(10))
{
	CompVAutoLock<CompVMachineLearningKNN>(this);
	if (!m_ptrGeneric->isBuilt()) {
		COMPV_CHECK_CODE_RETURN(m_ptrGeneric->build(n_trees));
	}
	COMPV_CHECK_CODE_RETURN(m_ptrGeneric->save(path));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMachineLearningKNN::load(const char* path)
{
	CompVAutoLock<CompVMachineLearningKNN>(this);
	COMPV_CHECK_CODE_RETURN(m_ptrGeneric->load(path));
	return COMPV_ERROR_CODE_S_OK;
}

// Thread-safe
COMPV_ERROR_CODE CompVMachineLearningKNN::search(const CompVMatPtr& vector, std::vector<size_t>& result, const size_t k COMPV_DEFAULT(1), std::vector<double>* distances COMPV_DEFAULT(nullptr))
{
	// "MUST NOT LOCK" -> called by many threads
	COMPV_CHECK_EXP_RETURN(!m_ptrGeneric->isLoaded(), COMPV_ERROR_CODE_E_INVALID_CALL, "Must load first");
	COMPV_CHECK_CODE_RETURN(m_ptrGeneric->search(vector, result, k, distances));
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
