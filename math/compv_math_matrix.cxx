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

// R must be <> A,B
// R = mul(A, B) = mulAB(A, B) = mulABt(A, B*)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulAB(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !A->rows() || !A->cols() || B->rows() != A->cols() || !B->cols() || R == A || R == B, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
#if 0 // TODO(dmi): check speed
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Should use mulABt, mulAB_square, mulAB_3x3, mulAB_2x2, SIMD accelerated....
	size_t i, j, k, a_rows = A->rows(), b_rows = B->rows(), b_cols = B->cols();
	const T *a0;
	T *r0;
	T sum;

	// Create R if not already done
	if (!R || R->rows() != A->rows() || R->cols() != B->cols()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&R, A->rows(), B->cols(), COMPV_SIMD_ALIGNV_DEFAULT));
	}

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
#else
	// AB = AB**= A(B*)* = AC*, with C = B*
	CompVPtrArray(T) C;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::transpose(B, C));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(A, C, R));
	return COMPV_ERROR_CODE_S_OK;
#endif
}

// R must be <> A
// R = mul(A*, A)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulAtA(const CompVPtrArray(T) &A, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || R == A, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// A*A = A*A** = (A*)(A*)* = BB*, with B = A*
	CompVPtrArray(T) B;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::transpose(A, B));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(B, B, R));
	return COMPV_ERROR_CODE_S_OK;
}

