#include "../tests_common.h"

#define TAG_TEST								"TestImageScale"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

#define LOOP_COUNT		100
#define MD5				"30e4701c1e5a28e2ee4d2d6b66e5a97c"

COMPV_ERROR_CODE sobel()
{
	CompVEdgeDetePtr dete;
	CompVMatPtr image, edges;

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&dete, COMPV_SOBEL_ID));
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_EQUIRECTANGULAR).c_str(), &image));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(dete->process(image, &edges));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Sobel Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5: %s", compv_tests_md5(edges).c_str());

#if COMPV_OS_WINDOWS
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(edges, "sobel.gray"));
#endif

	COMPV_CHECK_EXP_RETURN(std::string(MD5).compare(compv_tests_md5(edges)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sobel MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
