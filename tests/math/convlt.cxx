#include "../tests_common.h"

#define TAG_TEST								"TestImageConvlt"

#define LOOP_COUNT				1

static const struct compv_unittest_convlt {
	const bool fixedPoint;
	const COMPV_SUBTYPE inputType;
	const COMPV_SUBTYPE KernelType;
	const COMPV_SUBTYPE OutputType;
	const size_t data_width;
	const size_t data_height;
	const size_t data_stride;
	const size_t kernel_width;
	const size_t kernel_height;
	const char* md5;
	const char* md5_fma;
} COMPV_UNITTEST_CONVLT[] =
{
	/* 0 */ { true, COMPV_SUBTYPE_RAW_UINT8, COMPV_SUBTYPE_RAW_UINT16, COMPV_SUBTYPE_RAW_UINT8, 1285, 720, 1344, 7, 7, "2678b73a89681f12fb474dd8102fc37c" }, // FixedPoint
	/* 1 */{ false, COMPV_SUBTYPE_RAW_UINT8, COMPV_SUBTYPE_RAW_FLOAT32, COMPV_SUBTYPE_RAW_UINT8, 1285, 720, 1344, 7, 7, "e51d0396692e41ff6bdff088c4afb18f", "-" }, // FloatingPoint
	/* -2 */{ false, COMPV_SUBTYPE_RAW_UINT8, COMPV_SUBTYPE_RAW_FLOAT32, COMPV_SUBTYPE_RAW_FLOAT32, 1285, 720, 1344, 7, 7, "2bba5a06903dea9604bac64bb864644d", "-" }, // FloatingPoint
	/* -3 */{ false, COMPV_SUBTYPE_RAW_FLOAT32, COMPV_SUBTYPE_RAW_FLOAT32, COMPV_SUBTYPE_RAW_FLOAT32, 1285, 720, 1344, 7, 7, "dd714b2b7662e10ba5b5b60fb5977a35", "-" }, // FloatingPoint
	/* -4 */{ false, COMPV_SUBTYPE_RAW_FLOAT32, COMPV_SUBTYPE_RAW_FLOAT32, COMPV_SUBTYPE_RAW_UINT8, 1285, 720, 1344, 7, 7, "865c59de098283bc7b60cb1c6b6b33ef", "-" }, // FloatingPoint
	/* 5 */{ false, COMPV_SUBTYPE_RAW_UINT8, COMPV_SUBTYPE_RAW_INT16, COMPV_SUBTYPE_RAW_INT16, 1285, 720, 1344, 7, 7, "7f1116ade2a1cdb37842084c781ee05e" }, // Integer
	/* 6 */{ false, COMPV_SUBTYPE_RAW_INT16, COMPV_SUBTYPE_RAW_INT16, COMPV_SUBTYPE_RAW_INT16, 1285, 720, 1344, 7, 7, "cad2f4d2fd66e171997f39804e667699" }, // Integer
};
static const size_t COMPV_UNITTEST_CONVLT_COUNT = sizeof(COMPV_UNITTEST_CONVLT) / sizeof(COMPV_UNITTEST_CONVLT[0]);

#define TEST_INDEX	0

static COMPV_ERROR_CODE buildTest(const compv_unittest_convlt* test, CompVMatPtrPtr data, CompVMatPtrPtr kernel);
#define convolution(inputType, KernelType, OutputType) { \
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<OutputType>(&dataOut, dataIn->rows(), dataIn->cols(), dataIn->stride())); \
	OutputType* outPtr = dataOut->ptr<OutputType>(); \
	timeStart = CompVTime::nowMillis(); \
	for (size_t i = 0; i < LOOP_COUNT; ++i) { \
		COMPV_CHECK_CODE_RETURN(CompVMathConvlt::convlt1(dataIn->ptr<const inputType>(), dataIn->cols(), dataIn->rows(), dataIn->stride(), \
			kernel->ptr<const KernelType>(), kernel->ptr<const KernelType>(), test->kernel_width, outPtr \
		)); \
	} \
	timeEnd = CompVTime::nowMillis(); \
}

