#include "../tests_common.h"

#define TAG_TEST			"TestEigen"
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define ERR_MAX				4.7683715820312500e-07

COMPV_ERROR_CODE eigenS()
{
	static const size_t numpoints = 31;

	static const struct compv_unittest_eigen {
		size_t numpoints;
		TYP sum_d;
		TYP sum_q;
	}
	COMPV_UNITTEST_EIGEN_FLOAT64[] = {
		{ 31, static_cast<TYP>(5297858.3400000036), static_cast<TYP>(12.034372491407161) },
		{ 9, static_cast<TYP>(9332.8999999999942), static_cast<TYP>(3.6319384624951985) }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, static_cast<TYP>(26367.939999999995), static_cast<TYP>(4.3785011161705851) },
	},
	COMPV_UNITTEST_EIGEN_FLOAT32[] = {
		{ 31, static_cast<TYP>(611.246460), static_cast<TYP>(8.62506199) },
		{ 9, static_cast<TYP>(36.5654907), static_cast<TYP>(2.73758173) }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, static_cast<TYP>(54.4076004), static_cast<TYP>(2.98505616) },
	};

	const compv_unittest_eigen* test = NULL;
	const compv_unittest_eigen* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_EIGEN_FLOAT32
		: COMPV_UNITTEST_EIGEN_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_EIGEN_FLOAT64) / sizeof(COMPV_UNITTEST_EIGEN_FLOAT64[i]); ++i) {
		if (tests[i].numpoints == numpoints) {
			test = &tests[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr A;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, 3, test->numpoints));
	TYP* x = A->ptr<TYP>(0);
	TYP* y = A->ptr<TYP>(1);
	TYP* z = A->ptr<TYP>(2);
	if (std::is_same<TYP, compv_float64_t>::value) {
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			// Dense matrix
			x[i] = static_cast<TYP>(((i & 1) ? i : -i) + 0.5); // use "(T)((i & 1) ? i : (-i * 0.7)) + 0.5" instead. Otherwise i sign alterns with same values -> cancel when added
			y[i] = static_cast<TYP>(((i * 0.2)) + i + 0.7);
			z[i] = static_cast<TYP>(i*i);
		}
	}
	else {
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			// Sparse matrix (float is really bad for fast convergence)
			x[i] = static_cast<TYP>((i & 1) ? 0 : -1) + static_cast<TYP>(0.0001);
			y[i] = static_cast<TYP>(i * 0.2) + static_cast<TYP>(1) + static_cast<TYP>(0.0002);
			z[i] = static_cast<TYP>(0);
		}
	}
	
	// Symmetric(S) = A*A
	CompVMatPtr S;
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(A, &S));
	COMPV_GCC_DISABLE_WARNINGS_END()

	bool symmetric;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(S, symmetric));
	COMPV_CHECK_EXP_RETURN(!symmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Expected symetric matrix");

	CompVMatPtr D, Q;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		// D (diagonal) contains the eigenvalues
		// Q (square) contain eigenvectors (each col is an eigenvector)
		COMPV_CHECK_CODE_RETURN(CompVMatrix::eigenS(S, &D, &Q));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(eigenS) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	TYP d_sum = 0, q_sum = 0;
	for (size_t row = 0; row < D->rows(); ++row) {
		for (size_t col = 0; col < D->cols(); ++col) {
			d_sum += *D->ptr<TYP>(row, col);
		}
	}
	for (size_t row = 0; row < Q->rows(); ++row) {
		for (size_t col = 0; col < Q->cols(); ++col) {
			q_sum += *Q->ptr<TYP>(row, col);
		}
	}

	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(d_sum - test->sum_d) > std::numeric_limits<TYP>::epsilon()), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "eigenS: d_sum error value too high");
	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(q_sum - test->sum_q) > std::numeric_limits<TYP>::epsilon()), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "eigenS: q_sum error value too high");

	return COMPV_ERROR_CODE_S_OK;
}