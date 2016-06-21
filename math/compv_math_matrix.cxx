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
#include "compv/compv_engine.h"

#include "compv/intrinsics/x86/math/compv_math_matrix_intrin_sse2.h"
#include "compv/intrinsics/x86/math/compv_math_matrix_intrin_sse41.h"
#include "compv/intrinsics/x86/math/compv_math_matrix_intrin_avx.h"
#include "compv/intrinsics/x86/math/compv_math_matrix_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void MatrixMulGA_float64_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv::compv_float64_t* ri, COMPV_ALIGNED(SSE) compv::compv_float64_t* rj, const compv::compv_float64_t* c1, const compv::compv_float64_t* s1, compv::compv_uscalar_t count);
COMPV_EXTERNC void MatrixMulGA_float32_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv::compv_float32_t* ri, COMPV_ALIGNED(SSE) compv::compv_float32_t* rj, const compv::compv_float32_t* c1, const compv::compv_float32_t* s1, compv::compv_uscalar_t count);
COMPV_EXTERNC void MatrixMulGA_float64_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv::compv_float64_t* ri, COMPV_ALIGNED(AVX) compv::compv_float64_t* rj, const compv::compv_float64_t* c1, const compv::compv_float64_t* s1, compv::compv_uscalar_t count);
COMPV_EXTERNC void MatrixMulGA_float32_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv::compv_float32_t* ri, COMPV_ALIGNED(AVX) compv::compv_float32_t* rj, const compv::compv_float32_t* c1, const compv::compv_float32_t* s1, compv::compv_uscalar_t count);
COMPV_EXTERNC void MatrixMulABt_float64_minpack1_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* A, const COMPV_ALIGNED(SSE) compv::compv_float64_t* B, compv::compv_uscalar_t aRows, compv::compv_uscalar_t bRows, compv::compv_uscalar_t bCols, compv::compv_uscalar_t aStrideInBytes, compv::compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv::compv_float64_t* R, compv::compv_uscalar_t rStrideInBytes);
COMPV_EXTERNC void MatrixMulABt_float64_minpack1_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv::compv_float64_t* A, const COMPV_ALIGNED(AVX) compv::compv_float64_t* B, compv::compv_uscalar_t aRows, compv::compv_uscalar_t bRows, compv::compv_uscalar_t bCols, compv::compv_uscalar_t aStrideInBytes, compv::compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv::compv_float64_t* R, compv::compv_uscalar_t rStrideInBytes);
COMPV_EXTERNC void MatrixMulABt_float64_3x3_Asm_X86_SSE41(const COMPV_ALIGNED(AVX) compv::compv_float64_t* A, const COMPV_ALIGNED(AVX) compv::compv_float64_t* B, compv::compv_uscalar_t aRows, compv::compv_uscalar_t bRows, compv::compv_uscalar_t bCols, compv::compv_uscalar_t aStrideInBytes, compv::compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv::compv_float64_t* R, compv::compv_uscalar_t rStrideInBytes);
COMPV_EXTERNC void MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* S, compv::compv_uscalar_t *row, compv::compv_uscalar_t *col, compv::compv_float64_t* max, compv::compv_uscalar_t rowStart, compv::compv_uscalar_t rowEnd, compv::compv_uscalar_t strideInBytes);
COMPV_EXTERNC void MatrixBuildHomographyEqMatrix_float64_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv::compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv::compv_float64_t* dstX, const COMPV_ALIGNED(SSE) compv::compv_float64_t* dstY, COMPV_ALIGNED(SSE) compv::compv_float64_t* M, COMPV_ALIGNED(SSE)compv::compv_uscalar_t M_strideInBytes, compv::compv_uscalar_t numPoints);