COMPV_ERROR_CODE convlt()
{
	COMPV_ASSERT(TEST_INDEX >= 0 && TEST_INDEX < COMPV_UNITTEST_CONVLT_COUNT);
	const compv_unittest_convlt* test = &COMPV_UNITTEST_CONVLT[TEST_INDEX];

	// Build test
	CompVMatPtr dataIn, kernel;
	COMPV_CHECK_CODE_RETURN(buildTest(test, &dataIn, &kernel));

	uint64_t timeStart, timeEnd;
	CompVMatPtr dataOut;

	// Perform convolution
	if (test->fixedPoint && test->inputType == COMPV_SUBTYPE_RAW_UINT8 && test->KernelType == COMPV_SUBTYPE_RAW_UINT16 && test->OutputType == COMPV_SUBTYPE_RAW_UINT8) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&dataOut, dataIn->rows(), dataIn->cols(), dataIn->stride()));
		uint8_t* outPtr = dataOut->ptr<uint8_t>();
		timeStart = CompVTime::nowMillis();
		for (size_t i = 0; i < LOOP_COUNT; ++i) {
			COMPV_CHECK_CODE_RETURN(CompVMathConvlt::convlt1FixedPoint(dataIn->ptr<const uint8_t>(), dataIn->cols(), dataIn->rows(), dataIn->stride(),
				kernel->ptr<const uint16_t>(), kernel->ptr<const uint16_t>(), test->kernel_width, outPtr
			));
		}
		timeEnd = CompVTime::nowMillis();
	}
	else if (!test->fixedPoint && test->inputType == COMPV_SUBTYPE_RAW_UINT8 && test->KernelType == COMPV_SUBTYPE_RAW_FLOAT32 && test->OutputType == COMPV_SUBTYPE_RAW_UINT8) {
		convolution(uint8_t, compv_float32_t, uint8_t);
	}
	else if (!test->fixedPoint && test->inputType == COMPV_SUBTYPE_RAW_UINT8 && test->KernelType == COMPV_SUBTYPE_RAW_FLOAT32 && test->OutputType == COMPV_SUBTYPE_RAW_FLOAT32) {
		convolution(uint8_t, compv_float32_t, compv_float32_t);
	}
	else if (!test->fixedPoint && test->inputType == COMPV_SUBTYPE_RAW_FLOAT32 && test->KernelType == COMPV_SUBTYPE_RAW_FLOAT32 && test->OutputType == COMPV_SUBTYPE_RAW_FLOAT32) {
		convolution(compv_float32_t, compv_float32_t, compv_float32_t);
	}
	else if (!test->fixedPoint && test->inputType == COMPV_SUBTYPE_RAW_FLOAT32 && test->KernelType == COMPV_SUBTYPE_RAW_FLOAT32 && test->OutputType == COMPV_SUBTYPE_RAW_UINT8) {
		convolution(compv_float32_t, compv_float32_t, uint8_t);
	}
	else if (!test->fixedPoint && test->inputType == COMPV_SUBTYPE_RAW_UINT8 && test->KernelType == COMPV_SUBTYPE_RAW_INT16 && test->OutputType == COMPV_SUBTYPE_RAW_INT16) {
		convolution(uint8_t, int16_t, int16_t);
	}
	else if (!test->fixedPoint && test->inputType == COMPV_SUBTYPE_RAW_INT16 && test->KernelType == COMPV_SUBTYPE_RAW_INT16 && test->OutputType == COMPV_SUBTYPE_RAW_INT16) {
		convolution(int16_t, int16_t, int16_t);
	}
	else {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(dataOut).c_str());
	
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Convlt Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	const bool fma = (test->KernelType == COMPV_SUBTYPE_RAW_FLOAT32) && compv_tests_is_fma_enabled();
	COMPV_CHECK_EXP_RETURN(std::string(fma ? test->md5_fma : test->md5).compare(compv_tests_md5(dataOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Convlt MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE buildTest(const compv_unittest_convlt* test, CompVMatPtrPtr data, CompVMatPtrPtr kernel)
{
	switch (test->inputType) {
		case COMPV_SUBTYPE_RAW_UINT8: {
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<uint8_t>(data, test->data_height, test->data_width, test->data_stride)));
			uint8_t* ptr8u = (*data)->ptr<uint8_t>();
			for (size_t j = 0; j < test->data_height; ++j) {
				for (size_t i = 0; i < test->data_width; ++i) {
					ptr8u[i] = static_cast<uint8_t>((i * j) + 53);
				}
				ptr8u += test->data_stride;
			}
			break;
		}
		case COMPV_SUBTYPE_RAW_FLOAT32: {
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<compv_float32_t>(data, test->data_height, test->data_width, test->data_stride)));
			compv_float32_t* ptr32f = (*data)->ptr<compv_float32_t>();
			for (size_t j = 0; j < test->data_height; ++j) {
				for (size_t i = 0; i < test->data_width; ++i) {
					ptr32f[i] = static_cast<compv_float32_t>(((i * j) + 53.558f) * (((i * j) & 1) ? 1.f : -1.f)) / 1.2f; // make sure to have negative values
				}
				ptr32f += test->data_stride;
			}
			break;
		}
		case COMPV_SUBTYPE_RAW_INT16: {
			COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<int16_t>(data, test->data_height, test->data_width, test->data_stride)));
			int16_t* ptr16s = (*data)->ptr<int16_t>();
			for (size_t j = 0; j < test->data_height; ++j) {
				for (size_t i = 0; i < test->data_width; ++i) {
					ptr16s[i] = static_cast<int16_t>(((i * j) + 53) * ((i & 1) ? -1 : 1));
				}
				ptr16s += test->data_stride;
			}
			break;
		}
		default: {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
			break;
		}
	}

	if (test->fixedPoint) {
		COMPV_CHECK_CODE_RETURN(CompVMathGauss::kernelDim1(kernel, test->kernel_width, 3.5));
	}
	else {
		switch (test->KernelType) {
			case COMPV_SUBTYPE_RAW_FLOAT32: {
				COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<compv_float32_t>(kernel, 1, test->kernel_width)));
				compv_float32_t* ptr32f = (*kernel)->ptr<compv_float32_t>();
				for (size_t i = 0; i < test->kernel_width; ++i) {
					ptr32f[i] = static_cast<compv_float32_t>(((i * 1.f) + 53.558f) * (((i * 1) & 1) ? 1.f : -1.f)) / 1.2f; // make sure to have negative values
				}
				break;
			}
			case COMPV_SUBTYPE_RAW_INT16: {
				COMPV_CHECK_CODE_RETURN((CompVMat::newObjAligned<int16_t>(kernel, 1, test->kernel_width)));
				int16_t* ptr16s = (*kernel)->ptr<int16_t>();
				for (size_t i = 0; i < test->kernel_width; ++i) {
					ptr16s[i] = static_cast<int16_t>(((i * 1) + 53) * ((i & 1) ? -1 : 1));
				}
				break;
			}
			default: {
				COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
				break;
			}
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}