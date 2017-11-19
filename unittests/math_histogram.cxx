#include "../tests/tests_common.h"

#define TAG_UNITTESTS								"UnitTestHistogram"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_CHROMA_CONV_IMAGE_FOLDER)

#define EXPECTED_MD5		"9cc75b0ec5f1522cfc65b7fadc0aacdb"

COMPV_ERROR_CODE unittest_histogram()
{
	CompVMatPtr image, histogram;
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_IMAGE_CHROMA_CONV_PATH_TO_FILE("equirectangular_1282x720_gray.yuv").c_str(), &image));
	
	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "== Trying new test: histogram on 'equirectangular_1282x720_gray.yuv'");
	COMPV_CHECK_CODE_RETURN(CompVMathHistogram::build(image, &histogram));
	COMPV_CHECK_EXP_RETURN(compv_tests_md5(histogram).compare(EXPECTED_MD5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "MD5 mismatch");
	COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Test OK **");

	return COMPV_ERROR_CODE_S_OK;
}