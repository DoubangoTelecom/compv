#include "../tests_common.h"

#define TAG_TEST								"TestMatrixOps_SubMul"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/test_images/"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/test_images/"
#else
#	define COMPV_TEST_IMAGE_FOLDER				"/something/"
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"

static const struct compv_unittest_submul {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const float subVal;
	const float mulVal;
	const char* md5;
} COMPV_UNITTEST_SUBMUL[] = {
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, 127.f, 128.f, "38ac4717daf1b232857193f53a25d5cf" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, 50.f, -89.f, "124881221322d133a1f6be4c2e6b24a2" },
};
static const size_t COMPV_UNITTEST_SUBMUL_COUNT = sizeof(COMPV_UNITTEST_SUBMUL) / sizeof(COMPV_UNITTEST_SUBMUL[0]);

#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR

#define LOOP_COUNT			100

COMPV_ERROR_CODE matrix_ops_submul()
{
	const compv_unittest_submul* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_SUBMUL_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_SUBMUL[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_SUBMUL[i];
			break;
		}
	}
	COMPV_ASSERT(test != nullptr);

	CompVMatPtr inMat, outMat;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(
		COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride,
		CompVFileUtils::patchFullPath((std::string(COMPV_TEST_IMAGE_FOLDER) + std::string(test->filename)).c_str()).c_str(),
		&inMat
	));
	// I want the width/height to be odd (e.g. 1281x721) in order to have orphans
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(inMat, &inMat, test->width + 1, test->height + 1, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMath::subMul(inMat, test->subVal, test->mulVal, &outMat));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Math SubMul Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(outMat).c_str());

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(outMat).compare(test->md5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Math Scale mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
