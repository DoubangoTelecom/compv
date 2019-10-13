#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestSplit"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

static const struct compv_unittest_packing {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_SUBTYPE subtype;
}
COMPV_UNITTEST_PACKINGS[] =
{
	{ "equirectangular_1282x720_bgr.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_BGR24 },
	{ "equirectangular_1282x720_rgb.rgb", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_RGB24 },
	{ "equirectangular_1282x720_nv12.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_NV12 },
	{ "equirectangular_1282x720_nv21.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_NV21 },
	{ "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV420P },
	{ "equirectangular_1282x720_yuv422p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV422P },
	{ "equirectangular_1282x720_yuv444p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P },
};
static const size_t COMPV_UNITTEST_PACKINGS_COUNT = sizeof(COMPV_UNITTEST_PACKINGS) / sizeof(COMPV_UNITTEST_PACKINGS[0]);

COMPV_ERROR_CODE unittest_packing()
{
	for (size_t i = 0; i < COMPV_UNITTEST_PACKINGS_COUNT; ++i) {
		CompVMatPtr imageIn, imageOut;
		const compv_unittest_packing& test = COMPV_UNITTEST_PACKINGS[i];
		COMPV_CHECK_CODE_RETURN(CompVImage::read(test.subtype, test.width, test.height, test.stride, COMPV_TEST_PATH_TO_FILE(test.filename).c_str(), &imageIn));		
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: packing -> %s ==", test.filename);
		std::vector<CompVMatPtr> imageOutVector;
		COMPV_CHECK_CODE_RETURN(CompVImage::unpack(imageIn, imageOutVector));
		COMPV_CHECK_CODE_RETURN(CompVImage::pack(imageOutVector, test.subtype, &imageOut));
		COMPV_CHECK_EXP_RETURN(compv_tests_md5(imageOut).compare(compv_tests_md5(imageIn)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Packing MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
	return COMPV_ERROR_CODE_S_OK;
}
