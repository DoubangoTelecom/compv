#include "../tests/tests_common.h"

#define TAG_TEST			"UnitTestEigenS"
#define ERR_MAX				4.7683715820312500e-07

template <typename T>
static COMPV_ERROR_CODE __math_eigenS()
{
	static const struct compv_unittest_eigen {
		size_t numpoints;
		T sum_d;
		T sum_q;
	}
	COMPV_UNITTEST_EIGEN_FLOAT64[] = {
		//{ 209, static_cast<T>(78812080780.899628), static_cast<T>(26.998671037488755) },
		{ 9, static_cast<T>(9332.8999999999942), static_cast<T>(5.1300204064689812) }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, static_cast<T>(26367.939999999995), static_cast<T>(5.9769114712622589) },
	},
	COMPV_UNITTEST_EIGEN_FLOAT32[] = {
		//{ 209, static_cast<T>(129862.328), static_cast<T>(8.62506199) },
		{ 9, static_cast<T>(36.5654907), static_cast<T>(2.92296124) }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, static_cast<T>(54.4076004), static_cast<T>(3.56752634) },
	};

	const compv_unittest_eigen* test = NULL;
	const compv_unittest_eigen* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_EIGEN_FLOAT32
		: COMPV_UNITTEST_EIGEN_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_EIGEN_FLOAT64) / sizeof(COMPV_UNITTEST_EIGEN_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: eigenS -> %zu %zu ==", sizeof(T), test->numpoints);
		CompVMatPtr A;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, 3, test->numpoints));
		T* x = A->ptr<T>(0);
		T* y = A->ptr<T>(1);
		T* z = A->ptr<T>(2);
		if (std::is_same<T, compv_float64_t>::value) {
			for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
				// Dense matrix
				x[i] = static_cast<T>(((i & 1) ? i : -i) + 0.5); // use "(T)((i & 1) ? i : (-i * 0.7)) + 0.5" instead. Otherwise i sign alterns with same values -> cancel when added
				y[i] = static_cast<T>(((i * 0.2)) + i + 0.7);
				z[i] = static_cast<T>(i*i);
			}
		}
		else {
			for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
				// Sparse matrix (float is really bad for fast convergence)
				x[i] = static_cast<T>((i & 1) ? 0 : -1) + static_cast<T>(0.0001);
				y[i] = static_cast<T>(i * 0.2) + static_cast<T>(1) + static_cast<T>(0.0002);
				z[i] = static_cast<T>(0);
			}
		}

		// Symmetric(S) = A*A
		CompVMatPtr S;
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(A, &S));

		bool symmetric;
		COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(S, symmetric));
		COMPV_CHECK_EXP_RETURN(!symmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Expected symetric matrix");

		// D (diagonal) contains the eigenvalues
		// Q (square) contain eigenvectors (each col is an eigenvector)
		CompVMatPtr D, Q;
		COMPV_CHECK_CODE_RETURN(CompVMatrix::eigenS(S, &D, &Q));

		T d_sum = 0, q_sum = 0;
		for (size_t row = 0; row < D->rows(); ++row) {
			for (size_t col = 0; col < D->cols(); ++col) {
				d_sum += *D->ptr<T>(row, col);
			}
		}
		for (size_t row = 0; row < Q->rows(); ++row) {
			for (size_t col = 0; col < Q->cols(); ++col) {
				q_sum += *Q->ptr<T>(row, col);
			}
		}

		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(d_sum - test->sum_d) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "eigenS: d_sum error value too high");
		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(q_sum - test->sum_q) > ERR_MAX), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "eigenS: q_sum error value too high");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_math_eigenS()
{
	/* == eigenS == */
	COMPV_CHECK_CODE_RETURN((__math_eigenS<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_eigenS<compv_float32_t>()));
	
	return COMPV_ERROR_CODE_S_OK;
}
