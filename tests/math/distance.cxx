#include "../tests_common.h"

#define TAG_TEST			"TestDistance"
#define LOOP_COUNT			1


COMPV_ERROR_CODE hamming()
{
	static const  size_t width = 2005;
	static const struct compv_unittest_hamming {
		size_t width;
		size_t height;
		const char* md5;
	}
	COMPV_UNITTEST_HAMMING[] = {
		2005, 101, "d30eae4b27538e501fe19b66c7d55476",
		1280, 720, "506d638180bed20c7337b3ff00ca0458",
		32, 720, "f01a3e4bb7aa79fa602b6fdb14316219", // hamming256
	};

	const compv_unittest_hamming* test = NULL;
	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_HAMMING) / sizeof(COMPV_UNITTEST_HAMMING[i]); ++i) {
		if (COMPV_UNITTEST_HAMMING[i].width == width) {
			test = &COMPV_UNITTEST_HAMMING[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr patch1xn;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&patch1xn, 1, test->width));
	COMPV_CHECK_CODE_RETURN(patch1xn->zero_row(0));

	CompVMatPtr data;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&data, test->height, test->width));
	for (size_t j = 0; j < data->rows(); ++j) {
		uint8_t* dataPtr = data->ptr<uint8_t>(j);
		size_t row = (test->width * j);
		for (size_t i = 0; i < data->cols(); ++i) {
			dataPtr[i] = static_cast<uint8_t>((i + row)  & 0xff);
		}
	}

	CompVMatPtr dist;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&dist, test->height, test->width));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathDistance::hamming(data->ptr<const uint8_t>(), data->cols(), data->rows(), data->stride(), patch1xn->ptr<const uint8_t>(), dist->ptr<int32_t>()));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(Hamming) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(dist).c_str());

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(dist)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Hamming: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
