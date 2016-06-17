/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/calib/compv_calib_homography.h"
#include "compv/math/compv_math_eigen.h"
#include "compv/math/compv_math_matrix.h"
#include "compv/math/compv_math_stats.h"

#if !defined (COMPV_PRNG11)
#	define COMPV_PRNG11 1
#endif

#if !defined (COMPV_HOMOGRAPHY_OUTLIER_THRESHOLD)
#	define	COMPV_HOMOGRAPHY_OUTLIER_THRESHOLD 30
#endif

#if COMPV_PRNG11
#	include <random>
#endif

COMPV_NAMESPACE_BEGIN()

#define kModuleNameHomography "Homography"

template class CompVHomography<compv_float64_t >;
template class CompVHomography<compv_float32_t >;

template<typename T>
static COMPV_ERROR_CODE computeH(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, CompVPtrArray(T) &H, bool promoteZeros = false);
template<typename T>
static COMPV_ERROR_CODE countInliers(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, const CompVPtrArray(T) &H, size_t &inliersCount, CompVPtrArray(size_t)& inliers, T &variance);

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

	// Make sure coordinates are homogeneous 2D
	for (size_t i = 0; i < k_; ++i) {
		if (srcz1_[i] != 1 || dstz1_[i] != 1){
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		}
	}

	// No estimation model select -> compute homography using all points (inliers + outliers)
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
	
	uint32_t idx0, idx1, idx2, idx3;
	size_t inliersCount_, bestInlinersCount_ = 0;
	T variance_, bestVariance_ = T(FLT_MAX); // Using variance instead of standard deviation because it's the same result as we are doing comparison

#if COMPV_PRNG11
	std::mt19937 prng_(12345); // TODO(dmi): use device random source for multithreading to avoid generating same numbers for each thread
	std::uniform_int_distribution<> unifd_ { 0, static_cast<int>(k_ - 1) };
#else
	uint32_t rand4[4];
