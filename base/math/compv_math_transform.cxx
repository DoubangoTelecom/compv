/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_transform.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_utils.h"

#define COMPV_THIS_CLASSNAME	"CompVMathTransform"

COMPV_NAMESPACE_BEGIN()

// src = homogeneous 2D coordinate (X, Y, 1)
// dst = cartesian 2D coordinate (x, y)
// src *must* be different than dst
// Operation: dst = mul(M, src)
// For Homography (M = H): src must be homogeneous coordinates
template<class T>
COMPV_ERROR_CODE CompVMathTransform<T>::perspective2D(const CompVMatPtr &src, const CompVMatPtr &M, CompVMatPtrPtr dst)
{
	COMPV_CHECK_EXP_RETURN(!src || !M || !dst || src->isEmpty() || M->isEmpty() || src->rows() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// TODO(dmi): #4 points too common

	COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(M, src, dst));
	COMPV_CHECK_CODE_RETURN(CompVMathTransform<T>::homogeneousToCartesian2D(*dst, dst));

	return COMPV_ERROR_CODE_S_OK;
}

// Change from 2D homogeneous coordinates (X, Y, Z) to cartesian 2D coordinates (x, y)
template<class T>
COMPV_ERROR_CODE CompVMathTransform<T>::homogeneousToCartesian2D(const CompVMatPtr &src, CompVMatPtrPtr dst)
{
	COMPV_CHECK_EXP_RETURN(!src || !dst || src->rows() != 3 || !src->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); 
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No fast SIMD x4 implementation found"); // SIMD, Special version with cols == 4 (see deprecated code)
	
	if (*dst != src) { // do not override dst when "src == dst"
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(dst, src->rows(), src->cols()));
	}
	const T* srcX = src->ptr<const T>(0);
	const T* srcY = src->ptr<const T>(1);
	const T* srcZ = src->ptr<const T>(2);
	T* dstX = (*dst)->ptr<T>(0);
	T* dstY = (*dst)->ptr<T>(1);	
	T scale;
	const size_t cols = src->cols();
	for (size_t i = 0; i < cols; ++i) {
		scale = static_cast<T>(1) / srcZ[i]; // z = 0 -> point at infinity (no division error is T is floating point numer)
		dstX[i] = srcX[i] * scale;
		dstY[i] = srcY[i] * scale;
	}

	return COMPV_ERROR_CODE_S_OK;
}

template class CompVMathTransform<compv_float64_t >;
template class CompVMathTransform<compv_float32_t >;

COMPV_NAMESPACE_END() 