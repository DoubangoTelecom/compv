/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_ccl.h"
#include "compv/base/compv_base.h"

#define COMPV_THIS_CLASSNAME "CompVConnectedComponentLabeling"

COMPV_NAMESPACE_BEGIN()

std::map<int, const CompVConnectedComponentLabelingFactory*> CompVConnectedComponentLabeling::s_Factories;

CompVConnectedComponentLabeling::CompVConnectedComponentLabeling(int id)
	: m_nId(id)
{
}

CompVConnectedComponentLabeling::~CompVConnectedComponentLabeling()
{

}

COMPV_ERROR_CODE CompVConnectedComponentLabeling::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_CCL_SET_INT_CONNECTIVITY: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int connectivity = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_EXP_RETURN(connectivity != 4 && connectivity != 8, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Connectivity must be equal to 4 or 8");
		m_nConnectivity = connectivity;
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

COMPV_ERROR_CODE CompVConnectedComponentLabeling::addFactory(const CompVConnectedComponentLabelingFactory* factory)
{
	COMPV_CHECK_EXP_RETURN(!factory, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (s_Factories.find(factory->id) != s_Factories.end()) {
		const CompVConnectedComponentLabelingFactory* old = s_Factories.find(factory->id)->second;
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Connected component labeling factory with id = %d already exist and will be replaced old name=%s, new name=%s", factory->id, old->name, factory->name);
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Registering connected component labeling factory with id = %d and name = '%s'...", factory->id, factory->name);
	s_Factories[factory->id] = factory;
	return COMPV_ERROR_CODE_S_OK;
}

const CompVConnectedComponentLabelingFactory* CompVConnectedComponentLabeling::findFactory(int id)
{
	std::map<int, const CompVConnectedComponentLabelingFactory*>::const_iterator it = s_Factories.find(id);
	if (it == s_Factories.end()) {
		return nullptr;
	}
	return it->second;
}

COMPV_ERROR_CODE CompVConnectedComponentLabeling::newObj(CompVConnectedComponentLabelingPtrPtr ccl, int id,
	int delta COMPV_DEFAULT(kCompVConnectedComponentDeltaDefault),
	double min_area COMPV_DEFAULT(kCompVConnectedComponentMinAreaDefault), double max_area COMPV_DEFAULT(kCompVConnectedComponentMaxAreaDefault),
	double max_variation COMPV_DEFAULT(kCompVConnectedComponentMaxVariationDefault),
	double min_diversity COMPV_DEFAULT(kCompVConnectedComponentMinDiversityDefault),
	int connectivity COMPV_DEFAULT(kCompVConnectedComponentConnectivity))
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(!ccl || 
		delta <= 0 || delta > 255 ||
		min_area < 0.0 || min_area > 1.0 || min_area > max_area ||
		max_area < 0.0 || max_area > 1.0 || max_area < min_area ||
		max_variation < 0.0 || max_variation > 1.0 ||
		min_diversity < 0.0 || min_diversity > 1.0 ||
		(connectivity != 4 && connectivity != 8)
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVConnectedComponentLabelingFactory* factory_ = CompVConnectedComponentLabeling::findFactory(id);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find connected component labeling factory with id = %d", id);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObj) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	CompVConnectedComponentLabelingPtr ccl_;
	COMPV_CHECK_CODE_RETURN(factory_->newObj(&ccl_), "Failed to create connected component object");
	ccl_->m_nDelta = delta;
	ccl_->m_64fMinArea = min_area;
	ccl_->m_64fMaxArea = max_area;
	ccl_->m_64fMaxVariation = max_variation;
	ccl_->m_64fMinDiversity = min_diversity;
	ccl_->m_nConnectivity = connectivity;
	*ccl = ccl_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
