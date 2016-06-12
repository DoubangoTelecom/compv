/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/math/compv_math_matrix.h"
#include "compv/math/compv_math_eigen.h"
#include "compv/math/compv_math.h"
#include "compv/compv_mem.h"
#include "compv/compv_cpu.h"

#include "compv/intrinsics/x86/math/compv_math_matrix_mul_intrin_sse2.h"
#include "compv/intrinsics/x86/math/compv_math_matrix_mul_intrin_sse41.h"
#include "compv/intrinsics/x86/math/compv_math_matrix_mul_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void MatrixMulGA_float64_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv::compv_float64_t* ri, COMPV_ALIGNED(SSE) compv::compv_float64_t* rj, const compv::compv_float64_t* c1, const compv::compv_float64_t* s1, compv::compv_uscalar_t count);
COMPV_EXTERNC void MatrixMulGA_float32_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv::compv_float32_t* ri, COMPV_ALIGNED(SSE) compv::compv_float32_t* rj, const compv::compv_float32_t* c1, const compv::compv_float32_t* s1, compv::compv_uscalar_t count);
COMPV_EXTERNC void MatrixMulGA_float64_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv::compv_float64_t* ri, COMPV_ALIGNED(AVX) compv::compv_float64_t* rj, const compv::compv_float64_t* c1, const compv::compv_float64_t* s1, compv::compv_uscalar_t count);
COMPV_EXTERNC void MatrixMulGA_float32_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv::compv_float32_t* ri, COMPV_ALIGNED(AVX) compv::compv_float32_t* rj, const compv::compv_float32_t* c1, const compv::compv_float32_t* s1, compv::compv_uscalar_t count);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

COMPV_NAMESPACE_BEGIN()

template class CompVMatrix<int32_t >;
template class CompVMatrix<compv_float64_t >;
template class CompVMatrix<compv_float32_t >;
template class CompVMatrix<uint16_t >;
template class CompVMatrix<int16_t >;
template class CompVMatrix<uint8_t >;

// R must be <> A,B
// R = mul(A, B) = mulAB(A, B) = mulABt(A, B*)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulAB(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !A->rows() || !A->cols() || B->rows() != A->cols() || !B->cols() || R == A || R == B, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	// AB = AB**= A(B*)* = AC*, with C = B*
	CompVPtrArray(T) C;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::transpose(B, C));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(A, C, R));
	return COMPV_ERROR_CODE_S_OK;
}

// R must be <> A
// R = mul(A*, A)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulAtA(const CompVPtrArray(T) &A, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || R == A, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// A*A = A*A** = (A*)(A*)* = BB*, with B = A*
	CompVPtrArray(T) B;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::transpose(A, B));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(B, B, R));
	return COMPV_ERROR_CODE_S_OK;
}

