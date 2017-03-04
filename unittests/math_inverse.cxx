#include "../tests/tests_common.h"

#define TAG_TEST		"UnitTestInverse"
#define ERR_MAX_F64		8.5209617139980764e-14
#define ERR_MAX_F32		6.80796802e-09

template <typename T>
static COMPV_ERROR_CODE __math_pseudoinv()
{
	static const struct compv_unittest_psi {
		size_t rows;
		size_t cols;
		T sum;
	}
	COMPV_UNITTEST_PSI_FLOAT64[] = {
		{ 11, 7, static_cast<T>(-1.2316536679435330e-16) }, // non-square
		{ 9, 9, static_cast<T>(-6.0715321659188248e-17) },
		{ 3, 3, static_cast<T>(9.4702024000525853e-14) },
	},
	COMPV_UNITTEST_PSI_FLOAT32[] = {
		{ 11, 7, static_cast<T>(-7.91624188e-08) }, // non-square
		{ 9, 9, static_cast<T>(-1.67638063e-08) },
		{ 3, 3, static_cast<T>(5.96046448e-08) },
	};

	T err_max = std::is_same<T, compv_float32_t>::value
		? static_cast<T>(ERR_MAX_F32)
		: static_cast<T>(ERR_MAX_F64);
	const compv_unittest_psi* test = NULL;
	const compv_unittest_psi* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_PSI_FLOAT32
		: COMPV_UNITTEST_PSI_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_PSI_FLOAT64) / sizeof(COMPV_UNITTEST_PSI_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: pseudoinv -> %zu (%zu x %zu) ==", sizeof(T), test->rows, test->cols);

		// Build random data (must be deterministic to have same MD5)
		CompVMatPtr A;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, test->rows, test->cols));
		T* row;
		for (signed j = 0; j < static_cast<signed>(A->rows()); ++j) {
			row = A->ptr<T>(j);
			for (signed i = 0; i < static_cast<signed>(A->cols()); ++i) {
				row[i] = static_cast<T>(((i & 1) ? i : -i) + (j * 0.5) + j + 0.7);
			}
		}

		CompVMatPtr Ai;
		COMPV_CHECK_CODE_RETURN(CompVMatrix::pseudoinv(A, &Ai));

		T sum = 0;
		for (size_t row = 0; row < Ai->rows(); ++row) {
			for (size_t col = 0; col < Ai->cols(); ++col) {
				sum += *Ai->ptr<T>(row, col);
			}
		}

		COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum - test->sum) > err_max), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "pseudoinv: sum error value too high");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}	

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE __math_inv3x3()
{
	CompVMatPtr A, Ai, I;
	T sum;

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, 3, 3));

	/************************ Non Singular ************************/
	COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: inv3x3_nonsingular -> %zu ==", sizeof(T));
	static const T NonSingular[3][3] = { // non-singular
		{ 5, 7, 2 },
		{ 1, 9, 4 },
		{ 2, 6, 3 }
	};
	static const T Identity[3][3] = {
		{ static_cast<T>(1.0), static_cast<T>(0.0), static_cast<T>(0.0) },
		{ static_cast<T>(0.0), static_cast<T>(1.0), static_cast<T>(0.0) },
		{ static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0) },
	};
	T err_max_nonsingular = std::is_same<T, compv_float32_t>::value
		? static_cast<T>(1.31130219e-06)
		: static_cast<T>(3.7747582837255322e-15);
	
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			*A->ptr<T>(j, i) = NonSingular[j][i];
		}
	}
	
	COMPV_CHECK_CODE_RETURN(CompVMatrix::invA3x3(A, &Ai));

	// Compute error
	CompVMatrix::mulAB(A, Ai, &I);
	sum = 0;
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			sum += COMPV_MATH_ABS(Identity[j][i] - *I->ptr<T>(j, i));
		}
	}

	COMPV_CHECK_EXP_RETURN((sum > err_max_nonsingular), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "inv3x3_nonsingluar: sum error value too high");

	COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");

	/************************ Singular ************************/
	COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: inv3x3_singular -> %zu ==", sizeof(T));
	static const T Singular[3][3] = { // Singular
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 }
	};
	static const T SingularInverseFloat64[3][3] = { // Expected resul: http://comnuan.com/cmnn0100f/
		{ static_cast<T>(-0.63888888888888351), static_cast<T>(-0.16666666666666527), static_cast<T>(0.30555555555555336) },
		{ static_cast<T>(-0.055555555555554977), static_cast<T>(1.5612511283791264e-16), static_cast<T>(0.055555555555555323) },
		{ static_cast<T>(0.52777777777777357), static_cast<T>(0.16666666666666552), static_cast<T>(-0.19444444444444264) }
	};
	static const T SingularInverseFloat64_NEON_FMA[3][3] = {
		{ static_cast<T>(0.016780410445101677), static_cast<T>(0.081424419567815404), static_cast<T>(0.075185261194962272) },
		{ static_cast<T>(-1.366894154223544), static_cast<T>(-0.4961821724689689), static_cast<T>(0.51629614427674031) },
		{ static_cast<T>(1.1834470771117749), static_cast<T>(0.41475775290115291), static_cast<T>(-0.42481473880503628) }
	};
	static const T SingularInverseFloat32[3][3] = { // Expected resul: http://comnuan.com/cmnn0100f/
		{ static_cast<T>(-0.638891041), static_cast<T>(-0.166667357), static_cast<T>(0.305556685) },
		{ static_cast<T>(-0.0555560775), static_cast<T>(-1.52736902e-07), static_cast<T>(0.0555558056) },
		{ static_cast<T>(0.527779818), static_cast<T>(0.166667312), static_cast<T>(-0.194445491) }
	};
	const T(*inverse)[3][3] = std::is_same<T, compv_float32_t>::value
		? &SingularInverseFloat32
		: (compv_tests_is_fma_enabled() && CompVCpu::isEnabled(kCpuFlagARM)) ? &SingularInverseFloat64_NEON_FMA : &SingularInverseFloat64;
	T err_max_singular = std::is_same<T, compv_float32_t>::value
		? static_cast<T>(3.22833657e-05)
		: static_cast<T>(3.8163916471489756e-16);

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, 3, 3));
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			*A->ptr<T>(j, i) = Singular[j][i];
		}
	}
	
	COMPV_CHECK_CODE_RETURN(CompVMatrix::invA3x3(A, &Ai));

	// Compute error
	sum = 0;
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			sum += COMPV_MATH_ABS((*inverse)[j][i] - *Ai->ptr<T>(j, i));
		}
	}

	COMPV_CHECK_EXP_RETURN((sum > err_max_singular), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "inv3x3_singluar: sum error value too high");

	COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE unittest_math_inverse()
{
	/* == Moore–Penrose pseudoinverse == */
	COMPV_CHECK_CODE_RETURN((__math_pseudoinv<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_pseudoinv<compv_float32_t>()));

	/* Fast 3x3 inverse */
	COMPV_CHECK_CODE_RETURN((__math_inv3x3<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_inv3x3<compv_float32_t>()));

	return COMPV_ERROR_CODE_S_OK;
}
