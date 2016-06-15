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
	TYP array[209][209];

	for (size_t i = 0; i < 209; ++i) {
		for (size_t j = 0; j < 209; ++j) {
			array[i][j] = (TYP)rand();
		}
	}
	CompVPtrArray(TYP) A, S;
	TYP v;

	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(A, &array[0][0], 209, 209));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::mulAtA(A, S));
	bool isSymmetric;

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isSymmetric(A, isSymmetric));
		COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isSymmetric(S, isSymmetric));
		COMPV_CHECK_EXP_RETURN(!isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		// change one element in the last row (become last column when transposed - useful to make sure SIMD code will read up to the last column)
		v = *S->ptr(208, (208 >> 1));
		*const_cast<TYP*>(S->ptr(208, (208 >> 1))) = v + 1;
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isSymmetric(S, isSymmetric));
		COMPV_CHECK_EXP_RETURN(isSymmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
		*const_cast<TYP*>(S->ptr(208, (208 >> 1))) = v; // restore value for the next loop
	}
	timeEnd = CompVTime::getNowMills();

	COMPV_DEBUG_INFO("Elapsed time (TestIsSymmetric) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}