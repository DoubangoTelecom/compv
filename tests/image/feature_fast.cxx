#include "../tests_common.h"

#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

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
	{ 20, false, COMPV_FAST_TYPE_9, -1, TEST_TYPE_EQUIRECTANGULAR, 1282, 720, 1282, 9583, 381080.f, 5826908.f, 3722280.f },
	{ 20, true, COMPV_FAST_TYPE_9, -1, TEST_TYPE_EQUIRECTANGULAR, 1282, 720, 1282, 2757, 115673.f, 1749686.f, 1065867.f },
	{ 20, false, COMPV_FAST_TYPE_12, -1, TEST_TYPE_EQUIRECTANGULAR, 1282,720, 1282, 3953, 148518.f, 2431991.f, 1549595.f },
	{ 20, true, COMPV_FAST_TYPE_12, -1, TEST_TYPE_EQUIRECTANGULAR, 1282, 720, 1282, 1456, 56445.f, 938167.f, 561473.f },
#if defined(_MSC_VER) // the STL sorting algo gives slightly different result depending on the platforms
	{ 20, false, COMPV_FAST_TYPE_9, 2000, TEST_TYPE_EQUIRECTANGULAR, 1282,720, 1282, 2001, 158661.f, 1187708.f, 782604.f },
	{ 20, true, COMPV_FAST_TYPE_9, 2000, TEST_TYPE_EQUIRECTANGULAR, 1282,720, 1282, 2118, 102037.f, 1327795.f, 813533.f },
	{ 20, false, COMPV_FAST_TYPE_12, 2000, TEST_TYPE_EQUIRECTANGULAR, 1282, 720, 1282, 2118, 105732.f, 1280332.f, 829835.f },
	{ 20, true, COMPV_FAST_TYPE_12, 2000, TEST_TYPE_EQUIRECTANGULAR, 1282,720, 1282, 1456, 56445.f, 938167.f, 561473.f },
#endif

	{ 20, false, COMPV_FAST_TYPE_9, -1, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 3493, 144778.f, 310525.f, 435337.f },
	{ 20, true, COMPV_FAST_TYPE_9, -1, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 1011, 49064.f, 90965.f, 122555.f },
	{ 20, false, COMPV_FAST_TYPE_12, -1, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 1673, 62092.f, 147160.f, 202160.f },
	{ 20, true, COMPV_FAST_TYPE_12, -1, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 672, 27580.f, 59806.f, 79401.f },
#if defined(_MSC_VER) // the STL sorting algo gives slightly different result depending on the platforms
	{ 20, false, COMPV_FAST_TYPE_9, 2000, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 2024, 107468.f, 176872.f, 237262.f },
	{ 20, true, COMPV_FAST_TYPE_9, 2000, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 1011, 49064.f, 90965.f, 122555.f },
	{ 20, false, COMPV_FAST_TYPE_12, 2000, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 1673, 62092.f, 147160.f, 202160.f },
	{ 20, true, COMPV_FAST_TYPE_12, 2000, TEST_TYPE_OPENGLBOOK, 200, 258, 200, 672, 27580.f, 59806.f, 79401.f },
#endif

	{ 20, false, COMPV_FAST_TYPE_9, -1, TEST_TYPE_GRIOTS, 480, 640, 480, 13625, 535125.f, 2868526.f, 4554394.f },
	{ 20, true, COMPV_FAST_TYPE_9, -1, TEST_TYPE_GRIOTS, 480, 640, 480,3096, 144996.f, 650016.f, 1057869.f },
	{ 20, false, COMPV_FAST_TYPE_12, -1, TEST_TYPE_GRIOTS, 480, 640, 480, 5536, 207029.f, 1215982.f, 1696520.f },
	{ 20, true, COMPV_FAST_TYPE_12, -1, TEST_TYPE_GRIOTS, 480, 640, 480, 1700, 72175.f, 368078.f, 532697.f },
#if defined(_MSC_VER) // the STL sorting algo gives slightly different result depending on the platforms
	{ 20, false, COMPV_FAST_TYPE_9, 2000, TEST_TYPE_GRIOTS, 480, 640, 480, 2097, 161213.f, 479470.f, 583577.f },
	{ 20, true, COMPV_FAST_TYPE_9, 2000, TEST_TYPE_GRIOTS, 480, 640, 480, 2055, 119527.f, 439634.f, 649044.f },
	{ 20, false, COMPV_FAST_TYPE_12, 2000, TEST_TYPE_GRIOTS, 480, 640,480, 2073, 114260.f, 471587.f, 555306.f },
	{ 20, true, COMPV_FAST_TYPE_12, 2000, TEST_TYPE_GRIOTS, 480, 640, 480, 1700, 72175.f, 368078.f, 532697.f },
#endif
};
size_t COMPV_UNITTESTS_FEATURE_FAST_COUNT = sizeof(COMPV_UNITTESTS_FEATURE_FAST) / sizeof(COMPV_UNITTESTS_FEATURE_FAST[0]);

#define LOOP_COUNT		1
#define TEST_TYPE		TEST_TYPE_EQUIRECTANGULAR
#define NONMAXIMA		true
#define THRESHOLD		20
#define FASTID			COMPV_FAST_TYPE_9
#define MAXFEATURES		-1

COMPV_ERROR_CODE feature_fast()
{
	CompVCornerDetePtr fast;
	CompVMatPtr image;
	CompVInterestPointVector interestPoints;
	uint64_t timeStart, timeEnd;
	const compv_unittest_feature_fast* test = NULL;
	float sum_scores;
	float xf;
	float yf;

	for (size_t i = 0; i < COMPV_UNITTESTS_FEATURE_FAST_COUNT; ++i) {
		test = &COMPV_UNITTESTS_FEATURE_FAST[i];
		if (test->threshold == THRESHOLD && test->fasdId == FASTID && test->nonmax == NONMAXIMA && test->maxFeatures == MAXFEATURES && std::string(test->filename).compare(TEST_TYPE) == 0) {
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
		COMPV_CHECK_CODE_RETURN(fast->process(image, interestPoints));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO("Elapsed time (TestFAST) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	// Check result
	sum_scores = 0.f;
	xf = 0.f;
	yf = 0.f;
	for (CompVInterestPointVector::const_iterator i = interestPoints.begin(); i != interestPoints.end(); ++i) {
		sum_scores += (*i).strength;
		xf += (*i).x;
		yf += (*i).y;
	}
	COMPV_CHECK_EXP_RETURN(interestPoints.size() != test->corners, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Num corners mismatch");
	COMPV_CHECK_EXP_RETURN(sum_scores != test->scores, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of scores mismatch");
	COMPV_CHECK_EXP_RETURN(xf != test->xf, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of xf mismatch");
	COMPV_CHECK_EXP_RETURN(yf != test->yf, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of yf mismatch");
	return COMPV_ERROR_CODE_S_OK;
}
