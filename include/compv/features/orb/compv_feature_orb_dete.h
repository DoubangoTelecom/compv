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
#if !defined(_COMPV_FEATURES_ORB_DETE_H_)
#define _COMPV_FEATURES_ORB_DETE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/scale/compv_imagescale_pyramid.h"
#include "compv/features/compv_feature.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVFeatureDeteORB : public CompVFeatureDete
{
protected:
    CompVFeatureDeteORB();
public:
    virtual ~CompVFeatureDeteORB();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVFeatureDeteORB";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVSettable::get
    virtual COMPV_ERROR_CODE get(int id, const void*& valuePtr, size_t valueSize);
    // override CompVFeatureDete::process
    virtual COMPV_ERROR_CODE process(const CompVObjWrapper<CompVImage*>& image, CompVObjWrapper<CompVBoxInterestPoint* >& interestPoints);

    static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVFeatureDete* >* orb);

private:
	COMPV_ERROR_CODE createInterestPoints(int32_t count = -1);
	COMPV_ERROR_CODE freeInterestPoints(int32_t count = -1);
	COMPV_ERROR_CODE processLevelAt(const CompVObjWrapper<CompVImage*>& image, int level);

private:
    CompVObjWrapper<CompVFeatureDete* > m_internalDetector;
    CompVObjWrapper<CompVImageScalePyramid * > m_pyramid;
	CompVObjWrapper<CompVBoxInterestPoint* >* m_pInterestPointsAtLevelN;
    int32_t m_nMaxFeatures;
	int32_t m_nPyramidLevels;
	int* m_pCircleMaxI;
	size_t m_nCircleMaxICount;
	int m_nPatchDiameter;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_ORB_DETE_H_ */
