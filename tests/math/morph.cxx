#include "../tests_common.h"

#define TAG_TEST								"TestMathMorph"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/morpho"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/morpho"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_DIFFRACT		"diffract_1285x1285_gray.yuv"

#define LOOP_COUNT				1000

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
	/* 1 */{ COMPV_MATH_MORPH_STREL_TYPE_CROSS, CompVSizeSz(3, 3), COMPV_MATH_MORPH_OP_TYPE_DILATE, COMPV_BORDER_TYPE_REPLICATE, FILE_NAME_DIFFRACT, 1285, 1285, 1285, "-" },
	/* 2 */{ COMPV_MATH_MORPH_STREL_TYPE_CROSS, CompVSizeSz(3, 3), COMPV_MATH_MORPH_OP_TYPE_OPEN, COMPV_BORDER_TYPE_REPLICATE, FILE_NAME_DIFFRACT, 1285, 1285, 1285, "-" },
	/* 3 */{ COMPV_MATH_MORPH_STREL_TYPE_CROSS, CompVSizeSz(3, 3), COMPV_MATH_MORPH_OP_TYPE_CLOSE, COMPV_BORDER_TYPE_REPLICATE, FILE_NAME_DIFFRACT, 1285, 1285, 1285, "-" },
};
static const size_t COMPV_UNITTEST_MORPH_COUNT = sizeof(COMPV_UNITTEST_MORPH) / sizeof(COMPV_UNITTEST_MORPH[0]);

#define TEST_INDEX	0

COMPV_ERROR_CODE morph()
{
	COMPV_ASSERT(TEST_INDEX >= 0 && TEST_INDEX < COMPV_UNITTEST_MORPH_COUNT);
	const compv_unittest_morph* test = &COMPV_UNITTEST_MORPH[TEST_INDEX];

	// Build structuring element
	CompVMatPtr strel;
	COMPV_CHECK_CODE_RETURN(CompVMathMorph::buildStructuringElement(&strel, test->strelSize, test->strelType));
	// Read image and thresholding to binar (morph ops work on grayscale but binar for fun)
	CompVMatPtr imageIn, imageOut;
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::global(imageIn, &imageIn, 128.0));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathMorph::process(imageIn, strel, &imageOut, test->opType, test->borderType));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Morph Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 1
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, "morph.gray"));
#endif

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(imageOut).c_str());
	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Morph MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}