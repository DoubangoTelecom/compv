#include "../tests_common.h"

#define TAG_TEST								"TestImagePyramid"
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

static const struct compv_unittest_pyramid {
	COMPV_SCALE_TYPE scaleType;
	float factor; // should be < 1
	size_t levels;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_PYRAMID[] =
{
	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, 6, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "ea54d2b04018d355792add02535eae23" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, 8, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "c10b3b41a97e6416058a55cf71463409" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.99f, 3, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "a9f2631d847e92fa45638523fd508cb9" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.47f, 7, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "6452b6e6bb410d9457560a8bb756dd3e" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, 6, FILE_NAME_OPENGLBOOK, 200, 258, 200, "b4a941a27b7d07a4ca9f3ef4e5bca69b" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, 8, FILE_NAME_OPENGLBOOK, 200, 258, 200, "70901e3e2c9107eda3b4403382f4b564" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.99f, 3, FILE_NAME_OPENGLBOOK, 200, 258, 200, "64e35be23429de29cd3b5ce2ea66c6cf" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.47f, 7, FILE_NAME_OPENGLBOOK, 200, 258, 200, "94eb334401cd781bdc079c398ef4c938" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, 6, FILE_NAME_GRIOTS, 480, 640, 480, "66e2b3eb67b82b8e2bf8760ca6171dc5" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, 8, FILE_NAME_GRIOTS, 480, 640, 480, "47178356656332c41bd1cc30ac04fcf4" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.99f, 3, FILE_NAME_GRIOTS, 480, 640, 480, "2db1f8f8fe5cd8b73eddf059126f364e" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.47f, 7, FILE_NAME_GRIOTS, 480, 640, 480, "e11f97f76b1ede2bf883ddddd05549c1" },
};
static const size_t COMPV_UNITTEST_PYRAMID_COUNT = sizeof(COMPV_UNITTEST_PYRAMID) / sizeof(COMPV_UNITTEST_PYRAMID[0]);

#define IMAGE_PYRAMID_FACTOR		.83f
#define IMAGE_PYRAMID_LEVELS		8
#define IMAGE_PYRAMID_SCALE_TYPE	COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_PYRAMID_FILENAME		FILE_NAME_EQUIRECTANGULAR

#define IMAGE_PYRAMID_LOOP_COUNT	1

static std::string compv_tests_pyramid_md5(const CompVImageScalePyramidPtr & pyramid)
{
	CompVMatPtr image;
	std::string md5_level;
	CompVMd5Ptr md5;
	COMPV_CHECK_CODE_ASSERT(CompVMd5::newObj(&md5));
	for (size_t i = 0; i < pyramid->levels(); ++i) {
		COMPV_CHECK_CODE_ASSERT(pyramid->image(i, &image));
		md5_level = compv_tests_md5(image);
		COMPV_CHECK_CODE_ASSERT(md5->update(reinterpret_cast<const uint8_t*>(md5_level.c_str()), md5_level.length()));
	}
	return md5->compute();
}

COMPV_ERROR_CODE pyramid()
{
	CompVMatPtr image;
	CompVImageScalePyramidPtr pyramid;
	uint64_t timeStart, timeEnd;
	const compv_unittest_pyramid* test = NULL;

	for (size_t i = 0; i < COMPV_UNITTEST_PYRAMID_COUNT; ++i) {
		if (
			COMPV_UNITTEST_PYRAMID[i].factor == IMAGE_PYRAMID_FACTOR
			&& COMPV_UNITTEST_PYRAMID[i].levels == IMAGE_PYRAMID_LEVELS
			&& COMPV_UNITTEST_PYRAMID[i].scaleType == IMAGE_PYRAMID_SCALE_TYPE
			&& std::string(COMPV_UNITTEST_PYRAMID[i].filename).compare(IMAGE_PYRAMID_FILENAME) == 0
			)
		{
			test = &COMPV_UNITTEST_PYRAMID[i];
			break;
		}
	}

	if (!test) {
		COMPV_DEBUG_ERROR_EX("TAG_TEST", "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	// Read image
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
	// Create the pyramid
	COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(&pyramid, test->factor, test->levels, test->scaleType));
	// process
	timeStart = CompVTime::nowMillis();
	for (int i = 0; i < IMAGE_PYRAMID_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(pyramid->process(image));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO("Elapsed time(TestPyramid) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	// Check MD5 values
	COMPV_CHECK_EXP_RETURN(compv_tests_pyramid_md5(pyramid).compare(test->md5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Image scaling MD5 mismatch");

	// dump latest image to file
#if COMPV_OS_WINDOWS && 1
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(pyramid->image(pyramid->levels() - 1, &image));
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(image, "out.gray"));
#endif

	return COMPV_ERROR_CODE_S_OK;
}