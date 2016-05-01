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
#include "compv/compv_convlt.h"
#include "compv/compv_array.h"
#include "compv/image/scale/compv_imagescale_pyramid.h"
#include "compv/features/compv_feature.h"

#if !defined(COMPV_FEATURE_DESC_ORB_SIMD_ELMT_COUNT)
#	define COMPV_FEATURE_DESC_ORB_SIMD_ELMT_COUNT  32
#endif

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
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVFeatureDescORB";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVFeatureDesc::process
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, const CompVPtr<CompVBoxInterestPoint* >& interestPoints, CompVPtr<CompVFeatureDescriptions*>* descriptions);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVFeatureDesc* >* orb);

private:
	bool brief256_31(const CompVImage* image, int kpx, int kpy, float cosT, float sinT, COMPV_ALIGNED(x) void* desc);

private:
    // TODO(dmi): use internal detector: BRIEF (just like what is done for the detector and FAST internal dete)
    CompVPtr<CompVImageScalePyramid* > m_pyramid;
    CompVPtr<CompVConvlt* > m_convlt;
    CompVPtr<CompVArray<double>* > m_kern;
    const CompVImage* m_pcImages[COMPV_FEATURE_DESC_ORB_SIMD_ELMT_COUNT];
	int m_nPatchDiameter;
	int m_nPatchBits;
    struct {
        float *m_pxf;
        float *m_pyf;
        float *m_psf;
        float *m_pangleInDegree;
        int32_t *m_pxi;
        int32_t *m_pyi;
        float *m_pcos;
        float *m_psin;
    } m_simd;
	void(*m_funBrief256_31)(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_ORB_DESC_H_ */
