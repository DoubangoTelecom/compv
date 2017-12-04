#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestMathMorph"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/morpho"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/morpho"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_DIFFRACT		"diffract_1285x1285_gray.yuv"

#define LOOP_COUNT				1

static const struct compv_unittest_morph {
	const COMPV_MATH_MORPH_STREL_TYPE strelType;
	const CompVSizeSz strelSize;
	const COMPV_MATH_MORPH_OP_TYPE opType;
	const COMPV_BORDER_TYPE borderType;
	const char* filename;
	const size_t width;
	const size_t height;
	const size_t stride;
	const char* md5;
} COMPV_UNITTEST_MORPH[] =
{
	/* 0 */{ COMPV_MATH_MORPH_STREL_TYPE_CROSS, CompVSizeSz(3, 3), COMPV_MATH_MORPH_OP_TYPE_ERODE, COMPV_BORDER_TYPE_REPLICATE, FILE_NAME_DIFFRACT, 1285, 1285, 1285, "6656fd431e36238a89ac4d19e2b2e323" },
	/* 1 */{ COMPV_MATH_MORPH_STREL_TYPE_CROSS, CompVSizeSz(3, 3), COMPV_MATH_MORPH_OP_TYPE_DILATE, COMPV_BORDER_TYPE_REPLICATE, FILE_NAME_DIFFRACT, 1285, 1285, 1285, "c74f75cbb4bdcc82c29fe8a45ee9b913" },
	/* 2 */{ COMPV_MATH_MORPH_STREL_TYPE_CROSS, CompVSizeSz(3, 3), COMPV_MATH_MORPH_OP_TYPE_OPEN, COMPV_BORDER_TYPE_REPLICATE, FILE_NAME_DIFFRACT, 1285, 1285, 1285, "24ce54c1ff469e4a7a4b0ac9b8db409d" },
	/* 3 */{ COMPV_MATH_MORPH_STREL_TYPE_CROSS, CompVSizeSz(3, 3), COMPV_MATH_MORPH_OP_TYPE_CLOSE, COMPV_BORDER_TYPE_REPLICATE, FILE_NAME_DIFFRACT, 1285, 1285, 1285, "de50fffd6bab93d5859b00fbd52672f4" },
};
static const size_t COMPV_UNITTEST_MORPH_COUNT = sizeof(COMPV_UNITTEST_MORPH) / sizeof(COMPV_UNITTEST_MORPH[0]);

static const std::string compv_unittest_morph_to_string(const size_t index) {
	COMPV_ASSERT(index < COMPV_UNITTEST_MORPH_COUNT);
	const compv_unittest_morph* test = &COMPV_UNITTEST_MORPH[index];
		return
		std::string("index:") + CompVBase::to_string(index) + std::string(", ")
		+ std::string("filename:") + std::string(test->filename);
}

COMPV_ERROR_CODE unittest_math_morph()
{
	for (size_t index = 0; index < COMPV_UNITTEST_MORPH_COUNT; ++index) {
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: morph -> %s ==", compv_unittest_morph_to_string(index).c_str());
		const compv_unittest_morph* test = &COMPV_UNITTEST_MORPH[index];
		// Build structuring element
		CompVMatPtr strel;
		COMPV_CHECK_CODE_RETURN(CompVMathMorph::buildStructuringElement(&strel, test->strelSize, test->strelType));
		// Read image and thresholding to binar (morph ops work on grayscale but binar for fun)
		CompVMatPtr imageIn, imageOut;
		COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		COMPV_CHECK_CODE_RETURN(CompVImageThreshold::global(imageIn, &imageIn, 128.0));		
		COMPV_CHECK_CODE_RETURN(CompVMathMorph::process(imageIn, strel, &imageOut, test->opType, test->borderType));
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Morph MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}