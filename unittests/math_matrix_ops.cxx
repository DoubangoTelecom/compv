#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestMatrixOps"

template <typename T>
static COMPV_ERROR_CODE __math_matrix_ops_transpose()
{
	// 231 and 215 are SIMD-unfriendly
	// 3x3, 4x4... are full-optiz functions
	static const size_t sizes[5][2] = { { 231, 215 }, { 215, 231 } ,{ 3, 3 }, { 4, 4 },{ 16, 16 } };

	for (size_t size = 0; size < sizeof(sizes) / sizeof(sizes[0]); ++size) {
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Matrix op transpose -> (%zu x %zu) ==", sizes[size][0], sizes[size][1]);
		CompVMatPtr A, R;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, sizes[size][0], sizes[size][1]));
		T *aptr;
		size_t row, col, arow;
		const size_t arows = A->rows();
		const size_t acols = A->cols();
		for (row = 0; row < arows; ++row) {
			aptr = A->ptr<T>(row);
			arow = row * arows;
			for (col = 0; col < acols; ++col) {
				aptr[col] = static_cast<T>(col + arow);
			}
		}
		// transpose
		COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(A, &R));
		// check result
		for (row = 0; row < arows; ++row) {
			for (col = 0; col < acols; ++col) {
				COMPV_CHECK_EXP_RETURN(*A->ptr<T>(row, col) != *R->ptr<T>(col, row), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Matrix ops transpose failed");
			}
		}
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}
	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE __math_matrix_ops_mulAB()
{
	static const struct compv_unittest_mulAB {
		size_t arows;
		size_t acols;
		size_t brows;
		size_t bcols;
		const char* md5;
		const char* md5_fma;
	}
	COMPV_UNITTEST_MULAB_FLOAT64[] = {
		{ 215, 115, 115, 75, "5e1883dfd24448b64e1b6ae007b91758" },
		{ 3, 3, 3, 3, "de628e91457c329220badfb524016b99" },
		{ 4, 4, 4, 4, "5b2f3f02883ef6f944d90373be188104" },
	},
	COMPV_UNITTEST_MULAB_FLOAT32[] = {
		{ 215, 115, 115, 75, "7a7967e10f80ab6c7a68feb5d8deb8da" },
		{ 3, 3, 3, 3, "db5d1d779c11164e72964f6f69289c77" },
		{ 4, 4, 4, 4, "fccbc6a3eea330fd07b9325a13f462f2" },
	};

	const compv_unittest_mulAB* test = NULL;
	const compv_unittest_mulAB* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_MULAB_FLOAT32
		: COMPV_UNITTEST_MULAB_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_MULAB_FLOAT64) / sizeof(COMPV_UNITTEST_MULAB_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Matrix op mulAB -> (%zu x %zu) mul (%zu x%zu ) ==", test->arows, test->acols, test->brows, test->bcols);
		CompVMatPtr A, B, R;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, test->arows, test->acols));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&B, test->brows, test->bcols));

		for (size_t i = 0; i < test->arows; ++i) {
			for (size_t j = 0; j < test->acols; ++j) {
				*A->ptr<T>(i, j) = static_cast<T>((i + j) * (i + 1));
			}
		}
		for (size_t i = 0; i < test->brows; ++i) {
			for (size_t j = 0; j < test->bcols; ++j) {
				*B->ptr<T>(i, j) = static_cast<T>((i + j) * (i + 1));
			}
		}
		// mulAB
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(A, B, &R));
		// Check result
		COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(R)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Matrix ops mulAB: MD5 mismatch");

		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}


COMPV_ERROR_CODE unittest_math_matrix_ops()
{
	/* == Matrix transpose == */
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_transpose<compv_float32_t>()));
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_transpose<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_transpose<int32_t>()));

	/* == Matrix mulAB == */
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_mulAB<compv_float32_t>()));
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_mulAB<compv_float64_t>()));

	return COMPV_ERROR_CODE_S_OK;
}