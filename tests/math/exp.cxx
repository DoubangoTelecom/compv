#include "../tests_common.h"

#define TAG_TEST								"TestExp"
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

static const struct compv_unittest_exp {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const bool float64Typ;
	const char* md5;
	const char* md5_fma;
} COMPV_UNITTEST_EXP[] = {
#if 0 // Not implemented yet (std::exp used and produce different result on different CPUs)
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, false, "19b57f18429b1075c8ad945b40967d55", "19b57f18429b1075c8ad945b40967d55" }, // exp_32f32f - No FMA yet
#endif
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, true, "a19b56020ec76de44fefd60787ee237d", "f69e3f6f9203952f1f29ce2b73930f17" }, // exp_64f64f
};
static const size_t COMPV_UNITTEST_EXP_COUNT = sizeof(COMPV_UNITTEST_EXP) / sizeof(COMPV_UNITTEST_EXP[0]);

#define EXP_FLOAT64			true
#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR

#define LOOP_COUNT			1

COMPV_ERROR_CODE expo()
{
	const compv_unittest_exp* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_EXP_COUNT; ++i) {
		if (COMPV_UNITTEST_EXP[i].float64Typ == EXP_FLOAT64 && std::string(COMPV_UNITTEST_EXP[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_EXP[i];
			break;
		}
	}
	COMPV_ASSERT(test != nullptr);

	CompVMatPtr inMat, outMat;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &inMat));
	// I want the width to be odd (e.g. 1281x721) in order to have orphans
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(inMat, &inMat, 1283, 721, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32));
	if (test->float64Typ) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<float, double>(inMat, &inMat)));
		// Add some negative numbers and some out-of-rage values
		*inMat->ptr<double>(5, 6) *= -9.044;
		*inMat->ptr<double>(0, 1) *= -8.50;
		*inMat->ptr<double>(1, 7) *= -1.50;
		*inMat->ptr<double>(8, 11) = -708.39641853226408; // ret = 0
		*inMat->ptr<double>(4, 9) = 709.78271289338397; // ret = inf
		// MD5(inMat) = "c60c1e7f3363c77ec46b16e0b37017b9"
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMath::exp(inMat, &outMat));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Math Dot Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(outMat).c_str());
	const char* xmd5 = compv_tests_is_fma_enabled() ? test->md5_fma : test->md5;
	COMPV_CHECK_EXP_RETURN(std::string(xmd5).compare(compv_tests_md5(outMat)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Math Exp mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
