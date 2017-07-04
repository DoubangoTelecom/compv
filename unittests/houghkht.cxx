#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestHoughKht"
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

#define CANNY_THRESHOLD_LOW					0.8f
#define CANNY_THRESHOLD_HIGH				CANNY_THRESHOLD_LOW*2.f	
#define HOUGHKHT_RHO						(1.0f * 0.5f) // "rho-delta" (half-pixel)
#define HOUGHKHT_THETA						(kfMathTrigPiOver180 * 0.5f) // "theta-delta" (half-radian)
#define HOUGHKHT_THRESHOLD					1 // keep all votes and filter later using MAXLINES
#define HOUGHKHT_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGHKHT_CLUSTER_MIN_SIZE			10
#define HOUGHKHT_KERNEL_MIN_HEIGTH			0.002f

static const struct compv_unittest_hough {
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
#if COMPV_ARCH_X86
    { FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, 26740.4766f, 279.339935f, 17657 },
    { FILE_NAME_OPENGLBOOK, 200, 258, 200, 264.128479f, 226.875412f, 19993 },
    { FILE_NAME_GRIOTS, 480, 640, 480, 3146.00000f, 69.8306274f, 5318 },
#elif COMPV_ARCH_ARM
#   if COMPV_OS_IPHONE
    { FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, 28750.9551f, 287.211395, 17701 },
    { FILE_NAME_OPENGLBOOK, 200, 258, 200, 451.790833f, 216.769913f, 19825 },
    { FILE_NAME_GRIOTS, 480, 640, 480, 3016.f, 69.1150436f, 5286 },
#   else
    { FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, 27785.7383f, 297.718323f, 17865 },
    { FILE_NAME_OPENGLBOOK, 200, 258, 200, 830.949036f, 261.310699f, 20671 },
    { FILE_NAME_GRIOTS, 480, 640, 480, 3064.f, 74.3335876f, 5366 },
#   endif
#else
#   error "Not implemented"
#endif
};
static const size_t COMPV_UNITTEST_HOUGHKHT_COUNT = sizeof(COMPV_UNITTEST_HOUGHKHT) / sizeof(COMPV_UNITTEST_HOUGHKHT[0]);


COMPV_ERROR_CODE unittest_houghkht()
{
	COMPV_ERROR_CODE err;
	CompVMatPtr image, edges;
	const compv_unittest_hough* test = NULL;
	CompVEdgeDetePtr canny;
	CompVHoughPtr houghkht;
	CompVHoughLineVector lines;
	compv_float32_t sum_rho, sum_theta;
	size_t sum_strength;

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&canny, COMPV_CANNY_ID, CANNY_THRESHOLD_LOW, CANNY_THRESHOLD_HIGH));
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&houghkht, COMPV_HOUGHKHT_ID, HOUGHKHT_RHO, HOUGHKHT_THETA, HOUGHKHT_THRESHOLD));
	COMPV_CHECK_CODE_RETURN(houghkht->setFloat32(COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION, HOUGHKHT_CLUSTER_MIN_DEVIATION));
	COMPV_CHECK_CODE_RETURN(houghkht->setInt(COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE, HOUGHKHT_CLUSTER_MIN_SIZE));
	COMPV_CHECK_CODE_RETURN(houghkht->setFloat32(COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH, HOUGHKHT_KERNEL_MIN_HEIGTH));

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_HOUGHKHT_COUNT; ++i) {
		test = &COMPV_UNITTEST_HOUGHKHT[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Hough lines (KHT) detector -> %s ==", test->filename);
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
		COMPV_CHECK_CODE_RETURN(canny->process(image, &edges));
		COMPV_CHECK_CODE_RETURN(houghkht->process(edges, lines));
		sum_rho = 0.f;
		sum_theta = 0.f;
		sum_strength = 0;
		for (CompVHoughLineVector::const_iterator i = lines.begin(); i < lines.end(); ++i) {
			sum_rho += i->rho;
			sum_theta += i->theta;
			sum_strength += i->strength;
		}
		COMPV_CHECK_EXP_BAIL(sum_rho != test->sum_rho, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Houghkht sum_rho mismatch");
		COMPV_CHECK_EXP_BAIL(COMPV_MATH_ABS(sum_theta - test->sum_theta) > 0.0009765625, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Houghkht sum_theta mismatch");
		COMPV_CHECK_EXP_BAIL(sum_strength != test->sum_strength, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Houghkht sum_strength mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
bail:
	return err;
}
