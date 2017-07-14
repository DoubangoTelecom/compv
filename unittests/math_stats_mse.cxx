#include "../tests/tests_common.h"

#define TAG_TEST			"UnitTestStatsMSE"
#define ERR_MAX				4.7683715820312500e-07

template <typename T>
static COMPV_ERROR_CODE __math_stats_mse2D_homogeneous()
{
	CompVMatPtr mse;

	static const struct compv_unittest_mse {
		size_t numpoints;
		T mse_sum;
	}
	COMPV_UNITTEST_MSE_FLOAT64[] = {
		{ 4, static_cast<T>(332.337567) },
		{ 2015, static_cast<T>(3726586387.772121) },
	},
	COMPV_UNITTEST_MSE_FLOAT32[] = {
		{ 4, static_cast<T>(332.337585) },
		{ 2015, static_cast<T>(3726586880.000000) },
	};

	const compv_unittest_mse* test = NULL;
	const compv_unittest_mse* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_MSE_FLOAT32
		: COMPV_UNITTEST_MSE_FLOAT64;
	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_MSE_FLOAT64) / sizeof(COMPV_UNITTEST_MSE_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: stats MSE 2D homogeneous -> %zu %zu ==", sizeof(T), test->numpoints);
		CompVMatPtr aX_h, aY_h, aZ_h, bX, bY;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&aX_h, 1, test->numpoints));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&aY_h, 1, test->numpoints));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&aZ_h, 1, test->numpoints));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&bX, 1, test->numpoints));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&bY, 1, test->numpoints));
		T *paX_h = aX_h->ptr<T>(), *paY_h = aY_h->ptr<T>(), *paZ_h = aZ_h->ptr<T>(), *pbX = bX->ptr<T>(), *pbY = bY->ptr<T>();

		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			paX_h[i] = static_cast<T>(((i & 1) ? i : (-i * 0.7)) + 0.5);
			pbX[i] = static_cast<T>(((i & 1) ? i : (-i * 0.3)) + 5.);
			paY_h[i] = static_cast<T>((i * 0.2) + i + 0.7);
			pbY[i] = static_cast<T>(((i & 1) ? i : (-i * 0.8)) + 7.8);
			paZ_h[i] = static_cast<T>((i * 0.3) + i + 0.4);
		}

		COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::mse2D_homogeneous(&mse, paX_h, paY_h, paZ_h, pbX, pbY, test->numpoints));

		T mse_sum = 0;
		const T* msePtr = mse->ptr<T>();
		for (size_t i = 0; i < mse->cols(); ++i) {
			mse_sum += msePtr[i];
		}

		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(mse_sum - test->mse_sum) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsMSE: error value too high");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_math_stats_mse2D_homogeneous()
{
	COMPV_CHECK_CODE_RETURN((__math_stats_mse2D_homogeneous<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_stats_mse2D_homogeneous<compv_float32_t>()));

	return COMPV_ERROR_CODE_S_OK;
}
