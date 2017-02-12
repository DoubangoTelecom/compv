#include "../tests/tests_common.h"

#define TAG_TEST			"UnitTestMathTransform"
#define ERR_MAX				4.7683715820312500e-07

template <typename T>
static COMPV_ERROR_CODE __math_transform_homogeneousToCartesian2D()
{
	static const struct compv_unittest_trf {
		size_t numpoints;
		T sum_x;
		T sum_y;
	}
	COMPV_UNITTEST_TRF_FLOAT64[] = {
		{ 215, static_cast<T>(2.5678054377560273), static_cast<T>(9.7235173598526146) },
		{ 4, static_cast<T>(1.7749999999999999), static_cast<T>(3.3750000000000000) },
	},
	COMPV_UNITTEST_TRF_FLOAT32[] = {
		{ 215, static_cast<T>(2.56780767), static_cast<T>(9.72351551) },
		{ 4, static_cast<T>(1.77499998), static_cast<T>(3.37500000) },
	};

	const compv_unittest_trf* test = NULL;
	const compv_unittest_trf* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_TRF_FLOAT32
		: COMPV_UNITTEST_TRF_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_TRF_FLOAT64) / sizeof(COMPV_UNITTEST_TRF_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: math transform homogeneousToCartesian2D -> %zu %zu ==", sizeof(T), test->numpoints);
		CompVMatPtr src;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&src, 3, test->numpoints));
		T* x = src->ptr<T>(0);
		T* y = src->ptr<T>(1);
		T* z = src->ptr<T>(2);
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			x[i] = static_cast<T>(((i & 1) ? i : (-i * 0.7)) + 0.5);
			y[i] = static_cast<T>((i * 0.2) + i + 0.7);
			z[i] = static_cast<T>(i*i*0.8 + 0.8);
		}

		CompVMatPtr dst;
		COMPV_CHECK_CODE_RETURN(CompVMathTransform<T>::homogeneousToCartesian2D(src, &dst));

		x = dst->ptr<T>(0);
		y = dst->ptr<T>(1);
		T sum_x = 0, sum_y = 0;
		for (size_t i = 0; i < dst->cols(); ++i) sum_x += x[i];
		for (size_t i = 0; i < dst->cols(); ++i) sum_y += y[i];

		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum_x - test->sum_x) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homogeneousToCartesian2D: x_sum error value too high");
		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum_y - test->sum_y) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homogeneousToCartesian2D: y_sum error value too high");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}	

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_math_transform_homogeneousToCartesian2D()
{
	COMPV_CHECK_CODE_RETURN((__math_transform_homogeneousToCartesian2D<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_transform_homogeneousToCartesian2D<compv_float32_t>()));

	return COMPV_ERROR_CODE_S_OK;
}
