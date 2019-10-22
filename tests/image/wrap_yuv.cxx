#include "../tests_common.h"

#define TAG_TEST								"TestImagePacking"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define LOOP_COUNT				1

#define TEST_SUBTYPE			COMPV_SUBTYPE_PIXELS_NV21

static const struct compv_unittest_packing {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_SUBTYPE subtype;
}
COMPV_UNITTEST_PACKINGS[] =
{
	{ "equirectangular_1282x720_nv12.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_NV12 },
	{ "equirectangular_1282x720_nv21.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_NV21 },
	{ "equirectangular_1282x720_yuv420p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV420P },
	{ "equirectangular_1282x720_yuv422p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV422P },
	{ "equirectangular_1282x720_yuv444p.yuv", 1282, 720, 1282, COMPV_SUBTYPE_PIXELS_YUV444P },
};
static const size_t COMPV_UNITTEST_PACKINGS_COUNT = sizeof(COMPV_UNITTEST_PACKINGS) / sizeof(COMPV_UNITTEST_PACKINGS[0]);

COMPV_ERROR_CODE wrap_yuv()
{
	CompVMatPtr imageIn, imageOut;
	CompVMatPtrVector imageOutVector;

	for (size_t i = 0; i < COMPV_UNITTEST_PACKINGS_COUNT; ++i) {
		const compv_unittest_packing& test = COMPV_UNITTEST_PACKINGS[i];
		if (test.subtype == TEST_SUBTYPE) {
			COMPV_CHECK_CODE_RETURN(CompVImage::read(test.subtype, test.width, test.height, test.stride, COMPV_TEST_PATH_TO_FILE(test.filename).c_str(), &imageIn));
			break;
		}
	}
	COMPV_ASSERT(imageIn != nullptr);

#if COMPV_OS_WINDOWS && 0 // To emulate Android YUV420SP hacking as YUV420P
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(CompVImage::wrapYuv(
		COMPV_SUBTYPE_PIXELS_YUV420P,
		imageIn->ptr<const void>(0, 0, COMPV_PLANE_Y),
		imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_UV) + 1, // NV21: V/U interleaved -> V first
		imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_UV) + 0,
		imageIn->cols(),
		imageIn->rows(),
		imageIn->strideInBytes(COMPV_PLANE_Y),
		imageIn->strideInBytes(COMPV_PLANE_UV),
		imageIn->strideInBytes(COMPV_PLANE_UV),
		&imageOut,
		2
	));
	COMPV_CHECK_CODE_RETURN(CompVImage::convert(imageOut, COMPV_SUBTYPE_PIXELS_RGB24, &imageOut));
	COMPV_CHECK_CODE_RETURN(CompVImage::encode("rgb24.png", imageOut));
#endif

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
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
#if COMPV_OS_WINDOWS && 0
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
		CompVMatPtr imageRGB24;
		COMPV_CHECK_CODE_RETURN(CompVImage::convert(imageOut, COMPV_SUBTYPE_PIXELS_RGB24, &imageRGB24));
		COMPV_CHECK_CODE_RETURN(CompVImage::encode("rgb24.png", imageRGB24));
#endif
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "wrapYuv Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(imageOut).compare(compv_tests_md5(imageIn)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "wrapYuv MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
