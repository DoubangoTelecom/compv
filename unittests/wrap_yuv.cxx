#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestWrapYuv"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

static const struct compv_unittest_wrapYuv {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_SUBTYPE subtype;
}
COMPV_UNITTEST_wrapYuvS[] =
{
	{ "equirectangular_1282x720_nv12.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_NV12 },
	{ "equirectangular_1282x720_nv21.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_NV21 },
	{ "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV420P },
	{ "equirectangular_1282x720_yuv422p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV422P },
	{ "equirectangular_1282x720_yuv444p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P },
};
static const size_t COMPV_UNITTEST_wrapYuvS_COUNT = sizeof(COMPV_UNITTEST_wrapYuvS) / sizeof(COMPV_UNITTEST_wrapYuvS[0]);

COMPV_ERROR_CODE unittest_wrapYuv()
{
	for (size_t i = 0; i < COMPV_UNITTEST_wrapYuvS_COUNT; ++i) {
		CompVMatPtr imageIn, imageOut;
		const compv_unittest_wrapYuv& test = COMPV_UNITTEST_wrapYuvS[i];
		COMPV_CHECK_CODE_RETURN(CompVImage::read(test.subtype, test.width, test.height, test.stride, COMPV_TEST_PATH_TO_FILE(test.filename).c_str(), &imageIn));
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: wrapYuv -> %s ==", test.filename);
		const bool packedUV = (imageIn->subType() == COMPV_SUBTYPE_PIXELS_NV12 || imageIn->subType() == COMPV_SUBTYPE_PIXELS_NV21);
		const int planeU = packedUV ? COMPV_PLANE_UV : COMPV_PLANE_U;
		const int planeV = packedUV ? COMPV_PLANE_UV : COMPV_PLANE_V;
		COMPV_CHECK_CODE_RETURN(CompVImage::wrapYuv(
			imageIn->subType(),
			imageIn->ptr<const void>(0, 0, COMPV_PLANE_Y),
			imageIn->ptr<const uint8_t>(0, 0, planeU),
			imageIn->ptr<const uint8_t>(0, 0, planeV) + (planeU == planeV),
			imageIn->cols(),
			imageIn->rows(),
			imageIn->strideInBytes(COMPV_PLANE_Y),
			imageIn->strideInBytes(planeU),
			imageIn->strideInBytes(planeV),
			&imageOut
		));
		COMPV_CHECK_EXP_RETURN(compv_tests_md5(imageOut).compare(compv_tests_md5(imageIn)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "wrapYuv MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
	return COMPV_ERROR_CODE_S_OK;
}
