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
#if !defined(_COMPV_FEATURES_ORB_DESC_H_)
#define _COMPV_FEATURES_ORB_DESC_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/scale/compv_imagescale_pyramid.h"
#include "compv/features/compv_feature.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif


COMPV_NAMESPACE_BEGIN()

class CompVFeatureDescORB : public CompVFeatureDesc
{
protected:
	CompVFeatureDescORB();
public:
	virtual ~CompVFeatureDescORB();
	virtual COMPV_INLINE const char* getObjectId() { return "CompVFeatureDescORB"; };

	// override CompVSettable::set
	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
	// override CompVFeatureDesc::process
	virtual COMPV_ERROR_CODE process(const CompVObjWrapper<CompVImage*>& image, const std::vector<CompVInterestPoint >& interestPoints, CompVObjWrapper<CompVFeatureDescriptions*>* descriptions);

	static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVFeatureDesc* >* orb);

private:
	// TODO: use internal detector: BRIEF (just like what is done for the detector and FAST internal dete)
	CompVObjWrapper<CompVImageScalePyramid * > m_pyramid;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_ORB_DESC_H_ */
