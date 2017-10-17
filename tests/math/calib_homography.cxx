#include "../tests_common.h"

#define TAG_TEST			"TestCalibHomography"
#define NUM_POINTS			(5000 + 15) // +15 to make it SIMD-unfriendly for testing
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define SCALEX				5.0
#define SCALEY				5.0
#define ERRORPX				0.0 // percent, within [0.f, 1.f]
#define ERRORPY				0.0 // percent, within [0.f, 1.f]
#define TRANSX				28.5
#define TRANSY				-10.0
#define MSE_F64				9.2316868153284422e-017
#define MSE_F32				0.000657067809 // float is useless because of slow convergence issue and high MSE
#define ERR_MAX_F64			8.5209617139980764e-17
#define ERR_MAX_F32			6.80796802e-09
#define ANGLE				COMPV_MATH_PI / 4
#define MODE_EST			COMPV_MODELEST_TYPE_RANSAC

COMPV_ERROR_CODE buildHomographyMatrixEq()
{
	COMPV_ALIGN_DEFAULT() TYP srcX[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() TYP srcY[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() TYP dstX[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() TYP dstY[NUM_POINTS];
	CompVMatPtr M;
	const std::string& expectedMD5 = std::is_same<TYP, compv_float32_t>::value
		? "de09ded7d0dd1ef55aa09757a4570e0b"
		: "540181662bad9a3d001b8b8969a7cb5f";

	for (signed i = 0; i < NUM_POINTS; ++i) {
		srcX[i] = static_cast<TYP>(((i & 1) ? i : -i) + 0.5);
		srcY[i] = static_cast<TYP>((srcX[i] * 0.2) + i + 0.7);
		dstX[i] = static_cast<TYP>((srcX[i] * 8.2) + i + 0.7);
		dstY[i] = static_cast<TYP>(((i & 1) ? i : -i) + 8.5);
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::buildHomographyEqMatrix<TYP>(&M, &srcX[0], &srcY[0], &dstX[0], &dstY[0], NUM_POINTS));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(buildHomographyMatrixEq) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_EXP_RETURN(expectedMD5.compare(compv_tests_md5(M)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "buildHomographyMatrixEq: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE homography()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	bool colinear = false;

	// expected H
	const TYP H_expected[3][3] = {
		{ static_cast<TYP>(COMPV_MATH_COS(ANGLE)*SCALEX), static_cast<TYP>(-COMPV_MATH_SIN(ANGLE)), static_cast<TYP>(TRANSX) },
		{ static_cast<TYP>(COMPV_MATH_SIN(ANGLE)), static_cast<TYP>(COMPV_MATH_COS(ANGLE)*SCALEY), static_cast<TYP>(TRANSY) },
		{ static_cast<TYP>(0), static_cast<TYP>(0), static_cast<TYP>(1) },
	};
	CompVMatPtr input;
	CompVMatPtr output;
	CompVMatPtr h;
	TYP *x, *y, *z;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&input, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&output, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&h, 3, 3));

	for(size_t j = 0; j < 3; ++j) {
		*h->ptr<TYP>(j, 0) = H_expected[j][0];
		*h->ptr<TYP>(j, 1) = H_expected[j][1];
		*h->ptr<TYP>(j, 2) = H_expected[j][2];
	}

	// Initialize input
	// These points must not be colinear
	x = input->ptr<TYP>(0);
	y = input->ptr<TYP>(1);
	z = input->ptr<TYP>(2);
	for (signed i = 0; i < NUM_POINTS; ++i) {
		x[i] = static_cast<TYP>(((i & 1) ? i : -i) + 0.5);
		y[i] = static_cast<TYP>((x[i] * 0.2) + i + 0.7);
		z[i] = static_cast<TYP>(1); // required
	}

	// Check if input points are colinear
	COMPV_CHECK_CODE_RETURN(CompVMatrix::isColinear2D(input, colinear));
	COMPV_CHECK_EXP_ASSERT(colinear, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Homography requires no-colinear points");

	// Compute output = H:input
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(h, input, &output)); // output = mul(H, input)
	// Add error
	x = output->ptr<TYP>(0);
	y = output->ptr<TYP>(1);
	z = output->ptr<TYP>(2);
	for (size_t i = 0; i < NUM_POINTS; ++i) {
		x[i] += static_cast<TYP>(x[i] * ((i * ERRORPX) / NUM_POINTS));
		y[i] += static_cast<TYP>(y[i] * ((i * ERRORPY) / NUM_POINTS));
		// z[i] = 1; // required, but already set after mul(H, input)
	}

	h = nullptr;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVHomography<TYP>::find(input, output, &h, nullptr, MODE_EST)); // NONE to make sure we'll always have the same result (ransac is nondeterministic)
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(homography) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	TYP expected_mse = std::is_same<TYP, compv_float32_t>::value
		? static_cast<TYP>(MSE_F32)
		: static_cast<TYP>(MSE_F64);

	TYP expected_err = std::is_same<TYP, compv_float32_t>::value
		? static_cast<TYP>(ERR_MAX_F32)
		: static_cast<TYP>(ERR_MAX_F64);

	// Compute MSE
	TYP mse = COMPV_MATH_POW(H_expected[0][0] - *h->ptr<TYP>(0, 0), 2);
	mse += COMPV_MATH_POW(H_expected[0][1] - *h->ptr<TYP>(0, 1), 2);
	mse += COMPV_MATH_POW(H_expected[0][2] - *h->ptr<TYP>(0, 2), 2);
	mse += COMPV_MATH_POW(H_expected[1][0] - *h->ptr<TYP>(1, 0), 2);
	mse += COMPV_MATH_POW(H_expected[1][1] - *h->ptr<TYP>(1, 1), 2);
	mse += COMPV_MATH_POW(H_expected[1][2] - *h->ptr<TYP>(1, 2), 2);
	mse += COMPV_MATH_POW(H_expected[2][0] - *h->ptr<TYP>(2, 0), 2);
	mse += COMPV_MATH_POW(H_expected[2][1] - *h->ptr<TYP>(2, 1), 2);
	mse += COMPV_MATH_POW(H_expected[2][2] - *h->ptr<TYP>(2, 2), 2);
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
		*h->ptr<TYP>(0, 0), *h->ptr<TYP>(0, 1), *h->ptr<TYP>(0, 2),
		*h->ptr<TYP>(1, 0), *h->ptr<TYP>(1, 1), *h->ptr<TYP>(1, 2),
		*h->ptr<TYP>(2, 0), *h->ptr<TYP>(2, 1), *h->ptr<TYP>(2, 2));
			
	COMPV_CHECK_EXP_RETURN(COMPV_MATH_ABS(mse) > expected_err, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "homography: mse value too high");

	// Check MD5: This not accurate as it could change depending on the SIMD type (NEON, FMA, AVX, SSE, MMX...) and CPU (X64, X86, ARM...)
	// We're using it now for regression test for asm porting
#if 0
	const std::string md5 = arrayMD5<TYP>(h);
	if (CompVCpu::isEnabled(compv::kCpuFlagAVX)) {
		COMPV_CHECK_EXP_RETURN(md5 != MD5_EXPECTED_AVX, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}
	else if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
		COMPV_CHECK_EXP_RETURN(md5 != MD5_EXPECTED_SSE2, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}
	else {
		COMPV_CHECK_EXP_RETURN(md5 != MD5_EXPECTED, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}
#endif

	return err_;
}
