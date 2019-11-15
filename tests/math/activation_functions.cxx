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


#define	ACTIVATION_FUNCTION_TYPE_TANH			0
#define	ACTIVATION_FUNCTION_TYPE_TANH_MUL		1
#define	ACTIVATION_FUNCTION_TYPE_LOGISTIC		2
#define	ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL	3


static const struct compv_unittest_activation {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const int type;
	const char* md5;
	const char* md5_fma;
} COMPV_UNITTEST_ACTIVATIONS[] = {
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_TANH, "9622adfb4c7e0617836d8a7179ca0bfc", "9622adfb4c7e0617836d8a7179ca0bfc" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_TANH_MUL, "f7d0fdefaba072d845a14381ac0b90a0", "f7d0fdefaba072d845a14381ac0b90a0" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_LOGISTIC, "41a68ce983c99199e888bc5b52efd14a", "41a68ce983c99199e888bc5b52efd14a" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL, "e3444a12126a6e67d306e360e535243f", "e3444a12126a6e67d306e360e535243f" },
};
static const size_t COMPV_UNITTEST_ACTIVATIONS_COUNT = sizeof(COMPV_UNITTEST_ACTIVATIONS) / sizeof(COMPV_UNITTEST_ACTIVATIONS[0]);

#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR
#define ACTIVATION_TYPE		ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL

#define LOOP_COUNT			1

COMPV_ERROR_CODE activation_functions()
{
	const compv_unittest_activation* test = nullptr;
	for (size_t i = 0; i < COMPV_UNITTEST_ACTIVATIONS_COUNT; ++i) {
		if (COMPV_UNITTEST_ACTIVATIONS[i].type == ACTIVATION_TYPE) {
			test = &COMPV_UNITTEST_ACTIVATIONS[i];
			break;
		}
	}
	COMPV_ASSERT(test != nullptr);

	CompVMatPtr inMat, outMat;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &inMat));
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(inMat, &inMat, 1284, 721, COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32)); // With multiple 4 for AVX
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<float, double>(inMat, &inMat)));
	// Scale kScaleFactor is "256.0" and kTableSize is "4096" this means x values must be within [0, 4096./256.] = [0, 16].
	// To add some out of bound indices we'll request a mat within [0, 30] -> scale by (255. / 30.) = 0.12
	COMPV_CHECK_CODE_RETURN(CompVMath::scale(inMat, 0.12, &inMat));
	*inMat->ptr<double>(5, 6) *= -9.044;
	*inMat->ptr<double>(0, 1) *= -8.50;
	*inMat->ptr<double>(1, 7) *= -1.50;
	*inMat->ptr<double>(8, 11) = -708.39641853226408;
	*inMat->ptr<double>(4, 9) = 709.78271289338397; 
#if ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_TANH_MUL || ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL
	// Mul mat
	CompVMatPtr mulMat;
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<double, int>(inMat, &mulMat)));
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<int, double>(mulMat, &mulMat)));
	*mulMat->ptr<double>(95, 8) *= -0.0447;
	*mulMat->ptr<double>(99, 10) *= -98.502;
	*mulMat->ptr<double>(17, 70) *= -100000.507;
	*mulMat->ptr<double>(80, 101) = 7008.39641853226408;
	*mulMat->ptr<double>(48, 90) = -709.78271289338397;
#endif
	// LUT table (code from Tesseract "functions.cpp")
	constexpr int kTableSize = 4096;
	constexpr double kScaleFactor = 256.0;
	double TanhTable[kTableSize + 1/* SIMD padding */];
	double LogisticTable[kTableSize + 1/* SIMD padding */];
	for (int i = 0; i < kTableSize; i++) {
		TanhTable[i] = std::tanh(i / kScaleFactor);
		LogisticTable[i] = 1.0 / (1.0 + std::exp(-i / kScaleFactor));
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
#if ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_TANH
		COMPV_CHECK_CODE_RETURN(CompVMath::tanh(
			TanhTable, kTableSize, kScaleFactor, 
			inMat, &outMat
		));
#elif ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_TANH_MUL
		COMPV_CHECK_CODE_RETURN(CompVMath::tanhMul(
			TanhTable, kTableSize, kScaleFactor,
			inMat, mulMat, &outMat
		));
#elif ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_LOGISTIC
		COMPV_CHECK_CODE_RETURN(CompVMath::logistic(
			TanhTable, kTableSize, kScaleFactor,
			inMat, &outMat
		));
#elif ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL
		COMPV_CHECK_CODE_RETURN(CompVMath::logisticMul(
			TanhTable, kTableSize, kScaleFactor,
			inMat, mulMat, &outMat
		));
#else
#error "Invalid activation function"
#endif
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Math Activation Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "IN=%s", compv_tests_md5(inMat).c_str());
	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(outMat).c_str());
	const char* xmd5 = compv_tests_is_fma_enabled() ? test->md5_fma : test->md5;
	COMPV_CHECK_EXP_RETURN(std::string(xmd5).compare(compv_tests_md5(outMat)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Math Activation mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
