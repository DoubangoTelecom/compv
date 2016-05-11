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

#if !defined(COMPV_FEATURE_DESC_ORB_FXP_DESC)
#	define COMPV_FEATURE_DESC_ORB_FXP_DESC					0 // Disable/Enable 'describe()' fixed point implementation. /!\ Must be disabled as it's buggy.
#endif
#if !defined(COMPV_FEATURE_DESC_ORB_FXP_CONVLT)
#	define COMPV_FEATURE_DESC_ORB_FXP_CONVLT				1 // Disable/Enable 'convlt()' fixed point implementation
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
	virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, const CompVPtr<CompVBoxInterestPoint* >& interestPoints, CompVPtr<CompVArray<uint8_t>* >* descriptions);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVFeatureDesc* >* orb);

private:
	COMPV_ERROR_CODE convlt(CompVPtr<CompVImageScalePyramid * > pPyramid, int level);
	COMPV_ERROR_CODE describe(CompVPtr<CompVImageScalePyramid * > pPyramid, const CompVInterestPoint* begin, const CompVInterestPoint* end, uint8_t* desc);
	static COMPV_ERROR_CODE convlt_AsynExec(const struct compv_asynctoken_param_xs* pc_params);
	static COMPV_ERROR_CODE describe_AsynExec(const struct compv_asynctoken_param_xs* pc_params);

private:
    // TODO(dmi): use internal detector: BRIEF (just like what is done for the detector and FAST internal dete)
    CompVPtr<CompVImageScalePyramid* > m_pyramid;
	CompVPtr<CompVConvlt<float>* > m_convlt;
    CompVPtr<CompVArray<float>* > m_kern_float;
	CompVPtr<CompVArray<uint16_t>* > m_kern_fxp;
	CompVPtr<CompVImage* > m_image_blurred_prev;
	bool m_bMediaTypeVideo;
	int m_nPatchDiameter;
	int m_nPatchBits;
	void(*m_funBrief256_31_Float32)(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out);
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
	void(*m_funBrief256_31_Fxp)(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out);
#endif
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_ORB_DESC_H_ */
