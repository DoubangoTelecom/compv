#include "../tests_common.h"

#define TAG_TEST								"TestImageConvolution"
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

#define IMAGE_CONVLT_LOOP_COUNT		1

static const struct compv_unittest_convlt {
	size_t kernelSize;
	float kernelSigma;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_CONVLT_8u_32f_8u[] =
{
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "6596d8f0c5f52272adff71a548a1cb9e" },
	{ 5, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "09c819ea91d3e11fbde3f36113901126" },
	{ 17, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "774271e8273922e69f5cd1d71bed3c72" },
	{ 3, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "e775351bce89a3cfc3e284f3bbe52bad" },

	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "760e0946723fbb30a3dead816ede9749" },
	{ 5, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "714080ac9bbf1292f8800ad71facf724" },
	{ 17, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "1f36eab4c8e4b11169cfd8e758370b0d" },
	{ 3, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "1ac545cd2496b4eff5ebf49a56d34026" },

	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "77b312e6e2b1bbad3dd650c600bdfaf5" },
	{ 5, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "d11dbbfc5fc0476768d3d3e66d2561c7" },
	{ 17, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "59ec0d6aaa62b805de79ac986902ce0f" },
	{ 3, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "61d26a8d9483942b939efbf02d77fe55" },
},
COMPV_UNITTEST_CONVLT_8u_16s_16s[] =
{
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "4442e2267e199a5da9a385f2bb993515" },
	{ 5, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "a1d6c04f9cd28ae074e75fd4b13bc79e" },
	{ 17, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "cb1bcfe92f9c5ea9d12ea8891d2b39e0" },
	{ 3, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "e41aeb5a9845f272348464c6e5e9903c" },

	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "7900abed1f6984ca90015d2049347466" },
	{ 5, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "2143c1805df7b5e47a1e70a808446113" },
	{ 17, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "2065e148523fe82d5778ae754428e94c" },
	{ 3, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "bc80e067e8232de264a4eed37c86e740" },

	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "261bc2b3ca047e7d43064d4f7672a8ca" },
	{ 5, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "329949ceafe5635dbe9c0e710811a7f0" },
	{ 17, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "9102e43034e9ea0f86fe343ddea47dcf" },
	{ 3, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "fcb962f389b34c1130d54ab0418a155b" },
},
COMPV_UNITTEST_CONVLT_16s_16s_16s[] =
{
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "3aeb70d77d24c3025281034110ee02a3" },
	{ 5, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "1bf0f0e07f205d0c67be0146a883300b" },
	{ 17, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "82ca43a0abaf2cf8d6446ef3a47e6061" },
	{ 3, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "c015c617d88dc2005ecb2bb36e09b8ce" },

	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "86f8f16b8b5e5a609e252418f809c820" },
	{ 5, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "ff91c1d4a05a15c674fc7a8203120f10" },
	{ 17, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "0ceac82050eabc0e00271d8d7e2003b5" },
	{ 3, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "2e30084f07dfbb6d0bcb6aeaf8c97268" },

	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "3e3a4e2a0091c808aa700e08c065a536" },
	{ 5, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "3717d47fb90a377558685181d6ec68bc" },
	{ 17, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "ed3ad3405ee344f6b033582c26be27f2" },
	{ 3, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "1152934866b809a1cdc229ca4e63d667" },
};
static const size_t COMPV_UNITTEST_CONVLT_COUNT = 12;

template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
static COMPV_ERROR_CODE convlt_ext(size_t kernelSize, float kernelSigma, const char* filename)
{
	const compv_unittest_convlt* tests = NULL;
	const compv_unittest_convlt* test = NULL;
	CompVMatPtr imageIn, imageOut;
	uint64_t timeStart, timeEnd;
	CompVMatPtr kernel;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	OutputType* outPtr = NULL;

	if (std::is_same<InputType, uint8_t>::value && std::is_same<KernelType, compv_float32_t>::value && std::is_same<OutputType, uint8_t>::value) {
		tests = COMPV_UNITTEST_CONVLT_8u_32f_8u;
	}
	else if (std::is_same<InputType, uint8_t>::value && std::is_same<KernelType, int16_t>::value && std::is_same<OutputType, int16_t>::value) {
		tests = COMPV_UNITTEST_CONVLT_8u_16s_16s;
	}
	else if (std::is_same<InputType, int16_t>::value && std::is_same<KernelType, int16_t>::value && std::is_same<OutputType, int16_t>::value) {
		tests = COMPV_UNITTEST_CONVLT_16s_16s_16s;
	}
	else {
		COMPV_DEBUG_INFO_EX(TAG_TEST, "Not implemented");
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

#if 0
	for (size_t i = 0; i < COMPV_UNITTEST_CONVLT_COUNT; ++i) {
		test = &tests[i];

		// Read image and create output
		if (std::is_same<InputType, uint8_t>::value) {
			COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<OutputType, COMPV_MAT_TYPE_PIXELS, COMPV_SUBTYPE_PIXELS_Y>(&imageOut, imageIn->rows(), imageIn->cols(), imageIn->stride())));
		}
		else {
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<InputType>(&imageIn, test->height, test->width)));
			for (size_t row = 0; row < test->height; ++row) {
				InputType* rowPtr = imageIn->ptr<InputType>(row);
				for (size_t col = 0; col < test->width; ++col) {
					rowPtr[col] = static_cast<InputType>(col);
				}
			}
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<OutputType>(&imageOut, imageIn->rows(), imageIn->cols(), imageIn->stride())));
		}

		// Build kernel
		if (std::is_same<KernelType, compv_float32_t>::value || std::is_same<KernelType, compv_float64_t>::value) {
			COMPV_CHECK_CODE_BAIL(err = CompVMathGauss::kernelDim1<KernelType>(&kernel, test->kernelSize, test->kernelSigma));
		}
		else if (std::is_same<KernelType, int16_t>::value) {
			CompVMatPtr kernelGauss;
			COMPV_CHECK_CODE_BAIL(err = CompVMathGauss::kernelDim1<KernelType>(&kernelGauss, test->kernelSize, test->kernelSigma));
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<int16_t>(&kernel, 1, test->kernelSize, static_cast<size_t>(CompVMem::alignForward(test->kernelSize)))));
			for (size_t k = 0; k < test->kernelSize; ++k) {
				*kernel->ptr<int16_t>(0, k) *= 0xff;
			}
		}

		// Process
		outPtr = imageOut->ptr<OutputType>();
		COMPV_CHECK_CODE_BAIL(err = CompVMathConvlt::convlt1(imageIn->ptr<uint8_t>(), imageIn->cols(), imageIn->rows(), imageIn->stride(),
			kernel->ptr<KernelType>(), kernel->ptr<KernelType>(), test->kernelSize, outPtr
		));
		COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5: %s", compv_tests_md5(imageOut).c_str());
		kernel = NULL;
		imageIn = NULL;
		imageOut = NULL;
	}
