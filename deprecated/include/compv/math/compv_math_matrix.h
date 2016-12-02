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
    static COMPV_ERROR_CODE mulAB(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, CompVPtrArray(T) &R);
    static COMPV_ERROR_CODE mulABt(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, CompVPtrArray(T) &R);
    static COMPV_ERROR_CODE mulAtA(const CompVPtrArray(T) &A, CompVPtrArray(T) &R);
    static COMPV_ERROR_CODE mulAG(CompVPtrArray(T) &A, size_t ith, size_t jth, T c, T s);
    static COMPV_ERROR_CODE mulGA(CompVPtrArray(T) &A, size_t ith, size_t jth, T c, T s);
    static COMPV_ERROR_CODE transpose(const CompVPtrArray(T) &A, CompVPtrArray(T) &R);
    static COMPV_ERROR_CODE maxAbsOffDiag_symm(const CompVPtrArray(T) &S, size_t *row, size_t *col, T* max);
    static COMPV_ERROR_CODE eigenS(const CompVPtrArray(T) &S, CompVPtrArray(T) &D, CompVPtrArray(T) &Q, bool sort = true, bool rowVectors = false, bool forceZerosInD = true);
    static COMPV_ERROR_CODE svd(const CompVPtrArray(T) &A, CompVPtrArray(T) &U, CompVPtrArray(T) &D, CompVPtrArray(T) &V, bool sort = true);
    static COMPV_ERROR_CODE pseudoinv(const CompVPtrArray(T) &A, CompVPtrArray(T) &R);
    static COMPV_ERROR_CODE invA3x3(const CompVPtrArray(T) &A3x3, CompVPtrArray(T) &R);
    static COMPV_ERROR_CODE invD(const CompVPtrArray(T) &D, CompVPtrArray(T) &R, bool dIsSortedAndPositive = false);
    static COMPV_ERROR_CODE givens(CompVPtrArray(T) &G, size_t rows, size_t cols, size_t ith, size_t jth, T c, T s);
    static COMPV_ERROR_CODE identity(CompVPtrArray(T) &I, size_t rows, size_t cols);
    static COMPV_ERROR_CODE zero(CompVPtrArray(T) &Z, size_t rows, size_t cols);
    static COMPV_ERROR_CODE resize0(CompVPtrArray(T) &A, size_t rows, size_t cols);
    static COMPV_ERROR_CODE washDiag(const CompVPtrArray(T) &S, CompVPtrArray(T) &R, int alignv = -1);
    static COMPV_ERROR_CODE copy(CompVPtrArray(T) &A, const CompVPtrArray(T) &B);
    static COMPV_ERROR_CODE rank(const CompVPtrArray(T) &A, int &r, bool rowspace = true, size_t maxRows = 0, size_t maxCols = 0);
    static COMPV_ERROR_CODE isSymmetric(const CompVPtrArray(T) &A, bool &symmetric);
    static COMPV_ERROR_CODE isEqual(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, bool &equal);
    static COMPV_ERROR_CODE isColinear(const CompVPtrArray(T) &A, bool &colinear, bool rowspace = false, size_t maxRows = 0, size_t maxCols = 0);
    static COMPV_ERROR_CODE isColinear2D(const CompVPtrArray(T) &A, bool &colinear);
    static COMPV_ERROR_CODE isColinear3D(const CompVPtrArray(T) &A, bool &colinear);
    static COMPV_ERROR_CODE buildHomographyEqMatrix(const T* srcX, const T* srcY, const T* dstX, const T* dstY, CompVPtrArray(T)& M, size_t numPoints);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATH_MATRIX_H_ */
