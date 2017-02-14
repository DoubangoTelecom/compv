/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_matchers.h"
#include "compv/base/compv_base.h"

#define COMPV_THIS_CLASSNAME	"CompVMatcher"

COMPV_NAMESPACE_BEGIN()

std::map<int, const CompVMatcherFactory*> CompVMatcher::s_Factories;

//
//	CompVMatcher
//

CompVMatcher::CompVMatcher()
{

}

CompVMatcher::~CompVMatcher()
{

}

COMPV_ERROR_CODE CompVMatcher::init()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Matchers initialization");

	/* Register built-in matchers */

	// Brute Force
	// COMPV_CHECK_CODE_RETURN(addFactory(&bruteForceFactory));
	// FLANN
	// COMPV_CHECK_CODE_RETURN(addFactory(&flannFactory));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMatcher::addFactory(const CompVMatcherFactory* factory)
{
	COMPV_CHECK_EXP_RETURN(!factory, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (s_Factories.find(factory->id) != s_Factories.end()) {
		const CompVMatcherFactory* old = s_Factories.find(factory->id)->second;
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Matcher factory with id = %d already exist and will be replaced old name=%s, new name=%s", factory->id, old->name, factory->name);
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Registering matcher factory with id = %d and name = '%s'...", factory->id, factory->name);
	s_Factories[factory->id] = factory;
	return COMPV_ERROR_CODE_S_OK;
}

const CompVMatcherFactory* CompVMatcher::findFactory(int deteId)
{
	std::map<int, const CompVMatcherFactory*>::const_iterator it = s_Factories.find(deteId);
	if (it == s_Factories.end()) {
		return NULL;
	}
	return it->second;
}

COMPV_ERROR_CODE CompVMatcher::newObj(CompVMatcherPtrPtr matcher, int matcherId)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!matcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVMatcherFactory* factory_ = CompVMatcher::findFactory(matcherId);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find matcher factory with id = %d", matcherId);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObjMatcher) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	return factory_->newObjMatcher(matcher);
}

COMPV_NAMESPACE_END()
