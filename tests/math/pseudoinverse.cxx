#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT		1
#define TYPE			double  // double or float
#define NUM_ROWS		200 + 9 // +9 to make it SIMD-unfriendly for testing
#define NUM_COLS		100 + 9 // +9 to make it SIMD-unfriendly for testing
#define MD5_DOUBLE		"9b1d77c362d9b0ec5c98ba19fa7250cb"

COMPV_ERROR_CODE TestPseudoInverse()
{
	CompVPtrArray(TYPE) array, arrayInv;
	TYPE* row;
	uint64_t timeStart, timeEnd;
	
	COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::newObjAligned(&array, NUM_ROWS, NUM_COLS));
	
	// Build random data (must be deterministic to have same MD5)
	for (signed j = 0; j < NUM_ROWS; ++j) {
		row = const_cast<TYPE*>(array->ptr(j));
		for (signed i = 0; i < NUM_COLS; ++i) {
			row[i] = (TYPE)(((i & 1) ? i : -i) + (j * 0.5) + j + 0.7);
		}
	}

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::pseudoinv(array, arrayInv));
	}
	timeEnd = CompVTime::getNowMills();

	const std::string md5 = arrayMD5<TYPE>(arrayInv);
	COMPV_ASSERT(md5 == MD5_DOUBLE);

	COMPV_DEBUG_INFO("Elapsed time (TestPseudoInverse) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}