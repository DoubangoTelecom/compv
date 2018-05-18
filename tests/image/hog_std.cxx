#include "../tests_common.h"

#define TAG_TEST								"TestHogStd"
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
#define FILE_NAME				FILE_NAME_EQUIRECTANGULAR
#define NORM					COMPV_HOG_BLOCK_NORM_L2HYS

static const struct compv_unittest_hogstd {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	int norm;
	const char* md5;
}
COMPV_UNITTEST_HOGSTD[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_NONE, "b03b5891e9d1845c231b71470bf0278d" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L1, "eb66c514402471589763455838b4f6a4" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L1SQRT, "180541cfede3c371b75a42409d30c94c" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L2, "a86bbf2ffc518b93743b84d06fd4e8ba" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_HOG_BLOCK_NORM_L2HYS, "dcd6343cfc7ca7308c0e4dbbc3ad8707" },
};
static const size_t COMPV_UNITTEST_HOGSTD_COUNT = sizeof(COMPV_UNITTEST_HOGSTD) / sizeof(COMPV_UNITTEST_HOGSTD[0]);

COMPV_ERROR_CODE hogstd()
{
	CompVHOGPtr hogStd;
	const compv_unittest_hogstd* test = nullptr;
	CompVMatPtr image, features;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_HOGSTD_COUNT; ++i) {
		if (COMPV_UNITTEST_HOGSTD[i].norm == NORM && std::string(COMPV_UNITTEST_HOGSTD[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_HOGSTD[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	COMPV_CHECK_CODE_RETURN(CompVHOG::newObj(&hogStd, COMPV_HOGS_ID,
		CompVSizeSz(8, 8), // blockSize(8, 8),
		CompVSizeSz(4, 4), // blockStride(4, 4),
		CompVSizeSz(8, 8), // cellSize(8, 8),
		9, // nbins
		test->norm, // blockNorm
		true, // gradientSigned
		COMPV_HOG_INTERPOLATION_BILINEAR // interpolation
	));
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
	
	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(hogStd->process(image, &features));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "HogStd Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5:%s", compv_tests_md5(features).c_str());
	COMPV_CHECK_EXP_RETURN(compv_tests_md5(features).compare(test->md5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "S-HOG failed");

	return COMPV_ERROR_CODE_S_OK;
}
