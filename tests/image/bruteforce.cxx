#include "../tests_common.h"

#define TAG_TEST			"TestBruteForce"
#define LOOP_COUNT			1

COMPV_ERROR_CODE bruteforce()
{
	static const  size_t width = 32;
	static const struct compv_unittest_bf {
		size_t width;
		size_t height;
		const char* md5;
	}
	COMPV_UNITTEST_BF[] = {
		{ 200, 258, "442db0c57bd88bcf4ea3150abfca122b" }, // used for speed comparison against OpenCV
		{ 32, 715, "8149035c24dbb95e59d248460aa5a03f" }, // hamming256
		{ 2005, 101, "c01c1f8d044b1d79933ecd532052f0a4" },
        // 2007: very intensive (for local tesing only, do not add to unittest)
		{ 2007, 302, "98f3c40765e5d0d47f32278d5391832d" },
	};

	const compv_unittest_bf* test = NULL;
	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_BF) / sizeof(COMPV_UNITTEST_BF[i]); ++i) {
		if (COMPV_UNITTEST_BF[i].width == width) {
			test = &COMPV_UNITTEST_BF[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr queryDescriptions;
	CompVMatPtr trainDescriptions;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&queryDescriptions, test->height, test->width));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&trainDescriptions, test->height, test->width));
	for (size_t j = 0; j < queryDescriptions->rows(); ++j) {
		uint8_t* dataPtr = queryDescriptions->ptr<uint8_t>(j);
		for (size_t i = 0; i < queryDescriptions->cols(); ++i) {
			dataPtr[i] = static_cast<uint8_t>((((i + j) << 1) + 1) & 0xff);
		}
	}
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

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(bruteforce->process(queryDescriptions, trainDescriptions, &matches));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(BruteForce) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(matches).c_str());

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(matches)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "BruteForce: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
