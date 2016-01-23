#include <compv/compv_api.h>

using namespace compv;

#define FAST_THRESHOLD				10
#define FAST_NONMAXIMA				true
#define ORB_PYRAMID_LEVELS			8
#define ORB_PYRAMID_SCALEFACTOR		0.83f
#define ORB_PYRAMID_SCALE_TYPE		COMPV_SCALE_TYPE_BILINEAR
#define JPEG_IMG  "C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg"

bool TestORB()
{
	CompVObjWrapper<CompVFeatureDete* > orb; // feature detector
	CompVObjWrapper<CompVImage *> image;
	std::vector<CompVInterestPoint > interestPoints;
	int32_t val32;
	bool valBool;
	float valFloat;

	// Decode the jpeg image
	COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	// Convert the image to grayscal (required by feture detectors)
	COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	// Create the ORB feature detector
	COMPV_CHECK_CODE_ASSERT(CompVFeatureDete::newObj(COMPV_ORB_ID, &orb));

	// Set the default values
	val32 = FAST_THRESHOLD;
	COMPV_CHECK_CODE_ASSERT(orb->set(COMPV_ORB_SET_INT32_FAST_THRESHOLD, &val32, sizeof(val32)));
	valBool = FAST_NONMAXIMA;
	COMPV_CHECK_CODE_ASSERT(orb->set(COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP, &valBool, sizeof(valBool)));
	val32 = ORB_PYRAMID_LEVELS;
	COMPV_CHECK_CODE_ASSERT(orb->set(COMPV_ORB_SET_INT32_PYRAMID_LEVELS, &val32, sizeof(val32)));
	val32 = ORB_PYRAMID_SCALE_TYPE;
	COMPV_CHECK_CODE_ASSERT(orb->set(COMPV_ORB_SET_INT32_PYRAMID_SCALE_TYPE, &val32, sizeof(val32)));
	valFloat = ORB_PYRAMID_SCALEFACTOR;
	COMPV_CHECK_CODE_ASSERT(orb->set(COMPV_ORB_SET_FLOAT_PYRAMID_SCALE_FACTOR, &valFloat, sizeof(valFloat)));

	// Detect keypoints
	COMPV_CHECK_CODE_ASSERT(orb->process(image, interestPoints));

	return true;
}