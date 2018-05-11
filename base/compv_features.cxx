/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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
	COMPV_CHECK_EXP_RETURN(!factory, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (s_Factories.find(factory->id) != s_Factories.end()) {
		const CompVFeatureFactory* old = s_Factories.find(factory->id)->second;
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Feature factory with id = %d already exist and will be replaced old name=%s, new name=%s", factory->id, old->name, factory->name);
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Registering feature factory with id = %d and name = '%s'...", factory->id, factory->name);
	s_Factories[factory->id] = factory;
	return COMPV_ERROR_CODE_S_OK;
}

const CompVFeatureFactory* CompVFeature::findFactory(int deteId)
{
	std::map<int, const CompVFeatureFactory*>::const_iterator it = s_Factories.find(deteId);
	if (it == s_Factories.end()) {
		return nullptr;
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
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
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

COMPV_ERROR_CODE CompVCornerDesc::newObj(CompVCornerDescPtrPtr desc, int descId, CompVCornerDetePtr dete COMPV_DEFAULT(NULL))
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
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
	COMPV_CHECK_CODE_RETURN((*desc)->attachDete(dete));
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

COMPV_ERROR_CODE CompVEdgeDete::newObj(CompVEdgeDetePtrPtr dete, int deteId, float tLow COMPV_DEFAULT(COMPV_FEATURE_DETE_EDGE_THRESHOLD_LOW), float tHigh COMPV_DEFAULT(COMPV_FEATURE_DETE_EDGE_THRESHOLD_HIGH), size_t kernSize COMPV_DEFAULT(3))
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
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

COMPV_ERROR_CODE CompVHough::newObj(CompVHoughPtrPtr hough, int id, float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(1.f), size_t threshold COMPV_DEFAULT(1))
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED, "Not initialized");
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



//
// Class: CompVHOG
//

CompVHOG::CompVHOG(int id)
	: CompVFeatureBase(id)
{

}

CompVHOG::~CompVHOG()
{

}

COMPV_ERROR_CODE CompVHOG::newObj(
	CompVHOGPtrPtr hog,
	int id,
	const CompVSizeSz& blockSize COMPV_DEFAULT(CompVSizeSz(16, 16)),
	const CompVSizeSz& blockStride COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const CompVSizeSz& cellSize COMPV_DEFAULT(CompVSizeSz(8, 8)),
	const size_t nbins COMPV_DEFAULT(9),
	const int blockNorm COMPV_DEFAULT(COMPV_HOG_BLOCK_NORM_L2HYST),
	const bool gradientSigned COMPV_DEFAULT(true))
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED, "Not initialized");
	COMPV_CHECK_EXP_RETURN(!hog, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const CompVFeatureFactory* factory_ = CompVFeature::findFactory(id);
	if (!factory_) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to find feature factory with id = %d", id);
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
	}
	if (!factory_->newObjHOG) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Factory with id = %d and name = '%s' doesn't have a constructor for HOG ctor", factory_->id, factory_->name);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_CODE_RETURN(CompVHOG::checkParams(blockSize, blockStride, cellSize, nbins, blockNorm, gradientSigned));
	COMPV_CHECK_CODE_RETURN(factory_->newObjHOG(hog, blockSize, blockStride, cellSize, nbins, blockNorm, gradientSigned), "Failed to create HOG transform context");
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHOG::checkParams(
	const CompVSizeSz& blockSize,
	const CompVSizeSz& blockStride,
	const CompVSizeSz& cellSize,
	const size_t nbins,
	const int blockNorm,
	const bool gradientSigned)
{
	COMPV_CHECK_EXP_RETURN(
		!blockSize.width || !blockSize.height ||
		!blockStride.width || !blockStride.height ||
		!cellSize.width || !cellSize.height ||
		!nbins,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER,
		"Empty or null"
	);

	COMPV_CHECK_EXP_RETURN(blockSize.width % cellSize.width ||
		blockSize.height % cellSize.height,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER,
		"blockSize modulo cellSize must be equal to zero");

	COMPV_CHECK_EXP_RETURN(nbins > 32, 
		COMPV_ERROR_CODE_E_INVALID_PARAMETER, 
		"nbins must be within [1,32]");

	COMPV_CHECK_EXP_RETURN(
		blockNorm != COMPV_HOG_BLOCK_NORM_NONE &&
		blockNorm != COMPV_HOG_BLOCK_NORM_L1 &&
		blockNorm != COMPV_HOG_BLOCK_NORM_L1SQRT &&
		blockNorm != COMPV_HOG_BLOCK_NORM_L2 &&
		blockNorm != COMPV_HOG_BLOCK_NORM_L2HYST,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER,
		"blockNorm must be equal to COMPV_HOG_BLOCK_NORM_xxxx"
	);

	COMPV_DEBUG_INFO_CODE_TODO("Not complete");

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHOG::descriptorSize(
	const CompVSizeSz& winSize,
	const CompVSizeSz& blockSize,
	const CompVSizeSz& blockStride,
	const CompVSizeSz& cellSize,
	const size_t nbins,
	size_t* size)
{
	COMPV_CHECK_EXP_RETURN(!size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(winSize.width < blockSize.width || winSize.height < blockSize.height, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "winSize must be >= blockSize");
	COMPV_CHECK_CODE_RETURN(CompVHOG::checkParams(blockSize, blockStride, cellSize, nbins, COMPV_HOG_BLOCK_NORM_NONE, true));

	// https://books.google.fr/books?id=m9ByCwAAQBAJ&pg=PA223&lpg=PA223&dq=hog+cell+stride+bin+block&source=bl&ots=JLlVe4OZGQ&sig=BcJnu9ShbtMtlI8XdoYCTRNzVRw&hl=en&sa=X&ved=2ahUKEwi0xeiZ5_TaAhWD0RQKHVPAAdo4ChDoATAFegQIABBH#v=onepage&q=hog%20cell%20stride%20bin%20block&f=false
	*size = nbins *
		((blockSize.width / cellSize.width) * (blockSize.height / cellSize.height)) * // [BS/CD]^2
		(((winSize.width - blockSize.width) / blockStride.width + 1) * ((winSize.height - blockSize.height) / blockStride.height + 1)) // [(WS-BS)/SZ+1]^2
		;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
