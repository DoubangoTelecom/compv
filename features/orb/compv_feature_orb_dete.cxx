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
#include "compv/compv_mathutils.h"

#include <algorithm>
#include <vector> // FIXME: use box

COMPV_NAMESPACE_BEGIN()

// These default values are also used in the descriptor (extern...)
int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT = 20; // T: Default threshold (pixel intensity: [0-255])
int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT = 9; // N: Number of positive continuous pixel to have before declaring a candidate as an interest point
bool COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP = true; // NMS:
int COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS = 8; // number of levels
float COMPV_FEATURE_DETE_ORB_PYRAMID_SF = 0.83f; // scale factor
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

// FIXME
#define PATCH_DIAMETER	31
#define PATH_RADIUS		(PATCH_DIAMETER >> 1)

static const float atan2_p1 = 0.9997878412794807f*(float)(180 / COMPV_MATH_PI);
static const float atan2_p3 = -0.3258083974640975f*(float)(180 / COMPV_MATH_PI);
static const float atan2_p5 = 0.1555786518463281f*(float)(180 / COMPV_MATH_PI);
static const float atan2_p7 = -0.04432655554792128f*(float)(180 / COMPV_MATH_PI);

float fastAtan2(float y, float x)
{
#if !defined(DBL_EPSILON)
    #define DBL_EPSILON 2.2204460492503131E-16
#endif
    float ax = std::abs(x), ay = std::abs(y);
    float a, c, c2;
    if (ax >= ay) {
        c = ay / (ax + (float)DBL_EPSILON);
        c2 = c*c;
        a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c;
    }
    else {
        c = ax / (ay + (float)DBL_EPSILON);
        c2 = c*c;
        a = 90.f - (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c;
    }
    if (x < 0) {
        a = 180.f - a;
    }
    if (y < 0) {
        a = 360.f - a;
    }
    return a;
}

// FIXME: implement fast arctan for embedded devices
// FIXME: compute severam m_01||m_10 then usgin SIMD and fastArcTan compute the angle in degree
static float IC_Angle(const uint8_t* image, int imgw, int imgs, int imgh, const int half_k, const CompVInterestPoint* pt,
                      const std::vector<int> & u_max)
{
    int m_01 = 0, m_10 = 0;

    const uint8_t* center = &image[(pt->y * imgs) + pt->x];

    // Treat the center line differently, v=0
    for (int u = -half_k; u <= half_k; ++u) {
        m_10 += u * center[u];
    }

    // Go line by line in the circular patch
    int step = imgs; // (int)image.step1();
    for (int v = 1; v <= half_k; ++v) {
        // Proceed over the two lines
        int v_sum = 0;
        int d = u_max[v];
        for (int u = -d; u <= d; ++u) {
            // FIXME: not needed because corners close to border (+-31/patchsize) was removed
            if ((u + pt->x) >= imgw || (u + pt->x) < 0) {
                continue;
            }
            if ((v + pt->y) >= imgh || (pt->y - v) < 0) {
                continue;
            }

            int val_plus = center[u + v*step], val_minus = center[u - v*step];
            v_sum += (val_plus - val_minus);
            m_10 += u * (val_plus + val_minus);
        }
        m_01 += v * v_sum;
    }

#if 0
    float angle = atan2f((float)m_01, (float)m_10);
    angle = (float)COMPV_MATH_RADIAN_TO_DEGREE(angle);
    if (angle < 0) {
        angle += 320;
    }
    return angle;
#else
    return fastAtan2((float)m_01, (float)m_10);
#endif
}

// compute image moments
// Measuring Corner Properties: http://users.cs.cf.ac.uk/Paul.Rosin/corner2.pdf
static int moments(const uint8_t* ptr, int patch_size, int kpx, int kpy, int imgw, int imgs, int imgh, int p, int q)
{
    double mpq = 0;
    int patch_size_div2 = patch_size >> 1;
    double jq;

    // FIXME: must be circular patch?
    for (int j = -patch_size_div2; j <= patch_size_div2; ++j) { // FIXME: j = -patch_size_div2 ....
        int y = kpy + j;
        jq = pow(j, q);
        if (y < 0 || y >= imgh) {
            continue;    // FIXME: remove keypoints too close to the border
        }
        int maxx = ((int)sqrt(patch_size_div2 * patch_size_div2 - j * j)); // Compute once
        for (int i = -maxx; i <= maxx; ++i) {
            int x = kpx + i;
            if (x < 0 || x >= imgw) {
                continue;    // FIXME: remove keypoints too close to the border
            }
            mpq += pow(i, p) * jq * ptr[(y * imgs) + x];
        }
    }
    return (int)(mpq);
}

struct Size {
    int width;
    int height;
    Size(int w, int h) :width(w), height(h) {}
};
struct Point {
    int x;
    int y;
    Point(int x_, int y_) {
        x = x_;
        y = y_;
    }
};
// FIXME:
struct Rect {
    int x;
    int y;
    int width;
    int height;
    Rect(const Point& org, const Size& sz): x(org.x), y(org.y), width(sz.width), height(sz.height) {}
    Rect(const Point& pt1, Point& pt2) {
        x = std::min(pt1.x, pt2.x);
        y = std::min(pt1.y, pt2.y);
        width = std::max(pt1.x, pt2.x) - x;
        height = std::max(pt1.y, pt2.y) - y;
    }
    inline bool contains(const Point& pt) const {
        return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height;
    }
};
// FIXME:
struct RoiPredicate {
    RoiPredicate(const Rect& _r) : r(_r) {
    }

    bool operator()(const CompVInterestPoint& keyPt) const {
        return !r.contains(Point(keyPt.x, keyPt.y));
    }

    Rect r;
};

// FIXME:
void runByImageBorder(CompVObjWrapper<CompVBoxInterestPoint* >& keypoints, Size imageSize, int borderSize)
{
#if 0 // implement using box
    if (borderSize > 0) {
        if (imageSize.height <= borderSize * 2 || imageSize.width <= borderSize * 2) {
            keypoints.clear();
        }
        else
            keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(),
                                           RoiPredicate(Rect(Point(borderSize, borderSize),
                                                   Point(imageSize.width - borderSize, imageSize.height - borderSize)))),
                            keypoints.end());
    }
