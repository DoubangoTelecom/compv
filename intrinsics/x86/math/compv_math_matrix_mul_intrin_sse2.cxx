/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_matrix_mul_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// We'll read beyond the end of the data which means ri and rj must be strided
void MatrixMulGA_float64_Intrin_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM

	__m128d xmmC, xmmS, xmmRI, xmmRJ;

	xmmC = _mm_load1_pd(c1);
	xmmS = _mm_load1_pd(s1);

	for (compv_uscalar_t i = 0; i < count; i += 2) { // more than count (up to stride)
		xmmRI = _mm_load_pd(&ri[i]);
		xmmRJ = _mm_load_pd(&rj[i]);
		_mm_store_pd(&ri[i], _mm_add_pd(_mm_mul_pd(xmmRI, xmmC), _mm_mul_pd(xmmRJ, xmmS)));
		_mm_store_pd(&rj[i], _mm_sub_pd(_mm_mul_pd(xmmRJ, xmmC), _mm_mul_pd(xmmRI, xmmS)));
	}
}

// We'll read beyond the end of the data which means ri and rj must be strided
void MatrixMulGA_float32_Intrin_SSE2(COMPV_ALIGNED(SSE) compv_float32_t* ri, COMPV_ALIGNED(SSE) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM

	__m128 xmmC, xmmS, xmmRI, xmmRJ;

	xmmC = _mm_load1_ps(c1);
	xmmS = _mm_load1_ps(s1);

	for (compv_uscalar_t i = 0; i < count; i += 4) { // more than count (up to stride)
		xmmRI = _mm_load_ps(&ri[i]);
		xmmRJ = _mm_load_ps(&rj[i]);
		_mm_store_ps(&ri[i], _mm_add_ps(_mm_mul_ps(xmmRI, xmmC), _mm_mul_ps(xmmRJ, xmmS)));
		_mm_store_ps(&rj[i], _mm_sub_ps(_mm_mul_ps(xmmRJ, xmmC), _mm_mul_ps(xmmRI, xmmS)));
	}
}

// We'll read beyond the end of the data which means S must be strided and washed
// S must be symmetric matrix
void MatrixMaxAbsOffDiagSymm_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* S, compv_uscalar_t *row, compv_uscalar_t *col, compv_float64_t* max, compv_uscalar_t rows, compv_uscalar_t strideInBytes)
{
	compv_uscalar_t i, j, tmpi, tmpj;
	__m128d xmmAbsMask, xmmAbs, xmmMax;
	int cmp;

	xmmAbsMask = _mm_set1_pd(-0.); // Sign bit (bit-63) to 1 (https://en.wikipedia.org/wiki/Double-precision_floating-point_format) and all other bites to 0
	xmmMax = _mm_setzero_pd();

	tmpi = 0, tmpj = 0;
	*row = *col = 0;
	*max = 0;
	
	// Find max value without worrying about the indexes
	const uint8_t* S0_ = reinterpret_cast<const uint8_t*>(S) + strideInBytes; // j start at row index 1
	const compv_float64_t* S1_;
	for (j = 1; j < rows; ++j) { // j = 0 is on diagonal
		S1_ = reinterpret_cast<const compv_float64_t*>(S0_);
		for (i = 0; i < j; i += 2) { // i stops at j because the matrix is symmetric
			//COMPV_DEBUG_INFO_CODE_FOR_TESTING();
			//if (i == 206 && j == 207) {
				//__m128d xmm0 = _mm_load_pd(&S1_[i]);
			//	int kaka = 0;
			//}
			//if (i == j - 1) { // 1 double will cross or touch the diagonal ?
			//	xmmAbs = _mm_andnot_pd(xmmAbsMask, _mm_load_sd(&S1_[i])); // abs(S)
			//}
			//else {
				xmmAbs = _mm_andnot_pd(xmmAbsMask, _mm_load_pd(&S1_[i])); // abs(S)
			//}
			cmp = _mm_movemask_pd(_mm_cmpgt_pd(xmmAbs, xmmMax)); // must be "cmpge" instead of "cmpgt" to make sure we'll not stop when we cross the column throught the diagonal
			if (cmp) {
				tmpi = i;
				tmpj = j;
				xmmMax = _mm_max_pd(xmmMax, xmmAbs);
			}
		}
		S0_ += strideInBytes;
	}

#if 1
	// Now find the indexes
	if (tmpi && tmpj) {
		__m128d xmm0 = _mm_shuffle_pd(xmmMax, xmmMax, 0x01); // move high double to the low position
		cmp = _mm_movemask_pd(_mm_cmpgt_pd(xmmMax, xmm0));
		*row = tmpj;
		if (cmp & 1) {
			// max in the first position
			*max = _mm_cvtsd_f64(xmmMax);
			*col = tmpi + 0;
		}
		else {
			// max in the second position
			*max = _mm_cvtsd_f64(xmm0);
			*col = tmpi + 1;
		}
		COMPV_DEBUG_INFO_CODE_FOR_TESTING();
		if (*col >= *row) { // reading upper triangle ?
			// swap
			compv_uscalar_t tmp = *row;
			*row = *col;
			*col = tmp;
		}
	}
#else
	__m128d xmm0 = _mm_shuffle_pd(xmmMax, xmmMax, 0x01); // move high double to the low position
	cmp = _mm_movemask_pd(_mm_cmpgt_pd(xmmMax, xmm0));
	*max = _mm_cvtsd_f64((cmp & 1) ? xmmMax : xmm0);
	S0_ = reinterpret_cast<const uint8_t*>(S)+strideInBytes; // j start at row index 1
	for (j = 1; j < rows; ++j) { // j = 0 is on diagonal
		S1_ = reinterpret_cast<const compv_float64_t*>(S0_);
		for (i = 0; i < j; i += 1) {
			if (::abs(S1_[i]) == *max) {
				*row = j;
				*col = i;
				goto done;
			}
		}
		S0_ += strideInBytes;
	}
done:;
#endif

		
#if 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	compv_uscalar_t a = 0, b = 0;
	compv_float64_t r0_ = 0, r1_;
	S0_ = reinterpret_cast<const uint8_t*>(S)+strideInBytes; // j start at row index 1
	for (j = 1; j < rows; ++j) { // j = 0 is on diagonal
		S1_ = reinterpret_cast<const compv_float64_t*>(S0_);
		for (i = 0; i < j; i += 1) {
			if ((r1_ = ::abs(S1_[i])) > r0_) {
				r0_ = r1_;
				a = j;
				b = i;
			}
		}
		S0_ += strideInBytes;
	}

	if (b != *col || a != *row || r0_ != *max) {
		int kaka = 0;
	}
#endif

}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
