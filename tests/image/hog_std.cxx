#include "../tests_common.h"

// Depending on the number of threads, the mean value could be "+-1" compared to the single threaded version
// -> Unit-test -> use 8 threads

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
#define FILE_NAME				FILE_NAME_OPENGLBOOK

static const struct compv_unittest_hogstd {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_HOGSTD[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, "88828412a7b186f2ff9052d24366e34b" }, // gradX="30745c448a624e231b0c07e4d9df5dcf", gradY="b4fd82a74781cf2c030b22a0279e85db", mag="265f046667311421979d0a7a0be266ef", dir="88828412a7b186f2ff9052d24366e34b"
	{ FILE_NAME_GRIOTS, 480, 640, 480, "" },
};
static const size_t COMPV_UNITTEST_HOGSTD_COUNT = sizeof(COMPV_UNITTEST_HOGSTD) / sizeof(COMPV_UNITTEST_HOGSTD[0]);

COMPV_ERROR_CODE hogstd()
{
	CompVHOGPtr hogStd;
	const compv_unittest_hogstd* test = nullptr;
	CompVMatPtr image, features;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_HOGSTD_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_HOGSTD[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_HOGSTD[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	COMPV_CHECK_CODE_RETURN(CompVHOG::newObj(&hogStd, COMPV_HOGS_ID,
		CompVSizeSz(8, 8), // blockSize,
		CompVSizeSz(4, 4), // blockStride,
		CompVSizeSz(4, 4), // cellSize,
		9, // nbins
		COMPV_HOG_BLOCK_NORM_L2HYST, // blockNorm
		true // gradientSigned
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
