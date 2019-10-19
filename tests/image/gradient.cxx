#include "../tests_common.h"

#define TAG_TEST								"TestGradient"
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
#define IN_FLTP					true // testing gradXY_32f32f
#define OUT_FLTP				true // Floating point?
#define XDIR					true // XDIR(true) or YDIR(false)

static const struct compv_unittest_gradient {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	bool out_fltp;
	bool xdir;
	const char* md5;
}
COMPV_UNITTEST_GRADIENT[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, true, true, "a5ebcd257d4ac58fc6de89ad8bafd6e1" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, true, false, "1566ebcf7f31efd39be3118b3097bc26" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, false, true, "2dee710f47c0e668c7954ebabb0d4af9" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, false, false, "b20836fa3bb500a258478a5c8fb40647" },
};
static const size_t COMPV_UNITTEST_GRADIENT_COUNT = sizeof(COMPV_UNITTEST_GRADIENT) / sizeof(COMPV_UNITTEST_GRADIENT[0]);

COMPV_ERROR_CODE gradient()
{
	const compv_unittest_gradient* test = nullptr;
	CompVMatPtr image, grad;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_GRADIENT_COUNT; ++i) {
		if (COMPV_UNITTEST_GRADIENT[i].xdir == XDIR && COMPV_UNITTEST_GRADIENT[i].out_fltp == OUT_FLTP && std::string(COMPV_UNITTEST_GRADIENT[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_GRADIENT[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}
	COMPV_ERROR_CODE(*fncptr)(const CompVMatPtr& input, CompVMatPtrPtr output, bool outputFloat)
		= test->xdir ? CompVImage::gradientX : CompVImage::gradientY;
	
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));

	if (OUT_FLTP && IN_FLTP) { // Testing gradXY_32f32f
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, compv_float32_t>(image, &image)));
	}

	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(fncptr(image, &grad, test->out_fltp));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Gradient Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5:%s", compv_tests_md5(grad).c_str());
	COMPV_CHECK_EXP_RETURN(compv_tests_md5(grad).compare(test->md5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "GRADIENT failed");

	return COMPV_ERROR_CODE_S_OK;
}
