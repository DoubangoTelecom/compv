#include "../tests_common.h"

#define TAG_TEST			"TestTransform"
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define ERR_MAX				4.7683715820312500e-07

COMPV_ERROR_CODE homogeneousToCartesian2D()
{
	static const size_t numpoints = 215;

	static const struct compv_unittest_trf {
		size_t numpoints;
		TYP sum_x;
		TYP sum_y;
	}
	COMPV_UNITTEST_TRF_FLOAT64[] = {
		{ 215, static_cast<TYP>(2.5678054377560273), static_cast<TYP>(9.7235173598526146) },
		{ 4, static_cast<TYP>(1.7749999999999999), static_cast<TYP>(3.3750000000000000) }, // rectangle
	},
	COMPV_UNITTEST_TRF_FLOAT32[] = {
		{ 215, static_cast<TYP>(2.56780767), static_cast<TYP>(9.72351551) },
		{ 4, static_cast<TYP>(1.77499998), static_cast<TYP>(3.37500000) }, // rectangle
	};

	const compv_unittest_trf* test = NULL;
	const compv_unittest_trf* tests = std::is_same<TYP, compv_float32_t>::value
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
		COMPV_CHECK_CODE_RETURN(CompVMathTransform<TYP>::homogeneousToCartesian2D(&dst, src));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(homogeneousToCartesian2D) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	x = dst->ptr<TYP>(0);
	y = dst->ptr<TYP>(1);
	TYP sum_x = 0, sum_y = 0;
	for (size_t i = 0; i < dst->cols(); ++i) sum_x += x[i];
	for (size_t i = 0; i < dst->cols(); ++i) sum_y += y[i];

	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum_x - test->sum_x) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homogeneousToCartesian2D: x_sum error value too high");
	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum_y - test->sum_y) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homogeneousToCartesian2D: y_sum error value too high");

	return COMPV_ERROR_CODE_S_OK;
}