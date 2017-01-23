/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/compv_core.h"
#include "compv/core/compv_core_common.h"
#include "compv/core/features/fast/compv_core_feature_fast_dete.h"
#include "compv/base/compv_features.h"
#include "compv/base/compv_features.h"
#include "compv/base/compv_base.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/compv_simd_globals.h"



#define COMPV_THIS_CLASSNAME "CompVCore"

COMPV_NAMESPACE_BEGIN()

bool CompVCore::s_bInitialized = false;

// Declare built-in factories
static const CompVFeatureFactory fastFactory = {
	COMPV_FAST_ID,
	"FAST (Features from Accelerated Segment Test)",
	CompVCornerDeteFAST::newObj,
	NULL,
	NULL,
	NULL,
};
//static const CompVFeatureFactory orbFactory = {
//	COMPV_ORB_ID,
//	"ORB (Oriented FAST and Rotated BRIEF)",
//	CompVCornerDeteORB::newObj,
//	CompVCornerDescORB::newObj,
//	NULL,
//	NULL,
//};
//static const CompVFeatureFactory cannyFactory = {
//	COMPV_CANNY_ID,
//	"Canny edge detector",
//	NULL,
//	NULL,
//	CompVEdgeDeteCanny::newObj,
//	NULL,
//};
//static const CompVFeatureFactory sobelFactory = {
//	COMPV_SOBEL_ID,
//	"Sobel edge detector",
//	NULL,
//	NULL,
//	CompVEdgeDeteBASE::newObjSobel,
//	NULL,
//};
//static const CompVFeatureFactory scharrFactory = {
//	COMPV_SCHARR_ID,
//	"Scharr edge detector",
//	NULL,
//	NULL,
//	CompVEdgeDeteBASE::newObjScharr,
//	NULL,
//};
//static const CompVFeatureFactory prewittFactory = {
//	COMPV_PREWITT_ID,
//	"Prewitt edge detector",
//	NULL,
//	NULL,
//	CompVEdgeDeteBASE::newObjPrewitt,
//	NULL,
//};
//static const CompVFeatureFactory houghStdFactory = {
//	COMPV_HOUGH_STANDARD_ID,
//	"Hough standard",
//	NULL,
//	NULL,
//	NULL,
//	CompVHoughStd::newObj,
//};

COMPV_ERROR_CODE CompVCore::init()
{
	if (s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing [core] module (v %s)...", COMPV_VERSION_STRING);

	COMPV_CHECK_CODE_BAIL(err = CompVBase::init());

	COMPV_CHECK_CODE_BAIL(err = CompVFeature::addFactory(&fastFactory), "Failed to add FAST feature factory");

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
