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
/* @description
* This class implements ORB (Oriented FAST and Rotated BRIEF) feature detector.
* Some literature:
	* ORB final: https://www.willowgarage.com/sites/default/files/orb_final.pdf
	* Measuring Corner Properties: http://users.cs.cf.ac.uk/Paul.Rosin/corner2.pdf (check "Intensity centroid" used in ORB vs "Gradient centroid")
	* Image moments: https://en.wikipedia.org/wiki/Image_moment
	* Centroid: https://en.wikipedia.org/wiki/Centroid
*/


// TODO(dmi):
// OpenCV ORB implementation, pyramid with 8 levels, HARRIS scores, linear interpolation
// Bi-Linear interpolation: https://en.wikipedia.org/wiki/Bilinear_interpolation
// In our case, will use DCCI (https://en.wikipedia.org/wiki/Directional_Cubic_Convolution_Interpolation) to preserve edges
// Allow setting max number of features to retain
// Allow setting FAST threshold, N, non-maxima-supp
// Allow setting the strength type for fast: SAD or Highest*
// Allow setting the orb strength type = FAST or HARRIS
// Allow setting the interpolation type: BILINEAR (https://en.wikipedia.org/wiki/Bilinear_interpolation) or DCCI (https://en.wikipedia.org/wiki/Directional_Cubic_Convolution_Interpolation) which preserves edges
// Create your own detector + descriptor using "Hu's invariant moments" -> https://en.wikipedia.org/wiki/Image_moment#Rotation_invariant_moments


#include "compv/features/orb/compv_feature_orb_dete.h"
#include "compv/image/compv_image_moments.h"
#include "compv/compv_mathutils.h"

COMPV_NAMESPACE_BEGIN()

// These default values are also used in the descriptor (extern...)
int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT = 20; // T: Default threshold (pixel intensity: [0-255])
int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT = 9; // N: Number of positive continuous pixel to have before declaring a candidate as an interest point
bool COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP = true; // NMS:
int COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS = 8; // number of levels
float COMPV_FEATURE_DETE_ORB_PYRAMID_SF = 0.83f; // scale factor
int COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER = 31;
COMPV_SCALE_TYPE COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE = COMPV_SCALE_TYPE_BILINEAR;

CompVFeatureDeteORB::CompVFeatureDeteORB()
    : CompVFeatureDete(COMPV_ORB_ID)
    , m_nMaxFeatures(-1)
{

}

CompVFeatureDeteORB::~CompVFeatureDeteORB()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDeteORB::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_ORB_SET_INT32_FAST_THRESHOLD: {
        return m_internalDetector->set(COMPV_FAST_SET_INT32_THRESHOLD, valuePtr, valueSize);
    }
    case COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP: {
        return m_internalDetector->set(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, valuePtr, valueSize);
    }
    case COMPV_ORB_SET_INT32_MAX_FEATURES: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        // do not forward this value to the internal feature detector, it'll be set for each piramid level
        m_nMaxFeatures = *((int32_t*)valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_ORB_SET_INT32_PYRAMID_LEVELS:
    case COMPV_ORB_SET_INT32_PYRAMID_SCALE_TYPE:
    case COMPV_ORB_SET_FLOAT_PYRAMID_SCALE_FACTOR: {
        if (id == COMPV_ORB_SET_INT32_PYRAMID_LEVELS) {
            COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
            int32_t nLevels = *((int32_t*)valuePtr);
            if (m_pyramid->getLevels() != nLevels) {
                return CompVImageScalePyramid::newObj(m_pyramid->getScaleFactor(), nLevels, m_pyramid->getScaleType(), &m_pyramid);
            }
        }
        else if (id == COMPV_ORB_SET_INT32_PYRAMID_SCALE_TYPE) {
            COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
            COMPV_SCALE_TYPE eScaleType = *((COMPV_SCALE_TYPE*)valuePtr);
            if (m_pyramid->getScaleType() != eScaleType) {
                return CompVImageScalePyramid::newObj(m_pyramid->getScaleFactor(), m_pyramid->getLevels(), eScaleType, &m_pyramid);
            }
        }
        else if (id == COMPV_ORB_SET_FLOAT_PYRAMID_SCALE_FACTOR) {
            COMPV_CHECK_EXP_RETURN(valueSize != sizeof(float), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
            float fScaleFactor = *((float*)valuePtr);
            if (m_pyramid->getScaleFactor()!= fScaleFactor) {
                return CompVImageScalePyramid::newObj(fScaleFactor, m_pyramid->getLevels(), m_pyramid->getScaleType(), &m_pyramid);
            }
        }
        return COMPV_ERROR_CODE_S_OK;
    }
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}

COMPV_ERROR_CODE CompVFeatureDeteORB::get(int id, const void*& valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_FEATURE_GET_PTR_PYRAMID: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(CompVImageScalePyramid), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        valuePtr = *m_pyramid;
        return COMPV_ERROR_CODE_S_OK;
    }
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}

// FIXME
static bool cmp_strength_dec(const CompVInterestPoint* i, const CompVInterestPoint* j)
{
    return (i->strength > j->strength);
}

