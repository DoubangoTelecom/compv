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
#include "compv/base/image/compv_image.h"
#include "compv/base/parallel/compv_parallel.h"

COMPV_NAMESPACE_BEGIN()

// These default values are also used in the descriptor (extern...)
int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT = 20; // T: Default threshold (pixel intensity: [0-255])
int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT = 9; // N: Number of positive continuous pixel to have before declaring a candidate as an interest point
int COMPV_FEATURE_DETE_ORB_FAST_MAX_FEATURES = 1000; // #2000 is known to provide to provide good result but #1000 is also acceptable
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
	, m_nThreshold(COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT)
	, m_nFastType(COMPV_FAST_TYPE_9)
	, m_bNMS(COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP)
	, m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
{

}

CompVCornerDeteORB::~CompVCornerDeteORB()
{
	m_vecDetectors.clear();
	m_vecInterestPointsAtLevelN.clear();
	m_vecPatches.clear();
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
	case COMPV_ORB_SET_INT_INTERNAL_DETE_ID: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nFastType = *reinterpret_cast<const int*>(valuePtr);
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
					m_vecInterestPointsAtLevelN.clear();
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
	
COMPV_ERROR_CODE CompVCornerDeteORB::process(const CompVMatPtr& image_, CompVInterestPointVector& interestPoints) /*Overrides(CompVCornerDete)*/
{
	COMPV_CHECK_EXP_RETURN(!image_ || image_->isEmpty(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Convert the image to grayscale if not already the case
	CompVMatPtr image = image_;
	if (image_->subType() != COMPV_SUBTYPE_PIXELS_Y) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should convert the image to grayscale once and reuse it in all functions");
		COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image_, &m_ptrImageGray), "Failed to convert the image to grayscale");
		image = m_ptrImageGray;
	}

	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVInterestPointVector interestPointsAtLevelN;
	CompVMatPtr imageAtLevelN;

	// Clear old points
	interestPoints.clear();

	// Image scaling then feature could be multi-threaded but this requires a detector for each level -> memory issue

	const size_t levelsCount = m_pyramid->levels();

	// Compute number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	// The multi-threading is done across the levels using function "processLevelAt" and the most consuming part
	// is the internal FAST detector which is already multi-threaded.
	// This means it worth multi-threading across levels only if we have enough levels compared to maxThreads.
	const size_t threadsCount = (levelsCount > (maxThreads >> 2)) ? COMPV_MATH_MIN(maxThreads, levelsCount) : 1;

	// Create and init detectors
	// Number of detectors must be equal to the number of threads (not the case for interestpoints array which is equal to the number of levels)
	const size_t nDetectors = COMPV_MATH_CLIP3(1, threadsCount, levelsCount);
	if (m_vecDetectors.size() < nDetectors) {
		CompVCornerDetePtr dete;
		for (size_t d = m_vecDetectors.size(); d < nDetectors; ++d) {
			COMPV_CHECK_CODE_RETURN(err_ = CompVCornerDete::newObj(&dete, COMPV_FAST_ID));
			m_vecDetectors.push_back(dete);
		}
		COMPV_CHECK_CODE_RETURN(initDetectors());
	}

	// Create patches
	// Number of patches must be equal to the number of threads (not the case for interestpoints array which is equal to the number of levels)
	const size_t nPatches = COMPV_MATH_CLIP3(1, threadsCount, levelsCount);
	if (m_vecPatches.size() < nPatches) {
		CompVPatchPtr patch;
		for (size_t p = m_vecPatches.size(); p < nPatches; ++p) {
			COMPV_CHECK_CODE_RETURN(err_ = CompVPatch::newObj(&patch, m_nPatchDiameter));
			m_vecPatches.push_back(patch);
		}
	}
	
	if (threadsCount > 1) {
		CompVCornerDeteORBPtr This = this;
		// levelStart is used to make sure we won't schedule more than "threadsCount"
		size_t levelStart, level, levelMax;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(m_pyramid->levels());
		auto funcPtr = [&](const CompVMatPtr& image_, CompVPatchPtr& patch_, CompVCornerDetePtr& detector_, int level_) -> COMPV_ERROR_CODE {
			return processLevelAt(image_, patch_, detector_, level_);
		};
		for (levelStart = 0, levelMax = threadsCount; levelStart < levelsCount; levelStart += threadsCount, levelMax += threadsCount) {
			for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
				COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, image, m_vecPatches[level % nPatches], m_vecDetectors[level % nDetectors], static_cast<int>(level)), taskIds));
			}
			for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
				COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[level]));
				interestPoints.insert(interestPoints.end(), m_vecInterestPointsAtLevelN[level].begin(), m_vecInterestPointsAtLevelN[level].end());
			}
		}
	}
	else {
		for (size_t level = 0; level < levelsCount; ++level) {
			COMPV_CHECK_CODE_RETURN(processLevelAt(image, m_vecPatches[0], m_vecDetectors[0], static_cast<int>(level)));
			interestPoints.insert(interestPoints.end(), m_vecInterestPointsAtLevelN[level].begin(), m_vecInterestPointsAtLevelN[level].end());
		}
	}

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
	for (size_t i = m_vecInterestPointsAtLevelN.size(); i < levelsCount; ++i) {
		m_vecInterestPointsAtLevelN.push_back(CompVInterestPointVector());
	}
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
	for (size_t d = 0; d < m_vecDetectors.size(); ++d) {
		COMPV_CHECK_CODE_RETURN(initDetector(m_vecDetectors[d]));
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Private function (*must* be thread-safe)
COMPV_ERROR_CODE CompVCornerDeteORB::processLevelAt(const CompVMatPtr& image, CompVPatchPtr& patch, CompVCornerDetePtr& detector, int level)
{
	COMPV_CHECK_EXP_RETURN(level < 0 || level >= m_nPyramidLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr imageAtLevelN;
	float orientRad;
	int m10, m01;
	CompVInterestPointVector::iterator point_;
	int patch_diameter = m_nPatchDiameter;
	const uint8_t* imgPtr;
	int imgWidth, imgHeight, imgStride;

	// Scale the image for the current level, multi-threaded and thread safe
	COMPV_CHECK_CODE_RETURN(m_pyramid->process(image, level));

	// Get image at level N
	COMPV_CHECK_CODE_RETURN(m_pyramid->image(level, &imageAtLevelN));
	imgPtr = imageAtLevelN->ptr<const uint8_t>();
	imgWidth = static_cast<int>(imageAtLevelN->cols());
	imgStride = static_cast<int>(imageAtLevelN->stride());
	imgHeight = static_cast<int>(imageAtLevelN->rows());

	const float sfs = m_pyramid->scaleFactorsSum();
	const float sf = m_pyramid->scaleFactor(level);
	const float sfi = 1.f / sf; // inverse sf to avoid dividing by sf

	const float patchSize = m_nPatchDiameter / sf;
	// Clear previous points, will be done by the internal detector but we prefer do to it here to make sure it
	// will work for buggy detectors

	// Create or reset interest points for the current level then perform feature detection
	CompVInterestPointVector& interestPointsAtLevelN = m_vecInterestPointsAtLevelN[level];
	interestPointsAtLevelN.clear();
	
	// Detect features for level N (multi-threaded)
	// For example, "detector" would be FAST feature detector
	COMPV_CHECK_CODE_RETURN(detector->process(imageAtLevelN, interestPointsAtLevelN));

	// Retain best features only
	if (m_nMaxFeatures > 0 && interestPointsAtLevelN.size() > 0) {
		const float nf = ((m_nMaxFeatures / sfs) * sf);
		int32_t maxFeatures = COMPV_MATH_ROUNDFU_2_NEAREST_INT(nf, int32_t);
		maxFeatures = COMPV_MATH_MAX(COMPV_FEATURE_DETE_ORB_MIN_FEATUES_PER_LEVEL, maxFeatures);
		if (interestPointsAtLevelN.size() > static_cast<size_t>(maxFeatures)) {
			CompVInterestPoint::selectBest(interestPointsAtLevelN, static_cast<size_t>(maxFeatures));
		}
	}

	// Erase points too close to the border
	CompVInterestPoint::eraseTooCloseToBorder(interestPointsAtLevelN, static_cast<size_t>(imgWidth), static_cast<size_t>(imgHeight), ((patch_diameter + 5) >> 1));

	// For each point, set level and patch size, compute the orientation, scale (X,Y) coords...
	for (point_ = interestPointsAtLevelN.begin(); point_ < interestPointsAtLevelN.end(); ++point_) {
		point_->level = level;
		point_->size = patchSize;

		// computes moments
		patch->moments0110(imgPtr, COMPV_MATH_ROUNDF_2_NEAREST_INT(point_->x, int), COMPV_MATH_ROUNDF_2_NEAREST_INT(point_->y, int), imgWidth, imgStride, imgHeight, &m01, &m10);

		// compute orientation
		orientRad = COMPV_MATH_ATAN2(static_cast<float>(m01), static_cast<float>(m10));
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
			point_->x *= sfi;
			point_->y *= sfi;
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()