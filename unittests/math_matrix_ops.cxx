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
		COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(A, &R));
		for (row = 0; row < arows; ++row) {
			for (col = 0; col < acols; ++col) {
				COMPV_CHECK_EXP_RETURN(*A->ptr<T>(row, col) != *R->ptr<T>(col, row), COMPV_ERROR_CODE_E_UNITTEST_FAILED);
			}
		}
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
	

	return COMPV_ERROR_CODE_S_OK;
}