#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void MatrixMulABt_float64_minpack1_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* A, const COMPV_ALIGNED(SSE) compv::compv_float64_t* B, compv::compv_uscalar_t aRows, compv::compv_uscalar_t bRows, compv::compv_uscalar_t bCols, compv::compv_uscalar_t aStrideInBytes, compv::compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv::compv_float64_t* R, compv::compv_uscalar_t rStrideInBytes);
COMPV_EXTERNC void MatrixMulABt_float64_minpack1_Asm_X64_AVX(const COMPV_ALIGNED(AVX) compv::compv_float64_t* A, const COMPV_ALIGNED(AVX) compv::compv_float64_t* B, compv::compv_uscalar_t aRows, compv::compv_uscalar_t bRows, compv::compv_uscalar_t bCols, compv::compv_uscalar_t aStrideInBytes, compv::compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv::compv_float64_t* R, compv::compv_uscalar_t rStrideInBytes);
COMPV_EXTERNC void MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* S, compv::compv_uscalar_t *row, compv::compv_uscalar_t *col, compv::compv_float64_t* max, compv::compv_uscalar_t rowStart, compv::compv_uscalar_t rowEnd, compv::compv_uscalar_t strideInBytes);
COMPV_EXTERNC void MatrixBuildHomographyEqMatrix_float64_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv::compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv::compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv::compv_float64_t* dstX, const COMPV_ALIGNED(SSE) compv::compv_float64_t* dstY, COMPV_ALIGNED(SSE) compv::compv_float64_t* M, COMPV_ALIGNED(SSE)compv::compv_uscalar_t M_strideInBytes, compv::compv_uscalar_t numPoints);

#endif /* COMPV_ARCH_X64 && COMPV_ASM */

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
	// TODO(dmi): add support for mulAB_3x3 and mulAB_4x4 -> no transpose
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
	// TODO(dmi): add support for mulAB_3x3 and mulAB_4x4 -> no transpose

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

	size_t i, j, k, aRows = A->rows(), bRows = B->rows(), bCols = B->cols();

	// Create R if not already done
	if (!R || R->rows() != aRows || R->cols() != bRows) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&R, A->rows(), B->rows()));
	}

	int minpack = 1;
	const T* aPtr = A->ptr();
	const T* bPtr = B->ptr();
	T* rPtr = const_cast<T*>(R->ptr());
	size_t aStrideInBytes = A->strideInBytes();
	size_t bStrideInBytes = B->strideInBytes();
	size_t rStrideInBytes = R->strideInBytes();
	
	if (std::is_same<T, compv_float64_t>::value) {
		void (*MatrixMulABt_float64)(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes) = NULL;
		if (bCols > 1 && A->isAlignedSSE() && B->isAlignedSSE() && R->isAlignedSSE()) {
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Intrin_SSE2, minpack = 1));
				COMPV_EXEC_IFDEF_ASM_X86((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Asm_X86_SSE2, minpack = 1));
				COMPV_EXEC_IFDEF_ASM_X64((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Asm_X64_SSE2, minpack = 1));
			}
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
				if (bCols == 3 && bRows == 3 && A->cols() == 3 && aRows == 3) {
					COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulABt_float64 = MatrixMulABt_float64_3x3_Intrin_SSE41, minpack = 1));
					COMPV_EXEC_IFDEF_ASM_X86((MatrixMulABt_float64 = MatrixMulABt_float64_3x3_Asm_X86_SSE41, minpack = 1));
				}
			}
		}
		if (bCols > 3 && A->isAlignedAVX() && B->isAlignedAVX() && R->isAlignedAVX()) {
			if (CompVCpu::isEnabled(compv::kCpuFlagAVX)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Intrin_AVX, minpack = 1));
				COMPV_EXEC_IFDEF_ASM_X86((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Asm_X86_AVX, minpack = 1));
				COMPV_EXEC_IFDEF_ASM_X64((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Asm_X64_AVX, minpack = 1));
#if 0 // FMA3 not faster (even slower)
				if (CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
					COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // FMA3 not faster
					COMPV_EXEC_IFDEF_INTRIN_X86((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Intrin_FMA3_AVX, minpack = 1));
					COMPV_EXEC_IFDEF_ASM_X86((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Asm_X86_FMA3_AVX, minpack = 1));
					COMPV_EXEC_IFDEF_ASM_X64((MatrixMulABt_float64 = MatrixMulABt_float64_minpack1_Asm_X64_FMA3_AVX, minpack = 1));
				}
