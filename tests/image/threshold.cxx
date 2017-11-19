#include "../tests_common.h"

#define TAG_TEST								"TestImageThresholding"
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

COMPV_ERROR_CODE adaptiveThreshold()
{
#define BLOCK_SIZE				5
#define DELTA					8 // 8 / 21
#define MAXVAL					255
#define INVERT					1 // true(1) / false(0)

#define MD5_DELTA8_INVERT1		"71e1624e5da860a752d87deb6d9f7e96"
#define MD5_DELTA8_INVERT0		"4d8f0e6957567115dc096ac89456e2ae"
#define MD5_DELTA21_INVERT1		"fcbb0c4431f80d86d25520ce5ddf71ba"
#define MD5_DELTA21_INVERT0		"e3dab5d57f8db3763ee4a681e7e607f0"

	CompVMatPtr imageIn, imageOut, kernel;

	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_EQUIRECTANGULAR).c_str(), &imageIn));
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::kernelMean(BLOCK_SIZE, &kernel));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImageThreshold::adaptive(imageIn, &imageOut, kernel, DELTA, MAXVAL, INVERT));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Adaptive Threshold Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, "threshold.gray"));
#endif

#if DELTA == 8
#	if INVERT == 1
	const std::string expectedMD5 = MD5_DELTA8_INVERT1;
#	else
	const std::string expectedMD5 = MD5_DELTA8_INVERT0;
#	endif
#elif DELTA == 21
#	if INVERT == 1
	const std::string expectedMD5 = MD5_DELTA21_INVERT1;
#	else
	const std::string expectedMD5 = MD5_DELTA21_INVERT0;
#	endif
#else
#error "Not implemented"
#endif

	COMPV_CHECK_EXP_RETURN(std::string(expectedMD5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Adaptive Threshold MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}



static const struct compv_unittest_thresh_otsu {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const double threshold;
	const char* md5;
}
COMPV_UNITTEST_THESH_OTSU[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, 119.0, "8bea4b36fb34f778f3aade94af193efa" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, 115.0, "0b6344f47984f5e0b5910b5e737872fa" },
	{ FILE_NAME_GRIOTS, 480, 640, 480, 129.0, "e6d32aba3044e4b8aee7e260ecdf403e" },
};
static const size_t COMPV_UNITTEST_THESH_OTSU_COUNT = sizeof(COMPV_UNITTEST_THESH_OTSU) / sizeof(COMPV_UNITTEST_THESH_OTSU[0]);

#define OTSU_FILE	FILE_NAME_EQUIRECTANGULAR

COMPV_ERROR_CODE otsuThreshold()
{
	// Find test
	const compv_unittest_thresh_otsu* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_THESH_OTSU_COUNT && !test; ++i) {
		if (std::string(COMPV_UNITTEST_THESH_OTSU[i].filename).compare(OTSU_FILE) == 0) {
			test = &COMPV_UNITTEST_THESH_OTSU[i];
		}
	}
	COMPV_ASSERT(test != nullptr);

	// Read file
	CompVMatPtr imageIn;
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));

	// Otsu processing
	CompVMatPtr imageOut;
	double threshold = 0.0;

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImageThreshold::otsu(imageIn, threshold, &imageOut));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Otsu Threshold Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, "threshold.gray"));
#endif

	COMPV_CHECK_EXP_RETURN(test->threshold != threshold, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Otsu Thresholding value mismatch");
	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Otsu Thresholding MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}