/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_eigen.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_allocators.h"

#define COMPV_THIS_CLASSNAME	"CompVMathEigen"

#if !defined(COMPV_MATH_EIGEN_MAX_ROUNDS)
#	define COMPV_MATH_EIGEN_MAX_ROUNDS 30 // should be 30
#endif
#define COMPV_MATH_EIGEN_OFFDIAG_MAX_SAMPLES_PER_THREAD		(50 * 50)

COMPV_NAMESPACE_BEGIN()

// S: an (n x n) symmetric matrix
// D: a (n x n) diagonal matrix containing the eigenvalues
// Q: an (n x n) matrix containing the eigenvectors (columns unless transposed)
// rowVectors: true -> eigenvectors are rows, otherwise it's columns. True is faster.
// sort: Whether to sort the eigenvalues and eigenvectors (from higher to lower)
// Algorithm: https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
template <class T>
COMPV_ERROR_CODE CompVMathEigen<T>::findSymm(const CompVMatPtr &S, CompVMatPtrPtr D, CompVMatPtrPtr Q, bool sort COMPV_DEFAULT(true), bool rowVectors COMPV_DEFAULT(false), bool forceZerosInD COMPV_DEFAULT(true))
{
	COMPV_CHECK_EXP_RETURN(!S || !S->isRawTypeMatch<T>() || !D || !Q || !S->rows() || S->rows() != S->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_TODO("Remove T and create CompVMathEigenGeneric like what is done with CompVMathMatrix (type of S discovered at runtime)");

	const size_t dim = S->cols();

#if defined(_DEBUG) || defined(DEBUG)
	// For homography and Fundamental matrices S is 9x9 matrix
	if (dim > 9) {
		// TODO(dmi): For multithreading, change 'maxAbsOffDiag_symm' to add max rows and use it as guard
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation");
	}
	// Eigen values and vectors can be easily computed for 3x3 without using jacobi
	if (dim == 3) {
		// https://github.com/DoubangoTelecom/compv/issues/85
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Eigen values/vectors: No fast implementation for 3x3 matrix");
	}
#endif

	size_t row, col, rowOld, colOld;
	T gcos_, gsin_;
	const T epsilon_ = CompVMathEigen<T>::epsilon();
	size_t ops = 0, maxops = S->rows() * S->cols() * COMPV_MATH_EIGEN_MAX_ROUNDS;
	CompVMatPtr Qt;
	CompVMatPtr GD_2rows;
	T *GD_2rowsPtr0, *GD_2rowsPtr1;
	CompVMatPtr D_ = *D;
	bool transpose = !rowVectors;

	// Qt = I
	if (rowVectors) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::identity<T>(Q, S->rows(), S->cols()));
		Qt = *Q;
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::identity<T>(&Qt, S->rows(), S->cols()));
	}
	// D = S
	COMPV_CHECK_CODE_RETURN(CompVMatrix::copy(&D_, S));
	
	// Compute all max off-diagonal values
	
	struct CompVMaxAbsOffDiagValue {
		size_t index;
		T maxx;
		static COMPV_ERROR_CODE highest(const std::vector<CompVMaxAbsOffDiagValue, CompVAllocatorNoDefaultConstruct<CompVMaxAbsOffDiagValue> >& vec, size_t *row, T* max) {
			const size_t count = vec.size();
			T maxx = vec[1].maxx;
			*row = 1;
			for (size_t i = 2; i < count; ++i) { // #0 part of the diagonal, #1 used to init values
				if (vec[i].maxx > maxx) {
					*row = i;
					maxx = vec[i].maxx;
				}
			}
			*max = maxx;
			return COMPV_ERROR_CODE_S_OK;
		}
	};
	std::vector<CompVMaxAbsOffDiagValue, CompVAllocatorNoDefaultConstruct<CompVMaxAbsOffDiagValue> > maxAbsOffDiagValues(dim);
	CompVMaxAbsOffDiagValue& maxAbsOffDiagValueHighest = maxAbsOffDiagValues[0]; // Zero unused -> hack to store highest value
	auto funcPtrOffDiagMax = [&](const size_t row_start, const size_t row_end) -> COMPV_ERROR_CODE {
		const size_t row_start_ = COMPV_MATH_MAX(row_start, 1); // #0 part of the diagonal
		const T* rowPtr = D_->ptr<const T>(row_start_);
		const size_t stride = D_->stride();
		for (size_t row = row_start_; row < row_end; ++row) {
			CompVMaxAbsOffDiagValue& maxAbsOffDiagValue = maxAbsOffDiagValues[row];
			COMPV_CHECK_CODE_RETURN(CompVMathEigen<T>::maxAbsOffDiagSymm(rowPtr, row, &maxAbsOffDiagValue.index, &maxAbsOffDiagValue.maxx));
			rowPtr += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrOffDiagMax,
		dim,
		dim,
		COMPV_MATH_EIGEN_OFFDIAG_MAX_SAMPLES_PER_THREAD
	));

	// Fill index zero with the highest max val
	COMPV_CHECK_CODE_RETURN(CompVMaxAbsOffDiagValue::highest(maxAbsOffDiagValues, &maxAbsOffDiagValueHighest.index, &maxAbsOffDiagValueHighest.maxx));

	// Check is S is already diagonal or not
	if (maxAbsOffDiagValueHighest.maxx < epsilon_) { // S already diagonal -> D = S, Q = I
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Symmetric matrix already diagonal -> do nothing");
		goto done;
	}
	row = maxAbsOffDiagValueHighest.index;
	col = maxAbsOffDiagValues[maxAbsOffDiagValueHighest.index].index;

	// If matrix A is symmetric then, mulAG(c, s) = mulGA(c, -s), 'mulGA' is thread-safe which is not the case for 'mulAG'

	// Change D = GtDG :
	// D = GtDG = Gt(GtDt)t

	// Instead of returning Q = QG, return Qt, Qt = GtQt

	COMPV_DEBUG_INFO_CODE_TODO("Try to eliminate extract2Cols/insert2Cols which were made to make mulAG thread-safe and SIMD-friendly. Take too much time");

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&GD_2rows, 2, dim));
	GD_2rowsPtr0 = GD_2rows->ptr<T>(0), GD_2rowsPtr1 = GD_2rows->ptr<T>(1);
	rowOld = 0, colOld = 0;
	do {
		CompVMathEigen<T>::jacobiAngles(D_, row, col, &gcos_, &gsin_); // Thread-safe
		/* Qt = G*Qt */
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulGA<T>(Qt, row, col, gcos_, -gsin_)); // Thread-safe
		/* Extract #2 cols */
		if (rowOld != row) {
			CompVMathEigen<T>::extract1Col(D_, row, GD_2rows, 0);
		}
		else {
			GD_2rowsPtr0[rowOld] = *D_->ptr<const T>(rowOld, rowOld);
			GD_2rowsPtr0[colOld] = *D_->ptr<const T>(colOld, rowOld);
		}
		if (colOld != col) {
			CompVMathEigen<T>::extract1Col(D_, col, GD_2rows, 1);
		}
		else {
			GD_2rowsPtr1[colOld] = *D_->ptr<const T>(colOld, colOld);
			GD_2rowsPtr1[rowOld] = *D_->ptr<const T>(rowOld, colOld);
		}

		/* GtDt */
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulGA<T>(GD_2rows, 0, 1, gcos_, -gsin_)); // Thread-safe
		/* Insert #2 cols */
		CompVMathEigen<T>::insert2Cols(GD_2rows, D_, row, col); // GD_2rows = (GtDt)t
		/* Make sure the inserted cols doesn't change the max map */
		for (size_t i = row + 1; i < dim; ++i) {
			CompVMaxAbsOffDiagValue& vv = maxAbsOffDiagValues[i];
			const T maxx_ = std::abs(GD_2rowsPtr0[i]);
			if (maxx_ > vv.maxx) {
				vv.index = row;
				vv.maxx = maxx_;
			}
		}
		for (size_t i = col + 1; i < dim; ++i) {
			CompVMaxAbsOffDiagValue& vv = maxAbsOffDiagValues[i];
			const T maxx_ = std::abs(GD_2rowsPtr1[i]);
			if (maxx_ > vv.maxx) {
				vv.index = col;
				vv.maxx = maxx_;
			}
		}
		/* Gt(GtDt)t */
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulGA<T>(D_, row, col, gcos_, -gsin_)); // Thread-safe (only rows at 'row' and 'col' are modified)
		/* Check ops count */
		if (++ops >= maxops) {
			break;
		}
		/* Compute max-abs-off-diagonal */
		if (row) {
			CompVMaxAbsOffDiagValue& vv = maxAbsOffDiagValues[row];
			COMPV_CHECK_CODE_RETURN(CompVMathEigen<T>::maxAbsOffDiagSymm(D_->ptr<const T>(row), row, &vv.index, &vv.maxx));
			if (vv.maxx > maxAbsOffDiagValueHighest.maxx) {
				maxAbsOffDiagValueHighest.maxx = vv.maxx;
				maxAbsOffDiagValueHighest.index = row;
			}
		}
		if (col) {
			CompVMaxAbsOffDiagValue& vv = maxAbsOffDiagValues[col];
			COMPV_CHECK_CODE_RETURN(CompVMathEigen<T>::maxAbsOffDiagSymm(D_->ptr<const T>(col), col, &vv.index, &vv.maxx));
			if (vv.maxx > maxAbsOffDiagValueHighest.maxx) {
				maxAbsOffDiagValueHighest.maxx = vv.maxx;
				maxAbsOffDiagValueHighest.index = col;
			}
		}
		COMPV_CHECK_CODE_RETURN(CompVMaxAbsOffDiagValue::highest(maxAbsOffDiagValues, &maxAbsOffDiagValueHighest.index, &maxAbsOffDiagValueHighest.maxx));
		rowOld = row;
		colOld = col;
		row = maxAbsOffDiagValueHighest.index;
		col = maxAbsOffDiagValues[maxAbsOffDiagValueHighest.index].index;
	} 
	while (maxAbsOffDiagValueHighest.maxx > epsilon_);

	// Sort Qt (eigenvectors are rows)
	if (sort) {
		size_t eigenValuesCount = D_->cols(), index, oldIndex;
		CompVMatPtr Idx;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<size_t>(&Idx, 1, eigenValuesCount));
		size_t* indexes = Idx->ptr<size_t>();
		for (size_t i = 0; i < eigenValuesCount; ++i) {
			indexes[i] = i;
		}
		bool sorted, wasSorted = true;
		do {
			sorted = true;
			for (size_t i = 0; i < eigenValuesCount - 1; ++i) {
				index = indexes[i];
				if (*D_->ptr<T>(indexes[i], indexes[i]) < *D_->ptr<T>(indexes[i + 1], indexes[i + 1])) {
					oldIndex = indexes[i];
					indexes[i] = indexes[i + 1];
					indexes[i + 1] = oldIndex;
					sorted = false;
					wasSorted = false;
				}
			}
		} while (!sorted);

		if (!wasSorted) {
			COMPV_DEBUG_INFO_CODE_NOT_TESTED();
			CompVMatPtr Dsorted, Qsorted;
			COMPV_CHECK_CODE_RETURN(CompVMatrix::zero<T>(&Dsorted, D_->rows(), D_->cols()));
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&Qsorted, S->rows(), S->cols()));
			for (size_t i = 0; i < eigenValuesCount; ++i) {
				*Dsorted->ptr<T>(i, i) = *D_->ptr<const T>(indexes[i], indexes[i]);
				COMPV_CHECK_CODE_RETURN(CompVMem::copy(Qsorted->ptr<T>(i), Qt->ptr<T>(indexes[i]), Qsorted->rowInBytes()));
			}
			D_ = Dsorted;
			if (transpose) {
				COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(Qsorted, Q));
				transpose = false; // to avoid transpose after the done
			}
			else {
				*Q = Qsorted;
			}
		}
	}

