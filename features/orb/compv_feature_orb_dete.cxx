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
#include "compv/compv_engine.h"
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
	, m_nPyramidLevels(-1)
	, m_pCircleMaxI(NULL)
	, m_nCircleMaxICount(0)
	, m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
	, m_pInterestPointsAtLevelN(NULL)
	, m_bNMS(COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP)
	, m_nThreshold(COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT)
{

}

CompVFeatureDeteORB::~CompVFeatureDeteORB()
{
	CompVMem::free((void**)&m_pCircleMaxI);
	freeInterestPoints();
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
            if (m_nPyramidLevels != nLevels) {
				COMPV_ERROR_CODE err_ = CompVImageScalePyramid::newObj(m_pyramid->getScaleFactor(), nLevels, m_pyramid->getScaleType(), &m_pyramid);
				if (COMPV_ERROR_CODE_IS_OK(err_)) {
					err_ = createInterestPoints(nLevels);
				}
				if (COMPV_ERROR_CODE_IS_OK(err_)) {
					m_nPyramidLevels = nLevels;
				}
				else {
					m_pyramid = NULL;
					freeInterestPoints();
					m_nPyramidLevels = 0;
				}
				return err_;
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
            if (m_pyramid->getScaleFactor() != fScaleFactor) {
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

// FIXME(dmi):
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
	int patch_radius = (m_nPatchDiameter >> 1);
	CompVObjWrapper<CompVThreadDispatcher* >threadDip = CompVEngine::getThreadDispatcher();
	CompVObjWrapper<CompVBoxInterestPoint* >interestPointsAtLevelN;
	CompVObjWrapper<CompVImage*> imageAtLevelN;
	int32_t threadsCount = 1;

    // create or reset points
    if (!interestPoints) {
        COMPV_CHECK_CODE_RETURN(CompVBoxInterestPoint::newObj(&interestPoints));
    }
    else {
        interestPoints->reset();
    }

	// Create maximum abscissa for the circular patch if not already done
	if (m_nCircleMaxICount != (patch_radius + 1)) {
		CompVMem::free((void**)&m_pCircleMaxI);
		m_nCircleMaxICount = 0;
		m_pCircleMaxI = (int*)CompVMem::malloc((patch_radius + 1) * sizeof(int));
		COMPV_CHECK_EXP_RETURN(!m_pCircleMaxI, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		m_nCircleMaxICount = patch_radius + 1;

		int patch_radius_pow2 = patch_radius * patch_radius;
		for (int j = 0; j <= patch_radius; ++j) {
			// Pythagorean theorem: x = sqrt(r**2 - y**2)
			m_pCircleMaxI[j] = ((int)sqrt(patch_radius_pow2 - (j * j)));
		}
	}

	// Image scaling then feature could be multi-threaded but this requires a detector for each level -> memory issue

	// Scale the image (multi-threaded)
	COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image));

	// Create or reset interest points for each level then perform feature detection
	for (int level = 0; level < m_pyramid->getLevels(); ++level) {
		interestPointsAtLevelN = m_pInterestPointsAtLevelN[level];
		if (!interestPointsAtLevelN) {
			COMPV_CHECK_CODE_RETURN(err_ = CompVBoxInterestPoint::newObj(&interestPointsAtLevelN));
			m_pInterestPointsAtLevelN[level] = interestPointsAtLevelN;
		}
		else {
			interestPointsAtLevelN->reset();
		}
		// Get image at level N
		COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(level, &imageAtLevelN));
		// Detect features for level (multi-threaded)
		// For example, "m_internalDetector" would be FAST feature detector
		COMPV_CHECK_CODE_RETURN(m_internalDetector->process(imageAtLevelN, interestPointsAtLevelN));
	}

	// Compute number of threads
	if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
		threadsCount = threadDip->getThreadsCount();
	}

    // Process feature detection for each level
	if (threadsCount > 1) {
		CompVObjWrapper<CompVFeatureDeteORB* >This = this;
		uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
		// levelStart is used to make sure we won't schedule more than "threadsCount"
		int levelStart, level, levelMax;
		for (levelStart = 0, levelMax = threadsCount; levelStart < m_pyramid->getLevels(); levelStart += threadsCount, levelMax += threadsCount) {
			for (level = levelStart; level < m_pyramid->getLevels() && level < levelMax; ++level) {
				COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + level), COMPV_TOKENIDX0, CompVFeatureDeteORB::processLevelAt_AsynExec,
					COMPV_ASYNCTASK_SET_PARAM_ASISS(*This, *image, level),
					COMPV_ASYNCTASK_SET_PARAM_NULL()));
			}
			for (level = levelStart; level < m_pyramid->getLevels() && level < levelMax; ++level) {
				COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + level), COMPV_TOKENIDX0));
			}
		}
	}
	else {
		for (int level = 0; level < m_pyramid->getLevels(); ++level) {
			COMPV_CHECK_CODE_RETURN(err_ = processLevelAt(image, level));
		}
	}

	// Append features
	for (int level = 0; level < m_pyramid->getLevels(); ++level) {
		COMPV_CHECK_CODE_RETURN(err_ = interestPoints->append(m_pInterestPointsAtLevelN[level]->begin(), m_pInterestPointsAtLevelN[level]->end()));
	}

    return err_;
}

