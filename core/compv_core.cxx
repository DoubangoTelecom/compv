/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/compv_core.h"
#include "compv/core/compv_core_common.h"
#include "compv/core/features/hough/compv_core_feature_houghsht.h"
#include "compv/core/features/hough/compv_core_feature_houghkht.h"
#include "compv/core/features/hog/compv_core_feature_hog_std.h"
#include "compv/core/features/hog/compv_core_feature_hog_t.h"
#include "compv/core/features/edges/compv_core_feature_canny_dete.h"
#include "compv/core/features/edges/compv_core_feature_edge_dete.h"
#include "compv/core/features/fast/compv_core_feature_fast_dete.h"
#include "compv/core/features/orb/compv_core_feature_orb_dete.h"
#include "compv/core/features/orb/compv_core_feature_orb_desc.h"
#include "compv/core/ccl/compv_core_ccl_lsl.h"
#include "compv/core/ccl/compv_core_ccl_lmser.h"
#include "compv/core/matchers/compv_core_matcher_bruteforce.h"
#include "compv/core/video/compv_core_video_reader_ffmpeg.h"
#include "compv/core/video/compv_core_video_writer_ffmpeg.h"
#include "compv/base/compv_matchers.h"
#include "compv/base/compv_base.h"
#include "compv/base/parallel/compv_mutex.h"

#if defined(HAVE_FFMPEG)
COMPV_EXTERNC_BEGIN()
#	include <libavformat/avformat.h>
COMPV_EXTERNC_END()
#endif

#define COMPV_THIS_CLASSNAME "CompVCore"

COMPV_NAMESPACE_BEGIN()

bool CompVCore::s_bInitialized = false;

// Declare built-in factories
static const CompVFeatureFactory fastFactory = {
	COMPV_FAST_ID,
	"FAST (Features from Accelerated Segment Test)",
	CompVCornerDeteFAST::newObj,
	nullptr,
	nullptr,
	nullptr,
};
static const CompVFeatureFactory orbFactory = {
	COMPV_ORB_ID,
	"ORB (Oriented FAST and Rotated BRIEF)",
	CompVCornerDeteORB::newObj,
	CompVCornerDescORB::newObj,
	nullptr,
	nullptr,
};
static const CompVFeatureFactory cannyFactory = {
	COMPV_CANNY_ID,
	"Canny edge detector",
	nullptr,
	nullptr,
	CompVEdgeDeteCanny::newObj,
	nullptr,
};
static const CompVFeatureFactory sobelFactory = {
	COMPV_SOBEL_ID,
	"Sobel edge detector",
	nullptr,
	nullptr,
	CompVCornerDeteEdgeBase::newObjSobel,
	nullptr,
};
static const CompVFeatureFactory scharrFactory = {
	COMPV_SCHARR_ID,
	"Scharr edge detector",
	nullptr,
	nullptr,
	CompVCornerDeteEdgeBase::newObjScharr,
	nullptr,
};
static const CompVFeatureFactory prewittFactory = {
	COMPV_PREWITT_ID,
	"Prewitt edge detector",
	nullptr,
	nullptr,
	CompVCornerDeteEdgeBase::newObjPrewitt,
	nullptr,
};
static const CompVFeatureFactory houghStdFactory = {
	COMPV_HOUGHSHT_ID,
	"Hough standard (STD)",
	nullptr,
	nullptr,
	nullptr,
	CompVHoughSht::newObj,
};
static const CompVFeatureFactory houghKhtFactory = {
	COMPV_HOUGHKHT_ID,
	"Kernel-based Hough transform (KHT)",
	nullptr,
	nullptr,
	nullptr,
	CompVHoughKht::newObj,
};

static const CompVFeatureFactory hogStdFactory = {
	COMPV_HOGS_ID,
	"Standard Histogram of oriented gradients (S-HOG)",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	CompVHogStd::newObj,
};

static const CompVMatcherFactory bruteForceFactory = {
	COMPV_BRUTEFORCE_ID,
	"Brute force matcher",
	CompVMatcherBruteForce::newObj
};

static const CompVConnectedComponentLabelingFactory lslFactory = {
	COMPV_PLSL_ID,
	"PLSL (Parallel Light Speed Labeling)",
	CompVConnectedComponentLabelingLSL::newObj,
};

static const CompVConnectedComponentLabelingFactory lmserFactory = {
	COMPV_LMSER_ID,
	"LMSER (Linear Time Maximally Stable Extremal Regions)",
	CompVConnectedComponentLabelingLMSER::newObj,
};

COMPV_ERROR_CODE CompVCore::init()
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	if (isInitialized()) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing [core] module (v %s)...", COMPV_VERSION_STRING);

#if defined(HAVE_FFMPEG)
	av_register_all();
#endif /* HAVE_FFMPEG */

	// Features
	COMPV_CHECK_CODE_BAIL(err = CompVFeature::addFactory(&fastFactory), "Failed to add FAST feature factory");
	COMPV_CHECK_CODE_BAIL(err = CompVFeature::addFactory(&orbFactory), "Failed to add ORB feature factory");
	COMPV_CHECK_CODE_RETURN(err = CompVFeature::addFactory(&sobelFactory), "Failed to add Sobel edge detector factory");
	COMPV_CHECK_CODE_RETURN(err = CompVFeature::addFactory(&scharrFactory), "Failed to add Scharr edge detector factory");
	COMPV_CHECK_CODE_RETURN(err = CompVFeature::addFactory(&prewittFactory), "Failed to add Prewitt edge detector factory");
	COMPV_CHECK_CODE_RETURN(err = CompVFeature::addFactory(&cannyFactory), "Failed to add Canny edge detector factory");
	COMPV_CHECK_CODE_RETURN(err = CompVFeature::addFactory(&houghStdFactory), "Failed to add Hough standard line detector factory");
	COMPV_CHECK_CODE_RETURN(err = CompVFeature::addFactory(&houghKhtFactory), "Failed to add Hough kht line detector factory");
	COMPV_CHECK_CODE_RETURN(err = CompVFeature::addFactory(&hogStdFactory), "Failed to add HOG standard descriptor factory");

	// Matchers
	COMPV_CHECK_CODE_BAIL(err = CompVMatcher::addFactory(&bruteForceFactory), "Failed to add bruteforce matcher factory");

	// Connected Component Labeling
	COMPV_CHECK_CODE_BAIL(err = CompVConnectedComponentLabeling::addFactory(&lslFactory), "Failed to add 'Parallel Light Speed Labeling' factory");
	COMPV_CHECK_CODE_BAIL(err = CompVConnectedComponentLabeling::addFactory(&lmserFactory), "Failed to add 'Linear Time Maximally Stable Extremal Regions' factory");

	// Video Readers and Writers
#if defined(HAVE_FFMPEG)
	COMPV_CHECK_CODE_BAIL(err = CompVVideoReaderFactory::set(&CompVVideoReaderFactoryFFmpeg));
#endif /* HAVE_FFMPEG */

	s_bInitialized = true;

bail:
	return err;
}

COMPV_ERROR_CODE CompVCore::deInit()
{
	COMPV_CHECK_CODE_ASSERT(CompVBase::deInit());
	s_bInitialized = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
