#include "../tests/tests_common.h"

#define TAG_TEST			"UnitTestStatsVariance"
#define ERR_MAX				4.7683715820312500e-07

template <typename T>
static COMPV_ERROR_CODE __math_stats_variance()
{
	static const T mean = static_cast<T>(1234569.156325);

	static const struct compv_unittest_var {
		size_t numpoints;
		T var;
	}
	COMPV_UNITTEST_VAR_FLOAT64[] = {
		{ 4, static_cast<T>(2032210882991.8040) },
		{ 2015, static_cast<T>(1524545458375.9475) },
	},
	COMPV_UNITTEST_VAR_FLOAT32[] = {
		{ 4, static_cast<T>(2032210935808.000000) },
		{ 2015, static_cast<T>(1524546797568.000000) },
	};

	const compv_unittest_var* test = NULL;
	const compv_unittest_var* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_VAR_FLOAT32
		: COMPV_UNITTEST_VAR_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_VAR_FLOAT64) / sizeof(COMPV_UNITTEST_VAR_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: stats variance -> %zu %zu ==", sizeof(T), test->numpoints);
		CompVMatPtr data;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&data, 1, test->numpoints));
		T *pdata = data->ptr<T>();
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			pdata[i] = static_cast<T>(((i & 1) ? i : (-i * 0.7)) + 0.5);
		}

		T var;		
		COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::variance(pdata, test->numpoints, mean, &var));

		//COMPV_DEBUG_INFO("%lf", var);

		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(var - test->var) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsVariance: var error value too high");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_math_stats_variance()
{
	COMPV_CHECK_CODE_RETURN((__math_stats_variance<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_stats_variance<compv_float32_t>()));

	return COMPV_ERROR_CODE_S_OK;
}
