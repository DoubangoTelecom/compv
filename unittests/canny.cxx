#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestCanny"
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

COMPV_ERROR_CODE unittest_canny()
{
	COMPV_ERROR_CODE err;
	CompVMatPtr image, edges;
	const compv_unittest_canny* test = NULL;
	CompVEdgeDetePtr dete;

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&dete, COMPV_CANNY_ID, THRESHOLD_LOW, THRESHOLD_HIGH));
	COMPV_CHECK_CODE_RETURN(dete->setInt(COMPV_CANNY_SET_INT_THRESHOLD_TYPE, THRESHOLD_TYPE));

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_CANNY_COUNT; ++i) {
		test = &COMPV_UNITTEST_CANNY[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Canny edge detector -> %s ==", test->filename);
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
		COMPV_CHECK_CODE_RETURN(dete->process(image, &edges));
		COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(edges)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Canny edge detector MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
bail:
	return err;
}