// R must be <> A,B
// R = mul(A, B*)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulABt(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !A->rows() || !A->cols() || A->cols() != B->cols() || !B->cols() || R == A || R == B, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Should use mulAB_square, mulAB_3x3, mulAB_2x2, SIMD accelerated....

	size_t i, j, k, a_rows = A->rows(), b_rows = B->rows(), b_cols = B->cols();
	const T *a0, *b0;
	T *r0;
	T sum;

	// Create R if not already done
	if (!R || R->rows() != A->rows() || R->cols() != B->rows()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&R, A->rows(), B->rows()));
	}

	for (i = 0; i < a_rows; ++i) {
		a0 = A->ptr(i);
		r0 = const_cast<T*>(R->ptr(i));
		for (j = 0; j < b_rows; ++j) {
			b0 = B->ptr(j);
			sum = 0;
			for (k = 0; k < b_cols; ++k) {
				sum += a0[k] * b0[k]; // DotProduct
			}
			r0[j] = sum;
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

// A = mul(A, GivensRotMatrix)
// c: cos(theta)
// s: sin(theta)
// This function requires(ith > jth) always the case when dealing with symetric matrixes
// If A is symmetric then, mulAG(c, s) = mulGA(c, -s)
// Otherwise, mulAG(A, c, s) = mulGA(A*, c, -s)
// Not thread-safe
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulAG(CompVPtrArray(T) &A, size_t ith, size_t jth, T c, T s)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || ith <= jth || ith > A->cols() || jth > A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// This function isn't optimized and cannot be multithreaded, you should use mulGA() instead.
	// Not SIMD-friendly
	// Not Cache-friendly
	// Not MT-friendly
	// AG = (G*A*)*, if A is symmetric then = (G*A)*
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD

	// When Givens matrix is multiplied to the right of a matrix then, all rows change
	// -> this function cannot be multi-threaded and isn't (SIMD/Cache)-friendly

	size_t rows_ = A->rows();
	T* a;
	T ai, aj;
	for (size_t row_ = 0; row_ < rows_; ++row_) { // we don't need all these muls
		a = const_cast<T*>(A->ptr(row_));
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
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::mulGA(CompVPtrArray(T) &A, size_t ith, size_t jth, T c, T s)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() /*|| ith <= jth*/ || ith >= A->rows() || jth >= A->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// When Givens matrix is multiplied to the left of a matrix then, only ith and jth rows change
	// -> this function could be multi-threaded	
	
	T* ri_ = const_cast<T*>(A->ptr(ith));
	T* rj_ = const_cast<T*>(A->ptr(jth));
	T ai, aj;
	size_t cols_ = A->cols();

	if (std::is_same<T, compv_float64_t>::value) {
		void(*MatrixMulGA_float64)(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count) = NULL;
		if (A->isAlignedSSE()) {
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulGA_float64 = MatrixMulGA_float64_Intrin_SSE2));
				COMPV_EXEC_IFDEF_ASM_X86((MatrixMulGA_float64 = MatrixMulGA_float64_Asm_X86_SSE2));
			}
#if 0 // SSE2 faster
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulGA_float64 = MatrixMulGA_float64_Intrin_SSE41));
			}
