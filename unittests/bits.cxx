#include "../tests/tests_common.h"

#define TAG_TEST							"UnitTestBitsOp"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

#define BITS_OP_AND						0
#define BITS_OP_NOT_AND					1
#define BITS_OP_NOT						2

static const struct compv_unittest_bits {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	int op;
	const char* md5;
}
COMPV_UNITTEST_BITS[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, BITS_OP_AND, "0d5f7f7b0cd1dce06243c62282cced51" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, BITS_OP_NOT_AND, "e631fc32b9ed1a4f87689905de5d30e9" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, BITS_OP_NOT, "836a66d41cab3dec2ad185e220feeeb7" },
};
static const size_t COMPV_UNITTEST_BITS_COUNT = sizeof(COMPV_UNITTEST_BITS) / sizeof(COMPV_UNITTEST_BITS[0]);

static const std::string compv_unittest_bits_to_string(const compv_unittest_bits* test) {
	static const char* ops_string[] = {
		"BITS_OP_AND",
		"BITS_OP_NOT_AND",
		"BITS_OP_NOT"
	};
	return std::string("filename:") + std::string(test->filename)
		+ std::string(", operation=") + std::string(ops_string[test->op]);
}
static COMPV_ERROR_CODE __build_mat(CompVMatPtrPtr imageOp, const compv_unittest_bits* test);

COMPV_ERROR_CODE unittest_bits()
{
	const compv_unittest_bits* test;
	CompVMatPtr imageIn, imageOp, imageOut;

	for (size_t i = 0; i < COMPV_UNITTEST_BITS_COUNT; ++i) {
		test = &COMPV_UNITTEST_BITS[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Bits -> %s ==", compv_unittest_bits_to_string(test).c_str());
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		switch (test->op) {
		case BITS_OP_AND:
			COMPV_CHECK_CODE_RETURN(__build_mat(&imageOp, test));
			COMPV_CHECK_CODE_RETURN(CompVBits::logical_and(imageIn, imageOp, &imageOut));
			break;
		case BITS_OP_NOT_AND:
			COMPV_CHECK_CODE_RETURN(__build_mat(&imageOp, test));
			COMPV_CHECK_CODE_RETURN(CompVBits::logical_not_and(imageIn, imageOp, &imageOut));
			break;
		case BITS_OP_NOT:
			COMPV_CHECK_CODE_RETURN(CompVBits::logical_not(imageIn, &imageOut));
			break;
		default:
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Op not implemented");
			break;
		}
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "BitsOps MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE __build_mat(CompVMatPtrPtr imageOp, const compv_unittest_bits* test)
{
	CompVMatPtr imageOp_ = *imageOp;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&imageOp_, test->height, test->width));
	uint8_t* imageOpPtr = imageOp_->ptr<uint8_t>();
	const size_t imageOpStride = imageOp_->stride();
	for (size_t j = 0; j < test->height; ++j) {
		for (size_t i = 0; i < test->width; ++i) {
			imageOpPtr[i] = static_cast<uint8_t>((j * i) & 0xff);
		}
		imageOpPtr += imageOpStride;
	}
	*imageOp = imageOp_;
	return COMPV_ERROR_CODE_S_OK;
}