#endif
			}
		}
		
		if (MatrixMulABt_float64) {
			MatrixMulABt_float64((const compv_float64_t*)aPtr, (const compv_float64_t*)bPtr, (compv_uscalar_t)aRows, (compv_uscalar_t)bRows, (compv_uscalar_t)bCols, (compv_uscalar_t)aStrideInBytes, (compv_uscalar_t)bStrideInBytes, (compv_float64_t*)rPtr, (compv_uscalar_t)rStrideInBytes);
			if (minpack == 1) { // SIMD can handle all data
				return COMPV_ERROR_CODE_S_OK;
			}
		}
	}
	
	if (minpack == 1) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
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
				b0Ptr = reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(b0Ptr)+bStrideInBytes);
			}
			aPtr = reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(aPtr)+aStrideInBytes);
			rPtr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(rPtr)+rStrideInBytes);
		}
	}
	else {
		size_t bColsStart = bCols - (bCols & (minpack - 1));
		if (bColsStart != bCols) {
			const T* b0Ptr;
			for (i = 0; i < aRows; ++i) {
				b0Ptr = bPtr;
				for (j = 0; j < bRows; ++j) {
					for (k = bColsStart; k < bCols; ++k) {
						rPtr[j] += aPtr[k] * b0Ptr[k]; // DotProduct
					}
					b0Ptr = reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(b0Ptr) + bStrideInBytes);
				}
				aPtr = reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(aPtr) + aStrideInBytes);
				rPtr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(rPtr) + rStrideInBytes);
			}
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

	void(*maxAbsOffDiag_symm_float64)(const COMPV_ALIGNED(SSE) compv_float64_t* S, compv_uscalar_t *row, compv_uscalar_t *col, compv_float64_t* max, compv_uscalar_t rowStart, compv_uscalar_t rowEnd, compv_uscalar_t strideInBytes) = NULL;

	size_t rowStart = 1;
	size_t rowEnd = S->rows();

	if (std::is_same<T, compv_float64_t>::value) {
		if (S->isAlignedSSE()) {
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((maxAbsOffDiag_symm_float64 = MatrixMaxAbsOffDiagSymm_float64_Intrin_SSE2));
				COMPV_EXEC_IFDEF_ASM_X86((maxAbsOffDiag_symm_float64 = MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE2));
				COMPV_EXEC_IFDEF_ASM_X64((maxAbsOffDiag_symm_float64 = MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE2));
			}
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
#if 0
				// SSE2 is slightly faster. Probably because SSE41 'ptest' instruction expect integers instead of doubles
				// According to [1], the xmm registers are flagged with the data type. [1] http://www.popoloski.com/posts/sse_move_instructions/
				COMPV_EXEC_IFDEF_ASM_X86((maxAbsOffDiag_symm_float64 = MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE41));
				COMPV_EXEC_IFDEF_ASM_X64((maxAbsOffDiag_symm_float64 = MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE41));
#endif
			}
		}
		if (maxAbsOffDiag_symm_float64) {
			compv_uscalar_t row_ = 0, col_ = 0;
			compv_float64_t max_ = 0;
			maxAbsOffDiag_symm_float64(reinterpret_cast<const compv_float64_t*>(S->ptr()), &row_, &col_, &max_, rowStart, rowEnd, (compv_uscalar_t)S->strideInBytes());
			*row = (size_t)row_;
			*col = (size_t)col_;
			*max = (T)max_;
			return COMPV_ERROR_CODE_S_OK;
		}
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	*row = *col = 0;
	*max = 0;

	T r0_ = 0, r1_;
	size_t i, j;
	size_t strideInBytes = S->strideInBytes();
	const T* S1_;
	const uint8_t* S0_ = reinterpret_cast<const uint8_t*>(S->ptr(rowStart));
	for (j = rowStart; j < rowEnd; ++j) {
		S1_ = reinterpret_cast<const T*>(S0_);
		for (i = 0; i < j; ++i) { // i stops at j because the matrix is symmetric, for asm unroll the loop
			if ((r1_ = ::abs(S1_[i])) > r0_) {
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
	size_t maxRows_ = COMPV_MATH_MIN(rows, cols);
	for (size_t row_ = 0; row_ < maxRows_; ++row_) {
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

// Set bytes from the diagonal (included) up to the alignment to zero
// S must be square
//!\\ First row is ignored
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::washDiag(const CompVPtrArray(T) &S, CompVPtrArray(T) &R, int alignv /*= -1*/)
{
	COMPV_CHECK_EXP_RETURN(!S || !S->rows() || !S->cols() || S->rows() != S->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	
	if (!R || R->rows() != S->rows() || R->cols() != S->cols()) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&R, S->rows(), S->cols()));
	}
	if (alignv <= 0) {
		alignv = (int)R->alignV();
	}

	size_t row_, col_, elts_ = R->elmtInBytes();
	const T* src_;
	T* dst_;

	COMPV_CHECK_EXP_RETURN((alignv % elts_), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t istep_ = (size_t)(alignv / elts_);
	size_t ibound_ = istep_;

	for (row_ = 1; row_ < R->rows(); ++row_) {
		src_ = S->ptr(row_);
		dst_ = const_cast<T*>(R->ptr(row_));
		for (col_ = 0; col_ < row_; ++col_) {
			dst_[col_] = src_[col_];
		}
		for (; col_ < ibound_; ++col_) {
			dst_[col_] = 0;
		}
		if (row_ == ibound_) {
			ibound_ += istep_;
		}
	}
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
	COMPV_CHECK_CODE_RETURN(CompVEigen<T>::findSymm(S_, D_, Qt_, false, true, false)); // no-sort, no-rowvectors, no-zeropromotion
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
	COMPV_CHECK_EXP_RETURN(!A || !A->rows() || !A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (A->rows() != A->cols()) {
		symmetric = false; // must be square
		return COMPV_ERROR_CODE_S_OK;
	}

	CompVPtrArray(T) At;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::transpose(A, At)); // transpose to make it SIMD-friendly
	COMPV_CHECK_CODE_RETURN(CompVMatrix<T>::isEqual(A, At, symmetric));
	return COMPV_ERROR_CODE_S_OK;
}

template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::isEqual(const CompVPtrArray(T) &A, const CompVPtrArray(T) &B, bool &equal)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !A->rows() || !A->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	equal = false;

	if (A->rows() != B->rows() || A->cols() != B->cols()) {
		return COMPV_ERROR_CODE_S_OK;
	}

	size_t cols_ = A->cols();

	// SIMD requires A and B to have same alignment -> use newObjAligned() to create the arrays

	if (A->alignV() == B->alignV()) {
		if (std::is_same<T, compv_float64_t>::value) {
			void(*MatrixIsEqual_float64)(const COMPV_ALIGNED(V) compv_float64_t* A, const COMPV_ALIGNED(V) compv_float64_t* B, compv_uscalar_t rows, compv_uscalar_t cols, compv_uscalar_t strideInBytes, compv_scalar_t *equal) = NULL;
			if (A->isAlignedSSE() && B->isAlignedSSE()) {
				if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
					COMPV_EXEC_IFDEF_INTRIN_X86((MatrixIsEqual_float64 = MatrixIsEqual_float64_Intrin_SSE2));
				}
			}
			if (A->isAlignedAVX() && B->isAlignedAVX()) {
				if (CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
					COMPV_EXEC_IFDEF_INTRIN_X86((MatrixIsEqual_float64 = MatrixIsEqual_float64_Intrin_AVX2));
				}
			}
			if (MatrixIsEqual_float64) {
				compv_scalar_t equal_ = 0;
				MatrixIsEqual_float64((const compv_float64_t*)A->ptr(), (const compv_float64_t*)B->ptr(), (compv_uscalar_t)A->rows(), (compv_uscalar_t)A->cols(), (compv_uscalar_t)A->strideInBytes(), &equal_);
				equal = (equal_ != 0);
				return COMPV_ERROR_CODE_S_OK;
			}
		}
	}
	
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	
	const T *a, *b;
	for (size_t j = 0; j < A->rows(); ++j) {
		a = A->ptr(j);
		b = B->ptr(j);
		for (size_t i = 0; i < A->cols(); ++i) {
			if (a[i] != b[i]) {
				return COMPV_ERROR_CODE_S_OK;
			}
		}
	}
	equal = true;
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

// Build matrix M = Ah used to solve Ah = 0 homogeneous equation. A is an Nx9 matrix, h an 9x1 matrix.
// This equation is used to compute H (3x3) such that "Ha = b", "a" = "src" points and "b" = destination points.
// "src" and "dst" should be normized first.
// "M" has numPointsTimes2 rows and 9 columns (each column is an value for h).
// We need at least 4 points
template <class T>
COMPV_ERROR_CODE CompVMatrix<T>::buildHomographyEqMatrix(const T* srcX, const T* srcY, const T* dstX, const T* dstY, CompVPtrArray(T)& M, size_t numPoints)
{
	COMPV_CHECK_EXP_RETURN(!srcX || !srcY || !dstX || !dstY || numPoints < 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Each point (x, y) contribute two rows in M which means has (2 x numPoints) rows
	// "h" is a vector representing H (3x3) and is a 9x1 vector. This means M has 9 columns.
	size_t M_rows = 2 * numPoints;
	size_t M_cols = 9;
	if (!M || M->rows() != M_rows || M->cols() != M_cols) {
		COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObjAligned(&M, M_rows, M_cols));
	}

	size_t i;
	size_t M_strideInBytes = M->strideInBytes();
	T* M0_ptr = const_cast<T*>(M->ptr());

	// TODO(dmi): transpose M to make it more SIMD friendly

	if (std::is_same<T, compv_float64_t>::value) {
		void (*MatrixBuildHomographyEqMatrix_float64)(const COMPV_ALIGNED(X) compv_float64_t* srcX, const COMPV_ALIGNED(X) compv_float64_t* srcY, const COMPV_ALIGNED(X) compv_float64_t* dstX, const COMPV_ALIGNED(X) compv_float64_t* dstY, COMPV_ALIGNED(X) compv_float64_t* M, COMPV_ALIGNED(X)compv_uscalar_t M_strideInBytes, compv_uscalar_t numPoints) = NULL;
		if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && numPoints > 1 && COMPV_IS_ALIGNED_SSE(srcX) && COMPV_IS_ALIGNED_SSE(srcY) && COMPV_IS_ALIGNED_SSE(dstY) && M->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(MatrixBuildHomographyEqMatrix_float64 = MatrixBuildHomographyEqMatrix_float64_Intrin_SSE2);
			COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // Intrin code faster
			//COMPV_EXEC_IFDEF_ASM_X86(MatrixBuildHomographyEqMatrix_float64 = MatrixBuildHomographyEqMatrix_float64_Asm_X86_SSE2);
			//COMPV_EXEC_IFDEF_ASM_X64(MatrixBuildHomographyEqMatrix_float64 = MatrixBuildHomographyEqMatrix_float64_Asm_X64_SSE2);
		}
		if (MatrixBuildHomographyEqMatrix_float64) {
			MatrixBuildHomographyEqMatrix_float64((const compv_float64_t*)srcX, (const compv_float64_t*)srcY, (const compv_float64_t*)dstX, (const compv_float64_t*)dstY, (compv_float64_t*)M0_ptr, (compv_uscalar_t)M_strideInBytes, (compv_uscalar_t)numPoints);
			return COMPV_ERROR_CODE_S_OK;
		}
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	T* M1_ptr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(M0_ptr)+M_strideInBytes);
	size_t M_strideInBytesTimes2 = M_strideInBytes << 1;
	
	for (i = 0; i < numPoints; ++i) {
		// z' = 1

		// First #9 contributions
		M0_ptr[0] = -srcX[i]; // -x
		M0_ptr[1] = -srcY[i]; // -y
		M0_ptr[2] = -1; // -1
		M0_ptr[3] = 0;
		M0_ptr[4] = 0;
		M0_ptr[5] = 0;
		M0_ptr[6] = (dstX[i] * srcX[i]); // (x'x)/z'
		M0_ptr[7] = (dstX[i] * srcY[i]); // (x'y)/z'
		M0_ptr[8] = dstX[i]; // x'/z'

		// Second #9 contributions
		M1_ptr[0] = 0;
		M1_ptr[1] = 0;
		M1_ptr[2] = 0;
		M1_ptr[3] = M0_ptr[0]; // -x
		M1_ptr[4] = M0_ptr[1]; // -y
		M1_ptr[5] = -1; // -1
		M1_ptr[6] = (dstY[i] * srcX[i]); // (y'x)/z'
		M1_ptr[7] = (dstY[i] * srcY[i]); // (y'y)/z'
		M1_ptr[8] = dstY[i]; // y'/z'

		M0_ptr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(M0_ptr)+M_strideInBytesTimes2);
		M1_ptr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(M1_ptr)+M_strideInBytesTimes2);
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
