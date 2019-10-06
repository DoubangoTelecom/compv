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

#define FILE_NAME_SPLIT3		"equirectangular_1282x720_rgb.rgb"

COMPV_ERROR_CODE packing()
{
	CompVMatPtr imageIn, imageOut;
	CompVMatPtrVector imageOutVector;

	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_RGB24, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_SPLIT3).c_str(), &imageIn));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImage::unpack(imageIn, imageOutVector));
		COMPV_CHECK_CODE_RETURN(CompVImage::pack(imageOutVector, &imageOut));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Pack/Unpack Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(imageOut).compare(compv_tests_md5(imageIn)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Pack/Unpack MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}