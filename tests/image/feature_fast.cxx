#include "../tests_common.h"

#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define THRESHOLD			10

#define TEST_TYPE_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define TEST_TYPE_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define TEST_TYPE_GRIOTS				"mandekalou_480x640_gray.yuv"

// Reference implementation for FAST12, nonmax, #1000 times -> ellapsed: 6159 millis, CPU time: 6.427s
// OpenCV time for FAST9, nonmax, #1000 times -> 1889 millis

static const struct compv_unittest_feature_fast {
	int threshold;
	bool nonmax;
	int fasdId;
	int maxFeatures;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	size_t corners;
	compv_float32_t scores;
	compv_float32_t xf;
	compv_float32_t yf;
}
COMPV_UNITTESTS_FEATURE_FAST[] =
{
	{ 10, false, COMPV_FAST_TYPE_9, -1, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 24105, 574969.f, 15148142.f, 9577924.f },
	{ 10, true, COMPV_FAST_TYPE_9, -1, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 6598, 167156.f, 4326974.f, 2669475.f },
	{ 10, false, COMPV_FAST_TYPE_12, -1, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 10812, 239835.f, 6885661.f, 4326231.f },
	{ 10, true, COMPV_FAST_TYPE_12, -1, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 3920, 89172, 2584280.f, 1576105.f },

	{ 10, false, COMPV_FAST_TYPE_9, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 5540, 173438.f, 501490.f, 708067.f },
	{ 10, true, COMPV_FAST_TYPE_9, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 1282, 52936.f, 117722.f, 160696.f },
	{ 10, false, COMPV_FAST_TYPE_12, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 2930, 79671.f, 260098.f, 368964.f },
	{ 10, true, COMPV_FAST_TYPE_12, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 978, 31952.f, 88469.f, 121621.f },

	{ 10, false, COMPV_FAST_TYPE_9, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480, 27208, 720203.f, 5722419.f, 9740405.f },
	{ 10, true, COMPV_FAST_TYPE_9, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480, 5405, 176389.f, 1166708.f, 1973623.f },
	{ 10, false, COMPV_FAST_TYPE_12, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480, 12113, 295977.f, 2646636.f, 4152440.f },
	{ 10, true, COMPV_FAST_TYPE_12, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480, 3425, 95590.f, 750394.f, 1198537.f },
};
size_t COMPV_UNITTESTS_FEATURE_FAST_COUNT = sizeof(COMPV_UNITTESTS_FEATURE_FAST) / sizeof(COMPV_UNITTESTS_FEATURE_FAST[0]);

#define LOOP_COUNT		1
#define TEST_TYPE		TEST_TYPE_EQUIRECTANGULAR
#define NONMAXIMA		true
#define FASTID			COMPV_FAST_TYPE_9
#define MAXFEATURES		-1

COMPV_ERROR_CODE feature_fast()
{
	CompVCornerDetePtr fast;
	CompVMatPtr image;
	CompVBoxInterestPointPtr interestPoints;
	uint64_t timeStart, timeEnd;
	const compv_unittest_feature_fast* test = NULL;
	float sum_scores;
	float xf;
	float yf;

	for (size_t i = 0; i < COMPV_UNITTESTS_FEATURE_FAST_COUNT; ++i) {
		test = &COMPV_UNITTESTS_FEATURE_FAST[i];
		if (test->threshold == THRESHOLD && test->fasdId == FASTID && test->nonmax == NONMAXIMA && std::string(test->filename).compare(TEST_TYPE) == 0) {
			break;
		}
		test = NULL;
	}
	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	// Read file
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));

	// Create the FAST feature detector
	COMPV_CHECK_CODE_RETURN(CompVCornerDete::newObj(&fast, COMPV_FAST_ID));

	// Set the default values
	COMPV_CHECK_CODE_RETURN(fast->setInt(COMPV_FAST_SET_INT_THRESHOLD, test->threshold));
	COMPV_CHECK_CODE_RETURN(fast->setInt(COMPV_FAST_SET_INT_FAST_TYPE, test->fasdId));
	COMPV_CHECK_CODE_RETURN(fast->setInt(COMPV_FAST_SET_INT_MAX_FEATURES, test->maxFeatures));
	COMPV_CHECK_CODE_RETURN(fast->setBool(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, test->nonmax));

	// Detect keypoints
	timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(fast->process(image, &interestPoints));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO("Elapsed time (TestFAST) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// Check result
	sum_scores = 0.f;
	xf = 0.f;
	yf = 0.f;
	for (size_t i = 0; i < interestPoints->size(); ++i) {
		sum_scores += interestPoints->ptr(i)->strength;
		xf += interestPoints->ptr(i)->x;
		yf += interestPoints->ptr(i)->y;
	}
	COMPV_CHECK_EXP_RETURN(interestPoints->size() != test->corners, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Num corners mismatch");
	COMPV_CHECK_EXP_RETURN(sum_scores != test->scores, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of scores mismatch");
	COMPV_CHECK_EXP_RETURN(xf != test->xf, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of xf mismatch");
	COMPV_CHECK_EXP_RETURN(yf != test->yf, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of yf mismatch");
	return COMPV_ERROR_CODE_S_OK;
}