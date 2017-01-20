#include "../tests_common.h"

#define TAG_TEST								"TestImageScale"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

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

#define LOOP_COUNT		1000
#define SCALE_TYPE		COMPV_SCALE_TYPE_BILINEAR
#define FILE_NAME		FILE_NAME_EQUIRECTANGULAR
#define FACTOR			0.83f


COMPV_ERROR_CODE scale()
{
	COMPV_ERROR_CODE err;
	uint64_t timeStart, timeEnd;
	CompVMatPtr srcImage, dstImage;
	const compv_unittest_scale* test = NULL;
	size_t widthOut, heightOut;

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_SCALE_COUNT; ++i) {
		if (COMPV_UNITTEST_SCALE[i].type == SCALE_TYPE
			&& std::string(COMPV_UNITTEST_SCALE[i].filename).compare(FILE_NAME) == 0
			&& COMPV_UNITTEST_SCALE[i].factor == FACTOR) {
			test = &COMPV_UNITTEST_SCALE[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}
	
	// Read params
	COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &srcImage));
	widthOut = static_cast<size_t>(test->width * test->factor);
	heightOut = static_cast<size_t>(test->height * test->factor);

	// Perform test
	timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_BAIL(err = CompVImage::scale(srcImage, &dstImage, widthOut, heightOut, test->type));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 1
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_BAIL(err = compv_tests_write_to_file(dstImage, "out.gray"));
#endif

	COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(dstImage)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image scaling MD5 mismatch");

bail:
	return err;
}