done:
	if (transpose) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::transpose(Qt, Q));
	}

	// Off-diagonal values in D contains epsilons which is close to zero but not equal to zero
	if (forceZerosInD) {
		T* row;
		for (size_t j = 0; j < D_->rows(); ++j) {
			row = D_->ptr<T>(j);
			for (size_t i = 0; i < D_->cols(); ++i) {
				if (i == j) {
					if (CompVMathEigen<T>::isCloseToZero(row[i])) {
						row[i] = 0;
					}
				}
				else {
					row[i] = 0;
				}
			}
		}
	}

	if (ops >= maxops) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "ops(%zu) >= maxops(%zu). Using 'double': %s", ops, maxops, std::is_same<T, compv_float64_t>::value ? "true" : "false");
	}

	*D = D_;

	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE  CompVMathEigen<T>::find2x2(const T(&A)[4], T(&D)[4], T(&Q)[4], bool sort COMPV_DEFAULT(true), bool norm COMPV_DEFAULT(true))
{
#if 0 // https://github.com/DoubangoTelecom/compv/issues/138
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No GPU or SIMD implementation found");
#endif
	// https://en.wikipedia.org/wiki/Eigenvalue_algorithm#2.C3.972_matrices
	// http://www.math.harvard.edu/archive/21b_fall_04/exhibits/2dmatrices/index.html

	// Eigenvalues
	const T trace = A[0] + A[3];
	const T trace_div2 = trace / static_cast<T>(2.0);
	const T det = (A[0] * A[3]) - (A[1] * A[2]);
	const T sqrt_trace_square_div4_minus_det = std::sqrt(((trace * trace) / static_cast<T>(4.0)) - det);
	D[1] = D[2] = 0.0;
	D[0] = trace_div2 + sqrt_trace_square_div4_minus_det;
	D[3] = trace_div2 - sqrt_trace_square_div4_minus_det;

	// Eigenvectors
	if (A[2] != 0) {
		Q[0] = D[0] - A[3], Q[2] = A[2];
		Q[1] = D[3] - A[3], Q[3] = A[2];
	}
	else if (A[1] != 0) {
		Q[0] = A[1], Q[2] = D[0] - A[0];
		Q[1] = A[1], Q[3] = D[3] - A[0];
	}
	else {
		norm = false;
		if (A[3] != 0.0) { // *must*, otherwise in KHT 'M_Eq14_r0' could be equal to zero leading to inf. result when used as denominator
			Q[0] = 0.0, Q[2] = 1.0;
			Q[1] = 1.0, Q[3] = 0.0;
		}
		else {
			Q[0] = 1.0, Q[2] = 0.0;
			Q[1] = 0.0, Q[3] = 1.0;
		}
	}

	// Normalisation
	if (norm) {
		const T mag02 = static_cast<T>(1) / std::sqrt(Q[0] * Q[0] + Q[2] * Q[2]);
		const T mag13 = static_cast<T>(1) / std::sqrt(Q[1] * Q[1] + Q[3] * Q[3]);
		Q[0] *= mag02, Q[2] *= mag02;
		Q[1] *= mag13, Q[3] *= mag13;
	}

	// sort: highest to lowest
	if (sort && D[0] < D[3]) {
		// sort vectors
		T a = Q[0], b = Q[2];
		Q[0] = Q[1], Q[2] = Q[3];
		Q[1] = a, Q[3] = b;
		// sort values
		a = D[0];
		D[0] = D[3], D[3] = a;
	}
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
T CompVMathEigen<T>::epsilon()
{
	return std::numeric_limits<T>::epsilon();
}

template <class T>
bool CompVMathEigen<T>::isCloseToZero(T a)
{
	return (static_cast<T>(COMPV_MATH_ABS(a)) <= CompVMathEigen<T>::epsilon());
}

template <class T>
COMPV_ERROR_CODE CompVMathEigen<T>::maxAbsOffDiagSymm(const T* rowPtr, const size_t row, size_t *col, T* max)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_CHECK_EXP_RETURN(!row, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "row must be > 0"); // row zero cannot contain off-diagonal index
	T vv, maxx = rowPtr[0];
	*col = 0;
	for (size_t i = 1; i < row; ++i) { // i stops at j because the matrix is symmetric and break just before reaching the diagonal
		if ((vv = std::abs(rowPtr[i])) > maxx) {
			maxx = vv;
			*col = i;
		}
	}
	*max = maxx;
	return COMPV_ERROR_CODE_S_OK;
}

