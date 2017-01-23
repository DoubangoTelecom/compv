#include "../tests/tests_common.h"

#define TAG_UNITTESTS								"UnitTestChromaConv"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER)

static const struct compv_unittest_chroma_conv {
	COMPV_SUBTYPE srcPixelFormat;
	const char* srcFilename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_SUBTYPE dstPixelFormat;
	const char* dstFilename;
	const char* dstMD5;
}
COMPV_UNITTESTS_CHROMA_CONV[] =
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
};
static size_t COMPV_UNITTESTS_CHROMA_CONV_COUNT = sizeof(COMPV_UNITTESTS_CHROMA_CONV) / sizeof(COMPV_UNITTESTS_CHROMA_CONV[0]);

static std::string unittest_chroma_conv_tostring(const compv_unittest_chroma_conv* test)
{
	return
		std::string("srcPixelFormat:") + std::string(CompVGetSubtypeString(test->srcPixelFormat)) + std::string(", ")
		+ std::string("srcFilename:") + std::string(test->srcFilename) + std::string(", ")
		+ std::string("dstPixelFormat:") + std::string(CompVGetSubtypeString(test->dstPixelFormat)) + std::string(", ")
		+ std::string("dstFilename:") + std::string(test->dstFilename) + std::string(", ");
}

COMPV_ERROR_CODE unittest_chroma_conv()
{
	const compv_unittest_chroma_conv* test;
	CompVMatPtr srcImage, dstImage;
	for (size_t i = 0; i < COMPV_UNITTESTS_CHROMA_CONV_COUNT; ++i) {
		test = &COMPV_UNITTESTS_CHROMA_CONV[i];
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "== Trying new test: Chroma conversion -> %s ==", unittest_chroma_conv_tostring(test).c_str());
		COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(test->srcPixelFormat, test->width, test->height, test->stride, COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE(test->srcFilename).c_str(), &srcImage));
		COMPV_CHECK_CODE_RETURN(CompVImage::convert(srcImage, test->dstPixelFormat, &dstImage));
		COMPV_CHECK_EXP_RETURN(std::string(test->dstMD5).compare(compv_tests_md5(dstImage)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "MD5 mismatch");
		dstImage = NULL; // do not reuse
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Test OK **");
	}
	return COMPV_ERROR_CODE_S_OK;
}
