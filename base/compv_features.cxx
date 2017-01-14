/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_features.h"
#include "compv/base/compv_base.h"

#define COMPV_THIS_CLASSNAME "CompVFeature"

COMPV_NAMESPACE_BEGIN()

std::map<int, const CompVFeatureFactory*> CompVFeature::s_Factories;

//
//	CompVFeature
//

CompVFeature::CompVFeature()
{

}

CompVFeature::~CompVFeature()
{

}

COMPV_ERROR_CODE CompVFeature::addFactory(const CompVFeatureFactory* factory)
{
	COMPV_CHECK_EXP_RETURN(factory == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (s_Factories.find(factory->id) != s_Factories.end()) {
		const CompVFeatureFactory* old = s_Factories.find(factory->id)->second;
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Feature factory with id = %d already exist and will be replaced old name=%s, new name=%s", factory->id, old->name, factory->name);
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Registering feature factory with id = %d and name = '%s'...", factory->id, factory->name);
	s_Factories[factory->id] = factory;
	return COMPV_ERROR_CODE_S_OK;
}

const CompVFeatureFactory* CompVFeature::findFactory(int deteId)
{
	std::map<int, const CompVFeatureFactory*>::const_iterator it = s_Factories.find(deteId);
	if (it == s_Factories.end()) {
		return NULL;
	}
	return it->second;
}

// Class: CompVFeatureBase

CompVFeatureBase::CompVFeatureBase(int id)
	: m_nId(id)
{

}

CompVFeatureBase::~CompVFeatureBase()
{

}


//
//	CompVCornerDete
//

CompVCornerDete::CompVCornerDete(int id)
	: CompVFeatureBase(id)
{

}

CompVCornerDete::~CompVCornerDete()
{

}

COMPV_ERROR_CODE CompVCornerDete::newObj(CompVCornerDetePtrPtr dete, int deteId)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!dete, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVFeatureFactory* factory_ = CompVFeature::findFactory(deteId);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find feature factory with id = %d", deteId);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObjCornerDete) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor for detectors", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_CODE_RETURN(factory_->newObjCornerDete(dete), "Failed to create corner detector");
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVCornerDesc
//

CompVCornerDesc::CompVCornerDesc(int id)
	: CompVFeatureBase(id)
{

}

CompVCornerDesc::~CompVCornerDesc()
{

}

COMPV_ERROR_CODE CompVCornerDesc::newObj(CompVCornerDescPtrPtr desc, int descId)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!desc, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVFeatureFactory* factory_ = CompVFeature::findFactory(descId);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find feature factory with id = %d", descId);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObjCornerDesc) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor for descriptors", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_CODE_RETURN(factory_->newObjCornerDesc(desc), "Failed to create corner descriptor");
	return COMPV_ERROR_CODE_S_OK;
}

//
// Class: CompVEdgeDete
//

CompVEdgeDete::CompVEdgeDete(int id)
	: CompVFeatureBase(id)
{

}


CompVEdgeDete::~CompVEdgeDete()
{

}

COMPV_ERROR_CODE CompVEdgeDete::newObj(CompVEdgeDetePtrPtr dete, int deteId, float tLow COMPV_DEFAULT(0.68f), float tHigh COMPV_DEFAULT(0.68f*2.f), int32_t kernSize COMPV_DEFAULT(3))
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!dete, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVFeatureFactory* factory_ = CompVFeature::findFactory(deteId);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find feature factory with id = %d", deteId);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObjEdgeDete) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor for edge detector", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_CODE_RETURN(factory_->newObjEdgeDete(dete, tLow, tHigh, kernSize), "Failed to create edge detector");
	return COMPV_ERROR_CODE_S_OK;
}

//
// Class: CompVHough
//

CompVHough::CompVHough(int id)
	: CompVFeatureBase(id)
{
}

CompVHough::~CompVHough()
{
}

COMPV_ERROR_CODE CompVHough::newObj(CompVHoughPtrPtr hough, int id, float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), int32_t threshold COMPV_DEFAULT(1))
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!hough, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVFeatureFactory* factory_ = CompVFeature::findFactory(id);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find feature factory with id = %d", id);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObjHough) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor for hough ctor", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_CODE_RETURN(factory_->newObjHough(hough, rho, theta, threshold), "Failed to create hough transform context");
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
