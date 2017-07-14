#include "../tests/tests_common.h"

#define TAG_TEST			"UnitTestCalibHomography"
#define NUM_POINTS			(5000 + 15) // +15 to make it SIMD-unfriendly for testing
#define ERR_MAX_F64			8.5209617139980764e-17
#define ERR_MAX_F32			6.80796802e-09
#define SCALEX				5.0
#define SCALEY				5.0
#define ERRORPX				0.0 // percent, within [0.f, 1.f]
#define ERRORPY				0.0 // percent, within [0.f, 1.f]
#define TRANSX				28.5
#define TRANSY				-10.0
#define MSE_F64				9.2316868153284422e-017
#define MSE_F32				0.000657067809 // float is useless because of slow convergence issue and high MSE
#define ANGLE				COMPV_MATH_PI / 4

template <typename T>
static COMPV_ERROR_CODE __math_calib_homography_buildHomographyMatrixEq()
{
	COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: calib homography buildHomographyMatrixEq -> %zu %d ==", sizeof(T), NUM_POINTS);

	COMPV_ALIGN_DEFAULT() T srcX[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() T srcY[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() T dstX[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() T dstY[NUM_POINTS];
	CompVMatPtr M;
	const std::string& expectedMD5 = std::is_same<T, compv_float32_t>::value
		? "de09ded7d0dd1ef55aa09757a4570e0b"
		: "540181662bad9a3d001b8b8969a7cb5f";

	for (signed i = 0; i < NUM_POINTS; ++i) {
		srcX[i] = static_cast<T>(((i & 1) ? i : -i) + 0.5);
		srcY[i] = static_cast<T>((srcX[i] * 0.2) + i + 0.7);
		dstX[i] = static_cast<T>((srcX[i] * 8.2) + i + 0.7);
		dstY[i] = static_cast<T>(((i & 1) ? i : -i) + 8.5);
	}
	
	COMPV_CHECK_CODE_RETURN(CompVMatrix::buildHomographyEqMatrix<T>(&M, &srcX[0], &srcY[0], &dstX[0], &dstY[0], NUM_POINTS));
	
	//COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(M).c_str());

	COMPV_CHECK_EXP_RETURN(expectedMD5.compare(compv_tests_md5(M)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "buildHomographyMatrixEq: MD5 mismatch");

	COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");

	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static COMPV_ERROR_CODE __math_calib_homography(COMPV_MODELEST_TYPE estType)
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	bool colinear = false;

	COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: calib homography -> %zu %d %s ==", sizeof(T), NUM_POINTS, (estType == COMPV_MODELEST_TYPE_RANSAC) ? "ransac" : "none");

	// expected H
	const T H_expected[3][3] = {
		{ static_cast<T>(COMPV_MATH_COS(ANGLE)*SCALEX), static_cast<T>(-COMPV_MATH_SIN(ANGLE)), static_cast<T>(TRANSX) },
		{ static_cast<T>(COMPV_MATH_SIN(ANGLE)), static_cast<T>(COMPV_MATH_COS(ANGLE)*SCALEY), static_cast<T>(TRANSY) },
		{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(1) },
	};
	CompVMatPtr input;
	CompVMatPtr output;
	CompVMatPtr h;
	T *x, *y, *z;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&input, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&output, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&h, 3, 3));

	for (size_t j = 0; j < 3; ++j) {
		*h->ptr<T>(j, 0) = H_expected[j][0];
		*h->ptr<T>(j, 1) = H_expected[j][1];
		*h->ptr<T>(j, 2) = H_expected[j][2];
	}

	// Initialize input
	// These points must not be colinear
	x = input->ptr<T>(0);
	y = input->ptr<T>(1);
	z = input->ptr<T>(2);
	for (signed i = 0; i < NUM_POINTS; ++i) {
		x[i] = static_cast<T>(((i & 1) ? i : -i) + 0.5);
		y[i] = static_cast<T>((x[i] * 0.2) + i + 0.7);
		z[i] = static_cast<T>(1); // required
	}

	// Check if input points are colinear
	COMPV_CHECK_CODE_RETURN(CompVMatrix::isColinear2D(input, colinear));
	COMPV_CHECK_EXP_ASSERT(colinear, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Homography requires no-colinear points");

	// Compute output = H:input
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(h, input, &output)); // output = mul(H, input)
																	// Add error
	x = output->ptr<T>(0);
	y = output->ptr<T>(1);
	z = output->ptr<T>(2);
	for (size_t i = 0; i < NUM_POINTS; ++i) {
		x[i] += static_cast<T>(x[i] * ((i * ERRORPX) / NUM_POINTS));
		y[i] += static_cast<T>(y[i] * ((i * ERRORPY) / NUM_POINTS));
		// z[i] = 1; // required, but already set after mul(H, input)
	}

	h = NULL;
	COMPV_CHECK_CODE_RETURN(CompVHomography<T>::find(&h, input, output, estType)); // NONE to make sure we'll always have the same result (ransac is nondeterministic)

	T expected_mse = std::is_same<T, compv_float32_t>::value
		? static_cast<T>(MSE_F32)
		: static_cast<T>(MSE_F64);
	T expected_err = std::is_same<T, compv_float32_t>::value
		? static_cast<T>(ERR_MAX_F32)
		: static_cast<T>(ERR_MAX_F64);

	// Compute MSE
	T mse = COMPV_MATH_POW(H_expected[0][0] - *h->ptr<T>(0, 0), 2);
	mse += COMPV_MATH_POW(H_expected[0][1] - *h->ptr<T>(0, 1), 2);
	mse += COMPV_MATH_POW(H_expected[0][2] - *h->ptr<T>(0, 2), 2);
	mse += COMPV_MATH_POW(H_expected[1][0] - *h->ptr<T>(1, 0), 2);
	mse += COMPV_MATH_POW(H_expected[1][1] - *h->ptr<T>(1, 1), 2);
	mse += COMPV_MATH_POW(H_expected[1][2] - *h->ptr<T>(1, 2), 2);
	mse += COMPV_MATH_POW(H_expected[2][0] - *h->ptr<T>(2, 0), 2);
	mse += COMPV_MATH_POW(H_expected[2][1] - *h->ptr<T>(2, 1), 2);
	mse += COMPV_MATH_POW(H_expected[2][2] - *h->ptr<T>(2, 2), 2);
	COMPV_DEBUG_INFO_EX(TAG_TEST, "MSE= %e, expected=%e", mse, expected_mse);

	COMPV_DEBUG_INFO_EX(TAG_TEST, "H_expected:\n"
		"{\t%e\t%e\t%e\t}\n"
		"{\t%e\t%e\t%e\t}\n"
		"{\t%e\t%e\t%e\t}",
		H_expected[0][0], H_expected[0][1], H_expected[0][2],
		H_expected[1][0], H_expected[1][1], H_expected[1][2],
		H_expected[2][0], H_expected[2][1], H_expected[2][2]);
	COMPV_DEBUG_INFO_EX(TAG_TEST, "H_computed:\n"
		"{\t%e\t%e\t%e\t}\n"
		"{\t%e\t%e\t%e\t}\n"
		"{\t%e\t%e\t%e\t}",
		*h->ptr<T>(0, 0), *h->ptr<T>(0, 1), *h->ptr<T>(0, 2),
		*h->ptr<T>(1, 0), *h->ptr<T>(1, 1), *h->ptr<T>(1, 2),
		*h->ptr<T>(2, 0), *h->ptr<T>(2, 1), *h->ptr<T>(2, 2));

	COMPV_CHECK_EXP_RETURN(COMPV_MATH_ABS(mse) > expected_err, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homography: mse value too high");

	COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	
	return err_;
}

COMPV_ERROR_CODE unittest_math_calib_homography()
{
	/* == buildHomographyMatrixEq == */
	COMPV_CHECK_CODE_RETURN((__math_calib_homography_buildHomographyMatrixEq<compv_float64_t>()));
	COMPV_CHECK_CODE_RETURN((__math_calib_homography_buildHomographyMatrixEq<compv_float32_t>()));

	/* == Homography matrix (model estiation : ransac) */
	COMPV_CHECK_CODE_RETURN((__math_calib_homography<compv_float64_t>(COMPV_MODELEST_TYPE_RANSAC)));
	//COMPV_CHECK_CODE_RETURN((__math_calib_homography<compv_float32_t>(COMPV_MODELEST_TYPE_RANSAC)));

	/* == Homography matrix (model estiation : none) */
	COMPV_CHECK_CODE_RETURN((__math_calib_homography<compv_float64_t>(COMPV_MODELEST_TYPE_NONE)));
	//COMPV_CHECK_CODE_RETURN((__math_calib_homography<compv_float32_t>(COMPV_MODELEST_TYPE_NONE)));

	return COMPV_ERROR_CODE_S_OK;
}