#endif
}

// FIXME
static bool cmp_strength_dec(const CompVInterestPoint* i, const CompVInterestPoint* j)
{
    return (i->strength>j->strength);
}
// FIXME:
static inline float getScale(int level, int firstLevel, double scaleFactor)
{
    return (float)std::pow(scaleFactor, (double)(level - firstLevel));
}

// FIXME:
struct KeypointResponseGreaterThanThreshold {
    KeypointResponseGreaterThanThreshold(float _value) :
        value(_value) {
    }
    inline bool operator()(const CompVInterestPoint& kpt) const {
        return kpt.strength >= value;
    }
    float value;
};

// FIXME:
struct KeypointResponseGreater {
    inline bool operator()(const CompVInterestPoint& kp1, const CompVInterestPoint& kp2) const {
        return kp1.strength > kp2.strength;
    }
};

// FIXME:
static void retainBest(std::vector<CompVInterestPoint>& keypoints, int n_points)
{
    //this is only necessary if the keypoints size is greater than the number of desired points.
    if (n_points >= 0 && keypoints.size() > (size_t)n_points) {
        if (n_points == 0) {
            keypoints.clear();
            return;
        }
        //first use nth element to partition the keypoints into the best and worst.
        std::nth_element(keypoints.begin(), keypoints.begin() + n_points, keypoints.end(), KeypointResponseGreater());
        //this is the boundary response, and in the case of FAST may be ambigous
        float ambiguous_response = keypoints[n_points - 1].strength;
        //use std::partition to grab all of the keypoints with the boundary response.
        std::vector<CompVInterestPoint>::const_iterator new_end =
            std::partition(keypoints.begin() + n_points, keypoints.end(),
                           KeypointResponseGreaterThanThreshold(ambiguous_response));
        //resize the keypoints, given this new end point. nth_element and partition reordered the points inplace
        keypoints.resize(new_end - keypoints.begin());
    }
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
    for (int32_t level = 0; level < m_pyramid->getLevels(); ++level) {
        sf = m_pyramid->getScaleFactor(level);
        patchSize = PATCH_DIAMETER / sf; // TODO(dmi): in OpenCV the patch size increase (instead of decreasing) with the level. Doesn't look correct.
        // clear previous points for level N-1
		if (interestPointsAtLevelN) {
			interestPointsAtLevelN->reset();
		}
        // get image at level N
        COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(level, &imageAtLevelN));

        // detect features
        COMPV_CHECK_CODE_RETURN(m_internalDetector->process(imageAtLevelN, interestPointsAtLevelN));
        // FIXME: remove points too close to the border
        runByImageBorder(interestPointsAtLevelN, Size(imageAtLevelN->getWidth(), imageAtLevelN->getHeight()), 31); // FIXME: remove border then get topmost, otherwise we loose
        // Compute max features
        if (m_nMaxFeatures > 0) {
            int32_t maxFeatures = (int32_t)((m_nMaxFeatures / sfs) * sf);
            //if (m_internalDetector->getId() == COMPV_FAST_ID) {
            //COMPV_CHECK_CODE_RETURN(m_internalDetector->set(COMPV_FAST_SET_INT32_MAX_FEATURES, &maxFeatures, sizeof(maxFeatures)));
            //}
			interestPointsAtLevelN->sort(cmp_strength_dec);
            interestPointsAtLevelN->resize(maxFeatures);
            //retainBest(interestPointsAtLevelN, maxFeatures);
        }

        // FIXME: remove this and keep above line (#if 0)
        //patchSize = PATCH_DIAMETER * getScale(level, 0, 1.2f);

        // TODO(dmi): optimize
        for (size_t i = 0; i < interestPointsAtLevelN->size(); ++i) {
            interestPointsAtLevelN->at(i)->level = level;
			interestPointsAtLevelN->at(i)->size = patchSize;
        }
		// FIXME(box):
		interestPoints->append(interestPointsAtLevelN->begin(), interestPointsAtLevelN->end());
    }

    // FIXME:
    //runByImageBorder(interestPoints, Size(image->getWidth(), image->getHeight()), 31);

    // For each point compute the orientation
    for (size_t i = 0; i < interestPoints->size(); ++i) {
        CompVInterestPoint* point_ = interestPoints->at(i);
        COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(point_->level, &imageAtLevelN)); // Getting the image for each point instead of level -> slooow

