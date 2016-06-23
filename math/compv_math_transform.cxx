/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
/*
Most of trig approx. are implemented using document at "documentation/trig_approximations.pdf"
*/
#include "compv/math/compv_math_transform.h"
#include "compv/math/compv_math_matrix.h"

COMPV_NAMESPACE_BEGIN()

template class CompVTransform<compv_float64_t >;
template class CompVTransform<compv_float32_t >;

// src = homogeneous 2D coordinate (X, Y, 1)
// dst = cartesian 2D coordinate (x, y)
// src *must* be different than dst
// Operation: dst = mul(M, src)
// For Homography (M = H): src must be homogeneous coordinates
template<class T>
COMPV_ERROR_CODE CompVTransform<T>::perspective2D(const CompVPtrArray(T) &src, const CompVPtrArray(T) &M, CompVPtrArray(T) &dst)
{
	COMPV_CHECK_EXP_RETURN(!src || !M || !src->cols() || src->rows() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// TODO(dmi): #4 points too common

	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAB(M, src, dst));
	COMPV_CHECK_CODE_RETURN(CompVTransform<T>::homogeneousToCartesian2D(dst, dst));

	return COMPV_ERROR_CODE_S_OK;
}

// Change from 2D homogeneous coordinates (X, Y, Z) to cartesian 2D coordinates (x, y)
template<class T>
COMPV_ERROR_CODE CompVTransform<T>::homogeneousToCartesian2D(const CompVPtrArray(T) &src, CompVPtrArray(T) &dst)
{
	COMPV_CHECK_EXP_RETURN(!src || src->rows() != 3 || !src->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	size_t cols = src->cols();
	if (dst != src) { // do not override dst when "src == dst"
		if (!dst || dst->rows() != 2 || dst->cols() != cols || dst->alignV() != src->alignV()) {
			COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&dst, src->rows(), src->cols()));
		}
	}
	
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD, Special version with cols == 4
	const T* srcX = src->ptr(0);
	const T* srcY = src->ptr(1);
	const T* srcZ = src->ptr(2);
	T* dstX = const_cast<T*>(dst->ptr(0));
	T* dstY = const_cast<T*>(dst->ptr(1));
	T scale;
	for (size_t i = 0; i < cols; ++i) {
		scale = T(1) / srcZ[i]; // no point at infinity
		dstX[i] = srcX[i] * scale;
		dstY[i] = srcY[i] * scale;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
