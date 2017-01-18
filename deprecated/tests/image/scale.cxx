#if 1
#include <compv/compv_api.h>
#include "../common.h"

#define FILE_NAME_EQUIRECTANGULAR	"C:/Projects/GitHub/data/test_images/equirectangular_1282x720_gray.yuv" // "C:/Projects/compv/tests/7019363969_a80a5d6acc_o.jpg" // "/Users/mamadou/Documents/compv/tests/7019363969_a80a5d6acc_o.jpg" //  // voiture
#define FILE_NAME_OPENGLBOOK			"C:/Projects/GitHub/data/test_images/opengl_programming_guide_8th_edition_200x258_gray.yuv" // OpenGL book
#define FILE_NAME_GRIOTS				"C:/Projects/GitHub/data/test_images/mandekalou_480x640_gray.yuv" // Mande Griots

using namespace compv;

#define IMAGE_SCALE_FACTOR					0.83f // Values with MD5 check: 0.5f, 0.83f, 1.5f, 3.f
#define IMAGE_SCALE_TYPE					COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_SACLE_LOOP_COUNT				1
#define IMAGE_BILINEAR_FACTOR050_MD5_STRING	"1a97693193c8df7919f70a6869af3fa9" // MD5 for factor=0.50
#define IMAGE_BILINEAR_FACTOR083_MD5_STRING	"9777b441875d4f75c24fae8c1816c50d" // MD5 for factor=0.83
#define IMAGE_BILINEAR_FACTOR150_MD5_STRING	"57eae70543a3f8944c6699e8ee97f7e2" // MD5 for factor=1.50
#define IMAGE_BILINEAR_FACTOR300_MD5_STRING	"ae2a90b1560b463cd05cb24e95537c74" // MD5 for factor=3.00

static const struct compv_unittest_scale {
	COMPV_SCALE_TYPE type;
	float factor;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_SCALE[] =
{
	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "2199de43fe7cadd102a5f91cafef2e98" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "5cd9916ba99a280fc314c1767e16e7e8" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "a8321cce1364dfaf83e8b2fbf2ee6436" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "c0bca2ff876907af9c9568911128e0a0" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "3402c4bab7e3850cdb3b7df25025ad2b" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "a3290801d97b15d73fc523997acbdc70" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "9b7aad661ac2059fa5955e32f158a329" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "bebaa908c915d818f6d04e1105707485" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, FILE_NAME_GRIOTS, 480, 640, 480, "c44db54243b04d92ef7e9e373a8f88c5" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "1f67605aae23461222939da568dd7e29" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "b9461507269a7ddb7ce18bc2f1f836df" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "130ffba1d1bd05bd7a86067db6a5d082" },
};
static const size_t COMPV_UNITTEST_SCALE_COUNT = sizeof(COMPV_UNITTEST_SCALE) / sizeof(COMPV_UNITTEST_SCALE[0]);

COMPV_ERROR_CODE TestScale()
{
	CompVPtr<CompVImage *> image;
	CompVPtr<CompVBuffer *> buffer;
	uint64_t timeStart, timeEnd;
	
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(FILE_NAME_EQUIRECTANGULAR, &buffer));
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, buffer->getPtr(), 1282, 720, 1282, &image));
	int32_t outWidth = (int32_t)(image->getWidth() * IMAGE_SCALE_FACTOR);
	int32_t outHeight = (int32_t)(image->getHeight() * IMAGE_SCALE_FACTOR);
	// Scale the image
	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < IMAGE_SACLE_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(image->scale(IMAGE_SCALE_TYPE, outWidth, outHeight, &image));
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time(TestScale) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO("Image scaling: factor=%f, outWidth=%d, outStride=%d, outHeight=%d", IMAGE_SCALE_FACTOR, image->getWidth(), image->getStride(), image->getHeight());

	// dump image to file
	writeImgToFile(image);

	COMPV_CHECK_EXP_RETURN(imageMD5(image).compare("5cd9916ba99a280fc314c1767e16e7e8") != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	return COMPV_ERROR_CODE_S_OK;
}

#else
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

COMPV_ERROR_CODE TestScale()
{
    CompVPtr<CompVImage *> image;
    uint64_t timeStart, timeEnd;

    // Decode the jpeg image
    COMPV_CHECK_CODE_RETURN(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    int32_t outWidth = (int32_t)(image->getWidth() * IMAGE_SCALE_FACTOR);
    int32_t outHeight = (int32_t)(image->getHeight() * IMAGE_SCALE_FACTOR);
    // Convert image to GrayScale
    COMPV_CHECK_CODE_RETURN(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
    // Scale the image
    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < IMAGE_SACLE_LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_RETURN(image->scale(IMAGE_SCALE_TYPE, outWidth, outHeight, &image));
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
        COMPV_CHECK_EXP_RETURN(imageMD5(image) != expectedMD5, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
    }
    else {
        COMPV_DEBUG_INFO("/!\\ Not checking MD5");
    }

    return COMPV_ERROR_CODE_S_OK;
}
#endif