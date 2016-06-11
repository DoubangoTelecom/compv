#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT		1 // dense matrix -> will take time to compute eigenvalues and eigenvectors
#define NUM_POINTS		200 + 9 // +9 to make it SIMD-unfriendly for testing
#define TYPE			double  // double or float
#define MD5_D			"5e58169e3a19845ab01293febf562b24" // 200 + 9 points
#define MD5_Q			"13893b77ea53821b5b7539dacfda07ba" // 200 + 9 points
#define MD5_D9			"f0f6a4a9e49459240a6433578ab6d8ec" // 0 + 9 points
#define MD5_Q9			"c9b6584c289a6e618e2208fe438a7e17" // 0 + 9 points

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


	// FIXME
#if 0
#define TYP float
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();

	TYP mal = ((TYP)34494.203125000000 * (TYP)0.029227419973866218);
	TYP kal = ((TYP)-0.99957278770566338 * (TYP)1008.6075440000000);
	TYP val = ((TYP)34494.203125000000 * (TYP)0.029227419973866218) + ((TYP)-0.99957278770566338 * (TYP)1008.6075440000000);

	const TYP dataF[3][3] = {
		{ (TYP)1008.607544, (TYP)-841.387085, (TYP)34494.203125 },
		{ (TYP)-841.387085, (TYP)701.890686, (TYP)-28775.292969 },
		{ (TYP)34494.203125, (TYP)-28775.292969, (TYP)1179695.875000 },
	};
	CompVPtrArray(TYP) SF, DF, QF;
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(SF, &dataF[0][0], 3, 3));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::eigenS(SF, DF, QF, false, false, false));
	matrixPrint<TYP>(DF, "D");
#endif
	
	// Symmetric(S) = A*A
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::mulAtA(array, S));

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		// D (diagonal) contains the eigenvalues
		// Q (square) contain eigenvectors (each col is an eigenvector)
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::eigenS(S, D, Q));
	}
	timeEnd = CompVTime::getNowMills();

	const std::string md5D = arrayMD5<TYPE>(D);
	const std::string md5Q = arrayMD5<TYPE>(Q);
#if NUM_POINTS == 0 + 9 // homography (3x3)
	COMPV_ASSERT(md5D == MD5_D9);
	COMPV_ASSERT(md5Q == MD5_Q9);
#else
	COMPV_ASSERT(md5D == MD5_D);
	COMPV_ASSERT(md5Q == MD5_Q);
#endif

	COMPV_DEBUG_INFO("Elapsed time (TestEigen) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return err_;
}