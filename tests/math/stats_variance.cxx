#include "../tests_common.h"

#define TAG_TEST			"TestStatsVariance"
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define ERR_MAX				4.7683715820312500e-07

COMPV_ERROR_CODE stats_variance()
{
	static const size_t numpoints = 2015;
	static const TYP mean = static_cast<TYP>(1234569.156325);

	static const struct compv_unittest_var {
		size_t numpoints;
		TYP var;
	}
	COMPV_UNITTEST_VAR_FLOAT64[] = {
		{ 4, static_cast<TYP>(2032210882991.8040) },
		{ 2015, static_cast<TYP>(1524545458375.9475)},
	},
	COMPV_UNITTEST_VAR_FLOAT32[] = {
		{ 4, static_cast<TYP>(2032210935808.000000)},
		{ 2015, static_cast<TYP>(1524546797568.000000) },
	};

	const compv_unittest_var* test = NULL;
	const compv_unittest_var* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_VAR_FLOAT32
		: COMPV_UNITTEST_VAR_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_VAR_FLOAT64) / sizeof(COMPV_UNITTEST_VAR_FLOAT64[i]); ++i) {
		if (tests[i].numpoints == numpoints) {
			test = &tests[i];
			break;
		}
	}

	CompVMatPtr data;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&data, 1, test->numpoints));
	TYP *pdata = data->ptr<TYP>();
	for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
		pdata[i] = static_cast<TYP>(((i & 1) ? i : (-i * 0.7)) + 0.5);
	}

	TYP var;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathStats<TYP>::variance(pdata, test->numpoints, mean, &var));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(StatsVariance) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO("%lf", var);

	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(var - test->var) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsVariance: var error value too high");

	return COMPV_ERROR_CODE_S_OK;
}