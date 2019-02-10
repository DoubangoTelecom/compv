#include "../tests_common.h"

#define TAG_TEST								"TestScale"
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
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

static const struct compv_unittest_scale {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const bool float64Typ;
	double factor;
	const char* md5;
} COMPV_UNITTEST_SCALE[] = {
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, true, 98.257, "576c2c700366e3f191072e3c7985fb36" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, false, 798.257, "b9a962ba6daa3234fab47dbb87d108a6" },
};
static const size_t COMPV_UNITTEST_SCALE_COUNT = sizeof(COMPV_UNITTEST_SCALE) / sizeof(COMPV_UNITTEST_SCALE[0]);

#define SCALE_FLOAT64		false
#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR

#define LOOP_COUNT			1

COMPV_ERROR_CODE scale()
{
	const compv_unittest_scale* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_SCALE_COUNT; ++i) {
		if (COMPV_UNITTEST_SCALE[i].float64Typ == SCALE_FLOAT64 && std::string(COMPV_UNITTEST_SCALE[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_SCALE[i];
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
	// I want the width to be odd (e.g. 1281x721) in order to have orphans
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(inMat, &inMat, 1285, 721, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32));
	if (test->float64Typ) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<float, double>(inMat, &inMat)));
		// Add some negative numbers and some out-of-rage values
		*inMat->ptr<double>(5, 6) *= -9.044;
		*inMat->ptr<double>(0, 1) *= -8.50;
		*inMat->ptr<double>(1, 7) *= -1.50;
		*inMat->ptr<double>(8, 11) = -708.39641853226408; // ret = 0
		*inMat->ptr<double>(4, 9) = 709.78271289338397; // ret = inf
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMath::scale(inMat, test->factor, &outMat));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Math Scale Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(outMat).c_str());

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(outMat).compare(test->md5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Math Scale mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
