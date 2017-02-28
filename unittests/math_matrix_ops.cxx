#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestMatrixOps"

template <typename T>
static COMPV_ERROR_CODE __math_matrix_ops_transpose()
{
	// 231 and 215 are SIMD-unfriendly
	// 3x3, 4x4... are full-optiz functions
	static const size_t sizes[5][2] = { { 231, 215 }, { 215, 231 } ,{ 3, 3 }, { 4, 4 },{ 16, 16 } };

	for (size_t size = 0; size < sizeof(sizes) / sizeof(sizes[0]); ++size) {
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Matrix op transpose -> %zu (%zu x %zu) ==", sizeof(T), sizes[size][0], sizes[size][1]);
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
		{ 31, 31, 31, 31, "a8b3305da722f83834f3d04045466969" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all bCols cases
	},
	COMPV_UNITTEST_MULAB_FLOAT32[] = {
		{ 215, 115, 115, 75, "7a7967e10f80ab6c7a68feb5d8deb8da" },
		{ 3, 3, 3, 3, "db5d1d779c11164e72964f6f69289c77" },
		{ 4, 4, 4, 4, "fccbc6a3eea330fd07b9325a13f462f2" },
		{ 31, 31, 31, 31, "7918ffba8fb8566120f23dadaace4ae8" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all bCols cases
	};

	const compv_unittest_mulAB* test = NULL;
	const compv_unittest_mulAB* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_MULAB_FLOAT32
		: COMPV_UNITTEST_MULAB_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_MULAB_FLOAT64) / sizeof(COMPV_UNITTEST_MULAB_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Matrix op mulAB -> %zu (%zu x %zu) mul (%zu x%zu ) ==", sizeof(T), test->arows, test->acols, test->brows, test->bcols);
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

template <typename T>
static COMPV_ERROR_CODE __math_matrix_ops_mulGA()
{
	CompVMatPtr A;
	static const struct compv_unittest_mulGA {
		size_t rows;
		size_t cols;
		const char* md5;
		const char* md5_fma;
	}
	COMPV_UNITTEST_MULGA_FLOAT64[] = {
		{ 215, 215, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" },
		{ 19, 21, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" },
		{ 701, 71, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" },
		{ 31, 31, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all cases
		{ 9, 9, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" }, // Homography
	},
	COMPV_UNITTEST_MULGA_FLOAT32[] = {
		{ 215, 215, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" },
		{ 19, 21, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" },
		{ 701, 71, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" },
		{ 31, 31, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all cases
		{ 9, 9, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" }, // Homography
	};

	const compv_unittest_mulGA* test = NULL;
	const compv_unittest_mulGA* tests = std::is_same<T, compv_float32_t>::value
		? COMPV_UNITTEST_MULGA_FLOAT32
		: COMPV_UNITTEST_MULGA_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_MULGA_FLOAT64) / sizeof(COMPV_UNITTEST_MULGA_FLOAT64[i]); ++i) {
		test = &tests[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Matrix op mulGA -> %zu (%zu x %zu) ==", sizeof(T), test->rows, test->cols);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, tests->rows, tests->cols));

		for (size_t i = 0; i < tests->rows; ++i) {
			for (size_t j = 0; j < tests->cols; ++j) {
				*A->ptr<T>(i, j) = static_cast<T>((i + j) * (i + 1) + 0.7 + (100 * ((i & 1) ? -1 : 1)));
			}
		}
		for (size_t ith = 0; ith < tests->rows; ++ith) {
			for (size_t jth = 0; jth < ith; ++jth) {
				COMPV_CHECK_CODE_RETURN(CompVMatrix::mulGA<T>(A, ith, jth, static_cast<T>(-0.9855), static_cast<T>(0.777774)));
			}
		}
#	if COMPV_ARCH_X86
		const bool fma = compv_tests_is_fma_enabled() && CompVCpu::isAsmEnabled(); // no FMA3 intrin impl.
#	else
		const bool fma = false; // FMA not enabled for ARM yet
#	endif
		COMPV_CHECK_EXP_RETURN(std::string(fma ? test->md5_fma : test->md5).compare(compv_tests_md5(A)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Matrix ops mulGA: MD5 mismatch");
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE __math_matrix_ops_isSymetric(size_t matrixSize)
{
	COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Matrix op isSymetric -> %zu %zu ==", sizeof(T), matrixSize);

	CompVMatPtr A, S;
	T v, *aptr;

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&A, matrixSize, matrixSize));

	// build random matrix
	for (size_t j = 0; j < matrixSize; ++j) {
		aptr = A->ptr<T>(j);
		for (size_t i = 0; i < matrixSize; ++i) {
			aptr[i] = static_cast<T>(rand());
		}
	}

	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(A, &S)); // build symetric matrix
	bool isSymmetric;
	
	COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(A, isSymmetric));
	COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "A not symetric");
	COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(S, isSymmetric));
	COMPV_CHECK_EXP_RETURN(!isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "AtA is symetric");
	// change one element in the last row (become last column when transposed - useful to make sure SIMD code will read up to the last column)
	v = *S->ptr<const T>((matrixSize - 1), ((matrixSize - 1) >> 1));
	*S->ptr<T>((matrixSize - 1), ((matrixSize - 1) >> 1)) = static_cast<T>(rand()) + (v / 2) + 3;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(S, isSymmetric));
	COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "S no longer symetric");
	*S->ptr<T>((matrixSize - 1), ((matrixSize - 1) >> 1)) = v; // restore value for the next loop

	COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");

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

	/* == Matrix MulGA == */
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_mulGA<compv_float32_t>()));
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_mulGA<compv_float64_t>()));

	/* == Matrix isSymetric == */
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_isSymetric<compv_float64_t>(215)));
	COMPV_CHECK_CODE_RETURN((__math_matrix_ops_isSymetric<compv_float32_t>(215)));

	return COMPV_ERROR_CODE_S_OK;
}
