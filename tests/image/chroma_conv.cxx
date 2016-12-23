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

#define COMPV_TEST_IMAGE_CHROMA_CONV_RGB24_TO_YUV444			1

#define COMPV_loopCount											1

// To avoid timing 'readPixels' when loopCount is > 1
static CompVMatPtr imageRGB24, imageYUV444P;

static COMPV_ERROR_CODE chroma_conv_rgb24_to_yuv444p();

COMPV_ERROR_CODE chroma_conv()
{
	COMPV_DEBUG_INFO_EX(TAG_TEST_IMAGE_CHROMA_CONV, "%s", __FUNCTION__);
	COMPV_ERROR_CODE err;
	uint64_t timeStart, timeEnd;

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < COMPV_loopCount; ++i) {
		// RGB -> YUV444P
#		if COMPV_TEST_IMAGE_CHROMA_CONV_RGB24_TO_YUV444
		COMPV_CHECK_CODE_BAIL(err = chroma_conv_rgb24_to_yuv444p(), "[" TAG_TEST_IMAGE_CHROMA_CONV "]" "Failed test: RGB24 -> YUV444P");
#		endif
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO_EX(TAG_TEST_IMAGE_CHROMA_CONV, "Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// To make sure 'COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS' will be correct
	imageRGB24 = imageYUV444P = NULL;

bail:
	return err;
}

#define COMPV_TEST_WRITE_OUTPUT									COMPV_loopCount == 1 && !COMPV_OS_ANDROID
#define COMPV_TEST_CHECK_MD5									COMPV_loopCount == 1

/* RGB24 -> YUV444P */
static COMPV_ERROR_CODE chroma_conv_rgb24_to_yuv444p()
{
#if COMPV_loopCount == 1
	COMPV_DEBUG_INFO_EX(TAG_TEST_IMAGE_CHROMA_CONV, "%s", __FUNCTION__);
#endif
	static COMPV_ERROR_CODE err;
	static std::string md5;
	static const std::string kExpectedMD5 = "d0fcdce097c1368e9f741777984fe797";
	if (!imageRGB24) { // To avoid timing 'readPixels' when loopCount is > 1
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB24, 706, 472, 706, COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE("girl_706x472x706_rgb.rgb").c_str(), &imageRGB24));
	}
	COMPV_CHECK_CODE_BAIL(err = CompVImage::convert(imageRGB24, COMPV_SUBTYPE_PIXELS_YUV444P, &imageYUV444P));
#if COMPV_TEST_WRITE_OUTPUT
	COMPV_CHECK_CODE_BAIL(err = tests_write_to_file(imageYUV444P, "yuv444p.yuv"));
#endif
#if COMPV_TEST_CHECK_MD5
	COMPV_CHECK_EXP_BAIL(kExpectedMD5.compare(tests_md5(imageYUV444P)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "RGB24 -> YUV444P: MD5 mismatch");
#endif

bail:
	return err;
}