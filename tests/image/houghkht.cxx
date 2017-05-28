#include "../tests_common.h"

// Depending on the number of threads, the mean value could be "+-1" compared to the single threaded version
// -> Unit-test -> use 8 threads

#define TAG_TEST								"TestHoughKht"
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
#define CANNY_THRESHOLD_LOW		0.8f
#define CANNY_THRESHOLD_HIGH	CANNY_THRESHOLD_LOW*2.f	

#define HOUGHKHT_RHO						(1.0f * 0.5f) // "rho-delta" (half-pixel)
#define HOUGHKHT_THETA						(kfMathTrigPiOver180 * 0.5f) // "theta-delta" (half-degree)
#define HOUGHKHT_THRESHOLD					1 // keep all votes and filter later using MAXLINES
#define HOUGHKHT_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGHKHT_CLUSTER_MIN_SIZE			10
#define HOUGHKHT_KERNEL_MIN_HEIGTH			0.002f

static const struct compv_unittest_houghkht {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	compv_float32_t sum_rho;
	compv_float32_t sum_theta;
	size_t sum_strength;
}
COMPV_UNITTEST_HOUGHKHT[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, 36471.6445f, 371.179108f, 24634 },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, 3498.13647f, 530.903076f, 34071 },
	{ FILE_NAME_GRIOTS, 480, 640, 480, 4146.50000f, 101.927231f, 6992 },
};
static const size_t COMPV_UNITTEST_HOUGHKHT_COUNT = sizeof(COMPV_UNITTEST_HOUGHKHT) / sizeof(COMPV_UNITTEST_HOUGHKHT[0]);

COMPV_ERROR_CODE houghkht()
{
	CompVEdgeDetePtr canny;
	CompVHoughPtr houghkht;
	CompVMatPtr image, edges, directions;
	CompVHoughLineVector lines;
	const compv_unittest_houghkht* test = NULL;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_HOUGHKHT_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_HOUGHKHT[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_HOUGHKHT[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&canny, COMPV_CANNY_ID, CANNY_THRESHOLD_LOW, CANNY_THRESHOLD_HIGH));
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&houghkht, COMPV_HOUGHKHT_ID, HOUGHKHT_RHO, HOUGHKHT_THETA, HOUGHKHT_THRESHOLD));
	COMPV_CHECK_CODE_RETURN(houghkht->setFloat32(COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION, HOUGHKHT_CLUSTER_MIN_DEVIATION));
	COMPV_CHECK_CODE_RETURN(houghkht->setInt(COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE, HOUGHKHT_CLUSTER_MIN_SIZE));
	COMPV_CHECK_CODE_RETURN(houghkht->setFloat32(COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH, HOUGHKHT_KERNEL_MIN_HEIGTH));
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
	COMPV_CHECK_CODE_RETURN(canny->process(image, &edges, &directions));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(houghkht->process(edges, lines, directions));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "HoughKht Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	compv_float32_t sum_rho = 0.f;
	compv_float32_t sum_theta = 0.f;
	size_t sum_strength = 0;
	for (CompVHoughLineVector::const_iterator i = lines.begin(); i < lines.end(); ++i) {
		sum_rho += i->rho;
		sum_theta += i->theta;
		sum_strength += i->strength;
	}

	COMPV_CHECK_EXP_RETURN(sum_rho != test->sum_rho, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghkht sum_rho mismatch");
	COMPV_CHECK_EXP_RETURN(COMPV_MATH_ABS(sum_theta - test->sum_theta) > 0.0009765625, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghkht sum_theta mismatch");
	COMPV_CHECK_EXP_RETURN(sum_strength != test->sum_strength, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghkht sum_strength mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
