#include "../tests_common.h"

#define TAG_TEST_IMAGE_CHROMA_CONV								"TestImageChromaConv"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE(filename)		tests_path_from_file(filename, COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER)

#define COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_SRC				COMPV_SUBTYPE_PIXELS_RGB24
#define COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_DST				COMPV_SUBTYPE_PIXELS_YUV444P

#define COMPV_loopCount											100000

static const struct compv_test_image_chroma_conv_test {
	COMPV_SUBTYPE srcPixelFormat;
	const char* srcFilename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_SUBTYPE dstPixelFormat;
	const char* dstFilename;
	const char* dstMD5;
}
COMPV_TEST_IMAGE_CHROMA_CONV_TESTS[] =
{
	/* to YUV444P */
	{ COMPV_SUBTYPE_PIXELS_RGBA32, "girl_706x472x706_rgba.rgb", 706, 472, 706, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d0fcdce097c1368e9f741777984fe797" },
	{ COMPV_SUBTYPE_PIXELS_ARGB32, "girl_706x472x706_argb.rgb", 706, 472, 706, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "dbc3115b10c38f1a4799570a4ada7d59" },
	{ COMPV_SUBTYPE_PIXELS_BGRA32, "girl_706x472x706_bgra.rgb", 706, 472, 706, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d0fcdce097c1368e9f741777984fe797" },
	{ COMPV_SUBTYPE_PIXELS_RGB24, "girl_706x472x706_rgb.rgb", 706, 472, 706, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d0fcdce097c1368e9f741777984fe797" },
	{ COMPV_SUBTYPE_PIXELS_BGR24, "girl_706x472x706_bgr.rgb", 706, 472, 706, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d0fcdce097c1368e9f741777984fe797" },
	{ COMPV_SUBTYPE_PIXELS_RGB565LE, "girl_706x472x706_rgb565le.rgb", 706, 472, 706, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "" },
	{ COMPV_SUBTYPE_PIXELS_RGB565BE, "girl_706x472x706_rgb565be.rgb", 706, 472, 706, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "" },
};
static size_t COMPV_TEST_IMAGE_CHROMA_CONV_TESTS_COUNT = sizeof(COMPV_TEST_IMAGE_CHROMA_CONV_TESTS) / sizeof(COMPV_TEST_IMAGE_CHROMA_CONV_TESTS[0]);

COMPV_ERROR_CODE chroma_conv()
{
#	define COMPV_TEST_WRITE_OUTPUT	COMPV_loopCount == 1 && !COMPV_OS_ANDROID
#	define COMPV_TEST_CHECK_MD5	COMPV_loopCount == 1
	COMPV_DEBUG_INFO_EX(TAG_TEST_IMAGE_CHROMA_CONV, "%s(%s -> %s)", __FUNCTION__, CompVGetSubtypeString(COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_SRC), CompVGetSubtypeString(COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_DST));
	COMPV_ERROR_CODE err;
	uint64_t timeStart, timeEnd;
	CompVMatPtr srcImage, dstImage;
	const compv_test_image_chroma_conv_test* test = NULL;

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Find test
	for (size_t i = 0; i < COMPV_TEST_IMAGE_CHROMA_CONV_TESTS_COUNT; ++i) {
		if (COMPV_TEST_IMAGE_CHROMA_CONV_TESTS[i].srcPixelFormat == COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_SRC && COMPV_TEST_IMAGE_CHROMA_CONV_TESTS[i].dstPixelFormat == COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_DST) {
			test = &COMPV_TEST_IMAGE_CHROMA_CONV_TESTS[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST_IMAGE_CHROMA_CONV, "Failed to find test: %s -> %s", CompVGetSubtypeString(COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_SRC), CompVGetSubtypeString(COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_DST));
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	// Read source file
	COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(test->srcPixelFormat, test->width, test->height, test->stride, COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE(test->srcFilename).c_str(), &srcImage));

	// Perform test
	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < COMPV_loopCount; ++i) {
		COMPV_CHECK_CODE_BAIL(err = CompVImage::convert(srcImage, test->dstPixelFormat, &dstImage));
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO_EX(TAG_TEST_IMAGE_CHROMA_CONV, "Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

#if COMPV_TEST_WRITE_OUTPUT
	COMPV_CHECK_CODE_BAIL(err = tests_write_to_file(dstImage, test->dstFilename));
#endif
#if COMPV_TEST_CHECK_MD5
	COMPV_CHECK_EXP_BAIL(std::string(test->dstMD5).compare(tests_md5(dstImage)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "RGB24 -> YUV444P: MD5 mismatch");
#endif

bail:
	return err;
}