// Compute cos('c') and sin ('s')
template <class T>
void CompVMathEigen<T>::jacobiAngles(const CompVMatPtr &S, size_t ith, size_t jth, T *c, T *s)
{
	// From https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm#Description
	// https://github.com/DoubangoTelecom/compv/issues/86
#if 1
	T Sii = *S->ptr<T>(ith, ith);
	T Sjj = *S->ptr<T>(jth, jth);
	if (Sii == Sjj) {
		// theta = PI/4
		*c = static_cast<T>(0.70710678118654757); // :: cos(PI/4)
		*s = static_cast<T>(0.70710678118654757); // :: sin(PI/4)
	}
	else {
		T theta = static_cast<T>(0.5) * static_cast<T>(COMPV_MATH_ATAN2(2.0 * *S->ptr<T>(ith, jth), Sjj - Sii));
		*c = static_cast<T>(COMPV_MATH_COS(theta));
		*s = static_cast<T>(COMPV_MATH_SIN(theta));
	}
#else
	// Not correct
	// Using SQRT function isn't faster than using cos(), sin() - At least on Win64
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	T Sii = *S->ptr(ith, ith);
	T Sjj = *S->ptr(jth, jth);
	T Sij = *S->ptr(ith, jth);
	T b = (Sjj - Sii) / Sij;
	T b2 = b*b;
	T tan0 = (T)(-b + sqrt(b2 + 4)) / 2;
	T tan1 = (T)(-b - sqrt(b2 + 4)) / 2;
	T tan = (T)(abs(tan0) > abs(tan1) ? tan0 : tan1);
	T tan2 = tan * tan;
	*c = (T)sqrt(1 / (1 + tan2));
	*s = (*c * tan);
#endif
}

