#include "../tests_common.h"

#define TAG_TEST			"TestStatsNormalize"
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define ERR_MAX				4.7683715820312500e-07

COMPV_ERROR_CODE stats_normalize2D_hartley()
{
	static const size_t numpoints = 4;

	static const struct compv_unittest_norm {
		size_t numpoints;
		TYP tx;
		TYP ty;
		TYP scale;
	}
	COMPV_UNITTEST_NORM_FLOAT64[] = {
		{ 4, static_cast<TYP>(1.150000), static_cast<TYP>(2.500000), static_cast<TYP>(0.734223) },
		{ 2015, static_cast<TYP>(151.125211), static_cast<TYP>(1209.100000), static_cast<TYP>(0.001239) },
	},
	COMPV_UNITTEST_NORM_FLOAT32[] = {
		{ 4, static_cast<TYP>(1.150000), static_cast<TYP>(2.500000), static_cast<TYP>(0.734223) },
		{ 2015, static_cast<TYP>(151.125229), static_cast<TYP>(1209.100098), static_cast<TYP>(0.001239) },
	};

	const compv_unittest_norm* test = NULL;
	const compv_unittest_norm* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_NORM_FLOAT32
		: COMPV_UNITTEST_NORM_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_NORM_FLOAT64) / sizeof(COMPV_UNITTEST_NORM_FLOAT64[i]); ++i) {
		if (tests[i].numpoints == numpoints) {
			test = &tests[i];
			break;
		}
	}

	CompVMatPtr x, y;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&x, 1, test->numpoints));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&y, 1, test->numpoints));
	TYP *px = x->ptr<TYP>(), *py = y->ptr<TYP>();
	for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
		px[i] = static_cast<TYP>(((i & 1) ? i : (-i * 0.7)) + 0.5);
		py[i] = static_cast<TYP>((i * 0.2) + i + 0.7);
	}

	uint64_t timeStart = CompVTime::nowMillis();
	TYP tx = 0, ty = 0, scale = 0;
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathStats::normalize2D_hartley<TYP>(px, py, test->numpoints, &tx, &ty, &scale));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(StatsNormHartley) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO("%lf, %lf, %lf", tx, ty, scale);

	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(tx - test->tx) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsNormHartley: tx error value too high");
	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(ty - test->ty) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsNormHartley: ty error value too high");
	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(scale - test->scale) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsNormHartley: scale error value too high");

	return COMPV_ERROR_CODE_S_OK;
}
