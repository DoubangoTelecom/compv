#include "../tests_common.h"

#define TAG_TEST			"TestSVD"
#define LOOP_COUNT			1
#define TYP					compv_float64_t

COMPV_ERROR_CODE svd()
{
	static const size_t rows = 11;
	static const size_t cols = 7;

	static const struct compv_unittest_svd {
		size_t rows;
		size_t cols;
		const char* md5_u;
		const char* md5_d;
		const char* md5_v;
		const char* md5_u_fma;
		const char* md5_d_fma;
		const char* md5_v_fma;
	}
	COMPV_UNITTEST_SVD_FLOAT64[] = {
#if COMPV_ARCH_X64
		{ 11, 7, "072ad6f2927baea082e6d063e0bbe61d", "6d721c4b3f0fa87b2b9aa9798a4cc4c5", "91fc1edfbf9e9178540950fe80cc715c" }, // non-square
		{ 9, 9, "0e3106a72c2ff9a31ab0fe4e498e3edd", "ac7f3584395ff01003d7d9b74c6a9bb9", "c8750c8b494fdc6d39e21bc853ef5f46" },
		{ 3, 3, "811344ecc2ba37beb64ef285ed9915be", "f5a5fa1c6f2846f3923e523c07bebd7a", "bdf50b99776c5e7fadf2081b04a67994" },
#elif COMPV_ARCH_X86
		{ 11, 7, "f910df5e7df1a5cbf49bfb6d8b831d10", "25b48d65b29b0381ebea769c9710f138", "88a80cd4df88bd80cdf697e390c34340" }, // non-square
		{ 9, 9, "6694876940270d7e23893b687ee88077", "ac7f3584395ff01003d7d9b74c6a9bb9", "81cbfc5b983c4b9a462f2fab406fc1d5" },
		{ 3, 3, "811344ecc2ba37beb64ef285ed9915be", "f5a5fa1c6f2846f3923e523c07bebd7a", "bdf50b99776c5e7fadf2081b04a67994" },
#elif COMPV_ARCH_ARM
		{ 11, 7, "", "", "" }, // non-square
		{ 9, 9, "", "", "" },
		{ 3, 3, "", "", "" },
#else
		{ 11, 7, "", "", "" }, // non-square
		{ 9, 9, "", "", "" },
		{ 3, 3, "", "", "" },
#endif
	},
	COMPV_UNITTEST_SVD_FLOAT32[] = {
#if COMPV_ARCH_X64
		{ 11, 7, "603f425d646d7ee6a8ddbdaf7a9fde95", "adfecf1cfdb8e95c5604ad0e3d08bb41", "09c54d5e8b3f08b776027ef03867ed65" }, // non-square
		{ 9, 9, "d2acd6afd76720315c117107df6797f1", "cd3e8abdae68159157d4887d0eba37f0", "8b7961c51e8141c3f5ec08a32972707d" },
		{ 3, 3, "0c3fe161d0f72c37a4c2d6f2e73433a3", "e0d9283db9ce3324c74d597e08b5d5fc", "eaa9fd5f43f1536cc1b920142d4ece5c" },
#elif COMPV_ARCH_X86
		{ 11, 7, "603f425d646d7ee6a8ddbdaf7a9fde95", "adfecf1cfdb8e95c5604ad0e3d08bb41", "09c54d5e8b3f08b776027ef03867ed65" }, // non-square
		{ 9, 9, "d2acd6afd76720315c117107df6797f1", "cd3e8abdae68159157d4887d0eba37f0", "8b7961c51e8141c3f5ec08a32972707d" },
		{ 3, 3, "0c3fe161d0f72c37a4c2d6f2e73433a3", "e0d9283db9ce3324c74d597e08b5d5fc", "eaa9fd5f43f1536cc1b920142d4ece5c" },
#elif COMPV_ARCH_ARM
		{ 11, 7, "", "", "" }, // non-square
		{ 9, 9, "", "", "" },
		{ 3, 3, "", "", "" },
#else
		{ 11, 7, "", "", "" }, // non-square
		{ 9, 9, "", "", "" },
		{ 3, 3, "", "", "" },
#endif
	};

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

	//COMPV_DEBUG_INFO("MD5: %s, %s, %s", compv_tests_md5(U).c_str(), compv_tests_md5(D).c_str(), compv_tests_md5(V).c_str());

#if LOOP_COUNT == 1
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_u).compare(compv_tests_md5(U)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "SVD: MD5(U) mismatch");
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_d).compare(compv_tests_md5(D)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "SVD: MD5(D) mismatch");
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_v).compare(compv_tests_md5(V)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "SVD: MD5(V) mismatch");
#endif

	return COMPV_ERROR_CODE_S_OK;
}