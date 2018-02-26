#include "../tests_common.h"

#define TAG_TEST			"TestHistogram"
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

#define LOOP_COUNT				1

//					
//	== BUILD ==
//

COMPV_ERROR_CODE histogram_build()
{
#define BUILD_EXPECTED_MD5		"9cc75b0ec5f1522cfc65b7fadc0aacdb"
	CompVMatPtr image, histogram;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_EQUIRECTANGULAR).c_str(), &image));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImage::histogramBuild(image, &histogram));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(Histogram) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(histogram).compare(BUILD_EXPECTED_MD5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "MD5 mismatch");
	return COMPV_ERROR_CODE_S_OK;
}

//					
//	== EQUILIZ ==
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

COMPV_ERROR_CODE histogram_equaliz()
{
	// Find test
	const compv_unittest_histo_equaliz* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_HISTO_EQUALIZ_COUNT && !test; ++i) {
		if (std::string(COMPV_UNITTEST_HISTO_EQUALIZ[i].filename).compare(EQUALIZ_FILE) == 0) {
			test = &COMPV_UNITTEST_HISTO_EQUALIZ[i];
		}
	}
	COMPV_ASSERT(test != nullptr);

	// Read file
	CompVMatPtr imageIn;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));

	// Equalization
	CompVMatPtr imageOut;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImage::histogramEqualiz(imageIn, &imageOut));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Histogram equaliz Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, "equaliz.gray"));
#endif

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Histogram equaliz MD5 mismatch");

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

#define PROJ_TYPE_X	1
#define PROJ_TYPE_Y	2

#define PROJ_FILE	FILE_NAME_EQUIRECTANGULAR
#define PROJ_TYPE	PROJ_TYPE_X

COMPV_ERROR_CODE histogram_proj()
{
	// Find test
	const compv_unittest_histo_proj* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_HISTO_PROJ_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_HISTO_PROJ[i].filename).compare(PROJ_FILE) == 0) {
			test = &COMPV_UNITTEST_HISTO_PROJ[i];
			break;
		}
	}
	COMPV_ASSERT(test != nullptr);

	// Read file
	CompVMatPtr imageIn;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));

	// Equalization
	CompVMatPtr imageOut;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
#if PROJ_TYPE == PROJ_TYPE_X
		COMPV_CHECK_CODE_RETURN(CompVImage::histogramBuildProjectionX(imageIn, &imageOut));
#else
		COMPV_CHECK_CODE_RETURN(CompVImage::histogramBuildProjectionY(imageIn, &imageOut));
#endif
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Histogram proj Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, PROJ_FILE));
#endif

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5: %s", compv_tests_md5(imageOut).c_str());

#if PROJ_TYPE == PROJ_TYPE_X
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_x).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Histogram projx MD5 mismatch");
#else
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_y).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Histogram projy MD5 mismatch");
#endif

	return COMPV_ERROR_CODE_S_OK;
}