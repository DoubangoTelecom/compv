#include "../tests_common.h"

#define TAG_TEST								"TestExp"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"E:/Projects/GitHub/data/test_images"
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
#define	ACTIVATION_FUNCTION_TYPE_SOFTMAX		4


static const struct compv_unittest_activation {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const int type;
	const char* md5;
	const char* md5_alt; // SIMD may have different MD5 because of how the add is packed
} COMPV_UNITTEST_ACTIVATIONS[] = {
	// TODO(dmi): pre-compute LUTs and store/load to/from a file to make sure the MD5 is consistent
	// On Windows Debug and release provide difference MD5 due to optimization options
#if COMPV_ARCH_ARM
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_TANH, "059c8215387c21095527baf89e50d4b2", "059c8215387c21095527baf89e50d4b2" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_TANH_MUL, "37120acee156d1025e23f7562f4c70b8", "37120acee156d1025e23f7562f4c70b8" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_LOGISTIC, "3b7c7a49810ba42fe4aafc663ce777d2", "3b7c7a49810ba42fe4aafc663ce777d2" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL, "9e61debdf8645cbd276508a132f9910e", "9e61debdf8645cbd276508a132f9910e" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_SOFTMAX, "9c6c053b26c655aeb71c327fe72565df", "a936ee6a18dd0522fbf88638d7e34124" },
#else
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_TANH, "a8b1ab0089188b60698fed9e18b92bc5", "a8b1ab0089188b60698fed9e18b92bc5" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_TANH_MUL, "e85f8bd29258de4ddf31aa3dc24f19cf", "e85f8bd29258de4ddf31aa3dc24f19cf" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_LOGISTIC, "3825f49397ecc967642cdd51d5256b6c", "3825f49397ecc967642cdd51d5256b6c" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL, "cb5075ce97a65c5d6281752ff63600d4", "cb5075ce97a65c5d6281752ff63600d4" },
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, ACTIVATION_FUNCTION_TYPE_SOFTMAX, "9c6c053b26c655aeb71c327fe72565df", "a42c2cc82eeaeec0b4cf67829e9bfc37" },
#endif
};
static const size_t COMPV_UNITTEST_ACTIVATIONS_COUNT = sizeof(COMPV_UNITTEST_ACTIVATIONS) / sizeof(COMPV_UNITTEST_ACTIVATIONS[0]);

#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR
#define ACTIVATION_TYPE		ACTIVATION_FUNCTION_TYPE_SOFTMAX

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
	
#if ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_SOFTMAX
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(inMat, &inMat, 1211, 721, COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32)); // Odd width for Softmax
#else
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(inMat, &inMat, 1288, 721, COMPV_INTERPOLATION_TYPE_BILINEAR_FLOAT32)); // width always power of 2 (multiple of 2) for TanH and logistic
#endif
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<float, float>(inMat, &inMat)));
	// Scale kScaleFactor is "256.0" and kTableSize is "4096" this means x values must be within [0, 4096./256.] = [0, 16].
	// To add some out of bound indices we'll request a mat within [0, 30] -> scale by (255. / 30.) = 0.12
	COMPV_CHECK_CODE_RETURN(CompVMath::scale(inMat, 0.12, &inMat));
	*inMat->ptr<float>(5, 6) *= -9.044f;
	*inMat->ptr<float>(0, 1) *= -8.50f;
	*inMat->ptr<float>(1, 7) *= -1.50f;
	*inMat->ptr<float>(8, 11) = -708.39641853226408f;
	*inMat->ptr<float>(4, 9) = 709.78271289338397f; 
#if ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_TANH_MUL || ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_LOGISTIC_MUL
	// Mul mat
	CompVMatPtr mulMat;
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<float, int>(inMat, &mulMat)));
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<int, float>(mulMat, &mulMat)));
	*mulMat->ptr<float>(95, 8) *= -0.0447f;
	*mulMat->ptr<float>(99, 10) *= -98.502f;
	*mulMat->ptr<float>(17, 70) *= -100000.507f;
	*mulMat->ptr<float>(80, 101) = 7008.39641853226408f;
	*mulMat->ptr<float>(48, 90) = -709.78271289338397f;
#endif
	// LUT table (code from Tesseract "functions.cpp")
	constexpr int kTableSize = 4096;
	constexpr float kScaleFactor = 256.f;
	COMPV_ALIGN_DEFAULT() float TanhTable[kTableSize + 8/* SIMD padding */] = { 0 };
	COMPV_ALIGN_DEFAULT() float LogisticTable[kTableSize + 8/* SIMD padding */] = { 0 };
	for (int i = 0; i < kTableSize; i++) {
		TanhTable[i] = std::tanh(i / kScaleFactor);
		LogisticTable[i] = 1.f / (1.f + std::exp(-i / kScaleFactor));
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
#elif ACTIVATION_TYPE == ACTIVATION_FUNCTION_TYPE_SOFTMAX
		double maxx = 0, minn = 0;
		COMPV_CHECK_CODE_RETURN(CompVMath::minMax(inMat, minn, maxx));
		*inMat->ptr<float>(0, inMat->cols() - 1) = static_cast<float>(maxx + 1.); // to check if we'll catch the max at the latest position
		COMPV_CHECK_CODE_RETURN(CompVMath::softmaxInPlace(
			inMat
		));
		outMat = inMat;
#else
#error "Invalid activation function"
#endif
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Math Activation Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	// TODO(dmi): pre-compute LUTs and store/load to/from a file to make sure the MD5 is consistent
	// On Windows Debug and release provide difference MD5 due to optimization options
	const std::string outMd5 = compv_tests_md5(outMat);
	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", outMd5.c_str());
	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(outMd5) != 0 && std::string(test->md5_alt).compare(outMd5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Math Activation mismatch");
	COMPV_DEBUG_INFO_EX(TAG_TEST, "!!! DONE !!!");

	return COMPV_ERROR_CODE_S_OK;
}
