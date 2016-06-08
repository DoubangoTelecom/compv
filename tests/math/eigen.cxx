#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT		1 // dense matrix -> will take time to compute eigenvalues and eigenvectors
#define NUM_POINTS		200 + 9 // +9 to make it SIMD-unfriendly for testing
#define TYPE			double  // double or float
#define MD5_D			"c4df8cd26fb52a88c4adc42c6c3cfb03"
#define MD5_Q			"6860666cc77735dbcba4fb19125d19f7"

COMPV_ERROR_CODE TestEigen()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	uint64_t timeStart, timeEnd;
	CompVPtrArray(TYPE) array, S, D, Q;
	TYPE *x, *y, *z;

	// Build a dense matrix
	COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::newObjAligned(&array, 3, NUM_POINTS));
	x = const_cast<TYPE*>(array->ptr(0));
	y = const_cast<TYPE*>(array->ptr(1));
	z = const_cast<TYPE*>(array->ptr(2));
	for (signed i = 0; i < NUM_POINTS; ++i) {
		x[i] = (TYPE)((i & 1) ? i : -i) + 0.5;
		y[i] = ((TYPE)(x[i] * 0.2)) + i + 0.7;
		z[i] = i*i;
	}

	// Symmetric(S) = A*A
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::mulAtA(array, S));

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		// D (diagonal) contains the eigenvalues
		// Q (square) contain eigenvectors (each row is an eigenvector)
		COMPV_CHECK_CODE_RETURN(CompVEigen<TYPE>::findSymm(S, D, Q, false, true));
	}
	timeEnd = CompVTime::getNowMills();

	const std::string md5D = arrayMD5<TYPE>(D);
	const std::string md5Q = arrayMD5<TYPE>(Q);
	COMPV_ASSERT(md5D == MD5_D);
	COMPV_ASSERT(md5Q == MD5_Q);

	COMPV_DEBUG_INFO("Elapsed time (TestHomography) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return err_;
}