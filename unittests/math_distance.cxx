#include "../tests/tests_common.h"

#define TAG_TEST		"UnitTestDistance"

COMPV_ERROR_CODE unittest_math_distance_hamming()
{

	static const struct compv_unittest_hamming {
		size_t width;
		size_t height;
		const char* md5;
	}
	COMPV_UNITTEST_HAMMING[] = {
		{ 2005, 101, "47005cd9900cac52d530e0a3e0f746be" },
		{ 1280, 720, "2416808864658a997a39007fc1a548c4" },
		{ 32, 720, "f97c1bfb83b8f9d92e0f2c63b06b8bc8" }, // hamming256
	};

	const compv_unittest_hamming* test;
	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_HAMMING) / sizeof(COMPV_UNITTEST_HAMMING[i]); ++i) {
		test = &COMPV_UNITTEST_HAMMING[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: hamming distance -> %zu %zu ==", test->width, test->height);

		CompVMatPtr patch1xn;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&patch1xn, 1, test->width));
		COMPV_CHECK_CODE_RETURN(patch1xn->zero_row(0));

		CompVMatPtr data;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&data, test->height, test->width));
		for (size_t j = 0; j < data->rows(); ++j) {
			uint8_t* dataPtr = data->ptr<uint8_t>(j);
			size_t row = (test->width * j);
			for (size_t i = 0; i < data->cols(); ++i) {
				dataPtr[i] = static_cast<uint8_t>((i + row) & 0xff);
			}
		}

		CompVMatPtr dist;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&dist, 1, test->height)); // each row produce #1 hamming distance -> distance size = (1 x height)

		COMPV_CHECK_CODE_RETURN(CompVMathDistance::hamming(data->ptr<const uint8_t>(), data->cols(), data->rows(), data->stride(), patch1xn->ptr<const uint8_t>(), dist->ptr<int32_t>()));

		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(dist)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Hamming: MD5 mismatch");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}