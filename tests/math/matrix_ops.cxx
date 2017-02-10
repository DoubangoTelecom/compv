#include "../tests_common.h"

#define TAG_TEST			"TestMatrixOps"
#define NUM_POINTS			200 + 15 // +15 to make it SIMD-unfriendly for testing
#define LOOP_COUNT			1
#define TYP					compv_float32_t

COMPV_ERROR_CODE matrix_ops_transpose()
{
	CompVMatPtr A, R;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, NUM_POINTS, (NUM_POINTS * 2) + 1));
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
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(A, &R));
	}
#if LOOP_COUNT == 1
	for (row = 0; row < arows; ++row) {
		for (col = 0; col < acols; ++col) {
			COMPV_CHECK_EXP_RETURN(*A->ptr<TYP>(row, col) != *R->ptr<TYP>(col, row), COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		}
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}
