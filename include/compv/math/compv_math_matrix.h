/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_MATRIX_H_)
#define _COMPV_MATH_MATRIX_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVMatrix
{
public:
	static COMPV_ERROR_CODE mulAB(const CompVPtr<CompVArray<T>* >&A, const CompVPtr<CompVArray<T>* >&B, CompVPtr<CompVArray<T>* >&R);
	static COMPV_ERROR_CODE maxAbsOffDiag_symm(const CompVPtr<CompVArray<T>* >&S, int *row, int *col, T* max);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_MATRIX_H_ */
