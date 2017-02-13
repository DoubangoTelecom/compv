/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_eigen.h"
#include "compv/base/math/compv_math_utils.h"

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
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Invalid generic type: %s", CompVGetSubtypeString(subtype)); \
			return COMPV_ERROR_CODE_E_INVALID_SUBTYPE; \
	}

#define CompVMatrixGenericFloat64Invoke(subtype, funame, ...) \
	if (subtype != COMPV_SUBTYPE_RAW_FLOAT64) { \
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT64' but found '%s'", CompVGetSubtypeString(subtype)); \
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE; \
	} \
	return CompVMatrixGeneric<compv_float64_t>::funame(__VA_ARGS__)

#define CompVMatrixGenericFloat32Invoke(subtype, funame, ...) \
	if (subtype != COMPV_SUBTYPE_RAW_FLOAT32) { \
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT32' but found '%s'", CompVGetSubtypeString(subtype)); \
		return COMPV_ERROR_CODE_E_INVALID_SUBTYPE; \
	} \
	return CompVMatrixGeneric<compv_float32_t>::funame(__VA_ARGS__)

#define CompVMatrixGenericFloatInvoke(subtype, funame, ...) \
	switch (subtype) { \
	case COMPV_SUBTYPE_RAW_FLOAT32: return CompVMatrixGeneric<compv_float32_t>::funame(__VA_ARGS__); \
	case COMPV_SUBTYPE_RAW_FLOAT64: return CompVMatrixGeneric<compv_float64_t>::funame(__VA_ARGS__); \
	default: \
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Subtype mismatch. Expecting 'COMPV_SUBTYPE_RAW_FLOAT64' or 'COMPV_SUBTYPE_RAW_FLOAT32'  but found '%s'", CompVGetSubtypeString(subtype)); \
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
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::transpose(B, &C));
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::mulABt(A, C, R));
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
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::transpose(A, &B));
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::mulABt(B, B, R));
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

	// sort -> sort D and V
	static COMPV_ERROR_CODE svd(const CompVMatPtr &A, CompVMatPtrPtr U, CompVMatPtrPtr D, CompVMatPtrPtr V, bool sort)
	{
		// Input parameters checked in the calling function
		CompVMatPtr S_, D_;
		bool aIsSquare = (A->rows() == A->cols());
		bool dIsSortedAndPositive = sort;

		// D and V (columnspace)
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::mulAtA(A, &S_)); // AtA
		COMPV_CHECK_CODE_RETURN(CompVMathEigen<T>::findSymm(S_, aIsSquare ? D : &D_, V, sort)); // output D is nxn matrix

		if (aIsSquare) { // D is nxn and this is correct
			if (dIsSortedAndPositive) {
				T d_;
				for (size_t j = 0; j < (*D)->rows(); ++j) {
					d_ = *(*D)->ptr<T>(j, j);
					if (!d_) {
						break;
					}
					*(*D)->ptr<T>(j, j) = static_cast<T>(COMPV_MATH_SQRT(d_));
				}
			}
			else {
				for (size_t j = 0; j < (*D)->rows(); ++j) {
					*(*D)->ptr<T>(j, j) = static_cast<T>(COMPV_MATH_SQRT(*(*D)->ptr<T>(j, j)));
				}
			}
		}
		else { // -> D must be mxn -> complete with zeros
			size_t rows = COMPV_MATH_MIN(A->rows(), D_->rows());
			COMPV_CHECK_CODE_RETURN(CompVMatrix::zero<T>(D, A->rows(), A->cols()));
			if (dIsSortedAndPositive) {
				T d_;
				for (size_t j = 0; j < rows; ++j) {
					d_ = *D_->ptr<T>(j, j);
					if (!d_) {
						break;
					}
					*(*D)->ptr<T>(j, j) = static_cast<T>(COMPV_MATH_SQRT(d_));
				}
			}
			else {
				for (size_t j = 0; j < rows; ++j) {
					*(*D)->ptr<T>(j, j) = static_cast<T>(COMPV_MATH_SQRT(*D_->ptr<T>(j, j)));
				}
			}
		}

		// A = UDVt -> AV = UD -> AVDi = U
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::invD(*D, &D_, dIsSortedAndPositive));// D_ will contain inverse(D) = Di
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::mulABt(*V, D_, &S_)); // transpose inverseOf(D) -> nop for square matrix
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::mulAB(A, S_, U));

		return COMPV_ERROR_CODE_S_OK;
	}

	// Moore–Penrose: https://en.wikipedia.org/wiki/Moore%E2%80%93Penrose_pseudoinverse
	static COMPV_ERROR_CODE pseudoinv(const CompVMatPtr &A, CompVMatPtrPtr R)
	{
		// Input parameters checked in the calling function

		//!\\ Do not try to check size and call "invA3x3" -> undless loop when matrix is singular ("invA3x3" will call "pseudoinv")

		CompVMatPtr U, D, V;
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::svd(A, &U, &D, &V, true));

		// compute inverse (D), D already cleaned with zeros
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::invD(D, &D, true)); // will be transposed later

		CompVMatPtr B;
		// A^ = VDiUt
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::mulABt(V, D, &B));
		COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::mulABt(B, U, R));

		return COMPV_ERROR_CODE_S_OK;
	}

	// This function will return the pseudoinv if A is singular
	static COMPV_ERROR_CODE invA3x3(const CompVMatPtr &A3x3, CompVMatPtrPtr R)
	{
		// Input parameters checked in the calling function

		// Create R if not already done
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(R, 3, 3));
		bool hasSIMD = false;
		const char* nameSIMD = NULL;
		const T* a0 = A3x3->ptr<const T>(0);
		T* r0 = (*R)->ptr<T>(0);

