/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_matrix.h"

#define COMPV_THIS_CLASSNAME	"CompVMatrix"

COMPV_NAMESPACE_BEGIN()

#define CompVMatrixGenericInvoke(subtype, funame, ...) \
	switch (subtype) { \
		case COMPV_SUBTYPE_RAW_INT8: return CompVMatrixGeneric<int8_t>::funame(__VA_ARGS__); \
		case COMPV_SUBTYPE_RAW_UINT8: return CompVMatrixGeneric<uint8_t>::funame(__VA_ARGS__); \
		case COMPV_SUBTYPE_RAW_INT16: return CompVMatrixGeneric<int16_t>::funame(__VA_ARGS__); \
		case COMPV_SUBTYPE_RAW_UINT16: return CompVMatrixGeneric<uint16_t>::funame(__VA_ARGS__); \
		case COMPV_SUBTYPE_RAW_INT32: return CompVMatrixGeneric<int32_t>::funame(__VA_ARGS__); \
		case COMPV_SUBTYPE_RAW_UINT32: return CompVMatrixGeneric<uint32_t>::funame(__VA_ARGS__); \
		case COMPV_SUBTYPE_RAW_FLOAT32: return CompVMatrixGeneric<compv_float32_t>::funame(__VA_ARGS__); \
		case COMPV_SUBTYPE_RAW_FLOAT64: return CompVMatrixGeneric<compv_float64_t>::funame(__VA_ARGS__); \
		default: \
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid generic type: %d", static_cast<int>(subtype)); \
			return COMPV_ERROR_CODE_E_INVALID_SUBTYPE; \
	}

//
//	CompVMatrixGeneric
//

template<class T>
class CompVMatrixGeneric
{
	friend class CompVMatrix;

	// R must be <> A,B
	// R = mul(A, B) = mulAB(A, B) = mulABt(A, B*)
	static COMPV_ERROR_CODE mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
	{
		COMPV_CHECK_EXP_RETURN(!A || !B || !R || !A->rows() || !A->cols() || B->rows() != A->cols() || !B->cols() || (*R && (*R == A || *R == B)), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (A->rows() == 3 && A->cols() == 3 && B->rows() == 3 && B->cols() == 3) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			// TODO(dmi): add support for mulAB_3x3 -> no transpose
		}
		else if (A->rows() == 4 && A->cols() == 4 && B->rows() == 4 && B->cols() == 4) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			// TODO(dmi): add support for mulAB_4x4 -> no transpose
		}
		// AB = ABtt = A(Bt)t = ACt, with C = Bt
		CompVMatPtr C;
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric::transpose(B, &C));
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric::mulABt(A, C, R));
		return COMPV_ERROR_CODE_S_OK;
	}

	// R must be <> A,B
	// R = mul(A, B*)
	static COMPV_ERROR_CODE mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->subType() != B->subType() || !A->rows() || !A->cols() || A->cols() != B->cols() || !B->cols() || (*R && (*R == A || *R == B)), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		size_t i, j, k, aRows = A->rows(), bRows = B->rows(), bCols = B->cols();

		// Create R if not already done
		if (!*(R) || (*R)->rows() != aRows || (*R)->cols() != bRows || (*R)->subType() != A->subType()) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(R, A->rows(), B->rows()));
		}

		const T* aPtr = A->ptr<const T>();
		const T* bPtr = B->ptr<const T>();
		T* rPtr = (*R)->ptr<T>();
		size_t aStrideInBytes = A->strideInBytes();
		size_t bStrideInBytes = B->strideInBytes();
		size_t rStrideInBytes = (*R)->strideInBytes();
			
		T sum;
		const T* b0Ptr;
		for (i = 0; i < aRows; ++i) {
			b0Ptr = bPtr;
			for (j = 0; j < bRows; ++j) {
				sum = 0;
				for (k = 0; k < bCols; ++k) { // asm: unroll loop
					sum += aPtr[k] * b0Ptr[k]; // DotProduct
				}
				rPtr[j] = sum;
				b0Ptr = reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(b0Ptr) + bStrideInBytes);
			}
			aPtr = reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(aPtr) + aStrideInBytes);
			rPtr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(rPtr) + rStrideInBytes);
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE transpose(const CompVMatPtr &A, CompVMatPtrPtr R)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation and not multi-thread"); // TODO(dmi): do not print message for small matrices (e.g. (rows * cols) < 200)
		COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || (!R || A == *R), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// Create A if not already done
		if (!(*R) || (*R)->rows() != A->cols() || (*R)->cols() != A->rows() || (*R)->subType() != A->subType()) {
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

//
//	CompVMatrix
//

// R must be <> A,B
// R = mul(A, B) = mulAB(A, B) = mulABt(A, B*)
COMPV_ERROR_CODE CompVMatrix::mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	CompVMatrixGenericInvoke(A->subType(), mulAB, A, B, R);
}

// R must be <> A,B
// R = mul(A, B*)
COMPV_ERROR_CODE CompVMatrix::mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	CompVMatrixGenericInvoke(A->subType(), mulABt, A, B, R);
}

// R must be <> A
COMPV_ERROR_CODE CompVMatrix::transpose(const CompVMatPtr &A, CompVMatPtrPtr R) 
{ 
	CompVMatrixGenericInvoke(A->subType(), transpose, A, R); 
}

COMPV_NAMESPACE_END()