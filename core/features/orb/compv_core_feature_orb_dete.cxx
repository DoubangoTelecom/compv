/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
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

#include "compv/core/features/orb/compv_core_feature_orb_dete.h"

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
#define COMPV_FEATURE_DETE_ORB_MIN_FEATUES_PER_LEVEL 10
COMPV_SCALE_TYPE COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE = COMPV_SCALE_TYPE_BILINEAR;

CompVCornerDeteORB::CompVCornerDeteORB()
	: CompVCornerDete(COMPV_ORB_ID)
	, m_nMaxFeatures(COMPV_FEATURE_DETE_ORB_FAST_MAX_FEATURES)
	, m_nPyramidLevels(-1)
	, m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
	, m_ppInterestPointsAtLevelN(NULL)
	, m_bNMS(COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP)
	, m_nThreshold(COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT)
	, m_nFastType(COMPV_FAST_TYPE_9)
	, m_ppDetectors(NULL)
	, m_nDetectors(0)
	, m_ppPatches(NULL)
	, m_nPatches(0)
{

}

CompVCornerDeteORB::~CompVCornerDeteORB()
{
	freeInterestPoints();
	freePatches();
	freeDetectors();
}

COMPV_ERROR_CODE CompVCornerDeteORB::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_ORB_SET_INT_FAST_THRESHOLD: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nThreshold = *reinterpret_cast<const int*>(valuePtr);
		return initDetectors();
	}
	case COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_bNMS = *((bool*)valuePtr);
		return initDetectors();
	}
	case COMPV_ORB_SET_INT_MAX_FEATURES: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		// do not forward this value to the internal feature detector, it'll be set for each piramid level
		m_nMaxFeatures = *reinterpret_cast<const int*>(valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_ORB_SET_INT_PYRAMID_LEVELS:
	case COMPV_ORB_SET_INT_PYRAMID_SCALE_TYPE:
	case COMPV_ORB_SET_FLT32_PYRAMID_SCALE_FACTOR: {
		if (id == COMPV_ORB_SET_INT_PYRAMID_LEVELS) {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			int nLevels = *reinterpret_cast<const int*>(valuePtr);
			if (m_nPyramidLevels != nLevels) {
				COMPV_ERROR_CODE err_;
				COMPV_CHECK_CODE_NOP(err_ = CompVImageScalePyramid::newObj(&m_pyramid, m_pyramid->scaleFactor(), nLevels, m_pyramid->scaleType()));
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
		else if (id == COMPV_ORB_SET_INT_PYRAMID_SCALE_TYPE) {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			COMPV_SCALE_TYPE eScaleType = static_cast<COMPV_SCALE_TYPE>(*reinterpret_cast<const int*>(valuePtr));
			if (m_pyramid->scaleType() != eScaleType) {
				return CompVImageScalePyramid::newObj(&m_pyramid, m_pyramid->scaleFactor(), m_pyramid->levels(), eScaleType);
			}
		}
		else if (id == COMPV_ORB_SET_FLT32_PYRAMID_SCALE_FACTOR) {
			COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			compv_float32_t fScaleFactor = *reinterpret_cast<const compv_float32_t*>(valuePtr);
			if (m_pyramid->scaleFactor() != fScaleFactor) {
				return CompVImageScalePyramid::newObj(&m_pyramid, fScaleFactor, m_pyramid->levels(), m_pyramid->scaleType());
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		return CompVCaps::set(id, valuePtr, valueSize);
	}
}

COMPV_ERROR_CODE CompVCornerDeteORB::get(int id, const void** valuePtrPtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(valueSize == 0 || !valuePtrPtr, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_FEATURE_GET_PTR_PYRAMID: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(CompVImageScalePyramid), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		*valuePtrPtr = *m_pyramid;
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		return CompVCaps::set(id, valuePtrPtr, valueSize);
	}
}
	
COMPV_ERROR_CODE CompVCornerDeteORB::process(const CompVMatPtr& image, CompVInterestPointVector& interestPoints) /*Overrides(CompVCornerDete)*/
{
	COMPV_CHECK_EXP_RETURN(!image || image->isEmpty() || image->subType() != COMPV_SUBTYPE_PIXELS_Y,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVInterestPointVector interestPointsAtLevelN;
	CompVMatPtr imageAtLevelN;
	size_t threadsCount = 1, levelsCount;

	interestPoints.clear();

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation");
	

	// Image scaling then feature could be multi-threaded but this requires a detector for each level -> memory issue

	levelsCount = m_pyramid->levels();

	// Create and init detectors
	// Number of detectors must be equal to the number of threads (not the case for interestpoints array which is equal to the number of levels)
	size_t nDetectors = COMPV_MATH_CLIP3(1, threadsCount, levelsCount);
	if (m_nDetectors < nDetectors) {
		COMPV_CHECK_CODE_RETURN(createDetectors(nDetectors));
		for (size_t d = 0; d < nDetectors; ++d) {
			err_ = CompVCornerDete::newObj(&m_ppDetectors[d], COMPV_FAST_ID);
			if (COMPV_ERROR_CODE_IS_NOK(err_)) {
				freeDetectors();
				COMPV_CHECK_CODE_RETURN(err_);
			}
		}
		COMPV_CHECK_CODE_RETURN(initDetectors());
	}

	// Create patches
	// Number of patches must be equal to the number of threads (not the case for interestpoints array which is equal to the number of levels)
	size_t nPatches = COMPV_MATH_CLIP3(1, threadsCount, levelsCount);
	if (m_nPatches < nPatches) {
		COMPV_CHECK_CODE_RETURN(createPatches(nPatches));
		for (size_t p = 0; p < nPatches; ++p) {
			err_ = CompVPatch::newObj(&m_ppPatches[p], m_nPatchDiameter);
			if (COMPV_ERROR_CODE_IS_NOK(err_)) {
				freePatches();
				COMPV_CHECK_CODE_RETURN(err_);
			}
		}
	}

	// Process feature detection for each level
	// TODO(dmi): not optimized when levels > maxThreads, single-threaded when levels == 1
	//			Not a high prio. issue because most of the time consuming function is FAST feature detector and it's multi-threaded
#if 0
	if (threadsCount > 1) {
		CompVPtr<CompVCornerDeteORB* >This = this;
		// levelStart is used to make sure we won't schedule more than "threadsCount"
		int levelStart, level, levelMax;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(m_pyramid->getLevels());
		auto funcPtr = [&](const CompVPtr<CompVImage* >& image, CompVPtr<CompVPatch* >& patch, CompVPtr<CompVCornerDete* >& detector, int level) -> COMPV_ERROR_CODE {
			return processLevelAt(*image, patch, detector, level);
		};
		for (levelStart = 0, levelMax = threadsCount; levelStart < m_pyramid->getLevels(); levelStart += threadsCount, levelMax += threadsCount) {
			for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, image, m_pPatches[level % nPatches], m_pDetectors[level % nDetectors], level), taskIds));
			}
			for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
				COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[level]));
				COMPV_CHECK_CODE_RETURN(interestPoints->append(m_pInterestPointsAtLevelN[level]->begin(), m_pInterestPointsAtLevelN[level]->end()));
			}
		}
	}
	else {
#endif
		for (int level = 0; level < levelsCount; ++level) {
			COMPV_CHECK_CODE_RETURN(processLevelAt(image, m_ppPatches[0], m_ppDetectors[0], level));
			interestPoints.insert(interestPoints.end(), m_ppInterestPointsAtLevelN[level].begin(), m_ppInterestPointsAtLevelN[level].end());
		}
#if 0
	}