#if 0
		if (std::is_same<T, compv_float64_t>::value) {
			void(*MatrixInvA3x3_64f)(const COMPV_ALIGNED(X) compv_float64_t* A3x3, COMPV_ALIGNED(X) compv_float64_t* R, compv_uscalar_t strideInBytes, compv_float64_t* det1) = NULL;
			if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && A3x3->isAlignedSSE() && R->isAlignedSSE() && A3x3->strideInBytes() == R->strideInBytes()) {
				COMPV_EXEC_IFDEF_INTRIN_X86((MatrixInvA3x3_64f = MatrixInvA3x3_64f_Intrin_SSE2, hasSIMD = true, nameSIMD = "MatrixInvA3x3_64f_Intrin_SSE2"));
			}
			if (MatrixInvA3x3_64f) {
				compv_float64_t detA;
				MatrixInvA3x3_64f((const compv_float64_t*)a0, (compv_float64_t*)r0, (compv_uscalar_t)A3x3->strideInBytes(), &detA);
				if (detA != 0) {
					return COMPV_ERROR_CODE_S_OK; // Matrix not singular -> break process
				}
			}
		}
#endif

		if (hasSIMD) {
			// Matrix is singular (detA == 0)
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "3x3 Matrix is singluar according to '%s'... computing pseudoinverse instead of the inverse", nameSIMD);
			COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::pseudoinv(A3x3, R));
		}
		else {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found.");
			// http://mathworld.wolfram.com/MatrixInverse.html
			const T* a1 = A3x3->ptr<const T>(1);
			const T* a2 = A3x3->ptr<const T>(2);
			// det(A)
			T detA =
				a0[0] * (a1[1] * a2[2] - a2[1] * a1[2])
				- a1[0] * (a0[1] * a2[2] - a2[1] * a0[2])
				+ a2[0] * (a0[1] * a1[2] - a1[1] * a0[2]);
			if (detA == 0) {
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "3x3 Matrix is singluar... computing pseudoinverse instead of the inverse");
				COMPV_CHECK_CODE_RETURN(CompVMatrixGeneric<T>::pseudoinv(A3x3, R));
			}
			else {
				detA = static_cast<T>(1 / detA);
				T* r1 = (*R)->ptr<T>(1);
				T* r2 = (*R)->ptr<T>(2);
				r0[0] = ((a1[1] * a2[2]) - (a2[1] * a1[2])) * detA;
				r0[1] = ((a0[2] * a2[1]) - (a2[2] * a0[1])) * detA;
				r0[2] = ((a0[1] * a1[2]) - (a1[1] * a0[2])) * detA;

				r1[0] = ((a1[2] * a2[0]) - (a2[2] * a1[0])) * detA;
				r1[1] = ((a0[0] * a2[2]) - (a2[0] * a0[2])) * detA;
				r1[2] = ((a0[2] * a1[0]) - (a1[2] * a0[0])) * detA;

				r2[0] = ((a1[0] * a2[1]) - (a2[0] * a1[1])) * detA;
				r2[1] = ((a0[1] * a2[0]) - (a2[1] * a0[0])) * detA;
				r2[2] = ((a0[0] * a1[1]) - (a1[0] * a0[1])) * detA;
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// D must be diagonal matrix and could be equal to R
	static COMPV_ERROR_CODE invD(const CompVMatPtr &D, CompVMatPtrPtr R, bool dIsSortedAndPositive)
	{
		COMPV_CHECK_EXP_RETURN(!D || !R || !D->cols() || !D->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (*R != D) {
			COMPV_CHECK_CODE_RETURN(CompVMatrix::zero<T>(R, D->rows(), D->cols()));
		}
		CompVMatPtr R_ = *R;
		T v_;
		size_t dcount_ = COMPV_MATH_MIN(D->rows(), D->cols()); // Diagonal matrix could be rectangular
		if (dIsSortedAndPositive) {
			for (size_t j = 0; j < dcount_; ++j) {
				v_ = *D->ptr<T>(j, j);
				if (!v_) {
					break;
				}
				*R_->ptr<T>(j, j) = static_cast<T>(1 / v_);
			}
		}
		else {
			for (size_t j = 0; j < dcount_; ++j) {
				v_ = *D->ptr<T>(j, j);
				if (v_) {
					*R_->ptr<T>(j, j) = static_cast<T>(1 / v_);
				}
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// Build Givens rotation matrix
	// c: cos(theta)
	// s: sin(theta)
	static COMPV_ERROR_CODE givens(CompVMatPtrPtr G, size_t rows, size_t cols, size_t ith, size_t jth, T c, T s)
	{
		COMPV_CHECK_EXP_RETURN(!G || !rows || !cols || ith >= rows || jth >= cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// From https://en.wikipedia.org/wiki/Givens_rotation

		// Identity matrix
		COMPV_CHECK_CODE_RETURN(CompVMatrix::identity<T>(G, rows, cols));
		
		// Gii = c
		*(*G)->ptr<T>(ith, ith) = c;
		// Gij = s
		*(*G)->ptr<T>(ith, jth) = s;
		// Gjj = c
		*(*G)->ptr<T>(jth, jth) = c;
		// Gji = -s
		*(*G)->ptr<T>(jth, ith) = -s;

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

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

		const size_t acols = A->cols();
		const size_t arows = A->rows();

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


	// Build matrix M = Ah used to solve Ah = 0 homogeneous equation. A is an Nx9 matrix, h an 9x1 matrix.
	// This equation is used to compute H (3x3) such that "Ha = b", "a" = "src" points and "b" = destination points.
	// "src" and "dst" should be normized first.
	// "M" has numPointsTimes2 rows and 9 columns (each column is an value for h).
	// We need at least 4 points
	static COMPV_ERROR_CODE buildHomographyEqMatrix(CompVMatPtrPtr M, const T* srcX, const T* srcY, const T* dstX, const T* dstY, size_t numPoints)
	{
		COMPV_CHECK_EXP_RETURN(!M || !srcX || !srcY || !dstX || !dstY || numPoints < 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

		// Each point (x, y) contribute two rows in M which means has (2 x numPoints) rows
		// "h" is a vector representing H (3x3) and is a 9x1 vector. This means M has 9 columns.
		const size_t M_rows = 2 * numPoints;
		const size_t M_cols = 9;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(M, M_rows, M_cols));

		size_t i;
		const size_t M_strideInBytes = (*M)->strideInBytes();
		T* M0_ptr = (*M)->ptr<T>();

		// TODO(dmi): transpose M to make it more SIMD friendly
		
		T* M1_ptr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(M0_ptr) + M_strideInBytes);
		const size_t M_strideInBytesTimes2 = M_strideInBytes << 1;

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

			M0_ptr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(M0_ptr) + M_strideInBytesTimes2);
			M1_ptr = reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(M1_ptr) + M_strideInBytesTimes2);
		}

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
	CompVMatrixGenericFloat32Invoke(A->subType(), mulAG, A, ith, jth, c, s);
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
	CompVMatrixGenericFloat64Invoke(A->subType(), mulAG, A, ith, jth, c, s);
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
	CompVMatrixGenericFloat32Invoke(A->subType(), mulGA, A, ith, jth, c, s);
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
	CompVMatrixGenericFloat64Invoke(A->subType(), mulGA, A, ith, jth, c, s);
}

// S must be symetric
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, compv_float32_t* max)
{
	COMPV_CHECK_EXP_RETURN(!S || S->rows() != S->cols() || !S->rows() || !row || !col || !max, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatrixGenericFloat32Invoke(S->subType(), maxAbsOffDiag_symm, S, row, col, max);
}

// S must be symetric
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::maxAbsOffDiag_symm(const CompVMatPtr &S, size_t *row, size_t *col, compv_float64_t* max)
{
	COMPV_CHECK_EXP_RETURN(!S || S->rows() != S->cols() || !S->rows() || !row || !col || !max, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatrixGenericFloat64Invoke(S->subType(), maxAbsOffDiag_symm, S, row, col, max);
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
	CompVMatrixGenericFloatInvoke(S->subType(), eigenS, S, D, Q, sort, rowVectors, forceZerosInD);
}

// sort -> sort D and V
COMPV_ERROR_CODE CompVMatrix::svd(const CompVMatPtr &A, CompVMatPtrPtr U, CompVMatPtrPtr D, CompVMatPtrPtr V, bool sort COMPV_DEFAULT(true))
{
	COMPV_CHECK_EXP_RETURN(!A || A->isEmpty() || !U || !D || !V, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatrixGenericFloatInvoke(A->subType(), svd, A, U, D, V, sort);
}

// Moore–Penrose: https://en.wikipedia.org/wiki/Moore%E2%80%93Penrose_pseudoinverse
COMPV_ERROR_CODE CompVMatrix::pseudoinv(const CompVMatPtr &A, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !A->cols() || !A->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatrixGenericFloatInvoke(A->subType(), pseudoinv, A, R);
}

// This function will return the pseudoinv if A is singular
COMPV_ERROR_CODE CompVMatrix::invA3x3(const CompVMatPtr &A3x3, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A3x3 || A3x3->cols() != 3 || A3x3->rows() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatrixGenericFloatInvoke(A3x3->subType(), invA3x3, A3x3, R);
}

// D must be diagonal matrix and could be equal to R
COMPV_ERROR_CODE CompVMatrix::invD(const CompVMatPtr &D, CompVMatPtrPtr R, bool dIsSortedAndPositive COMPV_DEFAULT(false))
{
	CompVMatrixGenericInvoke(D->subType(), invD, D, R, dIsSortedAndPositive);
}

// Build Givens rotation matrix
// c: cos(theta)
// s: sin(theta)
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::givens(CompVMatPtrPtr G, size_t rows, size_t cols, size_t ith, size_t jth, compv_float32_t c, compv_float32_t s)
{
	return CompVMatrixGeneric<compv_float32_t>::givens(G, rows, cols, ith, jth, c, s);
}

// Build Givens rotation matrix
// c: cos(theta)
// s: sin(theta)
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::givens(CompVMatPtrPtr G, size_t rows, size_t cols, size_t ith, size_t jth, compv_float64_t c, compv_float64_t s)
{
	return CompVMatrixGeneric<compv_float64_t>::givens(G, rows, cols, ith, jth, c, s);
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

// Build matrix M = Ah used to solve Ah = 0 homogeneous equation. A is an Nx9 matrix, h an 9x1 matrix.
// This equation is used to compute H (3x3) such that "Ha = b", "a" = "src" points and "b" = destination points.
// "src" and "dst" should be normized first.
// "M" has numPointsTimes2 rows and 9 columns (each column is an value for h).
// We need at least 4 points
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::buildHomographyEqMatrix(CompVMatPtrPtr M, const compv_float32_t* srcX, const compv_float32_t* srcY, const compv_float32_t* dstX, const compv_float32_t* dstY, size_t numPoints)
{
	return CompVMatrixGeneric<compv_float32_t>::buildHomographyEqMatrix(M, srcX, srcY, dstX, dstY, numPoints);
}

// Build matrix M = Ah used to solve Ah = 0 homogeneous equation. A is an Nx9 matrix, h an 9x1 matrix.
// This equation is used to compute H (3x3) such that "Ha = b", "a" = "src" points and "b" = destination points.
// "src" and "dst" should be normized first.
// "M" has numPointsTimes2 rows and 9 columns (each column is an value for h).
// We need at least 4 points
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMatrix::buildHomographyEqMatrix(CompVMatPtrPtr M, const compv_float64_t* srcX, const compv_float64_t* srcY, const compv_float64_t* dstX, const compv_float64_t* dstY, size_t numPoints)
{
	return CompVMatrixGeneric<compv_float64_t>::buildHomographyEqMatrix(M, srcX, srcY, dstX, dstY, numPoints);
}

COMPV_NAMESPACE_END()
