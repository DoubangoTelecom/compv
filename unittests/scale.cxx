#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestScaling"
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

static const struct compv_unittest_scale {
	COMPV_INTERPOLATION_TYPE type;
	float factor;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_SCALE[] =
{
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 0.5f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "2199de43fe7cadd102a5f91cafef2e98" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "5cd9916ba99a280fc314c1767e16e7e8" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "a8321cce1364dfaf83e8b2fbf2ee6436" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "c0bca2ff876907af9c9568911128e0a0" },

	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 0.5f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "3402c4bab7e3850cdb3b7df25025ad2b" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "a3290801d97b15d73fc523997acbdc70" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "9b7aad661ac2059fa5955e32f158a329" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "bebaa908c915d818f6d04e1105707485" },

	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 0.5f, FILE_NAME_GRIOTS, 480, 640, 480, "c44db54243b04d92ef7e9e373a8f88c5" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "1f67605aae23461222939da568dd7e29" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "b9461507269a7ddb7ce18bc2f1f836df" },
	{ COMPV_INTERPOLATION_TYPE_BILINEAR, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "130ffba1d1bd05bd7a86067db6a5d082" },

	{ COMPV_INTERPOLATION_TYPE_BICUBIC, 0.5f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "d838342cf629f4d9bb7365cef1806336" },
	{ COMPV_INTERPOLATION_TYPE_BICUBIC, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "78afb82fe4b1816d1758dea313debf7a" },
	{ COMPV_INTERPOLATION_TYPE_BICUBIC, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "50a97eaa86300b5d6adb857fcd666eba" },
	{ COMPV_INTERPOLATION_TYPE_BICUBIC, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "c53a34751ead9d8ef0c91f7632df82c6" },
};
static const size_t COMPV_UNITTEST_SCALE_COUNT = sizeof(COMPV_UNITTEST_SCALE) / sizeof(COMPV_UNITTEST_SCALE[0]);

static const std::string compv_unittest_scale_to_string(const compv_unittest_scale* test) {
	return
		std::string("type:") + CompVBase::to_string(test->type) + std::string(", ")
		+ std::string("factor:") + CompVBase::to_string(test->factor) + std::string(", ")
		+ std::string("filename:") + std::string(test->filename);
}

COMPV_ERROR_CODE unittest_scale()
{
	COMPV_ERROR_CODE err;
	CompVMatPtr srcImage, dstImage;
	const compv_unittest_scale* test = NULL;

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_SCALE_COUNT; ++i) {
		test = &COMPV_UNITTEST_SCALE[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Image scaling -> %s ==", compv_unittest_scale_to_string(test).c_str());
		COMPV_CHECK_CODE_BAIL(err = CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &srcImage));
		COMPV_CHECK_CODE_BAIL(err = CompVImage::scale(srcImage, &dstImage, static_cast<size_t>(test->width * test->factor), static_cast<size_t>(test->height * test->factor), test->type));
		COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(dstImage)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image scaling MD5 mismatch");
		dstImage = NULL; // do not reuse
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
bail:
	return err;
}
