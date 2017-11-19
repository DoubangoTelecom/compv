#include "../tests/tests_common.h"

#define TAG_UNITTESTS								"UnitTestHistogram"
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

COMPV_ERROR_CODE unittest_histogram_build()
{
#define EXPECTED_MD5_BUILD		"9cc75b0ec5f1522cfc65b7fadc0aacdb"

	CompVMatPtr image, histogram;
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_EQUIRECTANGULAR).c_str(), &image));
	
	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "== Trying new test: histogram build on '%s'", FILE_NAME_EQUIRECTANGULAR);
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::build(image, &histogram));
	COMPV_CHECK_EXP_RETURN(compv_tests_md5(histogram).compare(EXPECTED_MD5_BUILD) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "MD5 mismatch");
	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Test OK **");

	return COMPV_ERROR_CODE_S_OK;
}


static const struct compv_unittest_histo_equaliz {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_HISTO_EQUALIZ[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "f1e7bb40dddfd90ba9dd877f566c8815" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, "4689da992f7d0e7f2a42b2c4a0ea79ca" },
	{ FILE_NAME_GRIOTS, 480, 640, 480, "8dc4e8234cabb09b6a0957b655f62b2d" },
};
static const size_t COMPV_UNITTEST_HISTO_EQUALIZ_COUNT = sizeof(COMPV_UNITTEST_HISTO_EQUALIZ) / sizeof(COMPV_UNITTEST_HISTO_EQUALIZ[0]);

#define EQUALIZ_FILE	FILE_NAME_EQUIRECTANGULAR

COMPV_ERROR_CODE unittest_histogram_equaliz()
{
	for (size_t i = 0; i < COMPV_UNITTEST_HISTO_EQUALIZ_COUNT; ++i) {
		const compv_unittest_histo_equaliz* test = &COMPV_UNITTEST_HISTO_EQUALIZ[i];
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "== Trying new test: histogram equaliz on '%s'", test->filename);
		// Read file
		CompVMatPtr imageIn;
		COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		// Equalization
		CompVMatPtr imageOut;
		COMPV_CHECK_CODE_RETURN(CompVMathHistogram::equaliz(imageIn, &imageOut));
		// Check MD5
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Histogram equaliz MD5 mismatch");
	}

	return COMPV_ERROR_CODE_S_OK;
}
