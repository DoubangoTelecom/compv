#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestHough"
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

#define CANNY_THRESHOLD_LOW		0.8f
#define CANNY_THRESHOLD_HIGH	CANNY_THRESHOLD_LOW*2.f	
#define HOUGHSTD_RHO			1.f
#define HOUGHSTD_THETA			kfMathTrigPiOver180 // radian(1d)
#define HOUGHSTD_THRESHOLD		100

static const struct compv_unittest_hough {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	compv_float32_t sum_rho;
	compv_float32_t sum_theta;
	size_t sum_strength;
}
COMPV_UNITTEST_HOUGHSTD[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, 1628983.f, 6085.76318f, 574468 },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, 1065.f, 9.42477798f, 882 },
	{ FILE_NAME_GRIOTS, 480, 640, 480, 35058.f, 1190.68298f, 95597 },
};
static const size_t COMPV_UNITTEST_HOUGHSTD_COUNT = sizeof(COMPV_UNITTEST_HOUGHSTD) / sizeof(COMPV_UNITTEST_HOUGHSTD[0]);


COMPV_ERROR_CODE unittest_hough()
{
	COMPV_ERROR_CODE err;
	CompVMatPtr image, edges;
	const compv_unittest_hough* test = NULL;
	CompVEdgeDetePtr canny;
	CompVHoughPtr houghstd;
	CompVHoughLineVector lines;
	compv_float32_t sum_rho, sum_theta;
	size_t sum_strength;

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&canny, COMPV_CANNY_ID, CANNY_THRESHOLD_LOW, CANNY_THRESHOLD_HIGH));
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&houghstd, COMPV_HOUGH_STANDARD_ID, HOUGHSTD_RHO, HOUGHSTD_THETA, HOUGHSTD_THRESHOLD));

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_HOUGHSTD_COUNT; ++i) {
		test = &COMPV_UNITTEST_HOUGHSTD[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Hough lines detector -> %s ==", test->filename);
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
		COMPV_CHECK_CODE_RETURN(canny->process(image, &edges));
		COMPV_CHECK_CODE_RETURN(houghstd->process(edges, lines));
		sum_rho = 0.f;
		sum_theta = 0.f;
		sum_strength = 0;
		for (CompVHoughLineVector::const_iterator i = lines.begin(); i < lines.end(); ++i) {
			sum_rho += i->rho;
			sum_theta += i->theta;
			sum_strength += i->strength;
		}
		COMPV_CHECK_EXP_BAIL(sum_rho != test->sum_rho, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghstd sum_rho mismatch");
		COMPV_CHECK_EXP_BAIL(COMPV_MATH_ABS(sum_theta - test->sum_theta) > 0.0009765625, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghstd sum_theta mismatch");
		COMPV_CHECK_EXP_BAIL(sum_strength != test->sum_strength, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghstd sum_strength mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
bail:
	return err;
}
