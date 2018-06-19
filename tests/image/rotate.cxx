#include "../tests_common.h"

#define TAG_TEST								"TestImageRotate"
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

#define ANGLE_45	0
#define ANGLE_180	1

static const struct compv_unittest_rotate {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_INTERPOLATION_TYPE interp;
	const double matrix[2*3];
	const char* md5;
}
COMPV_UNITTEST_ROTATE[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC, { 0.707107, 0.707107, -66.8139, -0.707107, 0.707107, 558.697 }, "800739b84f53a9c92123733906118fea" }, // 45°
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR, { -1, 1.22465e-16, 1282, -1.22465e-16, -1, 720 }, "88eb410ea856c451796e6dd5ba58f656" }, // 180°
};
static const size_t COMPV_UNITTEST_ROTATE_COUNT = sizeof(COMPV_UNITTEST_ROTATE) / sizeof(COMPV_UNITTEST_ROTATE[0]);

#define LOOP_COUNT		1
#define FILE_NAME		FILE_NAME_EQUIRECTANGULAR
#define ANGLE			ANGLE_45


COMPV_ERROR_CODE rotate()
{
	COMPV_ASSERT(ANGLE < COMPV_UNITTEST_ROTATE_COUNT);
	const compv_unittest_rotate* test = &COMPV_UNITTEST_ROTATE[ANGLE];
	
	CompVMatPtr imageIn, imageOut;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));

	CompVMatPtr matrix;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&matrix, 2, 3));
	*matrix->ptr<double>(0, 0) = test->matrix[0];
	*matrix->ptr<double>(0, 1) = test->matrix[1];
	*matrix->ptr<double>(0, 2) = test->matrix[2];
	*matrix->ptr<double>(1, 0) = test->matrix[3];
	*matrix->ptr<double>(1, 1) = test->matrix[4];
	*matrix->ptr<double>(1, 2) = test->matrix[5];

	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImage::warp(imageIn, &imageOut, matrix, CompVSizeSz(test->width, test->height), test->interp));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5: %s", compv_tests_md5(imageOut).c_str());

#if COMPV_OS_WINDOWS && 1
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, compv_tests_build_filename(imageOut).c_str()));
#endif

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, (COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image rotation MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
