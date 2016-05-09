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
#include "compv/compv_math_utils.h"

COMPV_NAMESPACE_BEGIN()

// These default values are also used in the descriptor (extern...)
int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT = 20; // T: Default threshold (pixel intensity: [0-255])
int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT = 9; // N: Number of positive continuous pixel to have before declaring a candidate as an interest point
int COMPV_FEATURE_DETE_ORB_FAST_MAX_FEATURES = 2000; // This is known to provide acceptable result
bool COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP = true; // NMS:
int COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS = 8; // number of levels
float COMPV_FEATURE_DETE_ORB_PYRAMID_SF = 0.83f; // scale factor
int COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER = 31;
int COMPV_FEATURE_DETE_ORB_PATCH_BITS = 256;
COMPV_SCALE_TYPE COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE = COMPV_SCALE_TYPE_BILINEAR;

CompVFeatureDeteORB::CompVFeatureDeteORB()
    : CompVFeatureDete(COMPV_ORB_ID)
	, m_nMaxFeatures(COMPV_FEATURE_DETE_ORB_FAST_MAX_FEATURES)
    , m_nPyramidLevels(-1)
    , m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
    , m_pInterestPointsAtLevelN(NULL)
    , m_bNMS(COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP)
    , m_nThreshold(COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT)
	, m_pDetectors(NULL)
	, m_nDetectors(0)
	, m_pPatches(NULL)
	, m_nPatches(0)
{

}

CompVFeatureDeteORB::~CompVFeatureDeteORB()
{
    freeInterestPoints();
	freePatches();
	freeDetectors();
}

// override CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDeteORB::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_ORB_SET_INT32_FAST_THRESHOLD: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nThreshold = *((int32_t*)valuePtr);
        return initDetectors();
    }
    case COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_bNMS = *((bool*)valuePtr);
		return initDetectors();
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

// override CompVFeatureDete::process
COMPV_ERROR_CODE CompVFeatureDeteORB::process(const CompVPtr<CompVImage*>& image, CompVPtr<CompVBoxInterestPoint* >& interestPoints)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    int patch_radius = (m_nPatchDiameter >> 1); // FIXME: remove
    CompVPtr<CompVThreadDispatcher* >threadDip = CompVEngine::getThreadDispatcher();
    CompVPtr<CompVBoxInterestPoint* >interestPointsAtLevelN;
    CompVPtr<CompVImage*> imageAtLevelN;
    int32_t threadsCount = 1;

    // create or reset points
    if (!interestPoints) {
        COMPV_CHECK_CODE_RETURN(CompVBoxInterestPoint::newObj(&interestPoints));
    }
    else {
        interestPoints->reset();
    }

    // Image scaling then feature could be multi-threaded but this requires a detector for each level -> memory issue

