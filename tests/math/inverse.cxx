#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT		1
#define TYP				double  // double or float
#define NUM_ROWS		200 + 9 // +9 to make it SIMD-unfriendly for testing
#define NUM_COLS		100 + 9 // +9 to make it SIMD-unfriendly for testing

COMPV_ERROR_CODE TestPseudoInverse()
{
#define MD5_DOUBLE		"9b1d77c362d9b0ec5c98ba19fa7250cb"
	CompVPtrArray(TYP) array, arrayInv;
	TYP* row;
	uint64_t timeStart, timeEnd;
	
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::newObjAligned(&array, NUM_ROWS, NUM_COLS));
	
	// Build random data (must be deterministic to have same MD5)
	for (signed j = 0; j < NUM_ROWS; ++j) {
		row = const_cast<TYP*>(array->ptr(j));
		for (signed i = 0; i < NUM_COLS; ++i) {
			row[i] = (TYP)(((i & 1) ? i : -i) + (j * 0.5) + j + 0.7);
		}
	}

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::pseudoinv(array, arrayInv));
	}
	timeEnd = CompVTime::getNowMills();

	const std::string md5 = arrayMD5<TYP>(arrayInv);
	COMPV_CHECK_EXP_RETURN(md5 != MD5_DOUBLE, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	COMPV_DEBUG_INFO("Elapsed time (TestPseudoInverse) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
#undef MD5_DOUBLE
}

COMPV_ERROR_CODE TestInverse3x3()
{
	uint64_t timeStart, timeEnd;
#define MD5A_DOUBLE		"5a0e43d0961d5b714aa11e835c555971" // non-singular
#define MD5B_DOUBLE		"25b38f92f51f06a2ac4e60f767583f69" // singluar
	static const TYP A[3][3] = { // non-singular
		{ 5, 7, 2 },
		{ 1, 9, 4 },
		{ 2, 6, 3 }
	};
	CompVPtrArray(TYP) A3x3, A3x3inv;
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(A3x3, &A[0][0], 3, 3));

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::invA3x3(A3x3, A3x3inv));
	}
	timeEnd = CompVTime::getNowMills();

	COMPV_DEBUG_INFO("Elapsed time (TestInverse3x3) = [[[ %llu millis ]]]", (timeEnd - timeStart));
	
	std::string md5 = arrayMD5<TYP>(A3x3inv);
	COMPV_CHECK_EXP_RETURN(md5 != MD5A_DOUBLE, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	// Check singular matrix
	static const TYP B[3][3] = {
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 }
	};
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(A3x3, &B[0][0], 3, 3));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::invA3x3(A3x3, A3x3inv)); // pseudo-inverse
	md5 = arrayMD5<TYP>(A3x3inv);
	COMPV_CHECK_EXP_RETURN(md5 != MD5B_DOUBLE, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	

	return COMPV_ERROR_CODE_S_OK;
#undef MD5A_DOUBLE
#undef MD5B_DOUBLE
}
