/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_eigen.h"

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
	// R = mul(A, B) = mulAB(A, B) = mulABt(A, Bt)
	static COMPV_ERROR_CODE mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
	{
		COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->isEmpty() || B->rows() != A->cols() || !B->cols() || *R == A || *R == B, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
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
	// R = mul(A, Bt)
	static COMPV_ERROR_CODE mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->subType() != B->subType() || A->isEmpty() || A->cols() != B->cols() || !B->cols() || *R == A || *R == B, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

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

	// R must be <> A
	// R = mul(At, A)
	static COMPV_ERROR_CODE mulAtA(const CompVMatPtr &A, CompVMatPtrPtr R)
	{
		COMPV_CHECK_EXP_RETURN(!A || !R || A->isEmpty() || *R == A, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		
		if (A->rows() == 3 && A->cols() == 3) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			// TODO(dmi): add support for mulAtA_3x3 -> no transpose
		}
		else if (A->rows() == 4 && A->cols() == 4) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			// TODO(dmi): add support for mulAtA_4x4 -> no transpose
		}

		// AtA = AtAtt = (At)(At)t = BBt, with B = At
		CompVMatPtr B;
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric::transpose(A, &B));
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric::mulABt(B, B, R));
		return COMPV_ERROR_CODE_S_OK;
	}
	
	// A = mul(A, GivensRotMatrix)
	// c: cos(theta)
	// s: sin(theta)
	// This function requires(ith > jth), always the case when dealing with symetric matrices
	// If A is symmetric then, mulAG(c, s) = mulGA(c, -s)
	// Otherwise, mulAG(A, c, s) = mulGA(At, c, -s)
	// Not thread-safe
	static COMPV_ERROR_CODE mulAG(CompVMatPtr &A, size_t ith, size_t jth, T c, T s)
	{
		// Input parameters checked in the calling function

		// This function isn't optimized and cannot be multithreaded, you should use mulGA() instead.
		// Not SIMD-friendly
		// Not Cache-friendly
		// Not MT-friendly
		// AG = (GtAt)t, if A is symmetric then = (GtA)t
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found. Also, no MT implementation.");

		// When Givens matrix is multiplied to the right of a matrix then, all rows change
		// -> this function cannot be multi-threaded and isn't (SIMD/Cache)-friendly

		size_t row_, rows_ = A->rows();
		T* a;
		T ai, aj;
		for (row_ = 0; row_ < rows_; ++row_) { // we don't need all these muls
			a = A->ptr<T>(row_);
			ai = a[ith] * c - a[jth] * s;
			aj = a[ith] * s + a[jth] * c;
			a[ith] = ai;
			a[jth] = aj;
		}

		return COMPV_ERROR_CODE_S_OK;
	}

	// A = mul(GivensRotMatrix * A)
	// c: cos(theta)
	// s: sin(theta)
	// This function requires(ith > jth) always the case when dealing with symetric matrixes
	// This function can be used to compute mulGtA. mulGtA = mulGA(A, ith, jth, c, -s)
	// Thread-safe
	static COMPV_ERROR_CODE mulGA(CompVMatPtr &A, size_t ith, size_t jth, T c, T s)
	{
		// Input parameters checked in the calling function

		// When Givens matrix is multiplied to the left of a matrix then, only ith and jth rows change
		// -> this function could be multi-threaded

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found.");

		T* ri_ = A->ptr<T>(ith);
		T* rj_ = A->ptr<T>(jth);
		T ai, aj;
		size_t col_, cols_ = A->cols();
		for (col_ = 0; col_ < cols_; ++col_) {
			ai = ri_[col_] * c + s* rj_[col_];
			aj = ri_[col_] * -s + c* rj_[col_];
			ri_[col_] = ai;
			rj_[col_] = aj;
		}

		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, T* max)
	{
		// Input parameters checked in the calling function

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found.");

		size_t rowStart = 1;
		size_t rowEnd = S->rows();
		*row = *col = 0;
		*max = 0;

		T r0_ = 0, r1_;
		size_t i, j;
		size_t strideInBytes = S->strideInBytes();
		const T* S1_;
		const uint8_t* S0_ = S->ptr<const uint8_t>(rowStart);
		for (j = rowStart; j < rowEnd; ++j) {
			S1_ = reinterpret_cast<const T*>(S0_);
			for (i = 0; i < j; ++i) { // i stops at j because the matrix is symmetric, for asm unroll the loop
				if ((r1_ = std::abs(S1_[i])) > r0_) {
					r0_ = r1_;
					*row = j;
					*col = i;
				}
			}
			S0_ += strideInBytes;
		}
		*max = r0_;

		return COMPV_ERROR_CODE_S_OK;
	}

	// S must be symmetric matrix
	static COMPV_ERROR_CODE eigenS(const CompVMatPtr &S, CompVMatPtrPtr D, CompVMatPtrPtr Q, bool sort, bool rowVectors, bool forceZerosInD)
	{
		// Input parameters checked in the calling function
		COMPV_CHECK_CODE_RETURN(CompVMathEigen<T>::findSymm(S, D, Q, sort, rowVectors, forceZerosInD));
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE transpose(const CompVMatPtr &A, CompVMatPtrPtr R)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation and not multi-thread"); // TODO(dmi): do not print message for small matrices (e.g. (rows * cols) < 200)
		COMPV_CHECK_EXP_RETURN(!A || !R || A->isEmpty() || A == *R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

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

	static COMPV_ERROR_CODE isSymmetric(const CompVMatPtr &A, bool &symmetric)
	{
		COMPV_CHECK_EXP_RETURN(!A || A->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		if (A->rows() != A->cols()) {
			symmetric = false; // must be square
			return COMPV_ERROR_CODE_S_OK;
		}

		CompVMatPtr At;
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric::transpose(A, &At)); // transpose to make it SIMD-friendly
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric::isEqual(A, At, symmetric));
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE isEqual(const CompVMatPtr &A, const CompVMatPtr &B, bool &equal)
	{
		COMPV_CHECK_EXP_RETURN(!A || !B || A->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		
		if (A->rows() != B->rows() || A->cols() != B->cols()) {
			equal = false;
			return COMPV_ERROR_CODE_S_OK;
		}

		const size_t acols = A->cols();
		const size_t arows = A->rows();

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		const T *aptr, *bptr;
		size_t i, j;
		for (j = 0; j < arows; ++j) {
			aptr = A->ptr<const T>(j);
			bptr = B->ptr<const T>(j);
			for (i = 0; i < acols; ++i) {
				if (aptr[i] != bptr[i]) {
					equal = false;
					return COMPV_ERROR_CODE_S_OK;
				}
			}
		}
		equal = true;
		return COMPV_ERROR_CODE_S_OK;
	}
};

//
//	CompVMatrix
//

// R must be <> A,B
// R = mul(A, B) = mulAB(A, B) = mulABt(A, Bt)
COMPV_ERROR_CODE CompVMatrix::mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	CompVMatrixGenericInvoke(A->subType(), mulAB, A, B, R);
}

// R must be <> A,B
// R = mul(A, Bt)
COMPV_ERROR_CODE CompVMatrix::mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	CompVMatrixGenericInvoke(A->subType(), mulABt, A, B, R);
}

