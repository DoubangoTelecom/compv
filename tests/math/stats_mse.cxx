#include "../tests_common.h"

#define TAG_TEST			"TestStatsMSE"
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define ERR_MAX				4.7683715820312500e-07

COMPV_ERROR_CODE stats_mse2D_homogeneous()
{
	static const size_t numpoints = 2015;

	static const struct compv_unittest_mse {
		size_t numpoints;
		TYP mse_sum;
	}
	COMPV_UNITTEST_MSE_FLOAT64[] = {
		{ 4, static_cast<TYP>(332.337567) },
		{ 2015, static_cast<TYP>(3726586387.772121) },
	},
	COMPV_UNITTEST_MSE_FLOAT32[] = {
		{ 4, static_cast<TYP>(332.337585) },
		{ 2015, static_cast<TYP>(3726586880.000000) },
	};

	const compv_unittest_mse* test = NULL;
	const compv_unittest_mse* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_MSE_FLOAT32
		: COMPV_UNITTEST_MSE_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_MSE_FLOAT64) / sizeof(COMPV_UNITTEST_MSE_FLOAT64[i]); ++i) {
		if (tests[i].numpoints == numpoints) {
			test = &tests[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr aX_h, aY_h, aZ_h, bX, bY;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&aX_h, 1, test->numpoints));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&aY_h, 1, test->numpoints));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&aZ_h, 1, test->numpoints));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&bX, 1, test->numpoints));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&bY, 1, test->numpoints));
	TYP *paX_h = aX_h->ptr<TYP>(), *paY_h = aY_h->ptr<TYP>(), *paZ_h = aZ_h->ptr<TYP>(), *pbX = bX->ptr<TYP>(), *pbY = bY->ptr<TYP>();

	for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
		paX_h[i] = static_cast<TYP>(((i & 1) ? i : (-i * 0.7)) + 0.5);
		pbX[i] = static_cast<TYP>(((i & 1) ? i : (-i * 0.3)) + 5.);
		paY_h[i] = static_cast<TYP>((i * 0.2) + i + 0.7);
		pbY[i] = static_cast<TYP>(((i & 1) ? i : (-i * 0.8)) + 7.8);
		paZ_h[i] = static_cast<TYP>((i * 0.3) + i + 0.4);
	}

	CompVMatPtr mse;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathStats<TYP>::mse2D_homogeneous(&mse, paX_h, paY_h, paZ_h, pbX, pbY, test->numpoints));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(StatsMSE) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	TYP mse_sum = 0;
	const TYP* msePtr = mse->ptr<TYP>();
	for (size_t i = 0; i < mse->cols(); ++i) {
		mse_sum += msePtr[i];
	}

	TYP err = COMPV_MATH_ABS(mse_sum - test->mse_sum);
	//COMPV_DEBUG_INFO("%lf", err);
	
	COMPV_CHECK_EXP_RETURN((err > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "StatsMSE: error value too high");

	return COMPV_ERROR_CODE_S_OK;
}