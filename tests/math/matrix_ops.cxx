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

static COMPV_ERROR_CODE _TestMulAB(size_t aRows, size_t aCols, size_t bRows, size_t bCols, CompVPtrArray(TYP)& R, const char* name)
{
	uint64_t timeStart, timeEnd;
	CompVPtrArray(TYP) A, B;

	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::newObjAligned(&A, aRows, aCols));
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::newObjAligned(&B, bRows, bCols));

	for (size_t i = 0; i < aRows; ++i) {
		for (size_t j = 0; j < aCols; ++j) {
			*const_cast<TYP*>(A->ptr(i, j)) = (TYP)(i + j) * (i + 1);
		}
	}
	for (size_t i = 0; i < bRows; ++i) {
		for (size_t j = 0; j < bCols; ++j) {
			*const_cast<TYP*>(B->ptr(i, j)) = (TYP)(i + j) * (i + 1);
		}
	}	

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::mulAB(A, B, R));
	}
	timeEnd = CompVTime::getNowMills();

	COMPV_DEBUG_INFO("Elapsed time (TestMulAB[%s]) = [[[ %llu millis ]]]", name, (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE TestMulAB()
{
#if TYPE == TYPE_DOUBLE
#	define MD5_EXPECTED		"5e1883dfd24448b64e1b6ae007b91758"
#	define MD5_EXPECTED3x3	"de628e91457c329220badfb524016b99"
#	define MD5_EXPECTED4x4	"5b2f3f02883ef6f944d90373be188104"
#else
#	define MD5_EXPECTED		"7a7967e10f80ab6c7a68feb5d8deb8da"
#	define MD5_EXPECTED3x3	"db5d1d779c11164e72964f6f69289c77"
#	define MD5_EXPECTED4x4	"fccbc6a3eea330fd07b9325a13f462f2"
#endif
	CompVPtrArray(TYP) R;
	
	// Random size
	//COMPV_CHECK_CODE_RETURN(_TestMulAB(215, 115, 115, 75, R, "nxn"));
	//COMPV_CHECK_EXP_RETURN(arrayMD5(R) != MD5_EXPECTED, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	// 3x3
	COMPV_CHECK_CODE_RETURN(_TestMulAB(3, 3, 3, 3, R, "3x3"));
	COMPV_CHECK_EXP_RETURN(arrayMD5(R) != MD5_EXPECTED3x3, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	// 4x4
	//COMPV_CHECK_CODE_RETURN(_TestMulAB(4, 4, 4, 4, R, "4x4"));
	//COMPV_CHECK_EXP_RETURN(arrayMD5(R) != MD5_EXPECTED4x4, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	return COMPV_ERROR_CODE_S_OK;
#undef MD5_EXPECTED
#undef MD5_EXPECTED3x3
#undef MD5_EXPECTED4x4
}