// Private function
// TODO(dmi): template function
COMPV_ERROR_CODE CompVFeatureDeteORB::createInterestPoints(int32_t count /*= -1*/)
{
	int32_t levelsCount = count > 0 ? count : m_nPyramidLevels;
	freeInterestPoints();
	m_pInterestPointsAtLevelN = (CompVObjWrapper<CompVBoxInterestPoint *>*)CompVMem::calloc(levelsCount, sizeof(CompVObjWrapper<CompVBoxInterestPoint *>));
	if (!m_pInterestPointsAtLevelN) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Private function
// TODO(dmi): template function
COMPV_ERROR_CODE CompVFeatureDeteORB::freeInterestPoints(int32_t count /*= -1*/)
{
	if (m_pInterestPointsAtLevelN) {
		int32_t levelsCount = count > 0 ? count : m_nPyramidLevels;
		for (int32_t i = 0; i < levelsCount; ++i) {
			m_pInterestPointsAtLevelN[i] = NULL;
		}
		CompVMem::free((void**)&m_pInterestPointsAtLevelN);
	}
	return COMPV_ERROR_CODE_S_OK;
}


// Private function
COMPV_ERROR_CODE CompVFeatureDeteORB::processLevelAt(const CompVObjWrapper<CompVImage*>& image, int level)
{
	COMPV_CHECK_EXP_RETURN(level < 0 || level >= m_nPyramidLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVObjWrapper<CompVImage*> imageAtLevelN;
	CompVObjWrapper<CompVBoxInterestPoint* > interestPointsAtLevelN;
	float sf, sfs, patchSize;
	double m10, m01, orientRad;
	CompVInterestPoint* point_;
	int patch_diameter = m_nPatchDiameter, patch_radius = (patch_diameter >> 1);
	const int* pCircleMaxI = m_pCircleMaxI;
	const uint8_t* imgPtr;
	int32_t imgWidth, imgHeight, imgStride;

	// Get image at level N
	COMPV_CHECK_CODE_RETURN(m_pyramid->getImage(level, &imageAtLevelN));
	imgPtr = (const uint8_t*)imageAtLevelN->getDataPtr();
	imgWidth = imageAtLevelN->getWidth();
	imgStride = imageAtLevelN->getStride();
	imgHeight = imageAtLevelN->getHeight();

	sfs = m_pyramid->getScaleFactorsSum();
	sf = m_pyramid->getScaleFactor(level);
	
	patchSize = m_nPatchDiameter / sf; // TODO(dmi): in OpenCV the patch size increase (instead of decreasing) with the level. Doesn't look correct.
	// Clear previous points, will be done by the internal detector but we prefer do to it here to make sure it
	// will work for buggy detectors
	
	// Retain best features only
	interestPointsAtLevelN = m_pInterestPointsAtLevelN[level];
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

	// For each point, set level and patch size, compute the orientation, scale (X,Y) coords...
	for (point_ = interestPointsAtLevelN->begin(); point_ < interestPointsAtLevelN->end(); ++point_) {
		point_->level = level;
		point_->size = patchSize;

		// computes moments
		CompVImageMoments::cirM01M10(imgPtr, patch_diameter, pCircleMaxI, point_->x, point_->y, imgWidth, imgStride, imgHeight, &m01, &m10);

		// compute orientation
		orientRad = COMPV_MATH_ATAN2(m01, m10);
		//double orientRad = COMPV_MATH_ATAN2(cy, cx);
		point_->orient = (float)COMPV_MATH_RADIAN_TO_DEGREE(orientRad);
		if (point_->orient < 0) {
			point_->orient += 360;    // ((point_->orient + 360) % 360)
		}

		// Now that orientation is computed (required real size), scaleup the size to match the original one
		// e.g. if original size is (100, 100) and sf = 0.5f
		//	- Real size = (100*0.5, 100*0.5) = (50, 50) which is used to compute the orientation
		//	- Once the orientation is computed, scaleup (x, y) which means (x/0.5, y/0.5) to have a representation in (100, 100) instead of (50, 50)
		//  - All points, regardless the scale factor will have their coords represented (scaledup) in the original size
		if (level != 0) {
			point_->setXYf(point_->x / sf, point_->y / sf);
		}
	}

	return err_;
}

// Private function
COMPV_ERROR_CODE CompVFeatureDeteORB::processLevelAt_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
	CompVFeatureDeteORB*  This = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVFeatureDeteORB*);
	CompVImage* image = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, CompVImage*);
	int level = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, int);
	return This->processLevelAt(image, level);
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
	_orb->m_nPyramidLevels = COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS;
	COMPV_CHECK_CODE_RETURN(_orb->createInterestPoints(COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS));

	*orb = *_orb;
	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()
