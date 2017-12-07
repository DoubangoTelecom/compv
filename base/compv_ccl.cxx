/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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

COMPV_ERROR_CODE CompVConnectedComponentLabeling::newObj(CompVConnectedComponentLabelingPtrPtr ccl, int id)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!ccl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVConnectedComponentLabelingFactory* factory_ = CompVConnectedComponentLabeling::findFactory(id);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find connected component labeling factory with id = %d", id);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObj) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_CODE_RETURN(factory_->newObj(ccl), "Failed to create connected component object");
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
