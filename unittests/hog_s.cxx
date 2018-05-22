#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestHogStandard"
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

static const struct compv_unittest_hogstd {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	int norm;
	const char* md5;
	const char* md5_fma;
}
COMPV_UNITTEST_HOGSTD[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_NONE, "b03b5891e9d1845c231b71470bf0278d", "a6ebc3cd0dbe8d65112badb9288345c0" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L1, "e92312dfa9698006dd6499d6e54f4563", "4ae71d377810ac5d120f839c7fd05e6d" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L1SQRT, "00cba3ac411dde65fff6d733be7289e6", "ad7342c69929eda0d45d735ffb6b7236" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L2, "ee03be50d8f26ac0c6877bf59f9bfdc6", "f4026800dc2223ab5943bca390dd1f29" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L2HYS, "9a3c3ade6a4e50a95ebfe226c3f1ce0f", "4dfc2e21fd381585174d8252bad7d3df" },
};
static const size_t COMPV_UNITTEST_HOGSTD_COUNT = sizeof(COMPV_UNITTEST_HOGSTD) / sizeof(COMPV_UNITTEST_HOGSTD[0]);

static const std::string compv_unittest_norm_to_string(const int& norm) {
	switch (norm) {
	case COMPV_HOG_BLOCK_NORM_NONE: return "none";
	case COMPV_HOG_BLOCK_NORM_L1: return "L1";
	case COMPV_HOG_BLOCK_NORM_L1SQRT: return "L1Sqrt";
	case COMPV_HOG_BLOCK_NORM_L2: return "L2";
	case COMPV_HOG_BLOCK_NORM_L2HYS: return "L2Hyst";
	default: return "Unknown";
	}
}
static const std::string compv_unittest_hogs_to_string(const compv_unittest_hogstd* test) {
	return std::string("filename:") + std::string(test->filename) + std::string(", norm:") + compv_unittest_norm_to_string(test->norm);
}

COMPV_ERROR_CODE unittest_hog_s()
{
	CompVMatPtr image, features;
	CompVHOGPtr hogStd;
	COMPV_CHECK_CODE_RETURN(CompVHOG::newObj(&hogStd, COMPV_HOGS_ID,
		CompVSizeSz(8, 8), // blockSize(8, 8),
		CompVSizeSz(4, 4), // blockStride(4, 4),
		CompVSizeSz(8, 8), // cellSize(8, 8),
		9, // nbins
		COMPV_HOG_BLOCK_NORM_NONE, // blockNorm
		true, // gradientSigned
		COMPV_HOG_INTERPOLATION_BILINEAR // interpolation
	));

	for (size_t i = 0; i < COMPV_UNITTEST_HOGSTD_COUNT; ++i) {
		const compv_unittest_hogstd* test = &COMPV_UNITTEST_HOGSTD[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: HOG(STD) -> %s ==", compv_unittest_hogs_to_string(test).c_str());
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
		COMPV_CHECK_CODE_RETURN(hogStd->setInt(COMPV_HOG_SET_INT_BLOCK_NORM, test->norm));
		COMPV_CHECK_CODE_RETURN(hogStd->process(image, &features));
		COMPV_CHECK_EXP_RETURN(std::string(compv_tests_is_fma_enabled() ? test->md5_fma : test->md5).compare(compv_tests_md5(features)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "HOG(STD) MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}