#if 0
        // computes moments
        //int m00 = moments((const uint8_t*)imageAtLevelN->getDataPtr(), point_->patchSize, point_->x, point_->y, imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), 0, 0);
        int m10 = moments((const uint8_t*)imageAtLevelN->getDataPtr(), PATCH_DIAMETER, point_->x, point_->y, imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), 1, 0);
        int m01 = moments((const uint8_t*)imageAtLevelN->getDataPtr(), PATCH_DIAMETER, point_->x, point_->y, imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), 0, 1);
        // compute centroid
        //int cx = m00 == 0 ? 0 : m10 / m00;
        //int cy = m01 == 0 ? 0 : m01 / m00;
        // compute orientation
        double orientRad = COMPV_MATH_ATAN2(m01, m10);
        //double orientRad = COMPV_MATH_ATAN2(cy, cx);
        point_->orient = (float)COMPV_MATH_RADIAN_TO_DEGREE(orientRad);
        if (point_->orient < 0) {
            point_->orient += 360;    // ((point_->orient + 360) % 360)
        }
#else
        // pre-compute the end of a row in a circular patch
        // TODO(dmi): OpenCV uses same patch size for all images and it looks correct (tested with mandekalou). Strange
        int halfPatchSize = (int)(PATCH_DIAMETER /** m_pyramid->getScaleFactor(point_->level)*/) / 2;
        std::vector<int> umax(halfPatchSize + 2);

        int v, v0, vmax = (int)floor(halfPatchSize * sqrt(2.f) / 2 + 1);
        int vmin = (int)ceil(halfPatchSize * sqrt(2.f) / 2);
        for (v = 0; v <= vmax; ++v) {
            umax[v] = (int)round(sqrt((double)halfPatchSize * halfPatchSize - v * v));
        }

        // Make sure we are symmetric
        for (v = halfPatchSize, v0 = 0; v >= vmin; --v) {
            while (umax[v0] == umax[v0 + 1]) {
                ++v0;
            }
            umax[v] = v0;
            ++v0;
        }
        point_->orient = (float)IC_Angle((const uint8_t*)imageAtLevelN->getDataPtr(),
                                         imageAtLevelN->getWidth(),
                                         imageAtLevelN->getStride(),
                                         imageAtLevelN->getHeight(),
                                         halfPatchSize,
                                         point_,
                                         umax); // already in degree
#endif
    }

    // Now that orientation is computed (required real size), scaleup the size
    for (size_t i = 0; i < interestPoints->size(); ++i) {
        CompVInterestPoint* point_ = interestPoints->at(i);
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
