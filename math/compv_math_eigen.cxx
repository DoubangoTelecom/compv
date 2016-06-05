/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
/*
Functions to compute Eigenvalues and Eigenvectors
*/
#include "compv/math/compv_math_eigen.h"
#include "compv/math/compv_math.h"
#include "compv/math/compv_math_utils.h"
#include "compv/math/compv_math_matrix.h"

#if !defined(COMPV_MATH_EIGEN_EPSILON)
#	define COMPV_MATH_EIGEN_EPSILON		FLT_EPSILON
#endif /* COMPV_MATH_EIGEN_EPSILON */

COMPV_NAMESPACE_BEGIN()

template class CompVEigen<int32_t >;
template class CompVEigen<double >;
template class CompVEigen<float >;
template class CompVEigen<uint16_t >;
template class CompVEigen<int16_t >;
template class CompVEigen<uint8_t >;

// S: an (n x n) symmetric matrix
// D: a (n x n) diagonal matrix containing the eigenvalues
// Qt: an (n x n) matrix containing the eigenvectors (rows)
// sort: Whether to sort the eigenvalues and eigenvectors (from higher to lower)
template <class T>
COMPV_ERROR_CODE CompVEigen<T>::findSymm(const CompVPtrArray(T) &S, CompVPtrArray(T) &D, CompVPtrArray(T) &Qt, bool sort /* = true*/, bool forceZerosInD /*= false*/)
{
	COMPV_CHECK_EXP_RETURN(!S || !S->rows() || S->rows() != S->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	
	// sorting not implemented yet
	COMPV_CHECK_EXP_RETURN(sort, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD and MT

	size_t row, col;
	T gcos_, gsin_;
	bool is_diag = false;
	size_t ops = 0, maxops = S->rows() * S->cols() * 30;
	T maxOffDiag;

	// Qt = I
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::identity(Qt, S->rows(), S->cols()));
	// D = S
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::copy(D, S));
	// Check is S is already diagonal or not
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::maxAbsOffDiag_symm(S, &row, &col, &maxOffDiag));
	if (maxOffDiag < COMPV_MATH_EIGEN_EPSILON) { // S already diagonal -> D = S, Q = I
		COMPV_DEBUG_INFO("Symmetric matrix already diagonal -> do nothing");
		return COMPV_ERROR_CODE_S_OK;
	}

	// If matrix A is symmetric then, mulAG(c, s) = mulGA(c, -s)

	// TODO(dmi): For multithreading, change 'maxAbsOffDiag_symm' to add max rows and use it as guard

	// TODO(dmi): Add JacobiAngles_Left() function to be used in mulGA() only

	// TODO(dmi): Change D = G*DG
	// D = G*DG = G*(G*D*)*

	// TODO(dmi): add mulGA9x9, transposeA9x9
	// Homography and Fundamental matrices are 3x3 which means we will be frequently working with 9x9 eigenvectors/eigenvalues matrices (Q* and D)

	// TODO(dmi): Moments, replace vpextrd r32, xmm, 0 with vmod r32, xmm
	
	// Instead of returning Q = QG, return Q*, Q* = G*Q*

	CompVPtrArray(T) GD_2rows;
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&GD_2rows, 2, D->rows()));
	do {
		CompVEigen<T>::jacobiAngles(D, row, col, &gcos_, &gsin_);
		// Qt = G*Qt
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulGA(Qt, row, col, gcos_, -gsin_)); // Thread-safe		
		// G*D*
		CompVEigen<T>::extract2Cols(D, row, col, GD_2rows); // GD_2rows = D*
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulGA(GD_2rows, 0, 1, gcos_, -gsin_)); // Thread-safe		
		// G*(G*D*)*
		CompVEigen<T>::insert2Cols(GD_2rows, D, row, col); // GD_2rows = (G*D*)*
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulGA(D, row, col, gcos_, -gsin_)); // Thread-safe
	} while (++ops < maxops &&  COMPV_ERROR_CODE_IS_OK(err_ = CompVMatrix<T>::maxAbsOffDiag_symm(D, &row, &col, &maxOffDiag)) && maxOffDiag > COMPV_MATH_EIGEN_EPSILON);

	// Off-diagonal values in D contains epsilons which is close to zero but not equal to zero
	if (forceZerosInD) {
		T* d;
		for (row = 0; row < D->rows(); ++row) {
			d = const_cast<T*>(D->ptr(row));
			for (col = 0; col < row; ++col) {
				d[col] = 0;
			}
			for (col = row + 1; col < D->cols(); ++col) {
				d[col] = 0;
			}
		}
	}

	if (ops >= maxops) {
		COMPV_DEBUG_ERROR("ops(%d) >= maxops(%d)", ops, maxops);
	}

	return err_;
}