#endif

	return err_;
}

COMPV_ERROR_CODE CompVCornerDeteORB::newObj(CompVCornerDetePtrPtr orb)
{
	COMPV_CHECK_EXP_RETURN(orb == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVImageScalePyramidPtr pyramid_;

	// Create the pyramid
	COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(&pyramid_, COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE));

	CompVCornerDeteORBPtr _orb = new CompVCornerDeteORB();
	if (!_orb) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	_orb->m_pyramid = pyramid_;
	_orb->m_nPyramidLevels = COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS;
	COMPV_CHECK_CODE_RETURN(_orb->createInterestPoints(COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS));

	*orb = *_orb;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::createInterestPoints(size_t count COMPV_DEFAULT(0))
{
	size_t levelsCount = count > 0 ? count : m_nPyramidLevels;
	freeInterestPoints();
	m_ppInterestPointsAtLevelN = reinterpret_cast<CompVInterestPointVector*>(CompVMem::calloc(levelsCount, sizeof(CompVInterestPointVector)));
	COMPV_CHECK_EXP_RETURN(!m_ppInterestPointsAtLevelN, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::freeInterestPoints(size_t count COMPV_DEFAULT(0))
{
	if (m_ppInterestPointsAtLevelN) {
		size_t levelsCount = count > 0 ? count : m_nPyramidLevels;
		for (int i = 0; i < levelsCount; ++i) {
			m_ppInterestPointsAtLevelN[i].clear();
		}
		CompVMem::free(reinterpret_cast<void**>(&m_ppInterestPointsAtLevelN));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::createPatches(size_t count COMPV_DEFAULT(0))
{
	size_t patchesCount = count > 0 ? count : m_nPatches;
	freePatches();
	m_ppPatches = reinterpret_cast<CompVPatchPtr*>(CompVMem::calloc(patchesCount, sizeof(CompVPatchPtr)));
	COMPV_CHECK_EXP_RETURN(!m_ppPatches, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	m_nPatches = patchesCount;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::freePatches(size_t count COMPV_DEFAULT(0))
{
	if (m_ppPatches) {
		size_t patchesCount = count > 0 ? count : m_nPatches;
		for (int i = 0; i < patchesCount; ++i) {
			m_ppPatches[i] = NULL;
		}
		CompVMem::free(reinterpret_cast<void**>(&m_ppPatches));
	}
	m_nPatches = 0;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::createDetectors(size_t count COMPV_DEFAULT(0))
{
	size_t detectorsCount = count > 0 ? count : m_nDetectors;
	freeDetectors();
	m_ppDetectors = reinterpret_cast<CompVCornerDetePtr*>(CompVMem::calloc(detectorsCount, sizeof(CompVCornerDetePtr)));
	COMPV_CHECK_EXP_RETURN(!m_ppDetectors, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	m_nDetectors = detectorsCount;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::initDetector(CompVCornerDetePtr detector)
{
	if (detector) {
		COMPV_CHECK_CODE_RETURN(detector->set(COMPV_FAST_SET_INT_THRESHOLD, &m_nThreshold, sizeof(m_nThreshold)));
		COMPV_CHECK_CODE_RETURN(detector->set(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, &m_bNMS, sizeof(m_bNMS)));
		COMPV_CHECK_CODE_ASSERT(detector->set(COMPV_FAST_SET_INT_FAST_TYPE, &m_nFastType, sizeof(m_nFastType)));
		// Do not set maxFeatures, let FAST detector returns all features then retain best (later Harris will be used to update scores)
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::initDetectors()
{
	if (m_ppDetectors) {
		for (size_t i = 0; i < m_nDetectors; ++i) {
			COMPV_CHECK_CODE_RETURN(initDetector(m_ppDetectors[i]));
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCornerDeteORB::freeDetectors(size_t count COMPV_DEFAULT(0))
{
	if (m_ppDetectors) {
		size_t detectorsCount = count > 0 ? count : m_nDetectors;
		for (size_t i = 0; i < detectorsCount; ++i) {
			m_ppDetectors[i] = NULL;
		}
		CompVMem::free(reinterpret_cast<void**>(&m_ppDetectors));
	}
	m_nDetectors = 0;
	return COMPV_ERROR_CODE_S_OK;
}

// Private function
// Must be thread-safe
COMPV_ERROR_CODE CompVCornerDeteORB::processLevelAt(const CompVMatPtr& image, CompVPatchPtr& patch, CompVCornerDetePtr& detector, int level)
{
	COMPV_CHECK_EXP_RETURN(level < 0 || level >= m_nPyramidLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVMatPtr imageAtLevelN;
	float sf, sfs, patchSize, nf, orientRad;
	int m10, m01;
	CompVInterestPointVector::iterator point_;
	int patch_diameter = m_nPatchDiameter, patch_radius = (patch_diameter >> 1);
	const uint8_t* imgPtr;
	int imgWidth, imgHeight, imgStride;

	// Scale the image for the current level, multi-threaded and thread safe
	COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image, level));

	// Get image at level N
	COMPV_CHECK_CODE_RETURN(m_pyramid->image(level, &imageAtLevelN));
	imgPtr = imageAtLevelN->ptr<const uint8_t>();
	imgWidth = static_cast<int>(imageAtLevelN->cols());
	imgStride = static_cast<int>(imageAtLevelN->stride());
	imgHeight = static_cast<int>(imageAtLevelN->rows());

	sfs = m_pyramid->scaleFactorsSum();
	sf = m_pyramid->scaleFactor(level);

	patchSize = m_nPatchDiameter / sf; // TODO(dmi): in OpenCV the patch size increase (instead of decreasing) with the level. Doesn't look correct.
	// Clear previous points, will be done by the internal detector but we prefer do to it here to make sure it
	// will work for buggy detectors

	// Create or reset interest points for the current level then perform feature detection
	CompVInterestPointVector& interestPointsAtLevelN = m_ppInterestPointsAtLevelN[level];
	interestPointsAtLevelN.clear();
	
	// Detect features for level N (multi-threaded)
	// For example, "detector" would be FAST feature detector
	COMPV_CHECK_CODE_RETURN(detector->process(imageAtLevelN, interestPointsAtLevelN));

	// Retain best features only
	if (m_nMaxFeatures > 0 && interestPointsAtLevelN.size() > 0) {
		nf = ((m_nMaxFeatures / sfs) * sf);
		int32_t maxFeatures = COMPV_MATH_ROUNDFU_2_INT(nf, int32_t);
		maxFeatures = COMPV_MATH_MAX(COMPV_FEATURE_DETE_ORB_MIN_FEATUES_PER_LEVEL, maxFeatures);
		if (interestPointsAtLevelN.size() > static_cast<size_t>(maxFeatures)) {
			CompVInterestPoint::selectBest(interestPointsAtLevelN, static_cast<size_t>(maxFeatures));
		}
	}

	// Erase points too close to the border
	CompVInterestPoint::eraseTooCloseToBorder(interestPointsAtLevelN, static_cast<size_t>(imgWidth), static_cast<size_t>(imgHeight), patch_radius);

	// For each point, set level and patch size, compute the orientation, scale (X,Y) coords...
	for (point_ = interestPointsAtLevelN.begin(); point_ < interestPointsAtLevelN.end(); ++point_) {
		point_->level = level;
		point_->size = patchSize;

		// computes moments (TODO(dmi): use COMPV_MATH_ROUNDF_2_INT to cast x and y)
		patch->moments0110(imgPtr, static_cast<int>(point_->x), static_cast<int>(point_->y), imgWidth, imgStride, imgHeight, &m01, &m10);

		// compute orientation
		orientRad = COMPV_MATH_ATAN2F(static_cast<float>(m01), static_cast<float>(m10));
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

COMPV_NAMESPACE_END()