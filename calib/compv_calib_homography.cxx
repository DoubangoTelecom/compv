/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/calib/compv_calib_homography.h"
#include "compv/math/compv_math_matrix.h"

COMPV_NAMESPACE_BEGIN()

template class CompVHomography<double >;
template class CompVHomography<float >;

// src: 3-rows array (X, Y, Z=1). N-cols with N >= 4.
// dst: 3-rows array (X, Y, Z=1). N-cols with N >= 4.
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
	size_t numPoints_ = src_->cols();
	size_t i;

	// TODO(dmi): use calib class and store "srcn", "dstn"....

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
	// -> b = (a+t)*s = a*s+t*s = a*s+t' with t'= t*s
	// T matrix
	//	scale	0		-Tx*scale
	//	0		scale	-Tx*scale
	//	0		0		1
	CompVPtrArray(T) mat3x3_;
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<T>::newObj(&mat3x3_, 3, 3, COMPV_SIMD_ALIGNV_DEFAULT));
	// Normalize src_: srcn_ = T.src_
	CompVPtrArray(T) srcn_;
	row_ = const_cast<T*>(mat3x3_->ptr(0)), row_[0] = srcScale_, row_[1] = 1, row_[2] = srcTX_ * srcScale_;
	row_ = const_cast<T*>(mat3x3_->ptr(1)), row_[0] = 0, row_[1] = srcScale_, row_[2] = srcTY_ * srcScale_;
	row_ = const_cast<T*>(mat3x3_->ptr(2)), row_[0] = 0, row_[1] = 1, row_[2] = 1;
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(mat3x3_, src, srcn_));
	// Normilize dst_: dstn_ = T.dst_
	CompVPtrArray(T) dstn_;
	row_ = const_cast<T*>(mat3x3_->ptr(0)), row_[0] = dstScale_, row_[1] = 1, row_[2] = dstTX_ * srcScale_;
	row_ = const_cast<T*>(mat3x3_->ptr(1)), row_[0] = 0, row_[1] = dstScale_, row_[2] = dstTY_ * srcScale_;
	row_ = const_cast<T*>(mat3x3_->ptr(2)), row_[0] = 0, row_[1] = 1, row_[2] = 1;
	COMPV_CHECK_CODE_RETURN(err_ = CompVMatrix<T>::mulAB(mat3x3_, dst, dstn_));

	// Build homogeneous equation: Mh = 0

	// Build M (FIXME: z' = 1 in compv as we'll append it to Xprime)
	/*double M[2 * kNumPoints][9], Mt[9][2 * kNumPoints];
	for (int j = 0, k = 0; k < kNumPoints; j += 2, ++k) {
		M[j][0] = -Xn_[0][k]; // -x
		M[j][1] = -Xn_[1][k]; // -y
		M[j][2] = -1; // -1
		M[j][3] = 0;
		M[j][4] = 0;
		M[j][5] = 0;
		M[j][6] = (XnPrime_[0][k] * Xn_[0][k]) / XnPrime_[2][k]; // (x'x)/z'
		M[j][7] = (XnPrime_[0][k] * Xn_[1][k]) / XnPrime_[2][k]; // (x'y)/z'
		M[j][8] = XnPrime_[0][k] / XnPrime_[2][k]; // x'/z'

		M[j + 1][0] = 0;
		M[j + 1][1] = 0;
		M[j + 1][2] = 0;
		M[j + 1][3] = -Xn_[0][k]; // -x
		M[j + 1][4] = -Xn_[1][k]; // -y
		M[j + 1][5] = -1; // -1
		M[j + 1][6] = (XnPrime_[1][k] * Xn_[0][k]) / XnPrime_[2][k]; // (y'x)/z'
		M[j + 1][7] = (XnPrime_[1][k] * Xn_[1][k]) / XnPrime_[2][k]; // (y'y)/z'
		M[j + 1][8] = XnPrime_[1][k] / XnPrime_[2][k]; // y'/z'
	}*/
	

	// Inverse operation
	// -> b = a*s+t'
	// -> a = b*(1/s)-t'*(1/s) = b*(1/s)+t'' whith t'' = -t'/s = -(t*s)/s = -t
	//	scale	0		-Tx*scale
	//	0		scale	-Tx*scale
	//	0		0		1
	//const double invT2[3][3] = {
	//	{ 1 / s1, 0, t1x_ },
	//	{ 0, 1 / s1, t1y_ },
	//	{ 0, 0, 1 }
	//};



	return err_;
}

COMPV_NAMESPACE_END()