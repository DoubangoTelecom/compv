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
#if !defined(_COMPV_FEATURES_FEATURE_H_)
#define _COMPV_FEATURES_FEATURE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"
#include "compv/compv_settable.h"
#include "compv/image/compv_image.h"

#include <map>
#include <vector>

COMPV_NAMESPACE_BEGIN()

class CompVFeatureDete;
class CompVFeatureDesc;


struct CompVFeatureFactory {
	int id;
	const char* name;
	COMPV_ERROR_CODE(*newObjDete)(CompVObjWrapper<CompVFeatureDete* >* dete);
	COMPV_ERROR_CODE(*newObjDesc)(CompVObjWrapper<CompVFeatureDesc* >* desc);
};

/* Feature detectors and descriptors IDs */
enum {
	/* Feature detectors */
	COMPV_FEATURE_DETE_ID_FAST,
	COMPV_FEATURE_DETE_ID_ORB,

	/* Feature descriptors */
	COMPV_FEATURE_DESC_ID_ORB,
};

/* Feature detectors and descriptors setters and getters */
enum {
	

	/* FAST (Features from Accelerated Segment Test) */
	COMPV_SET_INT32_FAST_THRESHOLD,
	COMPV_SET_BOOL_FAST_NON_MAXIMA_SUPP,
	COMPV_FAST_MOD9,
};

class COMPV_API CompVFeature : public CompVObj, public CompVSettable
{
protected:
	CompVFeature();
public:
	virtual ~CompVFeature();
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE addFactory(const CompVFeatureFactory* factory);
	static const CompVFeatureFactory* findFactory(int deteId);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	static std::map<int, const CompVFeatureFactory*> s_Factories;
	COMPV_DISABLE_WARNINGS_END()
};

class COMPV_API CompVFeatureDete : public CompVObj, public CompVSettable
{
protected:
	CompVFeatureDete();
public:
	virtual ~CompVFeatureDete();
	virtual COMPV_ERROR_CODE process(const CompVObjWrapper<CompVImage*>& image, std::vector<CompVInterestPoint >& interestPoints) = 0;
	static COMPV_ERROR_CODE newObj(int deteId, CompVObjWrapper<CompVFeatureDete* >* dete);
};

class COMPV_API CompVFeatureDesc : public CompVObj, public CompVSettable
{
protected:
	CompVFeatureDesc();
public:
	virtual ~CompVFeatureDesc();
	static COMPV_ERROR_CODE newObj(int descId, CompVObjWrapper<CompVFeatureDesc* >* desc);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_FEATURE_H_ */
