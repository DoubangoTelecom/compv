#include "../tests_common.h"

#define TAG_TEST			"TestInverse"
#define LOOP_COUNT			1
#define TYP					compv_float64_t
#define ERR_MAX_F64			3.8163916471489756e-16
#define ERR_MAX_F32			6.80796802e-09

// http://comnuan.com/cmnn0100f/

COMPV_ERROR_CODE pseudoinv()
{
	CompVMatPtr A, Ai;
	static const size_t rows = 11;
	static const size_t cols = 7;

	static const struct compv_unittest_psi {
		size_t rows;
		size_t cols;
		TYP sum;
	}
	COMPV_UNITTEST_PSI_FLOAT64[] = {
		{ 11, 7, static_cast<TYP>(-1.2316536679435330e-16) }, // non-square
		{ 9, 9, static_cast<TYP>(-6.0715321659188248e-17) },
		{ 3, 3, static_cast<TYP>(9.4702024000525853e-14) },
	},
	COMPV_UNITTEST_PSI_FLOAT32[] = {
		{ 11, 7, static_cast<TYP>(-7.91624188e-08) }, // non-square
		{ 9, 9, static_cast<TYP>(-1.67638063e-08) },
		{ 3, 3, static_cast<TYP>(5.96046448e-08) },
	};

	TYP err_max = std::is_same<TYP, compv_float32_t>::value
		? static_cast<TYP>(ERR_MAX_F32)
		: static_cast<TYP>(ERR_MAX_F64);
	const compv_unittest_psi* test = NULL;
	const compv_unittest_psi* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_PSI_FLOAT32
		: COMPV_UNITTEST_PSI_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_PSI_FLOAT64) / sizeof(COMPV_UNITTEST_PSI_FLOAT64[i]); ++i) {
		if (tests[i].rows == rows && tests[i].cols == cols) {
			test = &tests[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");
	
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, test->rows, test->cols));

	// Build random data (must be deterministic to have same MD5)
	TYP* row;
	for (signed j = 0; j < static_cast<signed>(A->rows()); ++j) {
		row = A->ptr<TYP>(j);
		for (signed i = 0; i < static_cast<signed>(A->cols()); ++i) {
			row[i] = static_cast<TYP>(((i & 1) ? i : -i) + (j * 0.5) + j + 0.7);
		}
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::pseudoinv(A, &Ai));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(pseudoinv) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	TYP sum = 0;
	for (size_t row = 0; row < Ai->rows(); ++row) {
		for (size_t col = 0; col < Ai->cols(); ++col) {
			sum += *Ai->ptr<TYP>(row, col);
		}
	}

	COMPV_CHECK_EXP_RETURN((COMPV_MATH_ABS(sum - test->sum) > err_max), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "pseudoinv: sum error value too high");

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE inv3x3()
{
	CompVMatPtr A, Ai, I;
	uint64_t timeStart, timeEnd;
	TYP sum;

	/* Non Singular */
#if 1
	static const TYP NonSingular[3][3] = { // non-singular
		{ 5, 7, 2 },
		{ 1, 9, 4 },
		{ 2, 6, 3 }
	};
	static const TYP Identity[3][3] = {
		{ static_cast<TYP>(1.0), static_cast<TYP>(0.0), static_cast<TYP>(0.0) },
		{ static_cast<TYP>(0.0), static_cast<TYP>(1.0), static_cast<TYP>(0.0) },
		{ static_cast<TYP>(0.0), static_cast<TYP>(0.0), static_cast<TYP>(1.0) },
	};
	TYP err_max_nonsingular = std::is_same<TYP, compv_float32_t>::value
		? static_cast<TYP>(1.31130219e-06)
		: static_cast<TYP>(3.7747582837255322e-15);

	
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, 3, 3));
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			*A->ptr<TYP>(j, i) = NonSingular[j][i];
		}
	}

	timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::invA3x3(A, &Ai));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(inv3x3 non singular) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	// Compute error
	CompVMatrix::mulAB(A, Ai, &I);
	sum = 0;
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			sum += COMPV_MATH_ABS(Identity[j][i] - *I->ptr<TYP>(j, i));
		}
	}

	COMPV_CHECK_EXP_RETURN((sum > err_max_nonsingular), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "inv3x3_nonsingluar: sum error value too high");

