/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_MATRIX_H_)
#define _COMPV_BASE_MATH_MATRIX_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMatrix
{
public:
	static COMPV_ERROR_CODE COMPV_DEPRECATED(mulAB)(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE COMPV_DEPRECATED(mulABt)(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE COMPV_DEPRECATED(mulAtA)(const CompVMatPtr &A, CompVMatPtrPtr R);
	template<typename FloatType>
	static COMPV_ERROR_CODE mulAG(CompVMatPtr &A, size_t ith, size_t jth, FloatType c, FloatType s);
	template<typename FloatType>
	static COMPV_ERROR_CODE mulGA(CompVMatPtr &A, size_t ith, size_t jth, FloatType c, FloatType s);
	template<typename FloatType>
	static COMPV_ERROR_CODE maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, FloatType* max);
	static COMPV_ERROR_CODE transpose(const CompVMatPtr &A, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE eigenS(const CompVMatPtr &S, CompVMatPtrPtr D, CompVMatPtrPtr Q, bool sort = true, bool rowVectors = false, bool forceZerosInD = true);
	static COMPV_ERROR_CODE svd(const CompVMatPtr &A, CompVMatPtrPtr U, CompVMatPtrPtr D, CompVMatPtrPtr V, bool sort = true);
	static COMPV_ERROR_CODE pseudoinv(const CompVMatPtr &A, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE invA3x3(const CompVMatPtr &A3x3, CompVMatPtrPtr R, bool pseudoInverseIfSingular = true, bool* isSingular = NULL);
	static COMPV_ERROR_CODE invD(const CompVMatPtr &D, CompVMatPtrPtr R, bool dIsSortedAndPositive = false);
	template<typename FloatType>
	static COMPV_ERROR_CODE givens(CompVMatPtrPtr G, size_t rows, size_t cols, size_t ith, size_t jth, FloatType c, FloatType s);

	template<typename T>
	static COMPV_ERROR_CODE identity(CompVMatPtrPtr I, size_t rows, size_t cols) {
		COMPV_CHECK_EXP_RETURN(!rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(I, rows, cols));
		
		COMPV_CHECK_CODE_RETURN((*I)->zero_rows());
		uint8_t* i0_ = (*I)->ptr<uint8_t>();
		static const T One = static_cast<T>(1);
		const size_t strideInBytes_ = (*I)->strideInBytes() + (*I)->elmtInBytes();
		const size_t maxRows_ = rows > cols ? cols : rows; /*COMPV_MATH_MIN(rows, cols)*/;
		for (size_t row_ = 0; row_ < maxRows_; ++row_) {
			*reinterpret_cast<T*>(i0_) = One;
			i0_ += strideInBytes_;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	template<typename T>
	static COMPV_ERROR_CODE zero(CompVMatPtrPtr Z, size_t rows, size_t cols)
	{
		COMPV_CHECK_EXP_RETURN(!rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(Z, rows, cols));
		COMPV_CHECK_CODE_RETURN((*Z)->zero_rows());
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE copy(CompVMatPtrPtr dst, const CompVMatPtr &src);
	static COMPV_ERROR_CODE rank(const CompVMatPtr &A, int &r, bool rowspace = true, size_t maxRows = 0, size_t maxCols = 0);
	static COMPV_ERROR_CODE isSymmetric(const CompVMatPtr &A, bool &symmetric);
	static COMPV_ERROR_CODE isEqual(const CompVMatPtr &A, const CompVMatPtr &B, bool &equal);
	static COMPV_ERROR_CODE isColinear(const CompVMatPtr &A, bool &colinear, bool rowspace = false, size_t maxRows = 0, size_t maxCols = 0);
	static COMPV_ERROR_CODE isColinear2D(const CompVMatPtr &A, bool &colinear);
	static COMPV_ERROR_CODE isColinear3D(const CompVMatPtr &A, bool &colinear);

	template<typename T>
	static COMPV_ERROR_CODE buildHomographyEqMatrix(CompVMatPtrPtr M, const T* srcX, const T* srcY, const T* dstX, const T* dstY, size_t numPoints);
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulAG(CompVMatPtr &A, size_t ith, size_t jth, compv_float32_t c, compv_float32_t s);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulAG(CompVMatPtr &A, size_t ith, size_t jth, compv_float64_t c, compv_float64_t s);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulGA(CompVMatPtr &A, size_t ith, size_t jth, compv_float32_t c, compv_float32_t s);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulGA(CompVMatPtr &A, size_t ith, size_t jth, compv_float64_t c, compv_float64_t s);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, compv_float32_t* max);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, compv_float64_t* max);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::givens(CompVMatPtrPtr G, size_t rows, size_t cols, size_t ith, size_t jth, compv_float32_t c, compv_float32_t s);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::givens(CompVMatPtrPtr G, size_t rows, size_t cols, size_t ith, size_t jth, compv_float64_t c, compv_float64_t s);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::buildHomographyEqMatrix(CompVMatPtrPtr M, const compv_float32_t* srcX, const compv_float32_t* srcY, const compv_float32_t* dstX, const compv_float32_t* dstY, size_t numPoints);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::buildHomographyEqMatrix(CompVMatPtrPtr M, const compv_float64_t* srcX, const compv_float64_t* srcY, const compv_float64_t* dstX, const compv_float64_t* dstY, size_t numPoints);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_MATRIX_H_ */

