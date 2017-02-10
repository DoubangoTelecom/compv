/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_matrix.h"

#define COMPV_THIS_CLASSNAME	"CompVMatrix"

COMPV_NAMESPACE_BEGIN()

//
//	CompVMatrixGeneric
//

template<class T>
class CompVMatrixGeneric
{
	friend class CompVMatrix;
	static COMPV_ERROR_CODE transpose(const CompVMatPtr &A, CompVMatPtrPtr R)
	{
		COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || (!R || A == *R), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// Create A if not already done
		if (!(*R) || (*R)->rows() != A->cols() || (*R)->cols() != A->rows()) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(R, A->cols(), A->rows()));
		}
		const T* a0_ = A->ptr<const T>();
		size_t rows_ = A->rows();
		signed cols_ = static_cast<signed>(A->cols());
		T *r0_ = (*R)->ptr<T>();
		size_t rstrideInElts_, row_;
		size_t astrideInElts_;
		T * r_;
		signed col_;
		COMPV_CHECK_CODE_RETURN((*R)->strideInElts(rstrideInElts_), "Failed to get stride in elts");
		COMPV_CHECK_CODE_RETURN(A->strideInElts(astrideInElts_), "Failed to get stride in elts");
		size_t rstrideInEltsTimes2_ = rstrideInElts_ << 1;
		size_t rstrideInEltsTimes3_ = rstrideInEltsTimes2_ + rstrideInElts_;
		size_t rstrideInEltsTimes4_ = rstrideInElts_ << 2;
		for (row_ = 0; row_ < rows_; ++row_) {
			r_ = r0_;
			for (col_ = 0; col_ < cols_ - 3; col_ += 4, r_ += rstrideInEltsTimes4_) {
				r_[0] = a0_[col_];
				r_[rstrideInElts_] = a0_[col_ + 1];
				r_[rstrideInEltsTimes2_] = a0_[col_ + 2];
				r_[rstrideInEltsTimes3_] = a0_[col_ + 3];
			}
			for (; col_ < cols_; ++col_, r_ += rstrideInElts_) {
				r_[0] = a0_[col_];
			}
			r0_ += 1;
			a0_ += astrideInElts_;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
};

#define CompVMatrixInvoke(Type)


//
//	CompVMatrix
//

COMPV_ERROR_CODE CompVMatrix::mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

// R must be <> A
COMPV_ERROR_CODE CompVMatrix::transpose(const CompVMatPtr &A, CompVMatPtrPtr R)
{
	switch (A->subType()) {
	case COMPV_SUBTYPE_RAW_INT8: return CompVMatrixGeneric<int8_t>::transpose(A, R);
	case COMPV_SUBTYPE_RAW_UINT8: return CompVMatrixGeneric<uint8_t>::transpose(A, R);
	case COMPV_SUBTYPE_RAW_INT16: return CompVMatrixGeneric<int16_t>::transpose(A, R);
	case COMPV_SUBTYPE_RAW_UINT16: return CompVMatrixGeneric<uint16_t>::transpose(A, R);
	case COMPV_SUBTYPE_RAW_INT32: return CompVMatrixGeneric<int32_t>::transpose(A, R);
	case COMPV_SUBTYPE_RAW_UINT32: return CompVMatrixGeneric<uint32_t>::transpose(A, R);
	case COMPV_SUBTYPE_RAW_FLOAT32: return CompVMatrixGeneric<compv_float32_t>::transpose(A, R);
	case COMPV_SUBTYPE_RAW_FLOAT64: return CompVMatrixGeneric<compv_float64_t>::transpose(A, R);
	default: 
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid generic type: %d", static_cast<int>(A->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
}

COMPV_NAMESPACE_END()