// Compute cos('c') and sin ('s')
// c and s can only be used for left-multiply (mulGA). Cannot be used for right multiply (mulAG)
// To be used for QR decomposion (https://en.wikipedia.org/wiki/QR_decomposition)
template <class T>
void CompVMathEigen<T>::jacobiAngles_Left(const CompVMatPtr &S, size_t ith, size_t jth, T *c, T *s)
{
	COMPV_DEBUG_INFO_CODE_NOT_TESTED("Not tested and not used yet!!");
	// From https://en.wikipedia.org/wiki/Givens_rotation#Stable_calculation
	const T a = *S->ptr<T>(ith);
	const T b = *S->ptr<T>(jth);
	const T r = CompVMathUtils::hypot<T>(a, b);
	const T ri = static_cast<T>(1) / r;
	*c = a * ri;
	*s = -b * ri;
}

// Extract 2 cols from A and insert as rows to R
template <class T>
void CompVMathEigen<T>::extract2Cols(const CompVMatPtr &A, size_t a_col0, size_t a_col1, CompVMatPtr &R)
{
	// Private function -> do not check input parameters
	T* r0 = R->ptr<T>(0);
	T* r1 = R->ptr<T>(1);
	const T* a0 = A->ptr<const T>(0, a_col0);
	const T* a1 = A->ptr<const T>(0, a_col1);
	size_t astrideInElts;
	COMPV_CHECK_CODE_ASSERT(A->strideInElts(astrideInElts));
	const size_t rows_ = A->rows();
	switch (rows_) {
	case 1:
		r0[0] = a0[0];
		r1[0] = a1[0];
		break;
	case 2:
		r0[0] = a0[0];
		r0[1] = a0[astrideInElts];
		r1[0] = a1[0];
		r1[1] = a1[astrideInElts];
		break;
	case 3:
		r0[0] = a0[0];
		r0[1] = a0[astrideInElts];
		r0[2] = a0[astrideInElts << 1];
		r1[0] = a1[0];
		r1[1] = a1[astrideInElts];
		r1[2] = a1[astrideInElts << 1];
		break;
	default:
		size_t row_, aidx_, rows4_ = rows_ & -4;
		size_t astrideInEltsTimes2 = astrideInElts << 1;
		size_t astrideInEltsTimes3 = astrideInEltsTimes2 + astrideInElts;
		size_t astrideInEltsTimes4 = astrideInEltsTimes3 + astrideInElts;
		for (row_ = 0, aidx_ = 0; row_ < rows4_; row_ += 4, aidx_ += astrideInEltsTimes4) {
			r0[row_] = a0[aidx_];
			r0[row_ + 1] = a0[aidx_ + astrideInElts];
			r0[row_ + 2] = a0[aidx_ + astrideInEltsTimes2];
			r0[row_ + 3] = a0[aidx_ + astrideInEltsTimes3];
			r1[row_] = a1[aidx_];
			r1[row_ + 1] = a1[aidx_ + astrideInElts];
			r1[row_ + 2] = a1[aidx_ + astrideInEltsTimes2];
			r1[row_ + 3] = a1[aidx_ + astrideInEltsTimes3];
		}
		for (; row_ < rows_; ++row_, aidx_ += astrideInElts) {
			r0[row_] = a0[aidx_];
			r1[row_] = a1[aidx_];
		}
		break;
	}
}

