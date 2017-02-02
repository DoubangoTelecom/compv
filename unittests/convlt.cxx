#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestConvolution"
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

static const struct compv_unittest_convlt {
	size_t kernelSize;
	float kernelSigma;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_CONVLT_8u_32f_8u[] = // FloatingPoint(COMPV_UNITTEST_CONVLT_FXP_8u_16s_8u)
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
COMPV_UNITTEST_CONVLT_FXP_8u_16u_8u[] = // FixedPoint(COMPV_UNITTEST_CONVLT_8u_32f_8u)
{
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "91b353b104e20232c1a386ea5e3d8020" },
	{ 5, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "5f41d7e67d83c7022ef517311ef1ecb3" },
	{ 17, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "793e7a9b03122e72a33c8b278cb821d3" },
	{ 3, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "8942d5e4f66ac77f7c627e2f9212bff6" },

	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "fb2bfb9f471066faaea9e26419a193bb" },
	{ 5, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "87883261159c17670928ee6cc4e929fd" },
	{ 17, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "ab78cd19bcbe08c32efc4e6968ca9a9b" },
	{ 3, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "c7118ef269763161326c45c85a82aebe" },

	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "9e25601883556c46e0605361f0dfb7e4" },
	{ 5, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "659f02992f75f6a11248d909040d230e" },
	{ 17, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "1074ed4de310d88dc48c3e35051febda" },
	{ 3, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "05f535a1deb81c7544fd6fa4b9c4efba" },
},
COMPV_UNITTEST_CONVLT_8u_16s_16s[] =
{
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "3af360b3e4ee5b627fa9b14dbd97bfce" },
	{ 5, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "2228ceb988a00794a30e7a9ae221948e" },
	{ 17, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "3156ffcee01f5104ab25cf5e3cedc95a" },
	{ 3, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "d9d2dd09932054185b3199c49f7ea74f" },

	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "cbada3ed090268e84f109968eafd0d9d" },
	{ 5, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "6e2ae4cf90701fb258ea6bdea36fb013" },
	{ 17, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "c0d90287c4f773b180d2331d079b3124" },
	{ 3, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "0ed5f1d858c4cfbaabd74a6939d35693" },

	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "5f1e225875ed5b278bf19cf60d734dc3" },
	{ 5, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "1c9b7ce6b79e406f9bb7e3264e62448e" },
	{ 17, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "31b5c10f7abb6d1d52c8837ec58d827e" },
	{ 3, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "f08801c2fd76515a8de48b0593501a2c" },
},
COMPV_UNITTEST_CONVLT_16s_16s_16s[] =
{
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "694505f56335061ef5f33a3b4d4da96c" },
	{ 5, 0.83f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "dae6940967d62a2daf4277dc4e8a70dc" },
	{ 17, 1.2f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "d3ca5066e36b258574d022d736c11115" },
	{ 3, 3.f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "321c09726d256a474af4cab362297622" },

	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "7cdd864ced0472d30b338daa80acefd3" },
	{ 5, 0.83f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "abb403dcfd8815e97750530081aeeced" },
	{ 17, 1.2f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "a58479fba2dbbb8d87b69fb0b47e3c7c" },
	{ 3, 3.f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "72f122947a0d3ccdaebe0ac3633a4bd4" },

	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "bddfd4e28933d82edc4a7955c89a8f0c" },
	{ 5, 0.83f, FILE_NAME_GRIOTS, 480, 640, 480, "45d812e838c4bb5f3edeebf62aff2e3b" },
	{ 17, 1.2f, FILE_NAME_GRIOTS, 480, 640, 480, "34a525a96b8b6c2a4ef420fea2d43d03" },
	{ 3, 3.f, FILE_NAME_GRIOTS, 480, 640, 480, "249036fd6f85aa883149f94743406590" },
};
static const size_t COMPV_UNITTEST_CONVLT_COUNT = 12;

static const std::string compv_unittest_convlt_to_string(const compv_unittest_convlt* test, bool fixedPoint) {
	return
		std::string("fixedPoint:") + CompVBase::to_string(fixedPoint) + std::string(", ")
		+std::string("kernelSize:") + CompVBase::to_string(test->kernelSize) + std::string(", ")
		+ std::string("kernelSigma:") + CompVBase::to_string(test->kernelSigma) + std::string(", ")
		+ std::string("filename:") + std::string(test->filename);
}

