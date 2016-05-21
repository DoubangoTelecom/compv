/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/matchers/compv_matcher.h"
#include "compv/matchers/compv_matcher_bruteforce.h"
#include "compv/compv_engine.h"

COMPV_NAMESPACE_BEGIN()

std::map<int, const CompVMatcherFactory*> CompVMatcher::s_Factories;

// Declare built-in factories
static const CompVMatcherFactory bruteForceFactory = {
    COMPV_BRUTEFORCE_ID,
    "Brute force matcher",
    CompVMatcherBruteForce::newObj
};

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
    COMPV_DEBUG_INFO("Matchers initialization");

    /* Register built-in matchers */

    // Brute Force
    COMPV_CHECK_CODE_RETURN(addFactory(&bruteForceFactory));
    // FLANN
    // COMPV_CHECK_CODE_RETURN(addFactory(&flannFactory));

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMatcher::addFactory(const CompVMatcherFactory* factory)
{
    COMPV_CHECK_EXP_RETURN(factory == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (s_Factories.find(factory->id) != s_Factories.end()) {
        const CompVMatcherFactory* old = s_Factories.find(factory->id)->second;
        COMPV_DEBUG_WARN("Matcher factory with id = %d already exist and will be replaced old name=%s, new name=%s", factory->id, old->name, factory->name);
    }
    COMPV_DEBUG_INFO("Registering matcher factory with id = %d and name = '%s'...", factory->id, factory->name);
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

COMPV_ERROR_CODE CompVMatcher::newObj(int matcherId, CompVPtr<CompVMatcher* >* matcher)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    COMPV_CHECK_EXP_RETURN(!matcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    const CompVMatcherFactory* factory_ = CompVMatcher::findFactory(matcherId);
    if (!factory_) {
        COMPV_DEBUG_ERROR("Failed to find matcher factory with id = %d", matcherId);
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    if (!factory_->newObj) {
        COMPV_DEBUG_ERROR("Factory with id = %d and name = '%s' doesn't have a constructor", factory_->id, factory_->name);
        return COMPV_ERROR_CODE_E_INVALID_CALL;
    }
    return factory_->newObj(matcher);
}

COMPV_NAMESPACE_END()
