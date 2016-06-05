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
#include "compv/math/compv_math_matrix.h"

#if !defined(COMPV_MATH_EIGEN_EPSILON)
#	define COMPV_MATH_EIGEN_EPSILON	1.1921e-07 // 1e-5
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
	// DG = (DG)** = (G*D*)*
	// -> G*DG = G*(G*D*)*
	// D is symmetric -> G*DG = G*(G*D)* = mulGA(transpose(mulGA(D))

	// TODO(dmi): add mulGA9x9, transposeA9x9
	// Homography and Fundamental matrices are 3x3 which means we will be frequently working with 9x9 eigenvectors/eigenvalues matrices (Q* and D)

	// TODO(dmi): Moments, replace vpextrd r32, xmm, 0 with vmod r32, xmm
	
	// Instead of returning Q = QG, return Q*, Q* = G*Q*

	do {
		CompVEigen<T>::jacobiAngles(D, row, col, &gcos_, &gsin_);
		// Qt = G*Qt
		CompVMatrix<T>::mulGA(Qt, row, col, gcos_, -gsin_); // Thread-safe
		// D = DG
		CompVMatrix<T>::mulAG(D, row, col, gcos_, gsin_); //!\\ NOT thread-safe
		// D = G*D = G*DG
		CompVMatrix<T>::mulGA(D, row, col, gcos_, -gsin_); // Thread-safe
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
	// From https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
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

COMPV_NAMESPACE_END()
