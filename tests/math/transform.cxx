#include "../tests_common.h"

#define TAG_TEST			"TestTransform"
#define LOOP_COUNT			1
#define TYP					compv_float64_t

COMPV_ERROR_CODE homogeneousToCartesian2D()
{
	static const size_t numpoints = 215;

	static const struct compv_unittest_svd {
		size_t numpoints;
		const char* md5;
	}
	COMPV_UNITTEST_TRF_FLOAT64[] = {
		{ 215, "28aa351d8531f8f140e51059bf0c2428" },
		{ 4, "3e7d29ad3635479a3763cc963d66c9a0" },
	},
	COMPV_UNITTEST_TRF_FLOAT32[] = {
		{ 215, "a47746c56687d18df5fe64a9abcdf570" },
		{ 4, "8bc15e9695f0e73d4b2a5cbb1ae6da3b" },
	};

	const compv_unittest_svd* test = NULL;
	const compv_unittest_svd* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_TRF_FLOAT32
		: COMPV_UNITTEST_TRF_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_TRF_FLOAT64) / sizeof(COMPV_UNITTEST_TRF_FLOAT64[i]); ++i) {
		if (tests[i].numpoints == numpoints) {
			test = &tests[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr src;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&src, 3, test->numpoints));
	TYP* x = src->ptr<TYP>(0);
	TYP* y = src->ptr<TYP>(1);
	TYP* z = src->ptr<TYP>(2);
	for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
		x[i] = static_cast<TYP>(((i & 1) ? i : (-i * 0.7)) + 0.5);
		y[i] = static_cast<TYP>((i * 0.2) + i + 0.7);
		z[i] = static_cast<TYP>(i*i*0.8 + 0.8);
	}

	CompVMatPtr dst;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathTransform<TYP>::homogeneousToCartesian2D(src, &dst));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(homogeneousToCartesian2D) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(dst).c_str());
	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(dst)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homogeneousToCartesian2D: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}