#endif
		}
		if (A->isAlignedAVX()) {
			if (CompVCpu::isEnabled(compv::kCpuFlagAVX)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulGA_float64 = MatrixMulGA_float64_Intrin_AVX));
				COMPV_EXEC_IFDEF_ASM_X86((MatrixMulGA_float64 = MatrixMulGA_float64_Asm_X86_AVX));
			}
		}
		if (MatrixMulGA_float64) {
			MatrixMulGA_float64((compv_float64_t*)ri_, (compv_float64_t*)rj_, (const compv_float64_t*)&c, (const compv_float64_t*)&s, cols_);
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	else if (std::is_same<T, compv_float32_t>::value) {
		void(*MatrixMulGA_float32)(COMPV_ALIGNED(SSE) compv_float32_t* ri, COMPV_ALIGNED(SSE) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count) = NULL;
		if (A->isAlignedSSE()) {
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulGA_float32 = MatrixMulGA_float32_Intrin_SSE2));
				COMPV_EXEC_IFDEF_ASM_X86((MatrixMulGA_float32 = MatrixMulGA_float32_Asm_X86_SSE2));
			}
		}
		if (A->isAlignedAVX()) {
			if (CompVCpu::isEnabled(compv::kCpuFlagAVX)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulGA_float32 = MatrixMulGA_float32_Intrin_AVX));
				COMPV_EXEC_IFDEF_ASM_X86((MatrixMulGA_float32 = MatrixMulGA_float32_Asm_X86_AVX));
			}
		}
		if (MatrixMulGA_float32) {
			MatrixMulGA_float32((compv_float32_t*)ri_, (compv_float32_t*)rj_, (const compv_float32_t*)&c, (const compv_float32_t*)&s, cols_);
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	for (size_t col_ = 0; col_ < cols_; ++col_) {
		ai = ri_[col_] * c + s* rj_[col_];
		aj = ri_[col_] * -s + c* rj_[col_];
		ri_[col_] = ai;
		rj_[col_] = aj;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// R<>A
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::transpose(const CompVPtrArray(T) &A, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || A == R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD

	// Create A if not already done
	if (!R || R->rows() != A->cols() || R->cols() != A->rows()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&R, A->cols(), A->rows()));
	}
	const T* a_;
	T *r0_ = const_cast<T*>(R->ptr(0, 0));
	uint8_t* r1_;
	size_t rstride_ = R->strideInBytes();
	size_t rows_ = A->rows();
	size_t cols_ = A->cols();
	for (size_t row_ = 0; row_ < rows_; ++row_, ++r0_) {
		a_ = A->ptr(row_);
		r1_ = reinterpret_cast<uint8_t*>(r0_);
		for (size_t col_ = 0; col_ < cols_; ++col_, r1_ += rstride_) {
			*reinterpret_cast<T*>(r1_) = a_[col_];
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// S must be symmetric matrix
// 
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::eigenS(const CompVPtrArray(T) &S, CompVPtrArray(T) &D, CompVPtrArray(T) &Q, bool sort /*= true*/, bool rowVectors /*= false*/, bool forceZerosInD /*= true*/)
{
	COMPV_CHECK_CODE_RETURN(CompVEigen<T>::findSymm(S, D, Q, sort, rowVectors, forceZerosInD));
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::maxAbsOffDiag_symm(const CompVPtrArray(T) &S, size_t *row, size_t *col, T* max)
{
	COMPV_CHECK_EXP_RETURN(!S || S->rows() != S->cols() || !S->rows() || !row || !col || !max, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();  // SIMD

	// TODO(dmi): SIMD, compute lower triangle abs

	*row = *col = 0;
	*max = 0;

	T r0_ = 0, r1_;
	int i, j;
	int rows = (int)S->rows();
	const T* S_;
	for (j = 1; j < rows; ++j) {
		S_ = S->ptr(j);
		for (i = 0; i < j; ++i) { // i stops at j because the matrix is symmetric
			if ((r1_ = ::abs(S_[i])) > r0_) {
				r0_ = r1_;
				*row = j;
				*col = i;
			}
		}
	}
	*max = r0_;

	return COMPV_ERROR_CODE_S_OK;
}

// sort -> sort D and V
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::svd(const CompVPtrArray(T) &A, CompVPtrArray(T) &U, CompVPtrArray(T) &D, CompVPtrArray(T) &V, bool sort /*= true*/)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->cols() || !A->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtrArray(T) S_, D_;
	bool aIsSquare = (A->rows() == A->cols());
	bool dIsSortedAndPositive = sort;
	
	// D and V (columnspace)
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAtA(A, S_)); // A*A
	COMPV_CHECK_CODE_RETURN(CompVEigen<T>::findSymm(S_, aIsSquare ? D : D_, V, sort)); // output D is nxn matrix	
	
	if (aIsSquare) { // D is nxn and this is correct
		if (dIsSortedAndPositive) {
			T d_;
			for (size_t j = 0; j < D->rows(); ++j) {
				d_ = *D->ptr(j, j);
				if (!d_) {
					break;
				}
				*const_cast<T*>(D->ptr(j, j)) = (T)COMPV_MATH_SQRT(d_);
			}
		}
		else {
			for (size_t j = 0; j < D->rows(); ++j) {
				*const_cast<T*>(D->ptr(j, j)) = (T)COMPV_MATH_SQRT(*D->ptr(j, j));
			}
		}
	}
	else { // -> D must be mxn -> complete with zeros
		size_t rows = COMPV_MATH_MIN(A->rows(), D_->rows());
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::zero(D, A->rows(), A->cols()));
		if (dIsSortedAndPositive) {
			T d_;
			for (size_t j = 0; j < rows; ++j) {
				d_ = *D_->ptr(j, j);
				if (!d_) {
					break;
				}
				*const_cast<T*>(D->ptr(j, j)) = (T)COMPV_MATH_SQRT(d_);
			}
		}
		else {
			for (size_t j = 0; j < rows; ++j) {
				*const_cast<T*>(D->ptr(j, j)) = (T)COMPV_MATH_SQRT(*D_->ptr(j, j));
			}
		}
	}


	// A = UDV* -> AV = UD -> AVD^ = U
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::invD(D, D_, dIsSortedAndPositive));// D_ will contain inverse(D) = D^
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(V, D_, S_)); // transpose inverseOf(D) -> nop for square matrix
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAB(A, S_, U));

	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Moore%E2%80%93Penrose_pseudoinverse
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::pseudoinv(const CompVPtrArray(T) &A, CompVPtrArray(T) &R)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->cols() || !A->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtrArray(T) U, D, V;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::svd(A, U, D, V));

	// compute inverse (D), D already cleaned with zeros
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::invD(D, D, true)); // will be transposed later

	CompVPtrArray(T) B;
	// A^ = VD^U*
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(V, D, B));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(B, U, R));
	
	return COMPV_ERROR_CODE_S_OK;
}

