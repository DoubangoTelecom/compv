#include <compv/compv_api.h>

using namespace compv;

#define THRESHOLD			10
#define NONMAXIMA			true
#define JPEG_IMG			"C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg"
#define FAST_LOOP_COUNT		1

bool TestFAST()
{
    CompVObjWrapper<CompVFeatureDete* > fast;
    CompVObjWrapper<CompVImage *> image;
    std::vector<CompVInterestPoint > interestPoints;
    int32_t val32;
    bool valBool;
	uint64_t timeStart, timeEnd;

    // Decode the jpeg image
    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    // Convert the image to grayscal (required by feture detectors)
    COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

    // Create the FAST feature detector
    COMPV_CHECK_CODE_ASSERT(CompVFeatureDete::newObj(COMPV_FAST_ID, &fast));

    // Set the default values
    val32 = THRESHOLD;
	COMPV_CHECK_CODE_ASSERT(fast->set(COMPV_FAST_SET_INT32_THRESHOLD, &val32, sizeof(val32)));
    valBool = NONMAXIMA;
	COMPV_CHECK_CODE_ASSERT(fast->set(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, &valBool, sizeof(valBool)));

    // Detect keypoints
	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < FAST_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_ASSERT(fast->process(image, interestPoints));
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

    return true;
}