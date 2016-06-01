/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/math/compv_math_matrix.h"

COMPV_NAMESPACE_BEGIN()

template class CompVMatrix<int32_t >;
template class CompVMatrix<double >;
template class CompVMatrix<float >;
template class CompVMatrix<uint16_t >;
template class CompVMatrix<int16_t >;
template class CompVMatrix<uint8_t >;

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulAB(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !R || !A->rows() || !A->cols() || B->rows() != A->cols() || !B->cols() || R->rows() != A->rows() || R->cols() != B->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Should use mulABt, mulAB_square, mulAB_3x3, mulAB_2x2, SIMD accelerated....

	size_t i, j, k, a_rows = A->rows(), b_rows = B->rows(), b_cols = B->cols();
	const T *a0;
	T *r0;
	T sum;

	for (i = 0; i < a_rows; ++i) {
		a0 = A->ptr(i);
		r0 = const_cast<T*>(R->ptr(i));
		for (j = 0; j < b_cols; ++j) {
			sum = 0;
			for (k = 0; k < b_rows; ++k) {
				sum += a0[k] * *B->ptr(k, j);
			}
			r0[j] = sum;
		}
	}
	
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::maxAbsOffDiag_symm(const CompVPtrArray(T) &S, int *row, int *col, T* max)
{
	COMPV_CHECK_EXP_RETURN(!S || S->rows() != S->cols() || !S->rows() || !row || !col || !max, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	*row = *col = -1;
	*max = 0;

	T r0_ = 0, r1_;
	int i, j;
	int rows = (int)S->rows();
	const T* S_;
	for (j = 1; j < rows; ++j) {
		S_ = S->ptr(j);
		for (i = 0; i < j; ++i) { // i stops at j because the matrix is symmetric
			if ((r1_ = ::abs(S_[i])) > r0_) {
				r0_ = r1_;
				*row = j;
				*col = i;
			}
		}
	}
	*max = r0_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