// Extract 1 col from A and insert as rows to R
template <class T>
void CompVMathEigen<T>::extract1Col(const CompVMatPtr &A, size_t a_col0, CompVMatPtr &R, size_t r_row)
{
	// Private function -> do not check input parameters
	T* r0 = R->ptr<T>(r_row);
	const T* a0 = A->ptr<const T>(0, a_col0);
	size_t astrideInElts;
	COMPV_CHECK_CODE_ASSERT(A->strideInElts(astrideInElts));
	const size_t rows_ = A->rows();
	switch (rows_) {
	case 1:
		r0[0] = a0[0];
		break;
	case 2:
		r0[0] = a0[0];
		r0[1] = a0[astrideInElts];
		break;
	case 3:
		r0[0] = a0[0];
		r0[1] = a0[astrideInElts];
		r0[2] = a0[astrideInElts << 1];
		break;
	default:
		size_t row_, aidx_, rows4_ = rows_ & -4;
		size_t astrideInEltsTimes2 = astrideInElts << 1;
		size_t astrideInEltsTimes3 = astrideInEltsTimes2 + astrideInElts;
		size_t astrideInEltsTimes4 = astrideInEltsTimes3 + astrideInElts;
		for (row_ = 0, aidx_ = 0; row_ < rows4_; row_ += 4, aidx_ += astrideInEltsTimes4) {
			r0[row_] = a0[aidx_];
			r0[row_ + 1] = a0[aidx_ + astrideInElts];
			r0[row_ + 2] = a0[aidx_ + astrideInEltsTimes2];
			r0[row_ + 3] = a0[aidx_ + astrideInEltsTimes3];
		}
		for (; row_ < rows_; ++row_, aidx_ += astrideInElts) {
			r0[row_] = a0[aidx_];
		}
		break;
	}
}