#endif

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_CONVLT_COUNT; ++i) {
		if (tests[i].kernelSize == kernelSize && tests[i].kernelSigma == kernelSigma && std::string(tests[i].filename).compare(filename) == 0) {
			test = &tests[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	// Read image and create output
	if (std::is_same<InputType, uint8_t>::value) {
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<OutputType, COMPV_MAT_TYPE_PIXELS, COMPV_SUBTYPE_PIXELS_Y>(&imageOut, imageIn->rows(), imageIn->cols(), imageIn->stride())));
	}
	else {
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<InputType>(&imageIn, test->height, test->width)));
		for (size_t row = 0; row < test->height; ++row) {
			InputType* rowPtr = imageIn->ptr<InputType>(row);
			for (size_t col = 0; col < test->width; ++col) {
				rowPtr[col] = static_cast<InputType>(col);
			}
		}
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<OutputType>(&imageOut, imageIn->rows(), imageIn->cols(), imageIn->stride())));
	}
	outPtr = imageOut->ptr<OutputType>();

	// Build kernel
	if (std::is_same<KernelType, compv_float32_t>::value || std::is_same<KernelType, compv_float64_t>::value) {
		COMPV_CHECK_CODE_BAIL(err = CompVMathGauss::kernelDim1<KernelType>(&kernel, test->kernelSize, test->kernelSigma));
	}
	else if (std::is_same<KernelType, int16_t>::value) {
		CompVMatPtr kernelGauss;
		COMPV_CHECK_CODE_BAIL(err = CompVMathGauss::kernelDim1<KernelType>(&kernelGauss, test->kernelSize, test->kernelSigma));
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<int16_t>(&kernel, 1, test->kernelSize, static_cast<size_t>(CompVMem::alignForward(test->kernelSize)))));
		for (size_t k = 0; k < test->kernelSize; ++k) {
			*kernel->ptr<int16_t>(0, k) *= 0xff;
		}
	}

	// process
	timeStart = CompVTime::nowMillis();
	for (int i = 0; i < IMAGE_CONVLT_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_BAIL(err = CompVMathConvlt::convlt1(imageIn->ptr<uint8_t>(), imageIn->cols(), imageIn->rows(), imageIn->stride(),
			kernel->ptr<KernelType>(), kernel->ptr<KernelType>(), test->kernelSize, outPtr
		));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(TestImageConvolution) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 1
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_BAIL(err = compv_tests_write_to_file(imageOut, "out.gray"));
#endif

#if IMAGE_CONVLT_LOOP_COUNT == 1
	COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image convolution MD5 mismatch");
#endif

bail:
	return err;
}

#define IMAGE_CONVLT_INPUT_TYPE		uint8_t // int16_t, uint8_t
#define IMAGE_CONVLT_KERNEL_TYPE	compv_float32_t // int16_t, float32
#define IMAGE_CONVLT_OUTPUT_TYPE	uint8_t // int16_t, uint8_t
#define IMAGE_CONVLT_FILE_NAME		FILE_NAME_EQUIRECTANGULAR
#define IMAGE_CONVLT_KERNEL_SIZE	7
#define IMAGE_CONVLT_KERNEL_SIGMA	2.0f

COMPV_ERROR_CODE convlt()
{
	COMPV_CHECK_CODE_RETURN((convlt_ext<IMAGE_CONVLT_INPUT_TYPE, IMAGE_CONVLT_KERNEL_TYPE, IMAGE_CONVLT_OUTPUT_TYPE>(IMAGE_CONVLT_KERNEL_SIZE, IMAGE_CONVLT_KERNEL_SIGMA, IMAGE_CONVLT_FILE_NAME)));
	return COMPV_ERROR_CODE_S_OK;
}
