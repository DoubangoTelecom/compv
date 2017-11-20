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

static COMPV_ERROR_CODE unittest_split3()
{
#define FILE_NAME_SPLIT3		"equirectangular_1282x720_rgb.rgb"
#define MD5_0_SPLIT3			"91e688549da821a29d6285ca21d49e95"
#define MD5_1_SPLIT3			"f03d15c2b7b6e01258bb9138673bfb76"
#define MD5_2_SPLIT3			"83d470c5385fda95ec272d80ede96578"
	CompVMatPtr imageIn;
	std::vector<CompVMatPtr> imageOutVector;

	COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Split3 -> %s ==", FILE_NAME_SPLIT3);
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_RGB24, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_SPLIT3).c_str(), &imageIn));
	COMPV_CHECK_CODE_RETURN(CompVImage::split(imageIn, imageOutVector));
	COMPV_CHECK_EXP_RETURN(std::string(MD5_0_SPLIT3).compare(compv_tests_md5(imageOutVector[0])) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Split3(0) MD5 mismatch");
	COMPV_CHECK_EXP_RETURN(std::string(MD5_1_SPLIT3).compare(compv_tests_md5(imageOutVector[1])) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Split3(1) MD5 mismatch");
	COMPV_CHECK_EXP_RETURN(std::string(MD5_2_SPLIT3).compare(compv_tests_md5(imageOutVector[2])) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Split3(2) MD5 mismatch");
	COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_split()
{
	COMPV_CHECK_CODE_RETURN(unittest_split3());
	//COMPV_CHECK_CODE_RETURN(unittest_split2());
	return COMPV_ERROR_CODE_S_OK;
}
