#include "../tests_common.h"

#define TAG_TEST			"TestSVD"
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define ERR_MAX_F64			8.5209617139980764e-13
#define ERR_MAX_F32			6.80796802e-07

// http://comnuan.com/cmnn0100f/

COMPV_ERROR_CODE svd()
{
	static const size_t rows = 11;
	static const size_t cols = 7;

	static const struct compv_unittest_svd {
		size_t rows;
		size_t cols;
		TYP sum_u;
		TYP sum_d;
		TYP sum_v;
	}
	COMPV_UNITTEST_SVD_FLOAT64[] = {
		{ 11, 7, static_cast<TYP>(4.4791792952900176), static_cast<TYP>(99.924535944946882), static_cast<TYP>(1.6270174930046877) }, // non-square
		{ 9, 9, static_cast<TYP>(3.9353354775194358), static_cast<TYP>(95.805939771678538), static_cast<TYP>(1.0299009246769808) },
		{ 3, 3, static_cast<TYP>(2.3177894550095850), static_cast<TYP>(9.2927635378145208), static_cast<TYP>(0.76441007609313960) },
	},
	COMPV_UNITTEST_SVD_FLOAT32[] = {
		{ 11, 7, static_cast<TYP>(4.48011875), static_cast<TYP>(99.9478226), static_cast<TYP>(1.62701797) }, // non-square
		{ 9, 9, static_cast<TYP>(3.93533564), static_cast<TYP>(95.8058014), static_cast<TYP>(1.02989936) },
		{ 3, 3, static_cast<TYP>(2.31778932), static_cast<TYP>(9.29276371), static_cast<TYP>(0.764434934) },
	};

	TYP err_max = std::is_same<TYP, compv_float32_t>::value
		? static_cast<TYP>(ERR_MAX_F32)
		: static_cast<TYP>(ERR_MAX_F64);
	const compv_unittest_svd* test = NULL;
	const compv_unittest_svd* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_SVD_FLOAT32
		: COMPV_UNITTEST_SVD_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_SVD_FLOAT64) / sizeof(COMPV_UNITTEST_SVD_FLOAT64[i]); ++i) {
		if (tests[i].rows == rows && tests[i].cols == cols) {
			test = &tests[i];
			break;
		}
	}	

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr A;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, test->rows, test->cols));

	// Build random data (must be deterministic to have same MD5)
	TYP* row;
	for (signed j = 0; j < static_cast<signed>(A->rows()); ++j) {
		row = A->ptr<TYP>(j);
		for (signed i = 0; i < static_cast<signed>(A->cols()); ++i) {
			row[i] = static_cast<TYP>(((i & 1) ? i : -i) + (j * 0.5) + j + 0.7);
		}
	}

	CompVMatPtr U, D, V;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::svd(A, &U, &D, &V));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(svd) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	TYP sum_u = 0, sum_d = 0, sum_v = 0;
	for (size_t row = 0; row < U->rows(); ++row) {
		for (size_t col = 0; col < U->cols(); ++col) {
			sum_u += *U->ptr<TYP>(row, col);
		}
	}
	for (size_t row = 0; row < D->rows(); ++row) {
		for (size_t col = 0; col < D->cols(); ++col) {
			sum_d += *D->ptr<TYP>(row, col);
		}
	}
	for (size_t row = 0; row < V->rows(); ++row) {
		for (size_t col = 0; col < V->cols(); ++col) {
			sum_v += *V->ptr<TYP>(row, col);
		}
	}

	//TYP err = COMPV_MATH_ABS(sum_v - test->sum_v);

	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum_u - test->sum_u) > err_max), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "svd: sum_u error value too high");
	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum_d - test->sum_d) > err_max), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "svd: sum_d error value too high");
	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum_v - test->sum_v) > err_max), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "svd: sum_v error value too high");

	return COMPV_ERROR_CODE_S_OK;
}