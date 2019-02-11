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

#define ANGLE_45	{ 0.707107, 0.707107, -66.8139, -0.707107, 0.707107, 558.697 }
#define ANGLE_180	{ -1, 1.22465e-16, 1282, -1.22465e-16, -1, 720 }

static const struct compv_unittest_rotate {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	COMPV_INTERPOLATION_TYPE interp;
	const double matrix[2*3];
	bool doublePrecision;
	const char* md5;
}
COMPV_UNITTEST_ROTATE[] =
{
	// Next MD5 values true when FMA3 enabled
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC, ANGLE_45, false, "db39e290bf8be7576a04ee9798283978" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC, ANGLE_180, true, "a81a807429dd6ec7046c2e74a5c6f0b6" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32, ANGLE_180, false, "6b7b15daaf46c0939a4cbebd2787df51" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32, ANGLE_45, true, "cd0c56a23a511c2d13d4e9def39a26f1" },

	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR, ANGLE_45, false, "489c6b8747c6fa7c52d17c00621fe000" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR, ANGLE_180, true, "ada2eb40c50ad02a270a26c746bc52a8" },
	//{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32, ANGLE_180, false, "-" },
	//{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32, ANGLE_45, true, "-" },
};
static const size_t COMPV_UNITTEST_ROTATE_COUNT = sizeof(COMPV_UNITTEST_ROTATE) / sizeof(COMPV_UNITTEST_ROTATE[0]);

#define LOOP_COUNT			1
#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR
#define DEFAULT_PIXEL		0x00
#define DOUBLE_PRECISION	true
#define INTERPOLATION		COMPV_INTERPOLATION_TYPE_BILINEAR


COMPV_ERROR_CODE rotate()
{
	const compv_unittest_rotate* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_ROTATE_COUNT; ++i) {
		if (COMPV_UNITTEST_ROTATE[i].doublePrecision == DOUBLE_PRECISION && COMPV_UNITTEST_ROTATE[i].interp == INTERPOLATION) {
			test = &COMPV_UNITTEST_ROTATE[i];
			break;
		}
	}
	COMPV_ASSERT(test != nullptr);
	
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
	if (!test->doublePrecision) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<double, float>(matrix, &matrix)));
	}

	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVImage::warp(imageIn, &imageOut, matrix, CompVSizeSz(test->width, test->height), test->interp, DEFAULT_PIXEL));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5: %s", compv_tests_md5(imageOut).c_str());

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, compv_tests_build_filename(imageOut).c_str()));
#endif

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, (COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image rotation MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
