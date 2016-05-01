#include <compv/compv_api.h>
#include "../common.h"

//#define JPEG_IMG  "C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg" // OpenGL livre
#define JPEG_IMG	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

using namespace compv;

#define IMAGE_SCALE_FACTOR					0.50f
#define IMAGE_SCALE_TYPE					COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_SACLE_LOOP_COUNT				1
#define IMAGE_BILINEAR_FACTOR83_MD5_STRING	"e1b919373264838dfb1e0a39dbb69191" // MD5 for factor=0.83
#define IMAGE_BILINEAR_FACTOR50_MD5_STRING	"8b04db64da0b882e255e7026d267cce4" // MD5 for factor=0.50

bool TestScale()
{
    CompVPtr<CompVImage *> image;
    uint64_t timeStart, timeEnd;

    // Decode the jpeg image
    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    int32_t outWidth = (int32_t)(image->getWidth() * IMAGE_SCALE_FACTOR);
    int32_t outHeight = (int32_t)(image->getHeight() * IMAGE_SCALE_FACTOR);
    // Convert image to GrayScale
    COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
    // Scale the image
    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < IMAGE_SACLE_LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_ASSERT(image->scale(IMAGE_SCALE_TYPE, outWidth, outHeight, &image));
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

    COMPV_DEBUG_INFO("Image scaling: factor=%f, outWidth=%d, outStride=%d, outHeight=%d", IMAGE_SCALE_FACTOR, image->getWidth(), image->getStride(), image->getHeight());

    const std::string expectedMD5 = (IMAGE_SCALE_FACTOR == 0.83f) ? IMAGE_BILINEAR_FACTOR83_MD5_STRING : ((IMAGE_SCALE_FACTOR == 0.5f ? IMAGE_BILINEAR_FACTOR50_MD5_STRING : ""));
    if (!expectedMD5.empty()) {
        if (imageMD5(image) != expectedMD5) {
            COMPV_DEBUG_ERROR("MD5 mismatch");
            COMPV_ASSERT(false);
            return false;
        }
    }

    // dump image to file
    writeImgToFile(image);

    return true;
}