// D must be diagonal matrix and could be equal to R
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::invD(const CompVPtrArray(T) &D, CompVPtrArray(T) &R, bool dIsSortedAndPositive /*= false*/)
{
	COMPV_CHECK_EXP_RETURN(!D || !D->cols() || !D->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (R != D) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::zero(R, D->rows(), D->cols()));
	}
	T v_;
	size_t dcount_ = COMPV_MATH_MIN(D->rows(), D->cols()); // Diagonal matrix could be rectangular
	if (dIsSortedAndPositive) {
		for (size_t j = 0; j < dcount_; ++j) {
			v_ = *D->ptr(j, j);
			if (!v_) {
				break;
			}
			*const_cast<T*>(R->ptr(j, j)) = (T)(1 / v_);
		}
	}
	else {
		for (size_t j = 0; j < dcount_; ++j) {
			v_ = *D->ptr(j, j);
			if (v_) {
				*const_cast<T*>(R->ptr(j, j)) = (T)(1 / v_);
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Build Givens rotation matrix
// c: cos(theta)
// s: sin(theta)
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::givens(CompVPtrArray(T) &G, size_t rows, size_t cols, size_t ith, size_t jth, T c, T s)
{
	// From https://en.wikipedia.org/wiki/Givens_rotation

	// Identity matrix
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::identity(G, rows, cols));

	// Gii = c
	*const_cast<T*>(G->ptr(ith, ith)) = c;
	// Gij = s
	*const_cast<T*>(G->ptr(ith, jth)) = s;
	// Gjj = c
	*const_cast<T*>(G->ptr(jth, jth)) = c;
	// Gji = -s
	*const_cast<T*>(G->ptr(jth, ith)) = -s;

	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::identity(CompVPtrArray(T) &I, size_t rows, size_t cols)
{
	COMPV_CHECK_EXP_RETURN(!rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!I || I->rows() != rows || I->cols() != cols) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&I, rows, cols));
	}
	COMPV_CHECK_CODE_RETURN(I->zero_rows());
	uint8_t* i0_ = (uint8_t*)I->ptr();
	size_t stride_ = I->strideInBytes() + I->elmtInBytes();
	for (size_t row_ = 0; row_ < rows; ++row_) {
		*((T*)i0_) = 1;
		i0_ += stride_;
	}

	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::zero(CompVPtrArray(T) &Z, size_t rows, size_t cols)
{
	COMPV_CHECK_EXP_RETURN(!rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!Z || Z->rows() != rows || Z->cols() != cols) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&Z, rows, cols));
	}
	COMPV_CHECK_CODE_RETURN(Z->zero_rows());
	return COMPV_ERROR_CODE_S_OK;
}

