#include <compv/compv_api.h>

using namespace compv;

#define THRESHOLD	10
#define NONMAXIMA	true
#define JPEG_IMG  "C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg"

bool TestFAST()
{
    CompVObjWrapper<CompVFeatureDete* > fast;
    CompVObjWrapper<CompVImage *> image;
    std::vector<CompVInterestPoint > interestPoints;
    int32_t val32;
    bool valBool;

    // Decode the jpeg image
    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    // Convert the image to grayscal (required by feture detectors)
    COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

    // Create the FAST feature detector
    COMPV_CHECK_CODE_ASSERT(CompVFeatureDete::newObj(COMPV_FEATURE_DETE_ID_FAST, &fast));

    // Set the default values
    val32 = THRESHOLD;
    COMPV_CHECK_CODE_ASSERT(fast->set(COMPV_SET_INT32_FAST_THRESHOLD, &val32, sizeof(val32)));
    valBool = NONMAXIMA;
    COMPV_CHECK_CODE_ASSERT(fast->set(COMPV_SET_BOOL_FAST_NON_MAXIMA_SUPP, &valBool, sizeof(valBool)));

    // Detect keypoints
    COMPV_CHECK_CODE_ASSERT(fast->process(image, interestPoints));

    return true;
}