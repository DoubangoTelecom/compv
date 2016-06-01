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

COMPV_NAMESPACE_BEGIN()

template <typename T>
static void JacobiAngles(const CompVPtrArray(T) &S, int ith, int jth, T *c, T *s);

// S: an (n x n) symmetric matrix
// D: a (n x n) diagonal matrix containing the eigenvalues
// V: an (n x n) matrix containing the eigenvectors
// sort: Whether to sort the eigenvalues and eigenvectors (from higher to lower)
template <class T>
COMPV_ERROR_CODE CompVEigen<T>::findSymm(const CompVPtrArray(T) &S, CompVPtrArray(T) &D, CompVPtrArray(T) &V, bool sort /* = true*/)
{
	COMPV_CHECK_EXP_RETURN(!S || !S->rows() || S->rows() != S->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	return err_;
}

// Compute cos('c') and sin ('s')
template <typename T>
static void JacobiAngles(const CompVPtrArray(T) &S, int ith, int jth, T *c, T *s)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Do not need to compute cos(x) and sin(x)
#if 1
	// From https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
	T theta;
	T Sii = *S->ptr(ith, ith);
	T Sjj = *S->ptr(jth, jth);
	if (Sii == Sjj) {
		theta = COMPV_MATH_PI / 4;
	}
	else {
		theta = 0.5 * ::atan2(2.0 * *S->ptr(ith, jth), Sjj - Sii);
	}
	*c = ::cos(theta);
	*s = ::sin(theta);
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
