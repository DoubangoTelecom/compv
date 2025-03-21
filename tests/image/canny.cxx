#include "../tests_common.h"

// Depending on the number of threads, the mean value could be "+-1" compared to the single threaded version
// -> Unit-test -> use 8 threads

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

#define LOOP_COUNT			1
#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR
#define THRESHOLD_LOW		59 // otsu*0.5
#define THRESHOLD_HIGH		119 // otsu*1.0
#define THRESHOLD_TYPE		COMPV_CANNY_THRESHOLD_TYPE_COMPARE_TO_GRADIENT	

static const struct compv_unittest_canny {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_CANNY[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "29e45ba7f87a3a967ec031f1cfb7ea35" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, "ea167033e8f2a8432bee4561b215bb4d" },
	{ FILE_NAME_GRIOTS, 480, 640, 480, "b25a7e8ee9632204e90849a39bf2f68c" },
};
static const size_t COMPV_UNITTEST_CANNY_COUNT = sizeof(COMPV_UNITTEST_CANNY) / sizeof(COMPV_UNITTEST_CANNY[0]);

COMPV_ERROR_CODE canny()
{
	CompVEdgeDetePtr dete;
	CompVMatPtr image, edges;
	const compv_unittest_canny* test = NULL;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_CANNY_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_CANNY[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_CANNY[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&dete, COMPV_CANNY_ID, THRESHOLD_LOW, THRESHOLD_HIGH));
	COMPV_CHECK_CODE_RETURN(dete->setInt(COMPV_CANNY_SET_INT_THRESHOLD_TYPE, THRESHOLD_TYPE));

	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(dete->process(image, &edges));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Canny Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5: %s", compv_tests_md5(edges).c_str());

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(edges, FILE_NAME));
#endif

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(edges)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Canny MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