#endif

	/* Singular */
#if 1
	static const TYP Singular[3][3] = { // Singular
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 }
	};
	static const TYP SingularInverseFloat64[3][3] = { // Expected resul: http://comnuan.com/cmnn0100f/
		{ static_cast<TYP>(-0.63888888888888351), static_cast<TYP>(-0.16666666666666527), static_cast<TYP>(0.30555555555555336) },
		{ static_cast<TYP>(-0.055555555555554977), static_cast<TYP>(1.5612511283791264e-16), static_cast<TYP>(0.055555555555555323) },
		{ static_cast<TYP>(0.52777777777777357), static_cast<TYP>(0.16666666666666552), static_cast<TYP>(-0.19444444444444264) }
	};
#if COMPV_OS_IPHONE
    static const TYP SingularInverseFloat64_NEON_FMA[3][3] = { // Expected resul: http://comnuan.com/cmnn0100f/
        { static_cast<TYP>(0.05222288547868309), static_cast<TYP>(0.081424739516563427), static_cast<TYP>(0.11062659355444356) },
        { static_cast<TYP>(-1.4377791042907073), static_cast<TYP>(-0.49618281236646494), static_cast<TYP>(0.44541347955777733) },
        { static_cast<TYP>(1.2188895521453567), static_cast<TYP>(0.41475807284990096), static_cast<TYP>(-0.38937340644555463) }
    };
#else
	static const TYP SingularInverseFloat64_NEON_FMA[3][3] = { // Expected resul: http://comnuan.com/cmnn0100f/
		{ static_cast<TYP>(0.016780410445101677), static_cast<TYP>(0.081424419567815404), static_cast<TYP>(0.075185261194962272) },
		{ static_cast<TYP>(-1.366894154223544), static_cast<TYP>(-0.4961821724689689), static_cast<TYP>(0.51629614427674031) },
		{ static_cast<TYP>(1.1834470771117749), static_cast<TYP>(0.41475775290115291), static_cast<TYP>(-0.42481473880503628) }
	};
#endif
	static const TYP SingularInverseFloat32[3][3] = {
		{ static_cast<TYP>(-0.638891041), static_cast<TYP>(-0.166667357), static_cast<TYP>(0.305556685) },
		{ static_cast<TYP>(-0.0555560775), static_cast<TYP>(-1.52736902e-07), static_cast<TYP>(0.0555558056) },
		{ static_cast<TYP>(0.527779818), static_cast<TYP>(0.166667312), static_cast<TYP>(-0.194445491) }
	};
	const TYP(*inverse)[3][3] = std::is_same<TYP, compv_float32_t>::value
		? &SingularInverseFloat32
		: (compv_tests_is_fma_enabled() && CompVCpu::isEnabled(kCpuFlagARM)) ? &SingularInverseFloat64_NEON_FMA : &SingularInverseFloat64;
	TYP err_max_singular = std::is_same<TYP, compv_float32_t>::value
		? static_cast<TYP>(3.22833657e-05)
		: static_cast<TYP>(3.8163916471489756e-16);

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, 3, 3));
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			*A->ptr<TYP>(j, i) = Singular[j][i];
		}
	}

	timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::invA3x3(A, &Ai));
	}
	timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(inv3x3 singular) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	// Compute error
	sum = 0;
	for (size_t j = 0; j < 3; ++j) {
		for (size_t i = 0; i < 3; ++i) {
			sum += COMPV_MATH_ABS((*inverse)[j][i] - *Ai->ptr<TYP>(j, i));
		}
	}

	COMPV_CHECK_EXP_RETURN((sum > err_max_singular), COMPV_ERROR_CODE_E_UNITTEST_FAILED, "inv3x3_singluar: sum error value too high");

#endif

	return COMPV_ERROR_CODE_S_OK;
}
