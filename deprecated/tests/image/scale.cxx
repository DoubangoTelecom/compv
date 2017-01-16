#if 1
#include <compv/compv_api.h>
#include "../common.h"

#define JPEG_IMG_EQUIRECTANGULAR	"C:/Projects/GitHub/data/test_images/equirectangular_1282x720_gray.yuv" // "C:/Projects/compv/tests/7019363969_a80a5d6acc_o.jpg" // "/Users/mamadou/Documents/compv/tests/7019363969_a80a5d6acc_o.jpg" //  // voiture
#define JPEG_IMG_OPENGLBOOK			"C:/Projects/GitHub/data/test_images/opengl_programming_guide_8th_edition_200x258_gray.yuv" // OpenGL book
#define JPEG_IMG_GRIOTS				"C:/Projects/GitHub/data/test_images/mandekalou_480x640_gray.yuv" // Mande Griots

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
	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, JPEG_IMG_EQUIRECTANGULAR, 1282, 720, 1282, "6b2068a9654b5446aeac9edadb7eea2a" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, JPEG_IMG_EQUIRECTANGULAR, 1282, 720, 1282, "e63616c7c1307268b5447c41906d2cd1" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, JPEG_IMG_EQUIRECTANGULAR, 1282, 720, 1282, "cf1041696dc70ed595306c21968e2a60" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, JPEG_IMG_EQUIRECTANGULAR, 1282, 720, 1282, "93d0e529581ecea87746076072babefc" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, JPEG_IMG_OPENGLBOOK, 200, 258, 200, "36f66634d0301becaf76478b53d234ec" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, JPEG_IMG_OPENGLBOOK, 200, 258, 200, "039918770e7157c8b10b6bb45e4aa3b4" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, JPEG_IMG_OPENGLBOOK, 200, 258, 200, "a535d1a0b7768cf3532fb48a7d3da234" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, JPEG_IMG_OPENGLBOOK, 200, 258, 200, "8362de24ca64dea651ca50fe2700f23a" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, JPEG_IMG_GRIOTS, 480, 640, 480, "7dee6e6e42a33c8ceb49f4a8a8cb7a6c" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, JPEG_IMG_GRIOTS, 480, 640, 480, "1e87b23e7736729769f8b252a8cf701e" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, JPEG_IMG_GRIOTS, 480, 640, 480, "4053ab9cb684573f17e9fc52265a6a32" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, JPEG_IMG_GRIOTS, 480, 640, 480, "f7c688791368aa1d8c2d8f4c66c5c394" },
};
static const size_t COMPV_UNITTEST_SCALE_COUNT = sizeof(COMPV_UNITTEST_SCALE) / sizeof(COMPV_UNITTEST_SCALE[0]);

COMPV_ERROR_CODE TestScale()
{
	CompVPtr<CompVImage *> image;
	CompVPtr<CompVBuffer *> buffer;
	uint64_t timeStart, timeEnd;
	
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(JPEG_IMG_EQUIRECTANGULAR, &buffer));
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
#if 0
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
#endif
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