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

#define kModuleNameHomography "Homography"

template class CompVHomography<double >;
template class CompVHomography<float >;

template<typename T>
static COMPV_ERROR_CODE computeH(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, CompVPtrArray(T) &H, bool promoteZeros = false);
template<typename T>
static COMPV_ERROR_CODE countInliers(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, const CompVPtrArray(T) &H, size_t &inliersCount, CompVPtrArray(size_t)& inliers, size_t &std2);
template<typename T>
static void promoteZeros(CompVPtrArray(T) &H);

// Homography 'double' is faster because EigenValues/EigenVectors computation converge faster (less residual error)
// src: 3xN homogeneous array (X, Y, Z=1). N-cols with N >= 4. The N points must not be colinear.
// dst: 3xN homogeneous array (X, Y, Z=1). N-cols with N >= 4. The N points must not be colinear.
// H: (3 x 3) array. Will be created if NULL.
// src and dst must have the same number of columns.
template<class T>
COMPV_ERROR_CODE CompVHomography<T>::find(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, CompVPtrArray(T) &H, COMPV_MODELEST_TYPE model /*= COMPV_MODELEST_TYPE_RANSAC*/)
{
	// Homography requires at least #4 points
	// src and dst must be 2-rows array. 1st row = X, 2nd-row = Y
	COMPV_CHECK_EXP_RETURN(!src || !dst || src->rows() != 3 || dst->rows() != 3 || src->cols() < 4 || src->cols() != dst->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	size_t k_ = src->cols(); // total number of elements (must be > 4)
	const T *srcx1_, *srcy1_, *srcz1_, *dstx1_, *dsty1_, *dstz1_, *hx1_, *hy1_, *hz1_;

	srcx1_ = src->ptr(0);
	srcy1_ = src->ptr(1);
	srcz1_ = src->ptr(2);
	dstx1_ = dst->ptr(0);
	dsty1_ = dst->ptr(1);
	dstz1_ = dst->ptr(2);

	// Make sure coordinates are homegeneous 2D
	for (size_t i = 0; i < k_; ++i) {
		COMPV_CHECK_EXP_RETURN(srcz1_[i] != 1 || dstz1_[i] != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	}

	if (model == COMPV_MODELEST_TYPE_NONE) {
		COMPV_CHECK_CODE_RETURN(computeH<T>(src, dst, H, true));
		return COMPV_ERROR_CODE_S_OK;
	}

	float p_ = 0.99f; // probability for inlier (TODO(dmi): try with 0.95f which is more realistic)
	size_t d_ = (size_t)(p_ * k_); // minimum number of inliers to stop the tries
	size_t s_ = 4; // subset size: 2 for line, 3 for plane, 4 for homography, 8 for essential / essential matrix
	float e_ = 0.50f; // outliers ratio (50% is a worst case, will be updated) = 1 - (inliersCount/total)
	size_t n_; // maximum number of tries
	size_t t_; // number of tries
	
	int idx0, idx1, idx2, idx3;
	size_t inliersCount_, bestInlinersCount_ = 0;
	size_t std2_, bestStd2_ = INT_MAX;

	CompVPtrArray(T) src_;
	CompVPtrArray(T) dst_;
	CompVPtrArray(T) H_;
	CompVPtrArray(size_t) inliers_; // inliers indexes
	CompVPtrArray(size_t) bestInliers_; // if you change the type (size_t) thenn change the below memcpy;
	T *srcx0_, *srcy0_, *srcz0_, *dstx0_, *dsty0_, *dstz0_, *hx0_, *hy0_, *hz0_;
	bool colinear;

	n_ = (size_t)(logf(1 - p_) / logf(1 - powf(1 - e_, (float)s_)));
	t_ = 0;

	// inliers_-> row-0: point indexes, row-1: distances
	COMPV_CHECK_CODE_RETURN(CompVArray<size_t>::newObjAligned(&inliers_, 2, k_));

	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&H, 3, 3));
	hx0_ = const_cast<T*>(H->ptr(0));
	hy0_ = const_cast<T*>(H->ptr(1));
	hz0_ = const_cast<T*>(H->ptr(2));

	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&src_, 3, s_));
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&dst_, 3, s_));

	srcx0_ = const_cast<T*>(src_->ptr(0));
	srcy0_ = const_cast<T*>(src_->ptr(1));
	srcz0_ = const_cast<T*>(src_->ptr(2));
	dstx0_ = const_cast<T*>(dst_->ptr(0));
	dsty0_ = const_cast<T*>(dst_->ptr(1));
	dstz0_ = const_cast<T*>(dst_->ptr(2));

	// 2D planar
	srcz0_[0] = srcz0_[1] = srcz0_[2] = srcz0_[3] = 1;
	dstz0_[0] = dstz0_[1] = dstz0_[2] = dstz0_[3] = 1;

	while (t_ < n_ && bestInlinersCount_ < d_) {
		// TODO(dmi): use rand4()
		// TODO(dmi): use prng
		do { idx0 = rand() % k_; } while (0);
		do { idx1 = rand() % k_; } while (idx1 == idx0);
		do { idx2 = rand() % k_; } while (idx2 == idx1 || idx2 == idx0);
		do { idx3 = rand() % k_; } while (idx3 == idx2 || idx3 == idx1 || idx3 == idx0);

		// Set the #4 random points
		srcx0_[0] = srcx1_[idx0], srcx0_[1] = srcx1_[idx1], srcx0_[2] = srcx1_[idx2], srcx0_[3] = srcx1_[idx3];
		srcy0_[0] = srcy1_[idx0], srcy0_[1] = srcy1_[idx1], srcy0_[2] = srcy1_[idx2], srcy0_[3] = srcy1_[idx3];
		dstx0_[0] = dstx1_[idx0], dstx0_[1] = dstx1_[idx1], dstx0_[2] = dstx1_[idx2], dstx0_[3] = dstx1_[idx3];
		dsty0_[0] = dsty1_[idx0], dsty0_[1] = dsty1_[idx1], dsty0_[2] = dsty1_[idx2], dsty0_[3] = dsty1_[idx3];
		
		// Colinears?
		// TODO(dmi): doesn't worth it -> colinear points will compute a wrong homography to much outliers -> not an issue
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::isColinear2D(src_, colinear));
		if (colinear) {
			COMPV_DEBUG_INFO_EX(kModuleNameHomography, "ignore colinear points ...");
			++t_; // to avoid endless loops
			continue;
		}

		// Compute Homography using the #4 random points
		COMPV_CHECK_CODE_RETURN(computeH<T>(src_, dst_, H_));

		// Count outliers using all points
		COMPV_CHECK_CODE_RETURN(countInliers(src, dst, H_, inliersCount_, inliers_, std2_));

		if (inliersCount_ >= s_ && (inliersCount_ > bestInlinersCount_ || (inliersCount_ == bestInlinersCount_ && std2_ < bestStd2_))) {
			bestInlinersCount_ = inliersCount_;
			bestStd2_ = std2_;
			// update H
			hx1_ = H_->ptr(0);
			hy1_ = H_->ptr(1);
			hz1_ = H_->ptr(2);
			hx0_[0] = hx1_[0], hx0_[1] = hx1_[1], hx0_[2] = hx1_[2];
			hy0_[0] = hy1_[0], hy0_[1] = hy1_[1], hy0_[2] = hy1_[2];
			hz0_[0] = hz1_[0], hz0_[1] = hz1_[1], hz0_[2] = hz1_[2];
			// Copy inliers
			COMPV_CHECK_CODE_RETURN(CompVArray<size_t>::newObjAligned(&bestInliers_, 1, inliersCount_));
			CompVMem::copyNTA(const_cast<size_t*>(bestInliers_->ptr(0)), inliers_->ptr(0), (inliersCount_ * sizeof(size_t)));
		}

		if (inliersCount_) {
			// update outliers ratio
			e_ = 1 - (inliersCount_ / (float)k_);
			// update total tries
			n_ = (size_t)(logf(1 - p_) / logf(1 - powf(1 - e_, (float)s_)));
		}

		++t_;
	}

	if (bestInlinersCount_ == k_ || bestInlinersCount_ < s_) { // all points are inliers or not enought points ?
		COMPV_DEBUG_INFO_EX(kModuleNameHomography, "All points are inliers or not enought inliers(< 4). InlinersCount = %lu, k = %lu", bestInlinersCount_, k_);
		return COMPV_ERROR_CODE_S_OK; // Return the best H
	}

	// Compute final H using inliers only
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&src_, 3, bestInlinersCount_));
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&dst_, 3, bestInlinersCount_));

	srcx0_ = const_cast<T*>(src_->ptr(0));
	srcy0_ = const_cast<T*>(src_->ptr(1));
	srcz0_ = const_cast<T*>(src_->ptr(2));
	dstx0_ = const_cast<T*>(dst_->ptr(0));
	dsty0_ = const_cast<T*>(dst_->ptr(1));
	dstz0_ = const_cast<T*>(dst_->ptr(2));

	size_t idx;
	const size_t* inliersIdx_ = inliers_->ptr();
	for (size_t i = 0; i < bestInlinersCount_; ++i) {
		idx = inliersIdx_[i];
		srcx0_[i] = srcx1_[idx], srcy0_[i] = srcy1_[idx], srcz0_[i] = 1;
		dstx0_[i] = dstx1_[idx], dsty0_[i] = dsty1_[idx], dstz0_[i] = 1;
	}

	COMPV_CHECK_CODE_RETURN(computeH<T>(src_, dst_, H, true));

	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static COMPV_ERROR_CODE computeH(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, CompVPtrArray(T) &H, bool promoteZeros /*= false*/)
{
	// Private function, do not check input parameters
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	const CompVArray<T >* src_ = *src;
	const CompVArray<T >* dst_ = *dst;
	const T *srcX_, *srcY_, *srcZ_, *dstX_, *dstY_, *dstZ_;
	T *row_;
	size_t numPoints_ = src_->cols(), numPointsTimes2_ = numPoints_ * 2;
	size_t i;

	// TODO(dmi): use calib class and store "srcn", "dstn", "M_", "S_", "D_", "Q_", "T1", "T2"....

	// TODO(dmi): Use SIMD for the normaization part to generate 3x3 matrix
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	srcX_ = src_->ptr(0);
	srcY_ = src_->ptr(1);
	srcZ_ = dst_->ptr(2);
	dstX_ = dst_->ptr(0);
	dstY_ = dst_->ptr(1);
	dstZ_ = dst_->ptr(2);

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
	T srcScale_ = srcMag_ ? (T)(COMPV_MATH_SQRT_2 / srcMag_) : (T)COMPV_MATH_SQRT_2;
	T dstScale_ = dstMag_ ? (T)(COMPV_MATH_SQRT_2 / dstMag_) : (T)COMPV_MATH_SQRT_2;

	// Translation(t) to centroid then scaling(s) operation:
	// -> b = (a+t)s = as+ts = as+t' with t'= ts
	// T matrix
	//	scale	0		-Tx*scale
	//	0		scale	-Ty*scale
	//	0		0		1
	CompVPtrArray(T) T1_;
	CompVPtrArray(T) srcn_;
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObjAligned(&T1_, 3, 3));
	// Normalize src_: srcn_ = T.src_
	row_ = const_cast<T*>(T1_->ptr(0)), row_[0] = srcScale_, row_[1] = 0, row_[2] = -srcTX_ * srcScale_;
	row_ = const_cast<T*>(T1_->ptr(1)), row_[0] = 0, row_[1] = srcScale_, row_[2] = -srcTY_ * srcScale_;
	row_ = const_cast<T*>(T1_->ptr(2)), row_[0] = 0, row_[1] = 0, row_[2] = 1;
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(T1_, src, srcn_));
	// Normilize dst_: dstn_ = T.dst_
	CompVPtrArray(T) T2_;
	CompVPtrArray(T) dstn_;
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObjAligned(&T2_, 3, 3));
	row_ = const_cast<T*>(T2_->ptr(0)), row_[0] = dstScale_, row_[1] = 0, row_[2] = -dstTX_ * dstScale_;
	row_ = const_cast<T*>(T2_->ptr(1)), row_[0] = 0, row_[1] = dstScale_, row_[2] = -dstTY_ * dstScale_;
	row_ = const_cast<T*>(T2_->ptr(2)), row_[0] = 0, row_[1] = 0, row_[2] = 1;
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(T2_, dst, dstn_));

	// Build homogeneous equation: Mh = 0
	// Each correpondance adds 2 rows
	CompVPtrArray(T) M_; // temp array
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObjAligned(&M_, numPointsTimes2_, 9));
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

	// Find eigenvalues and eigenvectors (no sorting and vectors in rows instead of columns)
	CompVPtrArray(T) D_; // 9x9 diagonal matrix containing the eigenvalues
	CompVPtrArray(T) Qt_; // 9x9 matrix containing the eigenvectors (rows) - transposed
	COMPV_CHECK_CODE_RETURN(err_ = CompVEigen<T>::findSymm(S_, D_, Qt_, false, true));
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
		COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObjAligned(&H, 3, 3));
	}
	const T* q_ = Qt_->ptr(minIndex_);
	T* hn0_ = const_cast<T*>(H->ptr(0));
	T* hn1_ = const_cast<T*>(H->ptr(1));
	T* hn2_ = const_cast<T*>(H->ptr(2));
	// Hn = H normalized
	hn0_[0] = q_[0];
	hn0_[1] = q_[1];
	hn0_[2] = q_[2];
	hn1_[0] = q_[3];
	hn1_[1] = q_[4];
	hn1_[2] = q_[5];
	hn2_[0] = q_[6];
	hn2_[1] = q_[7];
	hn2_[2] = q_[8];

	// Transpose for T1
	row_ = const_cast<T*>(T1_->ptr(0)), row_[0] = srcScale_, row_[1] = 0, row_[2] = 0;
	row_ = const_cast<T*>(T1_->ptr(1)), row_[0] = 0, row_[1] = srcScale_, row_[2] = 0;
	row_ = const_cast<T*>(T1_->ptr(2)), row_[0] = -srcTX_ * srcScale_, row_[1] = -srcTY_ * srcScale_, row_[2] = 1;

	// Inverse operation for T2
	// -> b = as+t'
	// -> a = b(1/s)-t'(1/s) = b(1/s)+t'' whith t'' = -t'/s = -(ts)/s = -t
	// { 1 / s, 0, +tx },
	// { 0, 1 / s, +ty },
	// { 0, 0, 1 }
	row_ = const_cast<T*>(T2_->ptr(0)), row_[0] = 1 / dstScale_, row_[1] = 0, row_[2] = dstTX_;
	row_ = const_cast<T*>(T2_->ptr(1)), row_[0] = 0, row_[1] = 1 / dstScale_, row_[2] = dstTY_;
	row_ = const_cast<T*>(T2_->ptr(2)), row_[0] = 0, row_[1] = 0, row_[2] = 1;

	// De-normalize
	// HnAn = Bn, where Hn, An=T1A and Bn=T2B are normalized points
	// ->HnT1A = T2B
	// ->T2^HnT1A = T2^T2B = B
	// ->(T2^HnT1)A = B -> H'A = B whith H' = T2^HnT1 our final homography matrix
	// T2^HnT1 = T2^(T1*Hn*)* = T2^(T3Hn*)* with T3 = T1*
	// TODO(dmi): add mulABt_3x3(a, b)
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulABt(T1_, H, M_));
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulABt(T2_, M_, H));

	if (promoteZeros) {
#define COMPV_PROMOTE_ZEROS(_h_, _i_) if (CompVEigen<T>::isCloseToZero((_h_)[(_i_)])) (_h_)[(_i_)] = 0;
		COMPV_PROMOTE_ZEROS(hn0_, 0); COMPV_PROMOTE_ZEROS(hn0_, 1); COMPV_PROMOTE_ZEROS(hn0_, 2);
		COMPV_PROMOTE_ZEROS(hn1_, 0); COMPV_PROMOTE_ZEROS(hn1_, 1); COMPV_PROMOTE_ZEROS(hn1_, 2);
		COMPV_PROMOTE_ZEROS(hn2_, 0); COMPV_PROMOTE_ZEROS(hn2_, 1);
	}

	// Scale H to make it homogeneous (Z = 1)
	T h22_ = hn2_[2] ? ((T)1 / hn2_[2]) : 1;
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

