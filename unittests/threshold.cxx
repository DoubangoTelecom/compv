#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestThresholding"
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

static const struct compv_unittest_thresh_adapt {
	size_t blockSize;
	double delta;
	double maxVal;
	bool invert;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_THESH_ADAPT[] =
{
	{ 5, 8, 255, true, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "71e1624e5da860a752d87deb6d9f7e96" },
	{ 5, 8, 255, false, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "4d8f0e6957567115dc096ac89456e2ae" },
	{ 5, 21, 255, true, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "fcbb0c4431f80d86d25520ce5ddf71ba" },
	{ 5, 21, 255, false, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "e3dab5d57f8db3763ee4a681e7e607f0" }
};
static const size_t COMPV_UNITTEST_THESH_ADAPT_COUNT = sizeof(COMPV_UNITTEST_THESH_ADAPT) / sizeof(COMPV_UNITTEST_THESH_ADAPT[0]);

static const std::string compv_unittest_thresh_adapt_to_string(const compv_unittest_thresh_adapt* test) {
	return
		std::string("blockSize:") + CompVBase::to_string(test->blockSize) + std::string(", ")
		+ std::string("delta:") + CompVBase::to_string(test->delta) + std::string(", ")
		+ std::string("maxVal:") + CompVBase::to_string(test->maxVal) + std::string(", ")
		+ std::string("invert:") + std::string(test->invert ? "true" : "false") + std::string(", ")
		+ std::string("filename:") + std::string(test->filename);
}

COMPV_ERROR_CODE unittest_thresh_adapt()
{
	COMPV_ERROR_CODE err;
	CompVMatPtr imageIn, imageOut;
	const compv_unittest_thresh_adapt* test = NULL;

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_THESH_ADAPT_COUNT; ++i) {
		test = &COMPV_UNITTEST_THESH_ADAPT[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Adaptive thresholding -> %s ==", compv_unittest_thresh_adapt_to_string(test).c_str());
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		COMPV_CHECK_CODE_BAIL(err = CompVImageThreshold::adaptive(imageIn, &imageOut, test->blockSize, test->delta, test->maxVal, test->invert));
		COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Adaptive thresholding MD5 mismatch");
		imageOut = nullptr; // do not reuse
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
bail:
	return err;
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

static const std::string compv_unittest_thresh_otsu_to_string(const compv_unittest_thresh_otsu* test) {
	return
		std::string("filename:") + std::string(test->filename);
}

COMPV_ERROR_CODE unittest_thresh_otsu()
{
	COMPV_ERROR_CODE err;
	CompVMatPtr imageIn, imageOut;
	const compv_unittest_thresh_otsu* test = NULL;
	double threshold;

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_THESH_OTSU_COUNT; ++i) {
		test = &COMPV_UNITTEST_THESH_OTSU[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Otsu thresholding -> %s ==", compv_unittest_thresh_otsu_to_string(test).c_str());
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		COMPV_CHECK_CODE_BAIL(err = CompVImageThreshold::otsu(imageIn, threshold, &imageOut));
		COMPV_CHECK_EXP_BAIL(test->threshold != threshold, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Otsu thresholding value mismatch");
		COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Otsu thresholding MD5 mismatch");
		imageOut = nullptr; // do not reuse
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
bail:
	return err;
}


