#include "../tests/tests_common.h"

#define TAG_TEST		"UnitTestMatches"

COMPV_ERROR_CODE unittest_mathes_bruteforce()
{
	static const struct compv_unittest_bf {
		size_t width;
		size_t height;
		const char* md5;
	}
	COMPV_UNITTEST_BF[] = {
		{ 200, 258, "18b5284792c5e08836913505eadfc22f" },
		{ 2005, 101, "22c9f8fa214cbea46160c93e6bfbab05" },
		{ 32, 720, "d03e995b417c287e4e45fecc1b88a749" }, // hamming256
	};

	const compv_unittest_bf* test = NULL;
	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_BF) / sizeof(COMPV_UNITTEST_BF[i]); ++i) {
		test = &COMPV_UNITTEST_BF[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: bruteforce -> %zu %zu ==", test->width, test->height);

		CompVMatPtr queryDescriptions;
		CompVMatPtr trainDescriptions;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&queryDescriptions, test->height, test->width));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&trainDescriptions, test->height, test->width));
		COMPV_CHECK_CODE_RETURN(queryDescriptions->zero_rows());
		for (size_t j = 0; j < trainDescriptions->rows(); ++j) {
			uint8_t* dataPtr = trainDescriptions->ptr<uint8_t>(j);
			size_t row = (test->width * j);
			for (size_t i = 0; i < trainDescriptions->cols(); ++i) {
				dataPtr[i] = static_cast<uint8_t>((i + row) & 0xff);
			}
		}

		CompVMatcherPtr bruteforce;
		COMPV_CHECK_CODE_RETURN(CompVMatcher::newObj(&bruteforce, COMPV_BRUTEFORCE_ID));
		CompVMatPtr matches;

		COMPV_CHECK_CODE_RETURN(bruteforce->process(queryDescriptions, trainDescriptions, &matches));

		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(matches)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "BruteForce: MD5 mismatch");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}
