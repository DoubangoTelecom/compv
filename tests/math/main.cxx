#include "../tests_common.h"

#define TAG_TEST					"TestMath"

#define TEST_MATRIX_OPS_TRANSPOSE			0
#define TEST_MATRIX_OPS_MUL_AB				0
#define TEST_MATRIX_OPS_MUL_GA				0
#define TEST_MATRIX_OPS_IS_SYMETRIC			0
#define TEST_MATRIX_OPS_SUBMUL				1
#define TEST_DOT							0
#define TEST_EXP							0
#define TEST_SCALE							0
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
#define TEST_DISTANCE_LINE					0
#define TEST_DISTANCE_PARABOLA				0
#define TEST_HISTOGRAM_BUILD				0
#define TEST_HISTOGRAM_EQUALIZ				0
#define TEST_HISTOGRAM_PROJ					0
#define TEST_CONVLT							0
#define TEST_MORPH							0
#define TEST_ML_SVM_PREDICT					0
#define TEST_ML_SVM_RBF						0
#define TEST_PCA							0
#define TEST_ACTIVATION_FUNCTIONS			0


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
#if TEST_MATRIX_OPS_SUBMUL
		extern COMPV_ERROR_CODE matrix_ops_submul();
		COMPV_CHECK_CODE_BAIL(err = matrix_ops_submul(), TAG_TEST "Math matrix subMul test failed");
#endif
#if TEST_DOT
		extern COMPV_ERROR_CODE dot();
		COMPV_CHECK_CODE_BAIL(err = dot(), TAG_TEST "Math matrix dot test failed");
#endif
#if TEST_EXP
		extern COMPV_ERROR_CODE expo();
		COMPV_CHECK_CODE_BAIL(err = expo(), TAG_TEST "Math matrix exp test failed");
#endif
#if TEST_SCALE
		extern COMPV_ERROR_CODE scale();
		COMPV_CHECK_CODE_BAIL(err = scale(), TAG_TEST "Math matrix scale test failed");
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
		extern COMPV_ERROR_CODE distance_hamming();
		COMPV_CHECK_CODE_BAIL(err = distance_hamming(), TAG_TEST "Math hamming distance test failed");
#endif
#if TEST_DISTANCE_LINE
		extern COMPV_ERROR_CODE distance_line();
		COMPV_CHECK_CODE_BAIL(err = distance_line(), TAG_TEST "Math line distance test failed");
#endif
#if TEST_DISTANCE_PARABOLA
		extern COMPV_ERROR_CODE distance_parabola();
		COMPV_CHECK_CODE_BAIL(err = distance_parabola(), TAG_TEST "Math parabola distance test failed");
#endif
#if TEST_HISTOGRAM_BUILD
		extern COMPV_ERROR_CODE histogram_build();
		COMPV_CHECK_CODE_BAIL(err = histogram_build(), TAG_TEST "Math histogram build test failed");
#endif
#if TEST_HISTOGRAM_EQUALIZ
		extern COMPV_ERROR_CODE histogram_equaliz();
		COMPV_CHECK_CODE_BAIL(err = histogram_equaliz(), TAG_TEST "Math histogram equaliz test failed");
#endif
#if TEST_HISTOGRAM_PROJ
		extern COMPV_ERROR_CODE histogram_proj();
		COMPV_CHECK_CODE_BAIL(err = histogram_proj(), TAG_TEST "Math histogram proj test failed");
#endif

#if TEST_CONVLT
		extern COMPV_ERROR_CODE convlt();
		COMPV_CHECK_CODE_BAIL(err = convlt(), TAG_TEST "Math convlt test failed");
#endif
#if TEST_MORPH
		extern COMPV_ERROR_CODE morph();
		COMPV_CHECK_CODE_BAIL(err = morph(), TAG_TEST "Math morph test failed");
#endif

#if TEST_ML_SVM_PREDICT
		extern COMPV_ERROR_CODE ml_svm_predict();
		COMPV_CHECK_CODE_BAIL(err = ml_svm_predict(), TAG_TEST "Math ML SVM predict test failed");
#endif
#if TEST_ML_SVM_RBF
		extern COMPV_ERROR_CODE ml_svm_rbf();
		COMPV_CHECK_CODE_BAIL(err = ml_svm_rbf(), TAG_TEST "Math ML SVM rbf test failed");
#endif

#if TEST_PCA
		extern COMPV_ERROR_CODE pca();
		COMPV_CHECK_CODE_BAIL(err = pca(), TAG_TEST "Math PCA test failed");
#endif

#if TEST_ACTIVATION_FUNCTIONS
		extern COMPV_ERROR_CODE activation_functions();
		COMPV_CHECK_CODE_BAIL(err = activation_functions(), TAG_TEST "Math Activation test failed");
#endif

	bail:
		COMPV_CHECK_CODE_ASSERT(err, TAG_TEST "Something went wrong!!");
		COMPV_CHECK_CODE_ASSERT(err = compv_tests_deInit());
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "************* Program ended!!! *************");

	compv_main_return(static_cast<int>(err));
}
