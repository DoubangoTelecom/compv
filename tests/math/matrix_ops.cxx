#include "../tests_common.h"

#define TAG_TEST			"TestMatrixOps"
#define LOOP_COUNT			1
#define TYP					compv_float32_t

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
	},
	COMPV_UNITTEST_MULAB_FLOAT32[] = {
		{ 215, 115, 115, 75, "7a7967e10f80ab6c7a68feb5d8deb8da" },
		{ 3, 3, 3, 3, "db5d1d779c11164e72964f6f69289c77" },
		{ 4, 4, 4, 4, "fccbc6a3eea330fd07b9325a13f462f2" },
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