template <class T>
T CompVEigen<T>::epsilon()
{
	return (T)COMPV_MATH_EIGEN_EPSILON;
}

template <class T>
bool CompVEigen<T>::isCloseToZero(T a)
{
	return (COMPV_MATH_ABS(a) <= CompVEigen<T>::epsilon());
}

// Compute cos('c') and sin ('s')
template <class T>
void CompVEigen<T>::jacobiAngles(const CompVPtrArray(T) &S, size_t ith, size_t jth, T *c, T *s)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Do not need to compute cos(x) and sin(x), use jacobiAngles_Left instead
	// From https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm#Description
	T Sii = *S->ptr(ith, ith);
	T Sjj = *S->ptr(jth, jth);
	if (Sii == Sjj) {
		// theta = PI/4
		*c = (T)0.70710678118654757; // :: cos(PI/4)
		*s = (T)0.70710678118654757; // :: cos(PI/4)
	}
	else {
		T theta = (T)0.5 * (T)COMPV_MATH_ATAN2(2.0 * *S->ptr(ith, jth), Sjj - Sii);
		*c = (T)COMPV_MATH_COS(theta);
		*s = (T)COMPV_MATH_SIN(theta);
	}
}

// Compute cos('c') and sin ('s')
// c and s can only be used for left-multiply (mulGA). Cannot be used for right multiply (mulAG)
// To be used for QR decomposion (https://en.wikipedia.org/wiki/QR_decomposition)
template <class T>
void CompVEigen<T>::jacobiAngles_Left(const CompVPtrArray(T) &S, size_t ith, size_t jth, T *c, T *s)
{
	COMPV_DEBUG_INFO_CODE_NOT_TESTED();
	// From https://en.wikipedia.org/wiki/Givens_rotation#Stable_calculation
	T a = *S->ptr(ith);
	T b = *S->ptr(jth);
	T r = CompVMathUtils::hypot(a, b);
	*c = a / r;
	*s = -b / r;
}

// Extract 2 cols from A and insert as rows to R
template <class T>
void CompVEigen<T>::extract2Cols(const CompVPtrArray(T) &A, size_t a_col0, size_t a_col1, CompVPtrArray(T) &R)
{
	// Private function -> do not check input parameters
	T* r0 = const_cast<T*>(R->ptr(0));
	T* r1 = const_cast<T*>(R->ptr(1));
	const uint8_t* a0 = reinterpret_cast<const uint8_t*>(A->ptr(0, a_col0));
	const uint8_t* a1 = reinterpret_cast<const uint8_t*>(A->ptr(0, a_col1));
	size_t astride = A->strideInBytes();
	size_t rows_ = A->rows();
	for (size_t row_ = 0, aidx = 0; row_ < rows_; ++row_, aidx += astride) {
		r0[row_] = *reinterpret_cast<const T*>(&a0[aidx]);
		r1[row_] = *reinterpret_cast<const T*>(&a1[aidx]);
	}
}

template <class T>
void CompVEigen<T>::insert2Cols(const CompVPtrArray(T) &A, CompVPtrArray(T) &R, size_t r_col0, size_t r_col1)
{
	// Private function -> do not check input parameters
	const T* a0 = A->ptr(0);
	const T* a1 = A->ptr(1);
	uint8_t* r0 = reinterpret_cast<uint8_t*>(const_cast<T*>(R->ptr(0, r_col0)));
	uint8_t* r1 = reinterpret_cast<uint8_t*>(const_cast<T*>(R->ptr(0, r_col1)));
	size_t rstride = R->strideInBytes();
	size_t rows_ = R->rows();
	for (size_t row_ = 0, ridx = 0; row_ < rows_; ++row_, ridx += rstride) {
		*reinterpret_cast<T*>(&r0[ridx]) = a0[row_];
		*reinterpret_cast<T*>(&r1[ridx]) = a1[row_];
	}
}

COMPV_NAMESPACE_END()