#endif

	CompVPtrArray(T) src_;
	CompVPtrArray(T) dst_;
	CompVPtrArray(T) H_;
	CompVPtrArray(size_t) inliers_; // inliers indexes
	CompVPtrArray(size_t) bestInliers_; // if you change the type (size_t) thenn change the below memcpy;
	T *srcx0_, *srcy0_, *srcz0_, *dstx0_, *dsty0_, *dstz0_, *hx0_, *hy0_, *hz0_;
	bool colinear;

	n_ = static_cast<size_t>(logf(1 - p_) / logf(1 - powf(1 - e_, static_cast<float>(s_))));
	t_ = 0;
	
	COMPV_CHECK_CODE_RETURN(CompVArray<size_t>::newObjAligned(&inliers_, 1, k_));

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
		do {
#if COMPV_PRNG11
			idx0 = static_cast<uint32_t>(unifd_(prng_));
			idx1 = static_cast<uint32_t>(unifd_(prng_));
			idx2 = static_cast<uint32_t>(unifd_(prng_));
			idx3 = static_cast<uint32_t>(unifd_(prng_));
#else
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
			COMPV_DEBUG_INFO_CODE_FOR_TESTING();
			CompVMathUtils::rand(rand4, 4);
			idx0 = rand4[0] % k_;
			idx1 = rand4[1] % k_;
			idx2 = rand4[2] % k_;
			idx3 = rand4[3] % k_;
#endif
		} while (idx0 == idx1 || idx0 == idx2 || idx0 == idx3 || idx1 == idx2 || idx1 == idx3 || idx2 == idx3);

		// Set the #4 random points
		srcx0_[0] = srcx1_[idx0], srcx0_[1] = srcx1_[idx1], srcx0_[2] = srcx1_[idx2], srcx0_[3] = srcx1_[idx3];
		srcy0_[0] = srcy1_[idx0], srcy0_[1] = srcy1_[idx1], srcy0_[2] = srcy1_[idx2], srcy0_[3] = srcy1_[idx3];
		dstx0_[0] = dstx1_[idx0], dstx0_[1] = dstx1_[idx1], dstx0_[2] = dstx1_[idx2], dstx0_[3] = dstx1_[idx3];
		dsty0_[0] = dsty1_[idx0], dsty0_[1] = dsty1_[idx1], dsty0_[2] = dsty1_[idx2], dsty0_[3] = dsty1_[idx3];
		
		// Reject colinear points
		// TODO(dmi): doesn't worth it -> colinear points will compute a wrong homography with too much outliers -> not an issue
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::isColinear2D(src_, colinear));
		if (colinear) {
			COMPV_DEBUG_INFO_EX(kModuleNameHomography, "ignore colinear points ...");
			++t_; // to avoid endless loops
			continue;
		}

		// Compute Homography using the #4 random points selected above
		COMPV_CHECK_CODE_RETURN(computeH<T>(src_, dst_, H_));

		// Count outliers using all points and current homography using the inliers only
		COMPV_CHECK_CODE_RETURN(countInliers<T>(src, dst, H_, inliersCount_, inliers_, variance_));

		if (inliersCount_ >= s_ && (inliersCount_ > bestInlinersCount_ || (inliersCount_ == bestInlinersCount_ && variance_ < bestVariance_))) {
			bestInlinersCount_ = inliersCount_;
			bestVariance_ = variance_;
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

		if (inliersCount_) { // zero will produce NaN
			// update outliers ratio
			e_ = 1 - (inliersCount_ / (float)k_);
			// update total tries
			n_ = (size_t)(logf(1 - p_) / logf(1 - powf(1 - e_, (float)s_)));
		}

		++t_;
	}

	if (bestInlinersCount_ < s_) { // not enought points ?
		COMPV_DEBUG_INFO_EX(kModuleNameHomography, "Not enought inliers(< 4). InlinersCount = %lu, k = %lu", bestInlinersCount_, k_);
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
	size_t numPoints_ = src_->cols();

	// TODO(dmi): use calib class and store "srcn", "dstn", "M_", "S_", "D_", "Q_", "T1", "T2"....

	// TODO(dmi): Use SIMD for the normaization part to generate 3x3 matrix
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	srcX_ = src_->ptr(0);
	srcY_ = src_->ptr(1);
	srcZ_ = dst_->ptr(2);
	dstX_ = dst_->ptr(0);
	dstY_ = dst_->ptr(1);
	dstZ_ = dst_->ptr(2);

	/* Resolving Ha = b equation (4-point algorithm) */
	// -> Ah = 0 (homogeneous equation), with h a 9x1 vector
	// -> h is in the nullspace of A, means eigenvector with the smallest eigenvalue. If the equation is exactly determined then, the smallest eigenvalue must be equal to zero.

	/* Normalize the points as described at https://en.wikipedia.org/wiki/Eight-point_algorithm#How_it_can_be_solved */
	T srcTX_, srcTY_, dstTX_, dstTY_; // translation (to the centroid) values
	T srcScale_, dstScale_; // scaling factors to have mean distance to the centroid = sqrt(2)
	COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::normalize2D_hartley(srcX_, srcY_, numPoints_, &srcTX_, &srcTY_, &srcScale_));
	COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::normalize2D_hartley(dstX_, dstY_, numPoints_, &dstTX_, &dstTY_, &dstScale_));

	/* Build transformation matrixes (T1 and T2) using the translation and scaling values from the normalization process */
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

	// Build M for homogeneous equation: Mh = 0
	CompVPtrArray(T) M_;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::buildHomographyEqMatrix(srcn_->ptr(0), srcn_->ptr(1), dstn_->ptr(0), dstn_->ptr(1), M_, numPoints_));

	// Build symmetric matrix S = M*M
	CompVPtrArray(T) S_; // temp symmetric array
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAtA(M_, S_));

	// Find eigenvalues and eigenvectors (no sorting and vectors in rows instead of columns)
	CompVPtrArray(T) D_; // 9x9 diagonal matrix containing the eigenvalues
	CompVPtrArray(T) Qt_; // 9x9 matrix containing the eigenvectors (rows) - transposed
	COMPV_CHECK_CODE_RETURN(err_ = CompVEigen<T>::findSymm(S_, D_, Qt_, false, true, false));
	// Find index of the smallest eigenvalue (this code is required because findSymm() is called without sorting for speed-up)
	// Eigenvector corresponding to the smallest eigenvalue is the nullspace of M and equal h (homogeneous equation: Ah = 0)
	signed minIndex_ = 8;
	T minEigenValue_ = *D_->ptr(8);
	for (signed j = 7; j >= 0; --j) { // starting at the end as the smallest value is probably there
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

	// change T1 = T1^
	row_ = const_cast<T*>(T1_->ptr(0)), row_[0] = srcScale_, row_[1] = 0, row_[2] = 0;
	row_ = const_cast<T*>(T1_->ptr(1)), row_[0] = 0, row_[1] = srcScale_, row_[2] = 0;
	row_ = const_cast<T*>(T1_->ptr(2)), row_[0] = -srcTX_ * srcScale_, row_[1] = -srcTY_ * srcScale_, row_[2] = 1;

	// change T2 = T2* - Matrix to obtain "a" from "b" (transformed by equation Ta=b)
	// -> b = as+t' (see above for t'=ts)
	// -> a = b(1/s)-t'(1/s) = b(1/s)+t'' whith t'' = -t'/s = -(ts)/s = -t
	// { 1 / s, 0, +tx },
	// { 0, 1 / s, +ty },
	// { 0, 0, 1 }
	row_ = const_cast<T*>(T2_->ptr(0)), row_[0] = 1 / dstScale_, row_[1] = 0, row_[2] = dstTX_;
	row_ = const_cast<T*>(T2_->ptr(1)), row_[0] = 0, row_[1] = 1 / dstScale_, row_[2] = dstTY_;
	row_ = const_cast<T*>(T2_->ptr(2)), row_[0] = 0, row_[1] = 0, row_[2] = 1;

	// De-normalize
	// HnAn = Bn, with An=T1A and Bn=T2B are normalized points
	// ->HnT1A = T2B
	// ->T2^HnT1A = T2^T2B = B
	// ->(T2^HnT1)A = B -> H'A = B whith H' = T2^HnT1 our final homography matrix
	// T2^HnT1 = T2^(T1*Hn*)* = T2^(T3Hn*)* with T3 = T1*
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulABt(T1_, H, M_));
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulABt(T2_, M_, H));

	if (promoteZeros) {
#define COMPV_PROMOTE_ZEROS(_h_, _i_) if (CompVEigen<T>::isCloseToZero((_h_)[(_i_)])) (_h_)[(_i_)] = 0;
		COMPV_PROMOTE_ZEROS(hn0_, 0); COMPV_PROMOTE_ZEROS(hn0_, 1); COMPV_PROMOTE_ZEROS(hn0_, 2);
		COMPV_PROMOTE_ZEROS(hn1_, 0); COMPV_PROMOTE_ZEROS(hn1_, 1); COMPV_PROMOTE_ZEROS(hn1_, 2);
		COMPV_PROMOTE_ZEROS(hn2_, 0); COMPV_PROMOTE_ZEROS(hn2_, 1);
	}

	// Scale H to make it homogeneous (Z = 1)
	T h22_ = hn2_[2] ? (T(1) / hn2_[2]) : T(1);
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
static COMPV_ERROR_CODE countInliers(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, const CompVPtrArray(T) &H, size_t &inliersCount, CompVPtrArray(size_t)& inliers, T &variance)
{
	// Private function, do not check input parameters
	size_t numPoints_ = src->cols();
	inliersCount = 0;
	variance = T(FLT_MAX);

	size_t* indexes_ = const_cast<size_t*>(inliers->ptr());

	// Apply H to the source and compute mse: Ha = b, mse(Ha, b)
	CompVPtrArray(T) b_;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAB(H, src, b_));
	COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::mse2D_homogeneous(b_->ptr(0), b_->ptr(1), b_->ptr(2), dst->ptr(0), dst->ptr(1), b_, numPoints_));


	// Apply H* to the destination and compute mse: a = H*b, mse(a, H*b)
	CompVPtrArray(T) a_;
	CompVPtrArray(T) Hinv_;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::pseudoinv(H, Hinv_)); // TODO(dmi): these are 3x3 matrixes -> add support for eigen_3x3 for speedup
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAB(Hinv_, dst, a_));
	COMPV_CHECK_CODE_RETURN(CompVMathStats<T>::mse2D_homogeneous(a_->ptr(0), a_->ptr(1), a_->ptr(2), src->ptr(0), src->ptr(1), a_, numPoints_));

	// Sum the MSE values and build the inliers
	const T* aPtr_ = a_->ptr(); // FIXME: remove
	const T* bPtr_ = b_->ptr();
	T sumd_ = 0; // sum deviations
	T d_;
	CompVPtrArray(T) distances_;
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&distances_, 1, numPoints_)); // "inliersCount" values only are needed but for now we don't now how many we have
	T* distancesPtr_ = const_cast<T*>(distances_->ptr());
	for (size_t i_ = 0; i_ < numPoints_; ++i_) {
		d_ = aPtr_[i_] + bPtr_[i_];
		if (d_ < COMPV_HOMOGRAPHY_OUTLIER_THRESHOLD) {
			indexes_[inliersCount] = i_;
			distancesPtr_[inliersCount] = d_;
			++inliersCount;
			sumd_ += d_;
		}
	}

	// Compute standard deviation (or variance)
	if (inliersCount > 1) {
		T mean_ = T(sumd_ / inliersCount);
		CompVMathStats<T>::variance(distancesPtr_, inliersCount, &mean_, &variance);
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()