// resize and fill the missing elements with zeros
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::resize0(CompVPtrArray(T) &A, size_t rows, size_t cols)
{
	COMPV_CHECK_EXP_RETURN(!A || !rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	if (A->rows() == rows && A->cols() == cols) {
		return COMPV_ERROR_CODE_S_OK;
	}
	CompVPtrArray(T) B;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::zero(B, rows, cols));
	rows = COMPV_MATH_MIN(rows, A->rows());
	if (cols == A->cols()) {
		CompVMem::copy(const_cast<T*>(B->ptr()), A->ptr(), A->rowInBytes() * rows);
	}
	else {
		cols = COMPV_MATH_MIN(cols, A->cols());
		for (size_t j = 0; j < rows; ++j) {
			CompVMem::copy(const_cast<T*>(B->ptr(j)), A->ptr(j), A->rowInBytes());
		}
	}
	A = B;
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::copy(CompVPtrArray(T) &A, const CompVPtrArray(T) &B)
{
	COMPV_CHECK_EXP_RETURN(!B || !B->rows() || !B->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!A || A->rows() != B->rows() || A->cols() != B->cols()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&A, B->rows(), B->cols()));
	}
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::copy(const_cast<T*>(A->ptr()), B, A->alignV()));
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::rank(const CompVPtrArray(T) &A, int &r, bool rowspace /*= true*/, size_t maxRows /*= 0*/, size_t maxCols /*= 0*/)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols() || A->rows() < maxRows || A->cols() < maxCols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtrArray(T) B_;
	if (maxRows != A->rows() || maxRows != A->cols()) {
		if (maxRows == 0) {
			maxRows = A->rows();
		}
		if (maxCols == 0) {
			maxCols = A->cols();
		}
		COMPV_CHECK_CODE_RETURN(A->shrink(B_, maxRows, maxCols));
	}

	CompVPtrArray(T) S_;
	if (rowspace) {
		// Row-space: S = A*A
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulAtA(B_ ? B_ : A, S_));
	}
	else {
		// Column-space: S = AA*
		COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::mulABt(B_ ? B_ : A, B_ ? B_ : A, S_));
	}
	CompVPtrArray(T) D_;
	CompVPtrArray(T) Qt_;
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // D must be sorted
	COMPV_CHECK_CODE_RETURN(CompVEigen<T>::findSymm(S_, D_, Qt_, false)); // FIXME: must be sorted
	r = 0;
	size_t rows_ = D_->rows();
	for (size_t row_ = 0; row_ < rows_; ++row_) {
		if (!CompVEigen<T>::isCloseToZero(*D_->ptr(row_, row_))) {
			++r;
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::isSymmetric(const CompVPtrArray(T) &A, bool &symmetric)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SIMD
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (A->rows() != A->cols()) {
		symmetric = false; // must be square
		return COMPV_ERROR_CODE_S_OK;
	}

	CompVPtrArray(T) At;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::transpose(A, At)); // transpose to make it SIMD-friendly

	const T *r, *rt;
	for (size_t j = 0; j < A->rows(); ++j) {
		r = A->ptr(j);
		rt = At->ptr(j);
		for (size_t i = 0; i < A->cols(); ++i) {
			if (r[i] != rt[i]) {
				symmetric = false;
				return COMPV_ERROR_CODE_S_OK;
			}
		}
	}
	symmetric = true;
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::isColinear(const CompVPtrArray(T) &A, bool &colinear, bool rowspace /*= false*/, size_t maxRows /*= 0*/, size_t maxCols /*= 0*/)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (rowspace) {
		if (A->rows() < 3) {
			colinear = true;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	else {
		if (A->cols() < 3) {
			colinear = true;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	int rank;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::rank(A, rank, rowspace, maxRows, maxCols));
	colinear = (rank == 1);
	return COMPV_ERROR_CODE_S_OK;
}

// A is an array of MxN elements, each row represent a dimension (x or y) and each column represent a point.
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::isColinear2D(const CompVPtrArray(T) &A, bool &colinear)
{
	COMPV_CHECK_EXP_RETURN(!A || A->rows() < 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	// A could be 3xN array (if homogeneous) and this is why we force the number of rows to #2
	return CompVMatrix<T>::isColinear(A, colinear, false/*column-space*/, 2, A->cols());
}

// A is an array of MxN elements, each row represent a dimension (x or y or z) and each column represent a point.
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::isColinear3D(const CompVPtrArray(T) &A, bool &colinear)
{
	COMPV_CHECK_EXP_RETURN(!A || A->rows() < 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	// A could be 4xN array (if homogeneous) and this is why we force the number of rows to #2
	return CompVMatrix<T>::isColinear(A, colinear, false/*column-space*/, 3, A->cols());
}

COMPV_NAMESPACE_END()
