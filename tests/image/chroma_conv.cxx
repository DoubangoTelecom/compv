#include "../tests_common.h"

#define TAG_TEST_IMAGE_CHROMA_CONV								"TestImageChromaConv"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER)

#define COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_SRC				COMPV_SUBTYPE_PIXELS_RGBA32
#define COMPV_TEST_IMAGE_CHROMA_CONV_SUBTYPE_DST				COMPV_SUBTYPE_PIXELS_HSV

#define COMPV_loopCount											1

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
	{ COMPV_SUBTYPE_PIXELS_RGBA32, "equirectangular_1282x720_rgba.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d59738f116f59328f8e2ec80312d2ab3" },
	{ COMPV_SUBTYPE_PIXELS_ARGB32, "equirectangular_1282x720_argb.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "266a6987b353e0836426a472ea43ac82" },
	{ COMPV_SUBTYPE_PIXELS_BGRA32, "equirectangular_1282x720_bgra.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d59738f116f59328f8e2ec80312d2ab3" },
	{ COMPV_SUBTYPE_PIXELS_RGB24, "equirectangular_1282x720_rgb.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d59738f116f59328f8e2ec80312d2ab3" },
	{ COMPV_SUBTYPE_PIXELS_BGR24, "equirectangular_1282x720_bgr.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "d59738f116f59328f8e2ec80312d2ab3" },
	{ COMPV_SUBTYPE_PIXELS_RGB565LE, "equirectangular_1282x720_rgb565le.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "58d4da51e8af9612fa35e8028c2effec" },
	{ COMPV_SUBTYPE_PIXELS_RGB565BE, "equirectangular_1282x720_rgb565be.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "53d957241829dd00bcab558837d6e6cb" },
	{ COMPV_SUBTYPE_PIXELS_BGR565LE, "equirectangular_1282x720_bgr565le.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "7c703cad0d6696016c9dae355ee8949e" },
	{ COMPV_SUBTYPE_PIXELS_BGR565BE, "equirectangular_1282x720_bgr565be.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P, "yuv444p.yuv", "53d957241829dd00bcab558837d6e6cb" },

	/* to Grayscale */
	{ COMPV_SUBTYPE_PIXELS_RGBA32, "equirectangular_1282x720_rgba.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "68036672dc25d5400c6fe989801791f3" },
	{ COMPV_SUBTYPE_PIXELS_ARGB32, "equirectangular_1282x720_argb.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "a2cf188bb8d06b043aa007f020b99700" },
	{ COMPV_SUBTYPE_PIXELS_BGRA32, "equirectangular_1282x720_bgra.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "68036672dc25d5400c6fe989801791f3" },
	{ COMPV_SUBTYPE_PIXELS_RGB24, "equirectangular_1282x720_rgb.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "68036672dc25d5400c6fe989801791f3" },
	{ COMPV_SUBTYPE_PIXELS_BGR24, "equirectangular_1282x720_bgr.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "68036672dc25d5400c6fe989801791f3" },
	{ COMPV_SUBTYPE_PIXELS_RGB565LE, "equirectangular_1282x720_rgb565le.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "d2f741f9a219d63f0f51be51c1f5c773" },
	{ COMPV_SUBTYPE_PIXELS_RGB565BE, "equirectangular_1282x720_rgb565be.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "f40fc9b5d0422a80c55acd3c52df6942" },
	{ COMPV_SUBTYPE_PIXELS_BGR565LE, "equirectangular_1282x720_bgr565le.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "b8250e94bdcd9ba7c99495923d0fffbf" },
	{ COMPV_SUBTYPE_PIXELS_BGR565BE, "equirectangular_1282x720_bgr565be.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "f40fc9b5d0422a80c55acd3c52df6942" },
	{ COMPV_SUBTYPE_PIXELS_Y, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "8749c4d0f7730b0b92ef492a2936eb84" },
	{ COMPV_SUBTYPE_PIXELS_NV12, "equirectangular_1282x720_nv12.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" },
	{ COMPV_SUBTYPE_PIXELS_NV21, "equirectangular_1282x720_nv21.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" },
	{ COMPV_SUBTYPE_PIXELS_YUV420P, "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" },
	{ COMPV_SUBTYPE_PIXELS_YVU420P, "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" },
	{ COMPV_SUBTYPE_PIXELS_YUV422P, "equirectangular_1282x720_yuv422p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" },
	{ COMPV_SUBTYPE_PIXELS_YUV444P, "equirectangular_1282x720_yuv444p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" },
	{ COMPV_SUBTYPE_PIXELS_YUYV422, "equirectangular_1282x720_yuyv422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" }, // not planar YUV
	{ COMPV_SUBTYPE_PIXELS_UYVY422, "equirectangular_1282x720_uyvy422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_Y, "gray.yuv", "70e11d274bf329c5680956ecdf8357f3" }, // not planar YUV

	/* to RGB24 */
	{ COMPV_SUBTYPE_PIXELS_NV12, "equirectangular_1282x720_nv12.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24, "rgb24.rbg", "79d19cba673365246d4de726a32b5897" },
	{ COMPV_SUBTYPE_PIXELS_NV21, "equirectangular_1282x720_nv21.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24, "rgb24.rbg", "79d19cba673365246d4de726a32b5897" },
	{ COMPV_SUBTYPE_PIXELS_YUV420P, "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24, "rgb24.rbg", "79d19cba673365246d4de726a32b5897" },
	{ COMPV_SUBTYPE_PIXELS_YUV422P, "equirectangular_1282x720_yuv422p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24, "rgb24.rbg", "955e5abb0ad04e499bf7b2f6d9bfdf31" },
	{ COMPV_SUBTYPE_PIXELS_YUV444P, "equirectangular_1282x720_yuv444p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24, "rgb24.rbg", "e99af916cebf41afaa24d0259b7d7ab5" },
	{ COMPV_SUBTYPE_PIXELS_YUYV422, "equirectangular_1282x720_yuyv422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24, "rgb24.rbg", "8247f529fc91dfb91f3688d47846371e" }, // not planar YUV
	{ COMPV_SUBTYPE_PIXELS_UYVY422, "equirectangular_1282x720_uyvy422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24, "rgb24.rbg", "39f01e1555d2df71d257dbf934599764" }, // not planar YUV

	/* to RGBA32 */
	{ COMPV_SUBTYPE_PIXELS_NV12, "equirectangular_1282x720_nv12.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGBA32, "rgba32.rbg", "76d449f6bf380803878f1145999f5b41" },
	{ COMPV_SUBTYPE_PIXELS_NV21, "equirectangular_1282x720_nv21.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGBA32, "rgba32.rbg", "76d449f6bf380803878f1145999f5b41" },
	{ COMPV_SUBTYPE_PIXELS_YUV420P, "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGBA32, "rgba32.rbg", "76d449f6bf380803878f1145999f5b41" },
	{ COMPV_SUBTYPE_PIXELS_YUV422P, "equirectangular_1282x720_yuv422p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGBA32, "rgba32.rbg", "82527b45e0501ad25321ff55bffcfadd" },
	{ COMPV_SUBTYPE_PIXELS_YUV444P, "equirectangular_1282x720_yuv444p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGBA32, "rgba32.rbg", "c846ea9cb63e8182bfaacd69b1af3cd3" },
	{ COMPV_SUBTYPE_PIXELS_YUYV422, "equirectangular_1282x720_yuyv422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGBA32, "rgba32.rbg", "6966e886a56a1344d5704e4929b9d9eb" }, // not planar YUV
	{ COMPV_SUBTYPE_PIXELS_UYVY422, "equirectangular_1282x720_uyvy422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGBA32, "rgba32.rbg", "afb8ed0445fd9d6e52a1e0c33c3ff2b3" }, // not planar YUV

	/* to HSV */
	{ COMPV_SUBTYPE_PIXELS_RGBA32, "equirectangular_1282x720_rgba.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "f662a39a8fc56e87120c62442dbc1bb2" },
	{ COMPV_SUBTYPE_PIXELS_RGB24, "equirectangular_1282x720_rgb.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "f662a39a8fc56e87120c62442dbc1bb2" },
	{ COMPV_SUBTYPE_PIXELS_NV12, "equirectangular_1282x720_nv12.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "8e0235911c9c8556f283ccc9c9cf06c0" },
	{ COMPV_SUBTYPE_PIXELS_NV21, "equirectangular_1282x720_nv21.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "8e0235911c9c8556f283ccc9c9cf06c0" },
	{ COMPV_SUBTYPE_PIXELS_YUV420P, "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "8e0235911c9c8556f283ccc9c9cf06c0" },
	{ COMPV_SUBTYPE_PIXELS_YUV422P, "equirectangular_1282x720_yuv422p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "86e1d070a433661131d120d96478c690" },
	{ COMPV_SUBTYPE_PIXELS_YUV444P, "equirectangular_1282x720_yuv444p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "447080a29c02f1acea8829c05bda5888" },
	{ COMPV_SUBTYPE_PIXELS_YUYV422, "equirectangular_1282x720_yuyv422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "39a8089b5b8ce0ff05d5cffb9b4acfdd" }, // not planar YUV
	{ COMPV_SUBTYPE_PIXELS_UYVY422, "equirectangular_1282x720_uyvy422.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_HSV, "hsv.rbg", "12b6ab6617f8ff7d99c199249392e5e6" }, // not planar YUV
};
static size_t COMPV_TEST_IMAGE_CHROMA_CONV_TESTS_COUNT = sizeof(COMPV_TEST_IMAGE_CHROMA_CONV_TESTS) / sizeof(COMPV_TEST_IMAGE_CHROMA_CONV_TESTS[0]);

COMPV_ERROR_CODE chroma_conv()
{
#	define COMPV_TEST_WRITE_OUTPUT	COMPV_loopCount == 1 && !COMPV_OS_ANDROID  && !COMPV_OS_IPHONE
#	define COMPV_TEST_CHECK_MD5		COMPV_loopCount == 1
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
	timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < COMPV_loopCount; ++i) {
		COMPV_CHECK_CODE_BAIL(err = CompVImage::convert(srcImage, test->dstPixelFormat, &dstImage));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST_IMAGE_CHROMA_CONV, "Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_TEST_WRITE_OUTPUT
	COMPV_CHECK_CODE_BAIL(err = compv_tests_write_to_file(dstImage, test->dstFilename));
#endif

#if COMPV_TEST_CHECK_MD5
	COMPV_DEBUG_INFO_EX(TAG_TEST_IMAGE_CHROMA_CONV, "MD5:%s", compv_tests_md5(dstImage).c_str());
	COMPV_CHECK_EXP_BAIL(std::string(test->dstMD5).compare(compv_tests_md5(dstImage)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "MD5 mismatch");
#endif

bail:
	return err;
}
