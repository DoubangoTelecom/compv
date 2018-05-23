#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestGradient"
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

static const struct compv_unittest_gradient {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	bool fltp;
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

static const std::string compv_unittest_gradient_to_string(const compv_unittest_gradient* test) {
	return std::string("filename:") + std::string(test->filename) 
		+ std::string(", flt:") + std::string(test->fltp ? "true" : "false")
		+ std::string(", xdir:") + std::string(test->xdir ? "true" : "false");
}

COMPV_ERROR_CODE unittest_gradient()
{
	CompVMatPtr image, grad;
	for (size_t i = 0; i < COMPV_UNITTEST_GRADIENT_COUNT; ++i) {
		const compv_unittest_gradient* test = &COMPV_UNITTEST_GRADIENT[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Gradient -> %s ==", compv_unittest_gradient_to_string(test).c_str());
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
		COMPV_ERROR_CODE(*fncptr)(const CompVMatPtr& input, CompVMatPtrPtr output, bool outputFloat)
			= test->xdir ? CompVImage::gradientX : CompVImage::gradientY;
		COMPV_CHECK_CODE_RETURN(fncptr(image, &grad, test->fltp));
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(grad)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Gradient MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}