template <typename InputType = uint8_t, typename KernelType = compv_float32_t, typename OutputType = uint8_t>
static COMPV_ERROR_CODE convlt_ext(bool fixedPoint)
{
	const compv_unittest_convlt* tests = NULL;
	const compv_unittest_convlt* test = NULL;
	CompVMatPtr imageIn, imageOut;
	CompVMatPtr kernel;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	OutputType* outPtr = NULL;

	if (std::is_same<InputType, uint8_t>::value && std::is_same<KernelType, compv_float32_t>::value && std::is_same<OutputType, uint8_t>::value) {
		tests = COMPV_UNITTEST_CONVLT_8u_32f_8u;
	}
	else if (fixedPoint && std::is_same<InputType, uint8_t>::value && std::is_same<KernelType, uint16_t>::value && std::is_same<OutputType, uint8_t>::value) {
		tests = COMPV_UNITTEST_CONVLT_FXP_8u_16u_8u;
	}
	else if (std::is_same<InputType, uint8_t>::value && std::is_same<KernelType, int16_t>::value && std::is_same<OutputType, int16_t>::value) {
		tests = COMPV_UNITTEST_CONVLT_8u_16s_16s;
	}
	else if (std::is_same<InputType, int16_t>::value && std::is_same<KernelType, int16_t>::value && std::is_same<OutputType, int16_t>::value) {
		tests = COMPV_UNITTEST_CONVLT_16s_16s_16s;
	}
	else {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Not implemented");
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	
	for (size_t i = 0; i < COMPV_UNITTEST_CONVLT_COUNT; ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Image convlt -> %s ==", compv_unittest_convlt_to_string(test, fixedPoint).c_str());
		// Read image and create output
		if (std::is_same<InputType, uint8_t>::value) {
			COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));
		}
		else {
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<InputType>(&imageIn, test->height, test->width)));
			for (size_t row = 0; row < test->height; ++row) {
				InputType* rowPtr = imageIn->ptr<InputType>(row);
				for (size_t col = 0; col < test->width; ++col) {
					rowPtr[col] = static_cast<InputType>(col);
				}
			}
		}
		COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<OutputType>(&imageOut, imageIn->rows(), imageIn->cols(), imageIn->stride())));
		outPtr = imageOut->ptr<OutputType>();

		// Build kernel
		if (std::is_same<KernelType, compv_float32_t>::value || std::is_same<KernelType, compv_float64_t>::value) {
			COMPV_CHECK_CODE_BAIL(err = CompVMathGauss::kernelDim1(&kernel, test->kernelSize, test->kernelSigma));
		}
		else if (std::is_same<KernelType, uint16_t>::value && fixedPoint) {
			COMPV_CHECK_CODE_BAIL(err = CompVMathGauss::kernelDim1FixedPoint(&kernel, test->kernelSize, test->kernelSigma));
		}
		else if (std::is_same<KernelType, int16_t>::value) {
			CompVMatPtr kernelGauss;
			COMPV_CHECK_CODE_BAIL(err = CompVMathGauss::kernelDim1(&kernelGauss, test->kernelSize, test->kernelSigma));
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<int16_t>(&kernel, 1, test->kernelSize, static_cast<size_t>(CompVMem::alignForward(test->kernelSize)))));
			for (size_t k = 0; k < test->kernelSize; ++k) {
				*kernel->ptr<int16_t>(0, k) = static_cast<int16_t>(*kernelGauss->ptr<compv_float32_t>(0, k) * 0xff);
			}
		}

		// process
		if (fixedPoint) {
			COMPV_CHECK_CODE_BAIL(err = CompVMathConvlt::convlt1FixedPoint(imageIn->ptr<const uint8_t>(), imageIn->cols(), imageIn->rows(), imageIn->stride(),
				kernel->ptr<uint16_t>(), kernel->ptr<uint16_t>(), test->kernelSize, reinterpret_cast<uint8_t*&>(outPtr)
			));
		}
		else {
			COMPV_CHECK_CODE_BAIL(err = CompVMathConvlt::convlt1(imageIn->ptr<const InputType>(), imageIn->cols(), imageIn->rows(), imageIn->stride(),
				kernel->ptr<KernelType>(), kernel->ptr<KernelType>(), test->kernelSize, outPtr
			));
		}

		COMPV_CHECK_EXP_BAIL(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "[" TAG_TEST "]" " Image convolution MD5 mismatch");

		kernel = NULL;
		imageIn = NULL;
		imageOut = NULL;

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

bail:
	return err;
}

COMPV_ERROR_CODE unittest_convlt()
{
	/* General */
	COMPV_CHECK_CODE_RETURN((convlt_ext<uint8_t, compv_float32_t, uint8_t>(false)));
	COMPV_CHECK_CODE_RETURN((convlt_ext<uint8_t, int16_t, int16_t>(false)));
	COMPV_CHECK_CODE_RETURN((convlt_ext<int16_t, int16_t, int16_t>(false)));
	/* FixedPoint */
	COMPV_CHECK_CODE_RETURN((convlt_ext<uint8_t, uint16_t, uint8_t>(true)));

	return COMPV_ERROR_CODE_S_OK;
}
