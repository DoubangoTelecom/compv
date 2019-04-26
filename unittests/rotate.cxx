#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestRotate"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER				"/something/"
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

#define DEFAULT_PIXEL		0x00
#define ANGLE_45	{ 0.707107, 0.707107, -66.8139, -0.707107, 0.707107, 558.697 }
#define ANGLE_180	{ -1, 1.22465e-16, 1282, -1.22465e-16, -1, 720 }

static const struct compv_unittest_rotate {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_INTERPOLATION_TYPE interp;
	const double matrix[2 * 3];
	bool doublePrecision;
	const char* md5;
}
COMPV_UNITTEST_ROTATE[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC, ANGLE_45, false, "db39e290bf8be7576a04ee9798283978" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC, ANGLE_180, true, "a81a807429dd6ec7046c2e74a5c6f0b6" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32, ANGLE_180, false, "6b7b15daaf46c0939a4cbebd2787df51" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32, ANGLE_45, true, "cd0c56a23a511c2d13d4e9def39a26f1" },
	
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR, ANGLE_45, false, "489c6b8747c6fa7c52d17c00621fe000" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR, ANGLE_180, true, "ada2eb40c50ad02a270a26c746bc52a8" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32, ANGLE_180, false, "6b7b15daaf46c0939a4cbebd2787df51" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32, ANGLE_45, true, "333df22146b77d369dc8db496f757788" },
};
static const size_t COMPV_UNITTEST_ROTATE_COUNT = sizeof(COMPV_UNITTEST_ROTATE) / sizeof(COMPV_UNITTEST_ROTATE[0]);

static const std::string compv_unittest_rotate_to_string(const compv_unittest_rotate* test) {
	return
		std::string("interp:") + CompVBase::to_string(test->interp) + std::string(", ")
		+ std::string("filename:") + std::string(test->filename);
}

COMPV_ERROR_CODE unittest_rotate()
{
	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_ROTATE_COUNT; ++i) {
		const compv_unittest_rotate* test = &COMPV_UNITTEST_ROTATE[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Image rotation -> %s ==", compv_unittest_rotate_to_string(test).c_str());
		CompVMatPtr srcImage, dstImage, matrix;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&matrix, 2, 3));
		*matrix->ptr<double>(0, 0) = test->matrix[0];
		*matrix->ptr<double>(0, 1) = test->matrix[1];
		*matrix->ptr<double>(0, 2) = test->matrix[2];
		*matrix->ptr<double>(1, 0) = test->matrix[3];
		*matrix->ptr<double>(1, 1) = test->matrix[4];
		*matrix->ptr<double>(1, 2) = test->matrix[5];
		if (!test->doublePrecision) {
			COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<double, float>(matrix, &matrix)));
		}
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &srcImage));
		COMPV_CHECK_CODE_RETURN(CompVImage::warp(srcImage, &dstImage, matrix, CompVSizeSz(test->width, test->height), test->interp, DEFAULT_PIXEL));
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(dstImage)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED,"Image rotate MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
	return COMPV_ERROR_CODE_S_OK;
}
