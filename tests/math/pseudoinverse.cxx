#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT		1
#define TYPE			double  // double or float



COMPV_ERROR_CODE TestPseudoInverse()
{
	const TYPE array[3][3] = {
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 }
	};
	CompVPtrArray(TYPE)m_, Inv;
	COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::copy(m_, &array[0][0], 3, 3));

	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::pseudoinverse(m_, Inv)); // do not sort

	matrixPrint<TYPE>(Inv, "Inverse");

	return COMPV_ERROR_CODE_S_OK;
}