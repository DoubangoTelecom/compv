#include "../tests_common.h"

#define TAG_TEST					"TestMath"

#define TEST_MATRIX_OPS_TRANSPOSE			0
#define TEST_MATRIX_OPS_MUL_AB				0
#define TEST_MATRIX_OPS_MUL_GA				0
#define TEST_MATRIX_OPS_IS_SYMETRIC			0
#define TEST_EIGEN_S						0
#define TEST_SVD							0
#define TEST_PSI							0 // Moore–Penrose pseudoinverse
#define TEST_INV3x3							0 // Fast 3x3 inverse
#define TEST_STATS_MSE_2D_HOMOG				0
#define TEST_STATS_NORMALIZE_HARTLEY		0
#define TEST_STATS_VARIANCE					0
#define TEST_TRF_HOMOG_TO_CART				0 // homogeneousToCartesian2D()
#define TEST_CALIB_HOMOGRAPHY_BUILD_MATRIX	0
#define TEST_CALIB_HOMOGRAPHY				0
#define TEST_CALIB_CAMERA					0
#define TEST_CALIB_UNDIST					0
#define TEST_DISTANCE_HAMMING				0
#define TEST_HISTOGRAM_BUILD				0
#define TEST_HISTOGRAM_EQUALIZ				0
#define TEST_CONVLT							1


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
#if TEST_INV3x3
		extern COMPV_ERROR_CODE inv3x3();
		COMPV_CHECK_CODE_BAIL(err = inv3x3(), TAG_TEST "Math inv3x3 test failed");
#endif

#if TEST_STATS_MSE_2D_HOMOG
		extern COMPV_ERROR_CODE stats_mse2D_homogeneous();
		COMPV_CHECK_CODE_BAIL(err = stats_mse2D_homogeneous(), TAG_TEST "Math stats mse 2D homogeneous test failed");
#endif
#if TEST_STATS_NORMALIZE_HARTLEY
		extern COMPV_ERROR_CODE stats_normalize2D_hartley();
		COMPV_CHECK_CODE_BAIL(err = stats_normalize2D_hartley(), TAG_TEST "Math stats norm 2D hartley test failed");
#endif
#if TEST_STATS_VARIANCE
		extern COMPV_ERROR_CODE stats_variance();
		COMPV_CHECK_CODE_BAIL(err = stats_variance(), TAG_TEST "Math stats variance test failed");
#endif
#if TEST_TRF_HOMOG_TO_CART
		extern COMPV_ERROR_CODE homogeneousToCartesian2D();
		COMPV_CHECK_CODE_BAIL(err = homogeneousToCartesian2D(), TAG_TEST "Math homogeneous 2 cartesian 2D test failed");
#endif

#if TEST_CALIB_HOMOGRAPHY_BUILD_MATRIX
		extern COMPV_ERROR_CODE buildHomographyMatrixEq();
		COMPV_CHECK_CODE_BAIL(err = buildHomographyMatrixEq(), TAG_TEST "Math buildHomographyMatrixEq test failed");
#endif
#if TEST_CALIB_HOMOGRAPHY
		extern COMPV_ERROR_CODE homography();
		COMPV_CHECK_CODE_BAIL(err = homography(), TAG_TEST "Math homography test failed");
#endif
#if TEST_CALIB_CAMERA
		extern COMPV_ERROR_CODE calib_camera();
		COMPV_CHECK_CODE_BAIL(err = calib_camera(), TAG_TEST "Math camera calibration test failed");
#endif
#if TEST_CALIB_UNDIST
		extern COMPV_ERROR_CODE calib_undist();
		COMPV_CHECK_CODE_BAIL(err = calib_undist(), TAG_TEST "Math calibration undist");
#endif

#if TEST_DISTANCE_HAMMING
		extern COMPV_ERROR_CODE hamming();
		COMPV_CHECK_CODE_BAIL(err = hamming(), TAG_TEST "Math hamming distance test failed");
#endif

#if TEST_HISTOGRAM_BUILD
		extern COMPV_ERROR_CODE histogram_build();
		COMPV_CHECK_CODE_BAIL(err = histogram_build(), TAG_TEST "Math histogram build test failed");
#endif
#if TEST_HISTOGRAM_EQUALIZ
		extern COMPV_ERROR_CODE histogram_equaliz();
		COMPV_CHECK_CODE_BAIL(err = histogram_equaliz(), TAG_TEST "Math histogram equaliz test failed");
#endif

#if TEST_CONVLT
		extern COMPV_ERROR_CODE convlt();
		COMPV_CHECK_CODE_BAIL(err = convlt(), TAG_TEST "Math convlt test failed");
#endif

	bail:
		COMPV_CHECK_CODE_ASSERT(err, TAG_TEST "Something went wrong!!");
		COMPV_CHECK_CODE_ASSERT(err = compv_tests_deInit());
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "************* Program ended!!! *************");

	compv_main_return(static_cast<int>(err));
}