// override CompVFeatureDete::process
COMPV_ERROR_CODE CompVFeatureDeteORB::process(const CompVObjWrapper<CompVImage*>& image, CompVObjWrapper<CompVBoxInterestPoint* >& interestPoints)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVObjWrapper<CompVBoxInterestPoint* > interestPointsAtLevelN;
    CompVObjWrapper<CompVImage*> imageAtLevelN;
    float sf, sfs, patchSize;
    int32_t prevLevel;
    double m10, m01, orientRad;
    CompVInterestPoint* point_;

    // create or reset points
    if (!interestPoints) {
        COMPV_CHECK_CODE_RETURN(CompVBoxInterestPoint::newObj(&interestPoints));
    }
    else {
        interestPoints->reset();
    }

    // Scale the image
    COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image));

    sfs = m_pyramid->getScaleFactorsSum();

    // Process feature detection for each level
    // FIXME: multi-thread
    for (int32_t level = 0; level < m_pyramid->getLevels(); ++level) {
        sf = m_pyramid->getScaleFactor(level);
        patchSize = COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER / sf; // TODO(dmi): in OpenCV the patch size increase (instead of decreasing) with the level. Doesn't look correct.
        // Clear previous points, will be done by the internal detector but we prefer do to it here to make sure it
        // will work for buggy detectors
        if (interestPointsAtLevelN) {
            interestPointsAtLevelN->reset();
        }
        // Get image at level N
        COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(level, &imageAtLevelN));

        // Detect features for level N
        // For example, "m_internalDetector" would be FAST feature detector
        COMPV_CHECK_CODE_RETURN(m_internalDetector->process(imageAtLevelN, interestPointsAtLevelN));
        // Compute max features
        if (m_nMaxFeatures > 0) {
            int32_t maxFeatures = (int32_t)((m_nMaxFeatures / sfs) * sf);
#if 0 // must not enable
            if (m_internalDetector->getId() == COMPV_FAST_ID) {
                COMPV_CHECK_CODE_RETURN(m_internalDetector->set(COMPV_FAST_SET_INT32_MAX_FEATURES, &maxFeatures, sizeof(maxFeatures)));
            }
#endif
            interestPointsAtLevelN->sort(cmp_strength_dec);
            interestPointsAtLevelN->resize(maxFeatures);
        }

        // TODO(dmi): optimize
        for (size_t i = 0; i < interestPointsAtLevelN->size(); ++i) {
            interestPointsAtLevelN->at(i)->level = level;
            interestPointsAtLevelN->at(i)->size = patchSize;
        }
        interestPoints->append(interestPointsAtLevelN->begin(), interestPointsAtLevelN->end());
    }

    // For each point compute the orientation and scale(X,Y)
    // FIXME: Multi-thread
    prevLevel = -1;
    for (size_t i = 0; i < interestPoints->size(); ++i) {
        point_ = interestPoints->at(i);
        if (point_->level != prevLevel) { // Guard to make sure we'll query for the image only when different
            COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(point_->level, &imageAtLevelN));
            prevLevel = point_->level;
        }

        // computes moments
        CompVImageMoments::cirM01M10((const uint8_t*)imageAtLevelN->getDataPtr(), COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER, point_->x, point_->y, imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), &m01, &m10);
        // compute orientation
        orientRad = COMPV_MATH_ATAN2(m01, m10);
        //double orientRad = COMPV_MATH_ATAN2(cy, cx);
        point_->orient = (float)COMPV_MATH_RADIAN_TO_DEGREE(orientRad);
        if (point_->orient < 0) {
            point_->orient += 360;    // ((point_->orient + 360) % 360)
        }

        // Now that orientation is computed (required real size), scaleup the size
        if (point_->level != 0) {
            sf = m_pyramid->getScaleFactor(point_->level);
            point_->setXYf(point_->x / sf, point_->y / sf);
        }
    }

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDeteORB::newObj(CompVObjWrapper<CompVFeatureDete* >* orb)
{
    COMPV_CHECK_EXP_RETURN(orb == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVFeatureDete* >fast_;
    CompVObjWrapper<CompVImageScalePyramid * > pyramid_;
    int32_t val32;
    bool valBool;

    // Create the FAST feature detector
    COMPV_CHECK_CODE_RETURN(CompVFeatureDete::newObj(COMPV_FAST_ID, &fast_));
    val32 = COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT;
    COMPV_CHECK_CODE_RETURN(fast_->set(COMPV_FAST_SET_INT32_THRESHOLD, &val32, sizeof(val32)));
    valBool = COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP;
    COMPV_CHECK_CODE_RETURN(fast_->set(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, &valBool, sizeof(valBool)));
    // Create the pyramid
    COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE, &pyramid_));

    CompVObjWrapper<CompVFeatureDeteORB* >_orb = new CompVFeatureDeteORB();
    if (!_orb) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    _orb->m_internalDetector = fast_;
    _orb->m_pyramid = pyramid_;

    *orb = *_orb;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