template <class T>
void CompVMathEigen<T>::insert2Cols(const CompVMatPtr &A, CompVMatPtr &R, size_t r_col0, size_t r_col1)
{
	// Private function -> do not check input parameters
	const T* a0 = A->ptr<const T>(0);
	const T* a1 = A->ptr<const T>(1);
	T* r0 = R->ptr<T>(0, r_col0);
	T* r1 = R->ptr<T>(0, r_col1);
	size_t rstrideInElts;
	COMPV_CHECK_CODE_ASSERT(R->strideInElts(rstrideInElts));
	const size_t rows_ = R->rows();
	switch (rows_) {
	case 1:
		r0[0] = a0[0];
		r1[0] = a1[0];
		break;
	case 2:
		r0[0] = a0[0];
		r0[rstrideInElts] = a0[1];
		r1[0] = a1[0];
		r1[rstrideInElts] = a1[1];
		break;
	case 3:
		r0[0] = a0[0];
		r0[rstrideInElts] = a0[1];
		r0[rstrideInElts << 1] = a0[2];
		r1[0] = a1[0];
		r1[rstrideInElts] = a1[1];
		r1[rstrideInElts << 1] = a1[2];
		break;
	default:
		size_t row_, ridx_, rows4_ = rows_ & -4;
		size_t rstrideInEltsTimes2 = rstrideInElts << 1;
		size_t rstrideInEltsTimes3 = rstrideInEltsTimes2 + rstrideInElts;
		size_t rstrideInEltsTimes4 = rstrideInEltsTimes3 + rstrideInElts;
		for (row_ = 0, ridx_ = 0; row_ < rows4_; row_ += 4, ridx_ += rstrideInEltsTimes4) {
			r0[ridx_] = a0[row_];
			r0[ridx_ + rstrideInElts] = a0[row_ + 1];
			r0[ridx_ + rstrideInEltsTimes2] = a0[row_ + 2];
			r0[ridx_ + rstrideInEltsTimes3] = a0[row_ + 3];

			r1[ridx_] = a1[row_];
			r1[ridx_ + rstrideInElts] = a1[row_ + 1];
			r1[ridx_ + rstrideInEltsTimes2] = a1[row_ + 2];
			r1[ridx_ + rstrideInEltsTimes3] = a1[row_ + 3];
		}
		for (; row_ < rows_; ++row_, ridx_ += rstrideInElts) {
			r0[ridx_] = a0[row_];
			r1[ridx_] = a1[row_];
		}
		break;
	}
}

template class CompVMathEigen<compv_float32_t>;
template class CompVMathEigen<compv_float64_t>;

COMPV_NAMESPACE_END()