#if 0
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
#endif

    // Compute number of threads
    if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
        threadsCount = threadDip->getThreadsCount();
    }

	// Create and init detectors
	// Number of detectors must be equal to the number of threads (not the case for interestpoints array which is equal to the number of levels)
	int32_t nDetectors = COMPV_MATH_CLIP3(1, threadsCount, m_pyramid->getLevels());
	if ((int32_t)m_nDetectors < nDetectors) {
		COMPV_CHECK_CODE_RETURN(createDetectors(nDetectors));
		for (int32_t d = 0; d < nDetectors; ++d) {
			err_ = CompVFeatureDete::newObj(COMPV_FAST_ID, &m_pDetectors[d]);
			if (COMPV_ERROR_CODE_IS_NOK(err_)) {
				freeDetectors();
				COMPV_CHECK_CODE_RETURN(err_);
			}
		}
		COMPV_CHECK_CODE_RETURN(initDetectors());
	}

	// Create patches
	// Number of patches must be equal to the number of threads (not the case for interestpoints array which is equal to the number of levels)
	int32_t nPatches = COMPV_MATH_CLIP3(1, threadsCount, m_pyramid->getLevels());
	if ((int32_t)m_nPatches < nPatches) {
		COMPV_CHECK_CODE_RETURN(createPatches(nPatches));
		for (int32_t p = 0; p < nPatches; ++p) {
			err_ = CompVPatch::newObj(&m_pPatches[p], m_nPatchDiameter);
			if (COMPV_ERROR_CODE_IS_NOK(err_)) {
				freePatches();
				COMPV_CHECK_CODE_RETURN(err_);
			}
		}
	}

    // Process feature detection for each level
    if (threadsCount > 1) {
        CompVPtr<CompVFeatureDeteORB* >This = this;
        uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
        // levelStart is used to make sure we won't schedule more than "threadsCount"
        int levelStart, level, levelMax;
        for (levelStart = 0, levelMax = threadsCount; levelStart < m_pyramid->getLevels(); levelStart += threadsCount, levelMax += threadsCount) {
            for (level = levelStart; level < m_pyramid->getLevels() && level < levelMax; ++level) {
                COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + level), COMPV_TOKENIDX0, CompVFeatureDeteORB::processLevelAt_AsynExec,
					COMPV_ASYNCTASK_SET_PARAM_ASISS(*This, *image, *m_pPatches[level % nPatches], *m_pDetectors[level % nDetectors], level),
                                        COMPV_ASYNCTASK_SET_PARAM_NULL()));
            }
            for (level = levelStart; level < m_pyramid->getLevels() && level < levelMax; ++level) {
                COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + level), COMPV_TOKENIDX0));
            }
        }
    }
    else {
        for (int level = 0; level < m_pyramid->getLevels(); ++level) {
            COMPV_CHECK_CODE_RETURN(err_ = processLevelAt(image, m_pPatches[0], m_pDetectors[0], level));
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
    m_pInterestPointsAtLevelN = (CompVPtr<CompVBoxInterestPoint *>*)CompVMem::calloc(levelsCount, sizeof(CompVPtr<CompVBoxInterestPoint *>));
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
// TODO(dmi): template function
COMPV_ERROR_CODE CompVFeatureDeteORB::createPatches(int32_t count /*= -1*/)
{
	int32_t patchesCount = count > 0 ? count : (int32_t)m_nPatches;
	freePatches();
	m_pPatches = (CompVPtr<CompVPatch *>*)CompVMem::calloc(patchesCount, sizeof(CompVPtr<CompVPatch *>));
	COMPV_CHECK_EXP_RETURN(!m_pPatches, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	m_nPatches = patchesCount;
	return COMPV_ERROR_CODE_S_OK;
}

// Private function
// TODO(dmi): template function
COMPV_ERROR_CODE CompVFeatureDeteORB::freePatches(int32_t count /*= -1*/)
{
	if (m_pPatches) {
		int32_t patchesCount = count > 0 ? count : (int32_t)m_nPatches;
		for (int32_t i = 0; i < patchesCount; ++i) {
			m_pPatches[i] = NULL;
		}
		CompVMem::free((void**)&m_pPatches);
	}
	m_nPatches = 0;
	return COMPV_ERROR_CODE_S_OK;
}

// Private function
// TODO(dmi): template function
COMPV_ERROR_CODE CompVFeatureDeteORB::createDetectors(int32_t count /*= -1*/)
{
	int32_t detectorsCount = count > 0 ? count : (int32_t)m_nDetectors;
	freeDetectors();
	m_pDetectors = (CompVPtr<CompVFeatureDete *>*)CompVMem::calloc(detectorsCount, sizeof(CompVPtr<CompVFeatureDete *>));
	COMPV_CHECK_EXP_RETURN(!m_pDetectors, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	m_nDetectors = detectorsCount;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFeatureDeteORB::initDetector(CompVPtr<CompVFeatureDete* >& detector)
{
	if (detector) {
		COMPV_CHECK_CODE_RETURN(detector->set(COMPV_FAST_SET_INT32_THRESHOLD, &m_nThreshold, sizeof(m_nThreshold)));
		COMPV_CHECK_CODE_RETURN(detector->set(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, &m_bNMS, sizeof(m_bNMS)));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFeatureDeteORB::initDetectors()
{
	if (m_pDetectors) {
		for (size_t i = 0; i < m_nDetectors; ++i) {
			COMPV_CHECK_CODE_RETURN(initDetector(m_pDetectors[i]));
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Private function
// TODO(dmi): template function
COMPV_ERROR_CODE CompVFeatureDeteORB::freeDetectors(int32_t count /*= -1*/)
{
	if (m_pDetectors) {
		int32_t detectorsCount = count > 0 ? count : (int32_t)m_nDetectors;
		for (int32_t i = 0; i < detectorsCount; ++i) {
			m_pDetectors[i] = NULL;
		}
		CompVMem::free((void**)&m_pDetectors);
	}
	m_nDetectors = 0;
	return COMPV_ERROR_CODE_S_OK;
}

// Private function
// Must be thread-safe
COMPV_ERROR_CODE CompVFeatureDeteORB::processLevelAt(const CompVPtr<CompVImage* >& image, CompVPtr<CompVPatch* >& patch, CompVPtr<CompVFeatureDete* >& detector, int level)
{
    COMPV_CHECK_EXP_RETURN(level < 0 || level >= m_nPyramidLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVPtr<CompVImage*> imageAtLevelN;
    CompVPtr<CompVBoxInterestPoint* > interestPointsAtLevelN;
	float sf, sfs, patchSize, nf, orientRad;
	int m10, m01;
    CompVInterestPoint* point_;
    int patch_diameter = m_nPatchDiameter, patch_radius = (patch_diameter >> 1);
    const uint8_t* imgPtr;
    int32_t imgWidth, imgHeight, imgStride;

	// Scale the image for the current level, multi-threaded and thread safe
	COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image, level));

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

	// Create or reset interest points for the current level then perform feature detection
	interestPointsAtLevelN = m_pInterestPointsAtLevelN[level];
	if (!interestPointsAtLevelN) {
		COMPV_CHECK_CODE_RETURN(err_ = CompVBoxInterestPoint::newObj(&interestPointsAtLevelN));
		m_pInterestPointsAtLevelN[level] = interestPointsAtLevelN;
	}
	else {
		interestPointsAtLevelN->reset();
	}
	// Detect features for level N (multi-threaded)
	// For example, "detector" would be FAST feature detector
	COMPV_CHECK_CODE_RETURN(detector->process(imageAtLevelN, interestPointsAtLevelN));

    // Retain best features only
    if (m_nMaxFeatures > 0 && interestPointsAtLevelN->size() > 0) {
		nf = ((m_nMaxFeatures / sfs) * sf);
		int32_t maxFeatures = COMPV_MATH_ROUNDFU_2_INT(nf, int32_t);
        if (interestPointsAtLevelN->size() > (size_t)maxFeatures) {
            COMPV_CHECK_CODE_RETURN(err_ = interestPointsAtLevelN->retainBest((size_t)maxFeatures));
        }
    }

	// Erase points too close to the border
	COMPV_CHECK_CODE_RETURN(err_ = interestPointsAtLevelN->eraseTooCloseToBorder(imgWidth, imgHeight, patch_radius));

    // For each point, set level and patch size, compute the orientation, scale (X,Y) coords...
    for (point_ = interestPointsAtLevelN->begin(); point_ < interestPointsAtLevelN->end(); ++point_) {
        point_->level = level;
        point_->size = patchSize;

        // computes moments
		patch->moments0110(imgPtr, (int)point_->x, (int)point_->y, imgWidth, imgStride, imgHeight, &m01, &m10);

        // compute orientation
		orientRad = COMPV_MATH_ATAN2F((float)m01, (float)m10);
        point_->orient = COMPV_MATH_RADIAN_TO_DEGREE_FLOAT(orientRad);
        if (point_->orient < 0) { // clamp orient within [0-360], required by many internal functions
            point_->orient += 360;    // ((point_->orient + 360) % 360)
        }
        // COMPV_ASSERT(point_->orient >= 0 && point_->orient < 360);

        // Now that orientation is computed (required real size), scaleup the size to match the original one
        // e.g. if original size is (100, 100) and sf = 0.5f
        //	- Real size = (100*0.5, 100*0.5) = (50, 50) which is used to compute the orientation
        //	- Once the orientation is computed, scaleup (x, y) which means (x/0.5, y/0.5) to have a representation in (100, 100) instead of (50, 50)
        //  - All points, regardless the scale factor will have their coords represented (scaledup) in the original size
        if (level != 0) {
            point_->x /= sf;
            point_->y /= sf;
        }
    }

    return err_;
}

// Private function
COMPV_ERROR_CODE CompVFeatureDeteORB::processLevelAt_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
    CompVFeatureDeteORB*  This = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVFeatureDeteORB*);
	CompVPtr< CompVImage* > image = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, CompVImage*);
	CompVPtr<CompVPatch* > patch = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, CompVPatch*);
	CompVPtr<CompVFeatureDete* > detector = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, CompVFeatureDete*);
    int level = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, int);
	return This->processLevelAt(image, patch, detector, level);
}

COMPV_ERROR_CODE CompVFeatureDeteORB::newObj(CompVPtr<CompVFeatureDete* >* orb)
{
    COMPV_CHECK_EXP_RETURN(orb == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVImageScalePyramid * > pyramid_;
    
    // Create the pyramid
    COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE, &pyramid_));

    CompVPtr<CompVFeatureDeteORB* >_orb = new CompVFeatureDeteORB();
    if (!_orb) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    _orb->m_pyramid = pyramid_;
    _orb->m_nPyramidLevels = COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS;
    COMPV_CHECK_CODE_RETURN(_orb->createInterestPoints(COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS));

    *orb = *_orb;
    return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()
