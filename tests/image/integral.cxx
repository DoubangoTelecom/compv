#include "../tests_common.h"

#define TAG_TEST							"TestIntegral"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

static const struct compv_unittest_integral {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5_sum;
	const char* md5_sumsq;
}
COMPV_UNITTEST_INTEGRALS[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "b2bf5ddd5aaaa7a5005ce36e3020689c", "09590699ea6d9b6b30fd5ec67122266f" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, "2115b3b135b5d0eddacaf5900496de5a", "7638deada88d4bb73dc00f9f33addfc0" },
	{ FILE_NAME_GRIOTS, 480, 640, 480, "4c2869ef980fba12407bdaa89b3d2b96", "ea17dbdf0777ef16fa441044a8e3127f" }
};
static const size_t COMPV_UNITTEST_INTEGRALS_COUNT = sizeof(COMPV_UNITTEST_INTEGRALS) / sizeof(COMPV_UNITTEST_INTEGRALS[0]);


#define LOOP_COUNT			1
#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR

COMPV_ERROR_CODE integral()
{
	CompVMatPtr image, sum, sumsq;
	const compv_unittest_integral* test = nullptr;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_INTEGRALS_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_INTEGRALS[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_INTEGRALS[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	// Read file
	CompVMatPtr imageIn;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));

	// Process
	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImage::integral(imageIn, &sum, &sumsq));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_EXP_RETURN(std::string(test->md5_sum).compare(compv_tests_md5(sum)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Integral MD5_sum mismatch");
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_sumsq).compare(compv_tests_md5(sumsq)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Integral MD5_sumsq mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
