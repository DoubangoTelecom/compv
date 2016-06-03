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
#include "compv/math/compv_math_matrix.h"

#if !defined(COMPV_MATH_EIGEN_EPSILON)
#	define COMPV_MATH_EIGEN_EPSILON	1.1921e-07 // 1e-5
#endif /* COMPV_MATH_EIGEN_EPSILON */

COMPV_NAMESPACE_BEGIN()

template class CompVEigen<double >;
template class CompVEigen<float >;


// S: an (n x n) symmetric matrix
// D: a (n x n) diagonal matrix containing the eigenvalues
// Q: an (n x n) matrix containing the eigenvectors
// sort: Whether to sort the eigenvalues and eigenvectors (from higher to lower)
template <class T>
COMPV_ERROR_CODE CompVEigen<T>::findSymm(const CompVPtrArray(T) &S, CompVPtrArray(T) &D, CompVPtrArray(T) &Q, bool sort /* = true*/)
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

	// Q = I
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::identity(Q, S->rows(), S->cols()));
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
	
	do {
		CompVEigen<T>::jacobiAngles(D, row, col, &gcos_, &gsin_);
		// Q = QG
		CompVMatrix<T>::mulAG(Q, row, col, gcos_, gsin_); // Not thread-safe
		// D = DG
		CompVMatrix<T>::mulAG(D, row, col, gcos_, gsin_); // Not thread-safe
		// D = G*D = G*AG
		CompVMatrix<T>::mulGA(D, row, col, gcos_, -gsin_);
	} while (++ops < maxops &&  COMPV_ERROR_CODE_IS_OK(err_ = CompVMatrix<T>::maxAbsOffDiag_symm(D, &row, &col, &maxOffDiag)) && maxOffDiag > COMPV_MATH_EIGEN_EPSILON);

	if (ops >= maxops) {
		COMPV_DEBUG_ERROR("ops(%d) >= maxops(%d)", ops, maxops);
	}

	return err_;
}

// Compute cos('c') and sin ('s')
template <class T>
void CompVEigen<T>::jacobiAngles(const CompVPtrArray(T) &S, size_t ith, size_t jth, T *c, T *s)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Do not need to compute cos(x) and sin(x)
#if 1
	// From https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
	T Sii = *S->ptr(ith, ith);
	T Sjj = *S->ptr(jth, jth);
	if (Sii == Sjj) {
		// theta = PI/4
		*c = (T)0.70710678118654757; // :: cos(PI/4)
		*s = (T)0.70710678118654757; // :: cos(PI/4)
	}
	else {
		T theta = (T)0.5 * (T)::atan2(2.0 * *S->ptr(ith, jth), Sjj - Sii);
		*c = (T)::cos(theta);
		*s = (T)::sin(theta);
	}
#elif 0
	// FIXME
	double d = (S[(ith * ARRAY_COLS) + ith] - S[(jth * ARRAY_COLS) + jth]) / (2.0*S[(ith * ARRAY_COLS) + jth]);
	double t = (d >= 0 ? +1 : -1) / (::abs(d) + ::sqrt(d*d + 1));
	*c = 1.0 / ::sqrt(t*t + 1);
	*s = t**c;
#else
	// FIXME: remove
	// FIXME: use this but find where comes the sign error
	double Sij = S[(ith * ARRAY_COLS) + jth];
	if (Sij == 0.0) {
		*c = 1.0;
		*s = 0.0;
	}
	else {
		// rho = (Aii - Ajj) / 2Aij
		double rho = (S[(ith * ARRAY_COLS) + ith] - S[(jth * ARRAY_COLS) + jth]) / (2.0 * Sij);
		double t;
		if (rho >= 0) {
			t = 1.0 / (rho + sqrt(1 + (rho * rho)));
		}
		else {
			t = -1 / (-rho + sqrt(1 + (rho * rho)));
		}
		*c = 1.0 / sqrt(1 + (t * t));
		*s = t **c;
	}
#endif
}

COMPV_NAMESPACE_END()
