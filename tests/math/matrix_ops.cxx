#include "../tests_common.h"

#define TAG_TEST			"TestMatrixOps"
#define LOOP_COUNT			1000
#define TYP					compv_float64_t

COMPV_ERROR_CODE matrix_ops_transpose()
{
	CompVMatPtr A, R;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, 215, 231)); // 215 and 231 are SIMD-unfriendly for testing
	TYP *aptr;
	size_t row, col, arow;
	const size_t arows = A->rows();
	const size_t acols = A->cols();
	for (row = 0; row < arows; ++row) {
		aptr = A->ptr<TYP>(row);
		arow = row * arows;
		for (col = 0; col < acols; ++col) {
			aptr[col] = static_cast<TYP>(col + arow);
		}
	}
	
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(A, &R));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(Transpose) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if LOOP_COUNT == 1
	for (row = 0; row < arows; ++row) {
		for (col = 0; col < acols; ++col) {
			COMPV_CHECK_EXP_RETURN(*A->ptr<TYP>(row, col) != *R->ptr<TYP>(col, row), COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		}
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE matrix_ops_mulAB()
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
	static const size_t arows = 215;
	static const size_t acols = 115;
	static const size_t brows = 115;
	static const size_t bcols = 75;

	const compv_unittest_mulAB* test = NULL;
	const compv_unittest_mulAB* tests = std::is_same<TYP, compv_float32_t>::value 
		? COMPV_UNITTEST_MULAB_FLOAT32 
		: COMPV_UNITTEST_MULAB_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_MULAB_FLOAT64) / sizeof(COMPV_UNITTEST_MULAB_FLOAT64[i]); ++i) {
		if (tests[i].arows == arows && tests[i].acols == acols
			&& tests[i].brows == brows && tests[i].bcols == bcols) {
			test = &tests[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr A, B, R;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, test->arows, test->acols));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&B, test->brows, test->bcols));

	for (size_t i = 0; i < test->arows; ++i) {
		for (size_t j = 0; j < test->acols; ++j) {
			*A->ptr<TYP>(i, j) = static_cast<TYP>((i + j) * (i + 1));
		}
	}
	for (size_t i = 0; i < test->brows; ++i) {
		for (size_t j = 0; j < test->bcols; ++j) {
			*B->ptr<TYP>(i, j) = static_cast<TYP>((i + j) * (i + 1));
		}
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(A, B, &R));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(MulAB) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if LOOP_COUNT == 1
	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(R)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Matrix ops mulAB: MD5 mismatch");
