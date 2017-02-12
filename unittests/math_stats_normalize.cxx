#include "../tests/tests_common.h"

#define TAG_TEST			"UnitTestStatsNorm"
#define ERR_MAX				4.7683715820312500e-07

template <typename T>
static COMPV_ERROR_CODE __math_stats_normalize2D_hartley()
{
	static const struct compv_unittest_norm {
		size_t numpoints;
		T tx;
		T ty;
		T scale;
	}
	COMPV_UNITTEST_NORM_FLOAT64[] = {
		{ 4, static_cast<T>(1.150000), static_cast<T>(2.500000), static_cast<T>(0.734223) },
		{ 2015, static_cast<T>(151.125211), static_cast<T>(1209.100000), static_cast<T>(0.001239) },
	},
	COMPV_UNITTEST_NORM_FLOAT32[] = {
		{ 4, static_cast<T>(1.150000), static_cast<T>(2.500000), static_cast<T>(0.734223) },
		{ 2015, static_cast<T>(151.125229), static_cast<T>(1209.100098), static_cast<T>(0.001239) },
	};

	const compv_unittest_norm* test = NULL;
	const compv_unittest_norm* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_NORM_FLOAT32
		: COMPV_UNITTEST_NORM_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_NORM_FLOAT64) / sizeof(COMPV_UNITTEST_NORM_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: stats norm 2D hartley -> %zu %zu ==", sizeof(T), test->numpoints);
		CompVMatPtr x, y;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&x, 1, test->numpoints));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&y, 1, test->numpoints));
		T *px = x->ptr<T>(), *py = y->ptr<T>();
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			px[i] = static_cast<T>(((i & 1) ? i : (-i * 0.7)) + 0.5);
			py[i] = static_cast<T>((i * 0.2) + i + 0.7);
		}

		T tx, ty, scale;
		COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::normalize2D_hartley(px, py, test->numpoints, &tx, &ty, &scale));

		//COMPV_DEBUG_INFO("%lf, %lf, %lf", tx, ty, scale);

		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(tx - test->tx) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsNormHartley: tx error value too high");
		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(ty - test->ty) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsNormHartley: ty error value too high");
		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(scale - test->scale) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsNormHartley: scale error value too high");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}	

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_math_stats_normalize2D_hartley()
{
	COMPV_CHECK_CODE_RETURN((__math_stats_normalize2D_hartley<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_stats_normalize2D_hartley<compv_float32_t>()));

	return COMPV_ERROR_CODE_S_OK;
}