// R must be <> A,B
// R = mul(A, B*)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulABt(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !A->rows() || !A->cols() || A->cols() != B->cols() || !B->cols() || R == A || R == B, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Should use mulAB_square, mulAB_3x3, mulAB_2x2, SIMD accelerated....

	size_t i, j, k, a_rows = A->rows(), b_rows = B->rows(), b_cols = B->cols();
	const T *a0, *b0;
	T *r0;
	T sum;

	// Create R if not already done
	if (!R || R->rows() != A->rows() || R->cols() != B->rows()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&R, A->rows(), B->rows(), COMPV_SIMD_ALIGNV_DEFAULT));
	}

	for (i = 0; i < a_rows; ++i) {
		a0 = A->ptr(i);
		r0 = const_cast<T*>(R->ptr(i));
		for (j = 0; j < b_rows; ++j) {
			b0 = B->ptr(j);
			sum = 0;
			for (k = 0; k < b_cols; ++k) {
				sum += a0[k] * b0[k]; // DotProduct
			}
			r0[j] = sum;
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

// A = mul(A, GivensRotMatrix)
// c: cos(theta)
// s: sin(theta)
// This function requires(ith > jth) always the case when dealing with symetric matrixes
// If A is symmetric then, mulAG(c, s) = mulGA(c, -s)
// Otherwise, mulAG(A, c, s) = mulGA(A*, c, -s)
// Not thread-safe
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulAG(CompVPtrArray(T) &A, size_t ith, size_t jth, T c, T s)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || ith <= jth || ith > A->cols() || jth > A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// This function isn't optimized and cannot be multithreaded, you should use mulGA() instead
	// mul(A, G) = R
	// -> mul(G*, A*) = R*
	// If A is symmetric then R is symetric
	// G* = swap(sign(s))
	// -> mul(G*, A) = R = mul(A, G)
	// -> mulGA(c, s-) = mulAG(c, s)	
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD

	// When Givens matrix is multiplied to the right of a matrix then, all rows change
	// -> this function cannot be multi-threaded

	size_t rows_ = A->rows();
	T* a;
	T ai, aj;
	// T Gij = s;
	// T Gjj = c;
	// T Gii = c;
	// T Gji = -s;
	for (size_t row_ = 0; row_ < rows_; ++row_) {
		a = const_cast<T*>(A->ptr(row_));
		ai = a[ith] * c - a[jth] * s;
		aj = a[ith] * s + a[jth] * c;
		a[ith] = ai;
		a[jth] = aj;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// A = mul(GivensRotMatrix * A)
// c: cos(theta)
// s: sin(theta)
// This function requires(ith > jth) always the case when dealing with symetric matrixes
// This function can be used to compute mulGtA. mulGtA = mulGA(A, ith, jth, c, -s)
// Thread-safe
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulGA(CompVPtrArray(T) &A, size_t ith, size_t jth, T c, T s)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || ith <= jth || ith > A->cols() || jth > A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD

	// When Givens matrix is multiplied to the left of a matrix then, only ith and jth rows change
	// -> this function could be multi-threaded

	// T Gij = s;
	// T Gjj = c;
	// T Gii = c;
	// T Gji = -s;

	size_t cols_ = A->cols();
	T* ri_ = const_cast<T*>(A->ptr(ith));
	T* rj_ = const_cast<T*>(A->ptr(jth));
	T ai, aj;

	for (size_t col_ = 0; col_ < cols_; ++col_) {
		ai = ri_[col_] * c + s* rj_[col_];
		aj = ri_[col_] * -s + c* rj_[col_];
		ri_[col_] = ai;
		rj_[col_] = aj;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// R<>A
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::transpose(const CompVPtrArray(T) &A, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || A == R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD

	// Create A if not already done
	if (!R || R->rows() != A->cols() || R->cols() != A->rows()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&R, A->cols(), A->rows(), COMPV_SIMD_ALIGNV_DEFAULT));
	}
	const T* a;
	size_t rows_ = A->rows();
	size_t cols_ = A->cols();
	for (size_t row_ = 0; row_ < rows_; ++row_) {
		a = A->ptr(row_);
		for (size_t col_ = 0; col_ < cols_; ++col_) {
			*const_cast<T*>(R->ptr(col_, row_)) = a[col_];
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::maxAbsOffDiag_symm(const CompVPtrArray(T) &S, size_t *row, size_t *col, T* max)
{
	COMPV_CHECK_EXP_RETURN(!S || S->rows() != S->cols() || !S->rows() || !row || !col || !max, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();  // SIMD

	*row = *col = 0;
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

// Build Givens rotation matrix
// c: cos(theta)
// s: sin(theta)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::givens(CompVPtrArray(T) &G, size_t rows, size_t cols, size_t ith, size_t jth, T c, T s)
{
	// From https://en.wikipedia.org/wiki/Givens_rotation

	// Identity matrix
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::identity(G, rows, cols));

	// Gii = c
	*const_cast<T*>(G->ptr(ith, ith)) = c;
	// Gij = s
	*const_cast<T*>(G->ptr(ith, jth)) = s;
	// Gjj = c
	*const_cast<T*>(G->ptr(jth, jth)) = c;
	// Gji = -s
	*const_cast<T*>(G->ptr(jth, ith)) = -s;

	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::identity(CompVPtrArray(T) &I, size_t rows, size_t cols)
{
	COMPV_CHECK_EXP_RETURN(!rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!I || I->rows() != rows || I->cols() != cols) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&I, rows, cols, COMPV_SIMD_ALIGNV_DEFAULT));
	}
	COMPV_CHECK_CODE_RETURN(I->zero_rows());
	uint8_t* i0_ = (uint8_t*)I->ptr();
	size_t stride_ = I->strideInBytes() + I->elmtInBytes();
	for (size_t row_ = 0; row_ < rows; ++row_) {
		*((T*)i0_) = 1;
		i0_ += stride_;
	}

	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::zero(CompVPtrArray(T) &Z, size_t rows, size_t cols)
{
	COMPV_CHECK_EXP_RETURN(!rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!Z || Z->rows() != rows || Z->cols() != cols) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&Z, rows, cols, COMPV_SIMD_ALIGNV_DEFAULT));
	}
	COMPV_CHECK_CODE_RETURN(Z->zero_rows());
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::copy(CompVPtrArray(T) &A, const CompVPtrArray(T) &B)
{
	COMPV_CHECK_EXP_RETURN(!B || !B->rows() || !B->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!A || A->rows() != B->rows() || A->cols() != B->cols()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&A, B->rows(), B->cols(), COMPV_SIMD_ALIGNV_DEFAULT));
	}
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::unwrap(const_cast<T*>(A->ptr()), B, A->alignV()));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