#endif

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE matrix_ops_mulGA()
{
	static const size_t rows = 215; // not SIMD friendly
	static const size_t cols = 215; // not SIMD friendly
	CompVMatPtr A;

	static const struct compv_unittest_mulGA {
		size_t rows;
		size_t cols;
		const char* md5;
		const char* md5_fma;
	}
	COMPV_UNITTEST_MULGA_FLOAT64[] = {
#if COMPV_ARCH_X86
		{ 215, 215, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" },
		{ 19, 21, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" },
		{ 701, 71, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" },
		{ 31, 31, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all cases
		{ 9, 9, "1d28996c99db6fdb058a487ed8a57c45", "32436ce316ff4b10a0becf87da478755" }, // Homography
#elif COMPV_ARCH_ARM
        { 215, 215, "1d28996c99db6fdb058a487ed8a57c45", "1ec77d8622cfe7837e56baeb0ef08a0c" },
        { 19, 21, "1d28996c99db6fdb058a487ed8a57c45", "1ec77d8622cfe7837e56baeb0ef08a0c" },
        { 701, 71, "1d28996c99db6fdb058a487ed8a57c45", "1ec77d8622cfe7837e56baeb0ef08a0c" },
        { 31, 31, "1d28996c99db6fdb058a487ed8a57c45", "1ec77d8622cfe7837e56baeb0ef08a0c" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all cases
        { 9, 9, "1d28996c99db6fdb058a487ed8a57c45", "1ec77d8622cfe7837e56baeb0ef08a0c" }, // Homography
#endif
	},
	COMPV_UNITTEST_MULGA_FLOAT32[] = {
#if COMPV_ARCH_X86
		{ 215, 215, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" },
		{ 19, 21, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" },
		{ 701, 71, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" },
		{ 31, 31, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all cases
		{ 9, 9, "23406cd31825fdbcd022edd8f8e76f96", "23406cd31825fdbcd022edd8f8e76f96" }, // Homography
#elif COMPV_ARCH_ARM
		{ 215, 215, "bbb0549857340991a13b4d8680fa8943", "bbb0549857340991a13b4d8680fa8943" },
		{ 19, 21, "bbb0549857340991a13b4d8680fa8943", "bbb0549857340991a13b4d8680fa8943" },
		{ 701, 71, "bbb0549857340991a13b4d8680fa8943", "bbb0549857340991a13b4d8680fa8943" },
		{ 31, 31, "bbb0549857340991a13b4d8680fa8943", "bbb0549857340991a13b4d8680fa8943" }, // 31 = (16 + 8 + 4 + 2 + 1) -> test all cases
		{ 9, 9, "bbb0549857340991a13b4d8680fa8943", "bbb0549857340991a13b4d8680fa8943" }, // Homography
#endif
	};

	const compv_unittest_mulGA* test = NULL;
	const compv_unittest_mulGA* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_MULGA_FLOAT32
		: COMPV_UNITTEST_MULGA_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_MULGA_FLOAT64) / sizeof(COMPV_UNITTEST_MULGA_FLOAT64[i]); ++i) {
		if (tests[i].rows == rows && tests[i].cols == cols) {
			test = &tests[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, tests->rows, tests->cols));

	for (size_t i = 0; i < tests->rows; ++i) {
		for (size_t j = 0; j < tests->cols; ++j) {
			*A->ptr<TYP>(i, j) = static_cast<TYP>((i + j) * (i + 1) + 0.7 + (100 * ((i & 1) ? -1 : 1)));
		}
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		for (size_t ith = 0; ith < tests->rows; ++ith) {
			for (size_t jth = 0; jth < ith; ++jth) {
				COMPV_CHECK_CODE_RETURN(CompVMatrix::mulGA<TYP>(A, ith, jth, static_cast<TYP>(-0.9855), static_cast<TYP>(0.777774)));
			}
		}
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(mulGA) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if 0
	TYP sum = 0; // should be 2.6090482639325790e+42 for 215 and float64
	for (size_t j = 0; j < A->rows(); ++j) {
		for (size_t i = 0; i < A->rows(); ++i) {
			sum += *A->ptr<const TYP>(j, i);
		}
	}
#endif

#if LOOP_COUNT == 1
	const bool fma = compv_tests_is_fma_enabled() && CompVCpu::isAsmEnabled(); // no FMA3 intrin impl.
	COMPV_CHECK_EXP_RETURN(std::string(fma ? test->md5_fma: test->md5).compare(compv_tests_md5(A)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Matrix ops mulGA: MD5 mismatch");
#endif

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE matrix_ops_isSymetric()
{
	static const size_t matrixSize = 215; // not SIMD friendly, no need for rows an cols as symetric matrix must be square
	CompVMatPtr A, S;
	TYP v, *aptr;

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, matrixSize, matrixSize));

	// build random matrix
	for (size_t j = 0; j < matrixSize; ++j) {
		aptr = A->ptr<TYP>(j);
		for (size_t i = 0; i < matrixSize; ++i) {
			aptr[i] = static_cast<TYP>(rand());
		}
	}

	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(A, &S));
	bool isSymmetric;

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(A, isSymmetric));
		COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "A not symetric");
		COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(S, isSymmetric));
		COMPV_CHECK_EXP_RETURN(!isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "AtA is symetric");
		// change one element in the last row (become last column when transposed - useful to make sure SIMD code will read up to the last column)
		v = *S->ptr<const TYP>((matrixSize - 1), ((matrixSize - 1) >> 1));
		*S->ptr<TYP>((matrixSize - 1), ((matrixSize - 1) >> 1)) = static_cast<TYP>(rand()) + (v / 2) + 3;
		COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(S, isSymmetric));
		COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "S no longer symetric");
		*S->ptr<TYP>((matrixSize - 1), ((matrixSize - 1) >> 1)) = v; // restore value for the next loop
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(isSymetric) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}

