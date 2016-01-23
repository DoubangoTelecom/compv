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
#include "compv/features/fast/compv_feature_fast_dete.h"
#include "compv/compv_mathutils.h"

COMPV_NAMESPACE_BEGIN()

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT	20 // T:
#define COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT			9 // N: Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP		true // NMS:
#define COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS			8 // number of levels
#define COMPV_FEATURE_DETE_ORB_PYRAMID_SF				0.83f // scale factor
#define COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE		COMPV_SCALE_TYPE_BILINEAR

CompVFeatureDeteORB::CompVFeatureDeteORB()
: CompVFeatureDete(COMPV_ORB_ID)
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
// compute image moments
static int moments(const uint8_t* ptr, int patch_size, int kpx, int kpy, int imgw, int imgs, int imgh, int p, int q)
{
	double mpq = 0;
	int patch_size_div2 = patch_size >> 1;
	double jq;
	// FIXME: must be circular patch?
	for (int j = -patch_size_div2; j < patch_size_div2; ++j) { // FIXME: j = -patch_size_div2 ....
		int y = kpy + j;
		jq = pow(j, q);
		y = COMPV_MATH_CLIP3(0, imgh - 1, y); // FIXME: duplicate border
		for (int i = -patch_size_div2; i < patch_size_div2; ++i) {
			int x = kpx + i;
			x = COMPV_MATH_CLIP3(0, imgw - 1, x);  // FIXME: duplicate border
			mpq += pow(i, p) * jq * ptr[(y * imgs) + x];
		}
	}
	return (int)(mpq);
}

// override CompVFeatureDete::set
COMPV_ERROR_CODE CompVFeatureDeteORB::process(const CompVObjWrapper<CompVImage*>& image, std::vector<CompVInterestPoint >& interestPoints)
{
	COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	std::vector<CompVInterestPoint > interestPointsAtLevelN;
	CompVObjWrapper<CompVImage*> imageAtLevelN;

	// Clear old interest points
	interestPoints.clear();

	// Scale the image
	COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image));
	
	// Process feature detection for each level
	int patchSize = PATCH_DIAMETER;
	for (int32_t level = 0; level < m_pyramid->getLevels(); ++level) {
		interestPointsAtLevelN.clear();
		COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(level, &imageAtLevelN));
		COMPV_CHECK_CODE_RETURN(m_internalDetector->process(imageAtLevelN, interestPointsAtLevelN));
		// TODO(dmi): optimize
		for (int i = 0; i < interestPointsAtLevelN.size(); ++i) {
			interestPointsAtLevelN[i].layer = level;
			interestPointsAtLevelN[i].patchSize = patchSize;
		}
		interestPoints.insert(std::end(interestPoints), std::begin(interestPointsAtLevelN), std::end(interestPointsAtLevelN));
		patchSize = (int)(patchSize / m_pyramid->getScaleFactor()); // FIXME: increase patch, no decrease. Correct?
	}
	// For each point compute the orientation
	for (int i = 0; i < interestPoints.size(); ++i) {
		CompVInterestPoint* point_ = &interestPoints[i];
		COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(point_->layer, &imageAtLevelN)); // Getting the image for each point instead of level -> slooow

		// computes moments
		//int m00 = moments((const uint8_t*)imageAtLevelN->getDataPtr(), point_->patchSize, point_->x, point_->y, imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), 0, 0);
		int m10 = moments((const uint8_t*)imageAtLevelN->getDataPtr(), point_->patchSize, point_->x, point_->y, imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), 1, 0);
		int m01 = moments((const uint8_t*)imageAtLevelN->getDataPtr(), point_->patchSize, point_->x, point_->y, imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), 0, 1);
		// compute centroid
		//int cx = m00 == 0 ? 0 : m10 / m00;
		//int cy = m01 == 0 ? 0 : m01 / m00;
		// compute orientation
		double orientRad = COMPV_MATH_ATAN2(m01, m10);
		//double orientRad = COMPV_MATH_ATAN2(cy, cx);
		point_->orient = (float)COMPV_MATH_RADIAN_TO_DEGREE(orientRad);
		// FIXME:
		//if (point_->orient < 0) point_->orient += 360;
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
	COMPV_CHECK_CODE_ASSERT(CompVImageScalePyramid::newObj(COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE, &pyramid_));

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
