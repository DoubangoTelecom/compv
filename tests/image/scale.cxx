#include <compv/compv_api.h>
#include "../common.h"

//#define JPEG_IMG  "C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg" // OpenGL livre
#define JPEG_IMG	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

using namespace compv;

#define IMAGE_SCALE_FACTOR					0.83f // Values with MD5 check: 0.5f, 0.83f, 1.5f, 3.f
#define IMAGE_SCALE_TYPE					COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_SACLE_LOOP_COUNT				1
#define IMAGE_BILINEAR_FACTOR050_MD5_STRING	"1a97693193c8df7919f70a6869af3fa9" // MD5 for factor=0.50
#define IMAGE_BILINEAR_FACTOR083_MD5_STRING	"9777b441875d4f75c24fae8c1816c50d" // MD5 for factor=0.83
#define IMAGE_BILINEAR_FACTOR150_MD5_STRING	"57eae70543a3f8944c6699e8ee97f7e2" // MD5 for factor=1.50
#define IMAGE_BILINEAR_FACTOR300_MD5_STRING	"ae2a90b1560b463cd05cb24e95537c74" // MD5 for factor=3.00

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
    COMPV_DEBUG_INFO("Elapsed time(TestScale) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    COMPV_DEBUG_INFO("Image scaling: factor=%f, outWidth=%d, outStride=%d, outHeight=%d", IMAGE_SCALE_FACTOR, image->getWidth(), image->getStride(), image->getHeight());

    // dump image to file
    writeImgToFile(image);

    std::string expectedMD5 = imageMD5(image);
    if (IMAGE_SCALE_FACTOR == 0.5f) {
        expectedMD5 = IMAGE_BILINEAR_FACTOR050_MD5_STRING;
    }
    else if (IMAGE_SCALE_FACTOR == 0.83f) {
        expectedMD5 = IMAGE_BILINEAR_FACTOR083_MD5_STRING;
    }
    else if (IMAGE_SCALE_FACTOR == 1.5f) {
        expectedMD5 = IMAGE_BILINEAR_FACTOR150_MD5_STRING;
    }
    else if (IMAGE_SCALE_FACTOR == 3.0f) {
        expectedMD5 = IMAGE_BILINEAR_FACTOR300_MD5_STRING;
    }
    if (!expectedMD5.empty()) {
        if (imageMD5(image) != expectedMD5) {
            COMPV_DEBUG_ERROR("MD5 mismatch");
            COMPV_ASSERT(false);
            return false;
        }
    }
    else {
        COMPV_DEBUG_INFO("/!\\ Not checking MD5");
    }

    return true;
}