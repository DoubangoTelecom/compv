#include "../tests_common.h"

#define TAG_TEST					"TestMath"

#define TEST_MATRIX_OPS_TRANSPOSE		0
#define TEST_MATRIX_OPS_MUL_AB			0
#define TEST_MATRIX_OPS_MUL_GA			0
#define TEST_MATRIX_OPS_IS_SYMETRIC		0
#define TEST_EIGEN_S					0
#define TEST_SVD						1
#define TEST_PSI						0 // Moore–Penrose pseudoinverse

/* Entry point function */
compv_main()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	{

		COMPV_CHECK_CODE_BAIL(err = compv_tests_init());
#if TEST_MATRIX_OPS_TRANSPOSE
		extern COMPV_ERROR_CODE matrix_ops_transpose();
		COMPV_CHECK_CODE_BAIL(err = matrix_ops_transpose(), TAG_TEST "Math matrix transpose test failed");
#endif
#if TEST_MATRIX_OPS_MUL_AB
		extern COMPV_ERROR_CODE matrix_ops_mulAB();
		COMPV_CHECK_CODE_BAIL(err = matrix_ops_mulAB(), TAG_TEST "Math matrix mulAB test failed");
#endif
#if TEST_MATRIX_OPS_MUL_GA
		extern COMPV_ERROR_CODE matrix_ops_mulGA();
		COMPV_CHECK_CODE_BAIL(err = matrix_ops_mulGA(), TAG_TEST "Math matrix mulGA test failed");
#endif
#if TEST_MATRIX_OPS_IS_SYMETRIC
		extern COMPV_ERROR_CODE matrix_ops_isSymetric();
		COMPV_CHECK_CODE_BAIL(err = matrix_ops_isSymetric(), TAG_TEST "Math matrix isSymetric test failed");
#endif
#if TEST_EIGEN_S
		extern COMPV_ERROR_CODE eigenS();
		COMPV_CHECK_CODE_BAIL(err = eigenS(), TAG_TEST "Math eigenS test failed");
#endif
#if TEST_SVD
		extern COMPV_ERROR_CODE svd();
		COMPV_CHECK_CODE_BAIL(err = svd(), TAG_TEST "Math SVD test failed");
#endif
#if TEST_PSI
		extern COMPV_ERROR_CODE pseudoinv();
		COMPV_CHECK_CODE_BAIL(err = pseudoinv(), TAG_TEST "Math pseudoinv test failed");
#endif

	bail:
		COMPV_CHECK_CODE_ASSERT(err, TAG_TEST "Something went wrong!!");
		COMPV_CHECK_CODE_ASSERT(err = compv_tests_deInit());
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "************* Program ended!!! *************");

	compv_main_return(static_cast<int>(err));
}