// R must be <> A
// R = mul(At, A)
COMPV_ERROR_CODE CompVMatrix::mulAtA(const CompVMatPtr &A, CompVMatPtrPtr R)
{
	CompVMatrixGenericInvoke(A->subType(), mulAtA, A, R);
}

// A = mul(A, GivensRotMatrix)
// c: cos(theta)
// s: sin(theta)
// This function requires(ith > jth), always the case when dealing with symetric matrices
// If A is symmetric then, mulAG(c, s) = mulGA(c, -s)
// Otherwise, mulAG(A, c, s) = mulGA(At, c, -s)
// Not thread-safe
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulAG(CompVMatPtr &A, size_t ith, size_t jth, compv_float32_t c, compv_float32_t s)
{
	COMPV_CHECK_EXP_RETURN(!A || A->isEmpty() || ith <= jth || ith > A->cols() || jth > A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (A->subType() != COMPV_SUBTYPE_RAW_FLOAT32) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT32' but found '%s'", CompVGetSubtypeString(A->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
	return CompVMatrixGeneric<compv_float32_t>::mulAG(A, ith, jth, c, s);
}

// A = mul(A, GivensRotMatrix)
// c: cos(theta)
// s: sin(theta)
// This function requires(ith > jth), always the case when dealing with symetric matrices
// If A is symmetric then, mulAG(c, s) = mulGA(c, -s)
// Otherwise, mulAG(A, c, s) = mulGA(At, c, -s)
// Not thread-safe
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulAG(CompVMatPtr &A, size_t ith, size_t jth, compv_float64_t c, compv_float64_t s)
{
	COMPV_CHECK_EXP_RETURN(!A || A->isEmpty() || ith <= jth || ith > A->cols() || jth > A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (A->subType() != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT64' but found '%s'", CompVGetSubtypeString(A->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
	return CompVMatrixGeneric<compv_float64_t>::mulAG(A, ith, jth, c, s);
}


// A = mul(GivensRotMatrix * A)
// c: cos(theta)
// s: sin(theta)
// This function requires(ith > jth) always the case when dealing with symetric matrixes
// This function can be used to compute mulGtA. mulGtA = mulGA(A, ith, jth, c, -s)
// Thread-safe
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulGA(CompVMatPtr &A, size_t ith, size_t jth, compv_float32_t c, compv_float32_t s)
{
	COMPV_CHECK_EXP_RETURN(!A || A->isEmpty() /*|| ith <= jth*/ || ith >= A->rows() || jth >= A->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (A->subType() != COMPV_SUBTYPE_RAW_FLOAT32) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT32' but found '%s'", CompVGetSubtypeString(A->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
	return CompVMatrixGeneric<compv_float32_t>::mulGA(A, ith, jth, c, s);
}

// A = mul(GivensRotMatrix * A)
// c: cos(theta)
// s: sin(theta)
// This function requires(ith > jth) always the case when dealing with symetric matrixes
// This function can be used to compute mulGtA. mulGtA = mulGA(A, ith, jth, c, -s)
// Thread-safe
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::mulGA(CompVMatPtr &A, size_t ith, size_t jth, compv_float64_t c, compv_float64_t s)
{
	COMPV_CHECK_EXP_RETURN(!A || A->isEmpty() /*|| ith <= jth*/ || ith >= A->rows() || jth >= A->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (A->subType() != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT64' but found '%s'", CompVGetSubtypeString(A->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
	return CompVMatrixGeneric<compv_float64_t>::mulGA(A, ith, jth, c, s);
}

// S must be symetric
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, compv_float32_t* max)
{
	COMPV_CHECK_EXP_RETURN(!S || S->rows() != S->cols() || !S->rows() || !row || !col || !max, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (S->subType() != COMPV_SUBTYPE_RAW_FLOAT32) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT32' but found '%s'", CompVGetSubtypeString(S->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
	return CompVMatrixGeneric<compv_float32_t>::maxAbsOffDiag_symm(S, row, col, max);
}

// S must be symetric
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, compv_float64_t* max)
{
	COMPV_CHECK_EXP_RETURN(!S || S->rows() != S->cols() || !S->rows() || !row || !col || !max, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (S->subType() != COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT64' but found '%s'", CompVGetSubtypeString(S->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
	return CompVMatrixGeneric<compv_float64_t>::maxAbsOffDiag_symm(S, row, col, max);
}

// R must be <> A
COMPV_ERROR_CODE CompVMatrix::transpose(const CompVMatPtr &A, CompVMatPtrPtr R) 
{ 
	CompVMatrixGenericInvoke(A->subType(), transpose, A, R); 
}

// S must be symmetric matrix
COMPV_ERROR_CODE CompVMatrix::eigenS(const CompVMatPtr &S, CompVMatPtrPtr D, CompVMatPtrPtr Q, bool sort COMPV_DEFAULT(true), bool rowVectors COMPV_DEFAULT(false), bool forceZerosInD COMPV_DEFAULT(true))
{
	COMPV_CHECK_EXP_RETURN(!S, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (S->subType() == COMPV_SUBTYPE_RAW_FLOAT32) {
		return CompVMatrixGeneric<compv_float32_t>::eigenS(S, D, Q, sort, rowVectors, forceZerosInD);
	}
	else if (S->subType() == COMPV_SUBTYPE_RAW_FLOAT64) {
		return CompVMatrixGeneric<compv_float64_t>::eigenS(S, D, Q, sort, rowVectors, forceZerosInD);
	}
	else {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT64' or 'COMPV_SUBTYPE_RAW_FLOAT32'  but found '%s'", CompVGetSubtypeString(S->subType()));
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
	}
}

COMPV_ERROR_CODE CompVMatrix::copy(CompVMatPtrPtr dst, const CompVMatPtr &src)
{
	COMPV_CHECK_EXP_RETURN(!src, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(src->copy(dst));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMatrix::isSymmetric(const CompVMatPtr &A, bool &symmetric)
{
	CompVMatrixGenericInvoke(A->subType(), isSymmetric, A, symmetric);
}

COMPV_ERROR_CODE CompVMatrix::isEqual(const CompVMatPtr &A, const CompVMatPtr &B, bool &equal)
{
	CompVMatrixGenericInvoke(A->subType(), isEqual, A, B, equal);
}

COMPV_NAMESPACE_END()