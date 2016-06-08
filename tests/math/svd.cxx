#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT		1
#define TYPE			double  // double or float


COMPV_ERROR_CODE TestSVD()
{
	const TYPE array[3][3] = {
		{1, 2, 3},
		{4, 5, 6},
		{7, 8, 9}
	};
	CompVPtrArray(TYPE)m_, U, D, V;
	COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::copy(m_, &array[0][0], 3, 3));

	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::svd(m_, U, D, V, false)); // do not sort

#if 0
	matrixPrint<TYPE>(U, "U");
	matrixPrint<TYPE>(D, "D");
	matrixPrint<TYPE>(V, "V");
#endif

	return COMPV_ERROR_CODE_S_OK;
}