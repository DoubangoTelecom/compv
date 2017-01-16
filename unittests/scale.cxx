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
	COMPV_SCALE_TYPE type;
	float factor;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_SCALE[] =
{
	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "6b2068a9654b5446aeac9edadb7eea2a" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "e63616c7c1307268b5447c41906d2cd1" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "cf1041696dc70ed595306c21968e2a60" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "93d0e529581ecea87746076072babefc" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "36f66634d0301becaf76478b53d234ec" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "039918770e7157c8b10b6bb45e4aa3b4" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "a535d1a0b7768cf3532fb48a7d3da234" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "8362de24ca64dea651ca50fe2700f23a" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, FILE_NAME_GRIOTS, 480, 640, 480, "7dee6e6e42a33c8ceb49f4a8a8cb7a6c" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "1e87b23e7736729769f8b252a8cf701e" },
	{ COMPV_SCALE_TYPE_BILINEAR, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "4053ab9cb684573f17e9fc52265a6a32" },
	{ COMPV_SCALE_TYPE_BILINEAR, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "f7c688791368aa1d8c2d8f4c66c5c394" },
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
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &srcImage));
		COMPV_CHECK_CODE_BAIL(err = CompVImage::scale(srcImage, &dstImage, static_cast<size_t>(test->width * test->factor), static_cast<size_t>(test->height * test->factor), test->type));
		COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(dstImage)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image scaling MD5 mismatch");
		dstImage = NULL; // do not reuse
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
bail:
	return err;
}
