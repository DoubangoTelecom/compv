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


//
//	== BUILD ==
//


COMPV_ERROR_CODE unittest_histogram_build()
{
#define EXPECTED_MD5_BUILD		"9cc75b0ec5f1522cfc65b7fadc0aacdb"

	CompVMatPtr image, histogram;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_EQUIRECTANGULAR).c_str(), &image));
	
	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "== Trying new test: histogram build on '%s'", FILE_NAME_EQUIRECTANGULAR);
	COMPV_CHECK_CODE_RETURN(CompVImage::histogramBuild(image, &histogram));
	COMPV_CHECK_EXP_RETURN(compv_tests_md5(histogram).compare(EXPECTED_MD5_BUILD) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "MD5 mismatch");
	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Test OK **");

	return COMPV_ERROR_CODE_S_OK;
}


//
//	== EQUALIZ ==
//

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
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		// Equalization
		CompVMatPtr imageOut;
		COMPV_CHECK_CODE_RETURN(CompVImage::histogramEqualiz(imageIn, &imageOut));
		// Check MD5
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Histogram equaliz MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}


//
//	== PROJ ==
//


static const struct compv_unittest_histo_proj {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5_x;
	const char* md5_y;
}
COMPV_UNITTEST_HISTO_PROJ[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "cb12d638ed42c1e63cbe3bfacce6a90d", "4ea410ff2f9ba7de0ed61eefb3578d71" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, "54330a1162f7d73af8ee19373644a003", "689d6a2a5a111e96391e42c18f46714e" },
	{ FILE_NAME_GRIOTS, 480, 640, 480, "52d8d21e674c85a68ddbd3e2234cd713", "820fd5b203b85b8739ffabfafab185d4" },
};
static const size_t COMPV_UNITTEST_HISTO_PROJ_COUNT = sizeof(COMPV_UNITTEST_HISTO_PROJ) / sizeof(COMPV_UNITTEST_HISTO_PROJ[0]);

COMPV_ERROR_CODE unittest_histogram_proj()
{
	CompVMatPtr imageIn, imageOutX, imageOutY;
	for (size_t i = 0; i < COMPV_UNITTEST_HISTO_PROJ_COUNT; ++i) {
		const compv_unittest_histo_proj* test = &COMPV_UNITTEST_HISTO_PROJ[i];	
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "== Trying new test: histogram proj on '%s'", test->filename);
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		COMPV_CHECK_CODE_RETURN(CompVImage::histogramBuildProjectionX(imageIn, &imageOutX));
		COMPV_CHECK_CODE_RETURN(CompVImage::histogramBuildProjectionY(imageIn, &imageOutY));
		COMPV_CHECK_EXP_RETURN(std::string(test->md5_x).compare(compv_tests_md5(imageOutX)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Histogram projx MD5 mismatch");
		COMPV_CHECK_EXP_RETURN(std::string(test->md5_y).compare(compv_tests_md5(imageOutY)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Histogram projy MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}