template<typename T>
static COMPV_ERROR_CODE countInliers(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, const CompVPtrArray(T) &H, size_t &inliersCount, CompVPtrArray(size_t)& inliers, size_t &std2)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // Hinv not correct
	// Private function, do not check input parameters
	size_t numPoints_ = src->cols();
	static const T threshold = 25; // FIXME
	inliersCount = 0;
	std2 = 0; // standard deviation square (std * std)
	
	// Ha = b, residual(Ha, b), a = src, b = dst
	// -> a = H^b, residual(a, H^b)
	CompVPtrArray(T) b_; // TODO(dmi): make member or a parameter to avoid allocating several times
	CompVPtrArray(T) a_; // TODO(dmi): make member or a parameter to avoid allocating several times
	CompVPtrArray(T) Hinv_;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::pseudoinv(H, Hinv_));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAB(H, src, b_));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAB(Hinv_, dst, a_));
	// compute residual
	const T* bx_ = b_->ptr(0);
	const T* by_ = b_->ptr(1);
	const T* bz_ = b_->ptr(2);
	const T* ax_ = a_->ptr(0);
	const T* ay_ = a_->ptr(1);
	const T* az_ = a_->ptr(2);
	const T* dstx_ = dst->ptr(0);
	const T* dsty_ = dst->ptr(1);
	const T* srcx_ = src->ptr(0);
	const T* srcy_ = src->ptr(1);
	T d_, scale_, ex_, ey_;
	size_t sd_;

	size_t* indexes_ = const_cast<size_t*>(inliers->ptr(0));
	size_t* distances_ = const_cast<size_t*>(inliers->ptr(1));

	// FIXME: compute inverse(H) and mse(a, h^b)
	// FIXME: compute standard deviation (STD) 

	sd_ = 0; // sum distances
	for (size_t n_ = 0; n_ < numPoints_; ++n_) {
		// Ha = b
		scale_ = 1 / bz_[n_];
		ex_ = (bx_[n_] * scale_) - dstx_[n_];
		ey_ = (by_[n_] * scale_) - dsty_[n_];
		d_ = ((ex_ * ex_) + (ey_ * ey_));
		// a = H^b
		scale_ = 1 / az_[n_];
		ex_ = (ax_[n_] * scale_) - srcx_[n_];
		ey_ = (ay_[n_] * scale_) - srcy_[n_];
		d_ += ((ex_ * ex_) + (ey_ * ey_));

		if (d_ < threshold) {
			indexes_[inliersCount] = n_;
			distances_[inliersCount] = (size_t)d_;
			++inliersCount;
			sd_ += (size_t)d_;
		}
	}

	// Standard deviation: https://en.wikipedia.org/wiki/Standard_deviation
	if (inliersCount > 1) {
		size_t md_ = sd_ / inliersCount; // mean
		signed dev_; // must be signed
		for (size_t n_ = 0; n_ < inliersCount; ++n_) {
			dev_ = (signed)(distances_[n_] - md_);
			std2 += (dev_ * dev_);
		}
		// variance = std2 (std squared)
		std2 /= (inliersCount - 1); // -1 for Bessel's correction: https://en.wikipedia.org/wiki/Bessel%27s_correction
		// standard deviation
		// std_ = COMPV_MATH_SQRT(std_);
		// for comparisons no need to use std, variance is enought
	}

	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
void promoteZeros(CompVPtrArray(T) &H)
{
	// Private function, do not check input parameters

	size_t i, j, rows = H->rows(), cols = H->cols();
	T* row;

	for (j = 0; j < rows; ++j) {
		row = const_cast<T*>(H->ptr(j));
		for (i = 0; i < cols; ++i) {
			if (CompVEigen<T>::isCloseToZero(row[i])) {
				row[i] = 0;
			}
		}
	}
}

COMPV_NAMESPACE_END()