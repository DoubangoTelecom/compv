/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/calib/compv_calib_homography.h"
#include "compv/math/compv_math_eigen.h"
#include "compv/math/compv_math_matrix.h"

COMPV_NAMESPACE_BEGIN()

template class CompVHomography<double >;
template class CompVHomography<float >;

// src: 3xN homogeneous array (X, Y, Z=1). N-cols with N >= 4.
// dst: 3xN homogeneous array (X, Y, Z=1). N-cols with N >= 4.
// H: (3 x 3) array. Will be created if NULL.
// src and dst must have the same number of columns.
template<class T>
COMPV_ERROR_CODE CompVHomography<T>::find(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, CompVPtrArray(T) &H)
{
	// Homography requires at least #4 points
	// src and dst must be 2-rows array. 1st row = X, 2nd-row = Y
	COMPV_CHECK_EXP_RETURN(!src || !dst || src->rows() != 3 || dst->rows() != 3 || src->cols() < 4 || src->cols() != dst->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	const CompVArray<T >* src_ = *src;
	const CompVArray<T >* dst_ = *dst;
	const T *srcX_, *srcY_, *dstX_, *dstY_;
	T *row_;
	size_t numPoints_ = src_->cols(), numPointsTimes2_ = numPoints_ * 2;
	size_t i;

	// TODO(dmi): use calib class and store "srcn", "dstn", "M_", "S_", "D_", "Q_", "T1", "T2"....

	// TODO(dmi): Use SIMD for the normaization part to generate 3x3 matrix
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	srcX_ = src_->ptr(0);
	srcY_ = src_->ptr(1);
	dstX_ = dst_->ptr(0);
	dstY_ = dst_->ptr(1);

	// Resolving Ha = b equation (4-point algorithm)
	// -> Ah = 0 (homogeneous equation)
	// -> h is in the nullspace of A, means eigenvector with the smallest eigenvalue. If the equation is exactly determined then, the smallest eigenvalue must be equal to zero.

	// Based on the same normalization as the 8-point algorithm (Hartley and Zisserman, https://en.wikipedia.org/wiki/Eight-point_algorithm#How_it_can_be_solved).
	// Compute the centroid (https://en.wikipedia.org/wiki/Centroid#Of_a_finite_set_of_points)
	T srcTX_ = 0, srcTY_ = 0, dstTX_ = 0, dstTY_ = 0;
	for (i = 0; i < numPoints_; ++i) {
		srcTX_ += srcX_[i];
		srcTY_ += srcY_[i];
		dstTX_ += dstX_[i];
		dstTY_ += dstY_[i];
	}
	srcTX_ /= numPoints_;
	srcTY_ /= numPoints_;
	dstTX_ /= numPoints_;
	dstTY_ /= numPoints_;
	// AFTER the translation the coordinates are uniformly scaled (Isotropic scaling) so that the mean distance from the origin to a point equals sqrt(2).
	// TODO(dmi): use classic normalization ((x,y)/(max_norm) € [0, 1])
	// TODO(dmi): norm(a) = sqrt(x^2 + y^2) = sqrt(dp(a, a))
	// Isotropic scaling -> scaling is invariant with respect to direction
	T srcMag_ = 0, dstMag_ = 0;
	for (i = 0; i < numPoints_; ++i) {
		// Using naive hypot because X and Y contains point coordinates (no risk for overflow / underflow)
		// TODO(dmi): check if OS built-in hypot() isn't faster than our naive implementation
		srcMag_ += CompVMathUtils::hypot_naive((srcX_[i] - srcTX_), (srcY_[i] - srcTY_));
		dstMag_ += CompVMathUtils::hypot_naive((dstX_[i] - dstTX_), (dstY_[i] - dstTY_));
	}
	srcMag_ /= numPoints_;
	dstMag_ /= numPoints_;
	T srcScale_ = (T)(COMPV_MATH_SQRT_2 / srcMag_);
	T dstScale_ = (T)(COMPV_MATH_SQRT_2 / dstMag_);

	// Translation(t) to centroid then scaling(s) operation:
	// -> b = (a+t)s = as+ts = as+t' with t'= ts
	// T matrix
	//	scale	0		-Tx*scale
	//	0		scale	-Ty*scale
	//	0		0		1
	CompVPtrArray(T) T1_;
	CompVPtrArray(T) srcn_;
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObj(&T1_, 3, 3, COMPV_SIMD_ALIGNV_DEFAULT));
	// Normalize src_: srcn_ = T.src_
	row_ = const_cast<T*>(T1_->ptr(0)), row_[0] = srcScale_, row_[1] = 0, row_[2] = -srcTX_ * srcScale_;
	row_ = const_cast<T*>(T1_->ptr(1)), row_[0] = 0, row_[1] = srcScale_, row_[2] = -srcTY_ * srcScale_;
	row_ = const_cast<T*>(T1_->ptr(2)), row_[0] = 0, row_[1] = 0, row_[2] = 1;
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(T1_, src, srcn_));
	// Normilize dst_: dstn_ = T.dst_
	CompVPtrArray(T) T2_;
	CompVPtrArray(T) dstn_;
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObj(&T2_, 3, 3, COMPV_SIMD_ALIGNV_DEFAULT));
	row_ = const_cast<T*>(T2_->ptr(0)), row_[0] = dstScale_, row_[1] = 0, row_[2] = -dstTX_ * dstScale_;
	row_ = const_cast<T*>(T2_->ptr(1)), row_[0] = 0, row_[1] = dstScale_, row_[2] = -dstTY_ * dstScale_;
	row_ = const_cast<T*>(T2_->ptr(2)), row_[0] = 0, row_[1] = 0, row_[2] = 1;
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(T2_, dst, dstn_));
	// Inverse operation for T2
	// -> b = as+t'
	// -> a = b(1/s)-t'(1/s) = b(1/s)+t'' whith t'' = -t'/s = -(ts)/s = -t
	// { 1 / s, 0, +tx },
	// { 0, 1 / s, +ty },
	// { 0, 0, 1 }
	row_ = const_cast<T*>(T2_->ptr(0)), row_[0] = 1 / dstScale_, row_[1] = 0, row_[2] = dstTX_;
	row_ = const_cast<T*>(T2_->ptr(1)), row_[0] = 0, row_[1] = 1 / dstScale_, row_[2] = dstTY_;
	row_ = const_cast<T*>(T2_->ptr(2)), row_[0] = 0, row_[1] = 0, row_[2] = 1;

	// Build homogeneous equation: Mh = 0
	// Each correpondance adds 2 rows
	CompVPtrArray(T) M_; // temp array
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObj(&M_, numPointsTimes2_, 9, COMPV_SIMD_ALIGNV_DEFAULT));
	const T *srcnx_, *srcny_, *dstnx_, *dstny_;
	T* m_;
	srcnx_ = srcn_->ptr(0);
	srcny_ = srcn_->ptr(1);
	dstnx_ = dstn_->ptr(0);
	dstny_ = dstn_->ptr(1);
	for (i = 0; i < numPoints_; ++i) {
		// z' = 1
		// TODO(dmi): srcnx_++ then *srcnx_

		m_ = const_cast<T*>(M_->ptr(i << 1));
		m_[0] = -srcnx_[i]; // -x
		m_[1] = -srcny_[i]; // -y
		m_[2] = -1; // -1
		m_[3] = 0;
		m_[4] = 0;
		m_[5] = 0;
		m_[6] = (dstnx_[i] * srcnx_[i]); // (x'x)/z'
		m_[7] = (dstnx_[i] * srcny_[i]); // (x'y)/z'
		m_[8] = dstnx_[i]; // x'/z'

		m_ = const_cast<T*>(M_->ptr((i << 1) + 1));
		m_[0] = 0;
		m_[1] = 0;
		m_[2] = 0;
		m_[3] = -srcnx_[i]; // -x
		m_[4] = -srcny_[i]; // -y
		m_[5] = -1; // -1
		m_[6] = (dstny_[i] * srcnx_[i]); // (y'x)/z'
		m_[7] = (dstny_[i] * srcny_[i]); // (y'y)/z'
		m_[8] = dstny_[i]; // y'/z'
	}

	// Build symmetric matrix S = M*M
	CompVPtrArray(T) S_; // temp symmetric array
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAtA(M_, S_));

	// Find eigenvalues and eigenvectors (no sorting)
	CompVPtrArray(T) D_; // 9x9 diagonal matrix containing the eigenvalues
	CompVPtrArray(T) Q_; // 9x9 matrix containing the eigenvectors (cols)
	COMPV_CHECK_CODE_RETURN(err_ = CompVEigen<T>::findSymm(S_, D_, Q_, false));
	// Find index of the smallest eigenvalue (this code is required because findSymm() is called without sorting for speed-up)
	int minIndex_ = 8;
	T minEigenValue_ = *D_->ptr(8);
	for (int j = 7; j >= 0; --j) { // starting at the end as the smallest value is probably there
		if (*D_->ptr(j, j) < minEigenValue_) {
			minEigenValue_ = *D_->ptr(j, j);
			minIndex_ = j;
		}
	}
	
	// Set homography values (normalized) using the egeinvector at "minIndex_"
	if (!H || H->rows() != 3 || H->cols() != 3) {
		COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObj(&H, 3, 3, COMPV_SIMD_ALIGNV_DEFAULT));
	}
	T* hn0_ = const_cast<T*>(H->ptr(0));
	T* hn1_ = const_cast<T*>(H->ptr(1));
	T* hn2_ = const_cast<T*>(H->ptr(2));
	hn0_[0] = *Q_->ptr(0, minIndex_);
	hn0_[1] = *Q_->ptr(1, minIndex_);
	hn0_[2] = *Q_->ptr(2, minIndex_);
	hn1_[0] = *Q_->ptr(3, minIndex_);
	hn1_[1] = *Q_->ptr(4, minIndex_);
	hn1_[2] = *Q_->ptr(5, minIndex_);
	hn2_[0] = *Q_->ptr(6, minIndex_);
	hn2_[1] = *Q_->ptr(7, minIndex_);
	hn2_[2] = *Q_->ptr(8, minIndex_);

	// De-normalize
	// HnAn = Bn, where Hn, An=T1A and Bn=T2B are normalized points
	// ->HnT1A = T2B
	// ->T2^HnT1A = T2^T2B = B
	// ->(T2^HnT1)A = B -> H'A = B whith H' = T2^HnT1 our final homography matrix
	// TODO(dmi): add mul3x3(a, b, c)
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(T2_, H, M_)); // T2^Hn
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(M_, T1_, H)); // T2^HnT1

	// Scale H to make it homogeneous (Z = 1)
	T h22_ = (T)1.0 / hn2_[2];
	hn0_[0] *= h22_;
	hn0_[1] *= h22_;
	hn0_[2] *= h22_;
	hn1_[0] *= h22_;
	hn1_[1] *= h22_;
	hn1_[2] *= h22_;
	hn2_[0] *= h22_;
	hn2_[1] *= h22_;
	hn2_[2] *= h22_; // should be #1

	return err_;
}

COMPV_NAMESPACE_END()