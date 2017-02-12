#include "../tests_common.h"

#define TAG_TEST			"TestEigen"
#define LOOP_COUNT			1
#define TYP					compv_float64_t

COMPV_ERROR_CODE eigenS()
{
	static const size_t numpoints = 11;

	static const struct compv_unittest_eigen {
		size_t numpoints;
		const char* md5_d;
		const char* md5_q;
		const char* md5_d_fma;
		const char* md5_q_fma;
	}
	COMPV_UNITTEST_EIGEN_FLOAT64[] = {
#if COMPV_ARCH_X64
		{ 209, "0c191d633cdf3f26b21c8badfd2ba3d4", "84722db880903f6a5fbfcfaf26e53998" },
		{ 9, "cbadb9f8c6be8aa7abf7172423f7d5bd", "473597e9b6f33c6308893f68e715a674" }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, "56383f623d7300595a680ca4318e5f0c", "a6091c9c38364e35d6c084beecce5508" },
#elif COMPV_ARCH_X86
		{ 209, "e643f74657501e838bacaeba7287ed0f", "5c23dd9118db5e3e72465d4791984fae" },
		{ 9, "904d259aac76f1fa495f4fbcdc072a07", "b974abccdb70eb65f80d16c66d675919" }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, "13261221fc03a0e7987e07f4c27b3f14", "c4ca65098418e3de9baa8fe55f324f9d" },
#else
		{ 209, "0c191d633cdf3f26b21c8badfd2ba3d4", "84722db880903f6a5fbfcfaf26e53998" },
		{ 9, "cbadb9f8c6be8aa7abf7172423f7d5bd", "473597e9b6f33c6308893f68e715a674" }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, "", "" },
#endif
	},
	COMPV_UNITTEST_EIGEN_FLOAT32[] = {
#if COMPV_ARCH_X64
		{ 209, "3434888c193281e5985902003beaf481", "2631c71d337040e32ba2c1b91d33117d" },
		{ 9, "3e1b312669b2c8806eb04ddc00578d4c", "476c0cc46432c9adeaa853b490440776" }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, "c5e9094307ba0f80adb83dc3af7e9155", "63ce637f8e41130faf33d3be5d00c738" },
#elif COMPV_ARCH_X86
		{ 209, "3434888c193281e5985902003beaf481", "0d82444216886adc07b30eb980a43ae3" },
		{ 9, "3e1b312669b2c8806eb04ddc00578d4c", "476c0cc46432c9adeaa853b490440776" }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, "c5e9094307ba0f80adb83dc3af7e9155", "63ce637f8e41130faf33d3be5d00c738" },
#else
		{ 209, "3434888c193281e5985902003beaf481", "2631c71d337040e32ba2c1b91d33117d" },
		{ 9, "3e1b312669b2c8806eb04ddc00578d4c", "476c0cc46432c9adeaa853b490440776" }, // 9 = fast eigen = homography (3x3) and fundamental matrix (3x3)
		{ 11, "", "" },
#endif
	};

	const compv_unittest_eigen* test = NULL;
	const compv_unittest_eigen* tests = std::is_same<TYP, compv_float32_t>::value
		? COMPV_UNITTEST_EIGEN_FLOAT32
		: COMPV_UNITTEST_EIGEN_FLOAT64;

	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_EIGEN_FLOAT64) / sizeof(COMPV_UNITTEST_EIGEN_FLOAT64[i]); ++i) {
		if (tests[i].numpoints == numpoints) {
			test = &tests[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr A;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<TYP>(&A, 3, test->numpoints));
	TYP* x = A->ptr<TYP>(0);
	TYP* y = A->ptr<TYP>(1);
	TYP* z = A->ptr<TYP>(2);
	if (std::is_same<TYP, compv_float64_t>::value) {
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			// Dense matrix
			x[i] = static_cast<TYP>(((i & 1) ? i : -i) + 0.5); // use "(TYP)((i & 1) ? i : (-i * 0.7)) + 0.5" instead. Otherwise i sign alterns with same values -> cancel when added
			y[i] = static_cast<TYP>(((i * 0.2)) + i + 0.7);
			z[i] = static_cast<TYP>(i*i);
		}
	}
	else {
		for (signed i = 0; i < static_cast<signed>(test->numpoints); ++i) {
			// Sparse matrix (float is really bad for fast convergence)
			x[i] = static_cast<TYP>((i & 1) ? 0 : -1) + static_cast<TYP>(0.0001);
			y[i] = static_cast<TYP>(i * 0.2) + static_cast<TYP>(1) + static_cast<TYP>(0.0002);
			z[i] = static_cast<TYP>(0);
		}
	}
	
	// Symmetric(S) = A*A
	CompVMatPtr S;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAtA(A, &S));

	bool symmetric;
	COMPV_CHECK_CODE_RETURN(CompVMatrix::isSymmetric(S, symmetric));
	COMPV_CHECK_EXP_RETURN(!symmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Expected symetric matrix");

	CompVMatPtr D, Q;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		// D (diagonal) contains the eigenvalues
		// Q (square) contain eigenvectors (each col is an eigenvector)
		COMPV_CHECK_CODE_RETURN(CompVMatrix::eigenS(S, &D, &Q));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(eigenS) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO("D: %s", compv_tests_md5(D).c_str());
	//COMPV_DEBUG_INFO("Q: %s", compv_tests_md5(Q).c_str());

#if LOOP_COUNT == 1
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_d).compare(compv_tests_md5(D)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Eigen: MD5(D) mismatch");
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_q).compare(compv_tests_md5(Q)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Eigen: MD5(Q) mismatch");
#endif

	return COMPV_ERROR_CODE_S_OK;
}