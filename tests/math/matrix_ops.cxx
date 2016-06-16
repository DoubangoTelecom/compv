#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define TYPE_DOUBLE			0
#define TYPE_FLOAT			1

#define TYPE	TYPE_DOUBLE  // double or float
#define LOOP_COUNT 1

#if TYPE == TYPE_DOUBLE
#	define	TYP double
#else
#	define	TYP float
#endif

COMPV_ERROR_CODE TestIsSymmetric()
{
	uint64_t timeStart, timeEnd;
	TYP array[215][215];

	for (size_t i = 0; i < 215; ++i) {
		for (size_t j = 0; j < 215; ++j) {
			array[i][j] = (TYP)rand();
		}
	}
	CompVPtrArray(TYP) A, S;
	TYP v;

	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(A, &array[0][0], 215, 215));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::mulAtA(A, S));
	bool isSymmetric;

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isSymmetric(A, isSymmetric));
		COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isSymmetric(S, isSymmetric));
		COMPV_CHECK_EXP_RETURN(!isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		// change one element in the last row (become last column when transposed - useful to make sure SIMD code will read up to the last column)
		v = *S->ptr(214, (214 >> 1));
		*const_cast<TYP*>(S->ptr(214, (214 >> 1))) = v + 1;
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isSymmetric(S, isSymmetric));
		COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		*const_cast<TYP*>(S->ptr(214, (214 >> 1))) = v; // restore value for the next loop
	}
	timeEnd = CompVTime::getNowMills();

	COMPV_DEBUG_INFO("Elapsed time (TestIsSymmetric) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE TestMulAB()
{
	uint64_t timeStart, timeEnd;
	TYP array0[215][115];
	TYP array1[115][75];
#if TYPE == TYPE_DOUBLE
#	define MD5_EXPECTED "5e1883dfd24448b64e1b6ae007b91758"
#else
#	define MD5_EXPECTED "7a7967e10f80ab6c7a68feb5d8deb8da"
#endif

	for (size_t i = 0; i < 215; ++i) {
		for (size_t j = 0; j < 115; ++j) {
			array0[i][j] = (TYP) (i + j) * (i + 1);
		}
	}
	for (size_t i = 0; i < 115; ++i) {
		for (size_t j = 0; j < 75; ++j) {
			array1[i][j] = (TYP)(i + j) * (i + 1);
		}
	}
	CompVPtrArray(TYP) A, B, R;

	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(A, &array0[0][0], 215, 115));
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(B, &array1[0][0], 115, 75));

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::mulAB(A, B, R));
	}
	timeEnd = CompVTime::getNowMills();

	COMPV_CHECK_EXP_RETURN(arrayMD5(R) != MD5_EXPECTED, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	COMPV_DEBUG_INFO("Elapsed time (TestMulAB) = [[[ %llu millis ]]]", (timeEnd - timeStart));

#undef MD5_EXPECTED
	return COMPV_ERROR_CODE_S_OK;
}
