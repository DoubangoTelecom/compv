/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_matrix_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void COMPV_DEPRECATED(CompVMathMatrixMulABt_64f_Intrin_SSE2)(const COMPV_ALIGNED(SSE) compv_float64_t* A, compv_uscalar_t aRows, COMPV_ALIGNED(SSE) compv_uscalar_t aStrideInBytes, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t bRows, compv_uscalar_t bCols, COMPV_ALIGNED(SSE) compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, COMPV_ALIGNED(SSE) compv_uscalar_t rStrideInBytes)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Not optimized at all and deprecated");
	const compv_float64_t* B0;
	compv_scalar_t k, bColsSigned = static_cast<compv_scalar_t>(bCols);

	compv_uscalar_t i, j;
	__m128d vec0, vec1, vec2, vec3, vecSum;
	
	for (i = 0; i < aRows; ++i) {
		B0 = B;
		for (j = 0; j < bRows; ++j) {
			vecSum = _mm_setzero_pd();
			for (k = 0; k < bColsSigned - 7; k += 8) {
				vec0 = _mm_mul_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0]));
				vec1 = _mm_mul_pd(_mm_load_pd(&A[k + 2]), _mm_load_pd(&B0[k + 2]));
				vec2 = _mm_mul_pd(_mm_load_pd(&A[k + 4]), _mm_load_pd(&B0[k + 4]));
				vec3 = _mm_mul_pd(_mm_load_pd(&A[k + 6]), _mm_load_pd(&B0[k + 6]));
				vec0 = _mm_add_pd(vec0, vec1);
				vec2 = _mm_add_pd(vec2, vec3);
				vecSum = _mm_add_pd(vecSum, vec0);
				vecSum = _mm_add_pd(vecSum, vec2);
			}
			if (k < bColsSigned - 3) {
				vec0 = _mm_mul_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0]));
				vec1 = _mm_mul_pd(_mm_load_pd(&A[k + 2]), _mm_load_pd(&B0[k + 2]));
				vec0 = _mm_add_pd(vec0, vec1);
				vecSum = _mm_add_pd(vecSum, vec0);
				k += 4;
			}
			if (k < bColsSigned - 1) {
				vecSum = _mm_add_pd(vecSum, _mm_mul_pd(_mm_load_pd(&A[k + 0]), _mm_load_pd(&B0[k + 0])));
				k += 2;
			}
			if (bColsSigned & 1) {
				vecSum = _mm_add_sd(vecSum, _mm_mul_sd(_mm_load_sd(&A[k + 0]), _mm_load_sd(&B0[k + 0])));
			}

			vecSum = _mm_add_pd(vecSum, _mm_shuffle_pd(vecSum, vecSum, 0x1));
			_mm_store_sd(&R[j], vecSum);

			B0 = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(B0) + bStrideInBytes);
		}
		A = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(A) + aStrideInBytes);
		R = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(R) + rStrideInBytes);
	}
}

// We'll read beyond the end of the data which means ri and rj must be strided and SSE-aligned
void CompVMathMatrixMulGA_64f_Intrin_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	__m128d vecRI0, vecRI1, vecRI2, vecRI3, vecRJ0, vecRJ1, vecRJ2, vecRJ3;
	compv_scalar_t i, countSigned = static_cast<compv_scalar_t>(count);

	const __m128d vecC = _mm_load1_pd(c1); // From Intel intrinsic guide _mm_load1_pd = 'movapd vec, m128' which is not correct, should be 'shufpd movsd, movsd, 0x0'
	const __m128d vecS = _mm_load1_pd(s1);

	// Case #8
	for (i = 0; i < countSigned - 7; i += 8) { // countSigned is equal to #9 for homography
		vecRI0 = _mm_load_pd(&ri[i + 0]);
		vecRI1 = _mm_load_pd(&ri[i + 2]);
		vecRI2 = _mm_load_pd(&ri[i + 4]);
		vecRI3 = _mm_load_pd(&ri[i + 6]);
		vecRJ0 = _mm_load_pd(&rj[i + 0]);
		vecRJ1 = _mm_load_pd(&rj[i + 2]);
		vecRJ2 = _mm_load_pd(&rj[i + 4]);
		vecRJ3 = _mm_load_pd(&rj[i + 6]);
		_mm_store_pd(&ri[i + 0], _mm_add_pd(_mm_mul_pd(vecRI0, vecC), _mm_mul_pd(vecRJ0, vecS)));
		_mm_store_pd(&ri[i + 2], _mm_add_pd(_mm_mul_pd(vecRI1, vecC), _mm_mul_pd(vecRJ1, vecS)));
		_mm_store_pd(&ri[i + 4], _mm_add_pd(_mm_mul_pd(vecRI2, vecC), _mm_mul_pd(vecRJ2, vecS)));
		_mm_store_pd(&ri[i + 6], _mm_add_pd(_mm_mul_pd(vecRI3, vecC), _mm_mul_pd(vecRJ3, vecS)));
		_mm_store_pd(&rj[i + 0], _mm_sub_pd(_mm_mul_pd(vecRJ0, vecC), _mm_mul_pd(vecRI0, vecS)));
		_mm_store_pd(&rj[i + 2], _mm_sub_pd(_mm_mul_pd(vecRJ1, vecC), _mm_mul_pd(vecRI1, vecS)));
		_mm_store_pd(&rj[i + 4], _mm_sub_pd(_mm_mul_pd(vecRJ2, vecC), _mm_mul_pd(vecRI2, vecS)));
		_mm_store_pd(&rj[i + 6], _mm_sub_pd(_mm_mul_pd(vecRJ3, vecC), _mm_mul_pd(vecRI3, vecS)));
	}

	// Case #4
	if (i < countSigned - 3) {
		vecRI0 = _mm_load_pd(&ri[i + 0]);
		vecRI1 = _mm_load_pd(&ri[i + 2]);
		vecRJ0 = _mm_load_pd(&rj[i + 0]);
		vecRJ1 = _mm_load_pd(&rj[i + 2]);
		_mm_store_pd(&ri[i + 0], _mm_add_pd(_mm_mul_pd(vecRI0, vecC), _mm_mul_pd(vecRJ0, vecS)));
		_mm_store_pd(&ri[i + 2], _mm_add_pd(_mm_mul_pd(vecRI1, vecC), _mm_mul_pd(vecRJ1, vecS)));
		_mm_store_pd(&rj[i + 0], _mm_sub_pd(_mm_mul_pd(vecRJ0, vecC), _mm_mul_pd(vecRI0, vecS)));
		_mm_store_pd(&rj[i + 2], _mm_sub_pd(_mm_mul_pd(vecRJ1, vecC), _mm_mul_pd(vecRI1, vecS)));
		i += 4;
	}

	// Cases #2 and #1
	for (; i < countSigned; i += 2) { // event if only #1 sample remains we can read beyond count (up to stride)
		vecRI0 = _mm_load_pd(&ri[i + 0]);
		vecRJ0 = _mm_load_pd(&rj[i + 0]);
		_mm_store_pd(&ri[i + 0], _mm_add_pd(_mm_mul_pd(vecRI0, vecC), _mm_mul_pd(vecRJ0, vecS)));
		_mm_store_pd(&rj[i + 0], _mm_sub_pd(_mm_mul_pd(vecRJ0, vecC), _mm_mul_pd(vecRI0, vecS)));
	}
}

void CompVMathMatrixBuildHomographyEqMatrix_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* dstX, const COMPV_ALIGNED(SSE) compv_float64_t* dstY, COMPV_ALIGNED(SSE) compv_float64_t* M, COMPV_ALIGNED(SSE) compv_uscalar_t M_strideInBytes, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_float64_t* M0_ptr = const_cast<compv_float64_t*>(M);
	compv_float64_t* M1_ptr = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(M0_ptr) + M_strideInBytes);
	const size_t M_strideInBytesTimes2 = M_strideInBytes << 1;
	const __m128d vecZero = _mm_setzero_pd();
	const __m128d vecMinusOne = _mm_load_pd(reinterpret_cast<const compv_float64_t*>(km1_64f));
	const __m128d vecMinusOneZero = _mm_load_pd(reinterpret_cast<const compv_float64_t*>(km1_0_64f));
	const __m128d vecMaskNegate = _mm_load_pd(reinterpret_cast<const compv_float64_t*>(kAVXFloat64MaskNegate));
	__m128d vecSrcXY, vecDstX, vecDstY;
	__m128d vecMinusXMinusY;

	for (compv_uscalar_t i = 0; i < numPoints; ++i) {
		vecSrcXY = _mm_unpacklo_pd(_mm_load_sd(&srcX[i]), _mm_load_sd(&srcY[i]));
		vecDstX = _mm_load_sd(&dstX[i]);
		vecDstY = _mm_load_sd(&dstY[i]);
		vecMinusXMinusY = _mm_xor_pd(vecSrcXY, vecMaskNegate); // -x, -y
		vecDstX = _mm_unpacklo_pd(vecDstX, vecDstX);
		vecDstY = _mm_unpacklo_pd(vecDstY, vecDstY);
		// First #9 contributions
		_mm_store_pd(&M0_ptr[0], vecMinusXMinusY); // -x, -y
		_mm_store_pd(&M0_ptr[2], vecMinusOneZero); // -1, 0
		_mm_store_pd(&M0_ptr[4], vecZero); // 0, 0
		_mm_store_pd(&M0_ptr[6], _mm_mul_pd(vecDstX, vecSrcXY)); // (dstX * srcX), (dstX * srcY)
		_mm_store_sd(&M0_ptr[8], vecDstX);
		// Second #9 contributions
		_mm_store_pd(&M1_ptr[0], vecZero); // 0, 0
		_mm_store_pd(&M1_ptr[2], _mm_unpacklo_pd(vecZero, vecMinusXMinusY)); // 0, -x
		_mm_store_pd(&M1_ptr[4], _mm_unpackhi_pd(vecMinusXMinusY, vecMinusOne)); // -y, -1
		_mm_store_pd(&M1_ptr[6], _mm_mul_pd(vecDstY, vecSrcXY)); // (dstY * srcX), (dstY * srcY)
		_mm_store_sd(&M1_ptr[8], vecDstY);
		// Move to the next point
		M0_ptr = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(M0_ptr) + M_strideInBytesTimes2);
		M1_ptr = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(M1_ptr) + M_strideInBytesTimes2);
	}
}

// A and R must have same stride
// This function returns det(A). If det(A) = 0 then, A is singluar and no inverse is computed.
void CompVMathMatrixInvA3x3_64f_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t strideInBytes, compv_float64_t* det1)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_float64_t* a0 = A;
	const compv_float64_t* a1 = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(a0) + strideInBytes);
	const compv_float64_t* a2 = reinterpret_cast<const compv_float64_t*>(reinterpret_cast<const uint8_t*>(a1) + strideInBytes);
	// det(A)
	__m128d vec0 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a1[1]), _mm_load_sd(&a0[1])), _mm_unpacklo_pd(_mm_load_sd(&a2[2]), _mm_load_sd(&a2[2])));
	__m128d vec1 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a2[1]), _mm_load_sd(&a2[1])), _mm_unpacklo_pd(_mm_load_sd(&a1[2]), _mm_load_sd(&a0[2])));
	__m128d vec2 = _mm_unpacklo_pd(_mm_load_sd(&a0[0]), _mm_load_sd(&a1[0]));
	__m128d vec3 = _mm_mul_pd(vec2, _mm_sub_pd(vec0, vec1));
	vec3 = _mm_sub_sd(vec3, _mm_shuffle_pd(vec3, vec3, 0x01));
	vec0 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a0[1]), _mm_load_sd(&a1[1])), _mm_unpacklo_pd(_mm_load_sd(&a1[2]), _mm_load_sd(&a0[2])));
	vec0 = _mm_sub_sd(vec0, _mm_shuffle_pd(vec0, vec0, 0x01));
	vec0 = _mm_mul_sd(vec0, _mm_load_sd(&a2[0]));
	compv_float64_t detA = _mm_cvtsd_f64(_mm_add_sd(vec0, vec3));
	if (detA == 0) {
		COMPV_DEBUG_INFO_EX("CompVMathMatrixInvA3x3_64f_Intrin_SSE2", "3x3 Matrix is singluar... computing pseudoinverse instead of the inverse");
	}
	else {
		__m128d vecDetA = _mm_set1_pd(1. / detA);
		compv_float64_t* r0 = R;
		compv_float64_t* r1 = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(r0) + strideInBytes);
		compv_float64_t* r2 = reinterpret_cast<compv_float64_t*>(reinterpret_cast<uint8_t*>(r1) + strideInBytes);

		vec0 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a1[1]), _mm_load_sd(&a0[2])), _mm_unpacklo_pd(_mm_load_sd(&a2[2]), _mm_load_sd(&a2[1])));
		vec1 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a2[1]), _mm_load_sd(&a2[2])), _mm_unpacklo_pd(_mm_load_sd(&a1[2]), _mm_load_sd(&a0[1])));
		_mm_store_pd(&r0[0], _mm_mul_pd(_mm_sub_pd(vec0, vec1), vecDetA));

		vec0 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a0[1]), _mm_load_sd(&a1[2])), _mm_unpacklo_pd(_mm_load_sd(&a1[2]), _mm_load_sd(&a2[0])));
		vec1 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a1[1]), _mm_load_sd(&a2[2])), _mm_unpacklo_pd(_mm_load_sd(&a0[2]), _mm_load_sd(&a1[0])));
		vec3 = _mm_mul_pd(_mm_sub_pd(vec0, vec1), vecDetA);
		_mm_store_sd(&r0[2], vec3);
		_mm_store_sd(&r1[0], _mm_shuffle_pd(vec3, vec3, 0x1));

		vec0 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a0[0]), _mm_load_sd(&a0[2])), _mm_unpacklo_pd(_mm_load_sd(&a2[2]), _mm_load_sd(&a1[0])));
		vec1 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a2[0]), _mm_load_sd(&a1[2])), _mm_unpacklo_pd(_mm_load_sd(&a0[2]), _mm_load_sd(&a0[0])));
		_mm_storeu_pd(&r1[1], _mm_mul_pd(_mm_sub_pd(vec0, vec1), vecDetA));

		vec0 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a1[0]), _mm_load_sd(&a0[1])), _mm_unpacklo_pd(_mm_load_sd(&a2[1]), _mm_load_sd(&a2[0])));
		vec1 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a2[0]), _mm_load_sd(&a2[1])), _mm_unpacklo_pd(_mm_load_sd(&a1[1]), _mm_load_sd(&a0[0])));
		_mm_store_pd(&r2[0], _mm_mul_pd(_mm_sub_pd(vec0, vec1), vecDetA));

		vec0 = _mm_mul_pd(_mm_unpacklo_pd(_mm_load_sd(&a0[0]), _mm_load_sd(&a1[0])), _mm_unpacklo_pd(_mm_load_sd(&a1[1]), _mm_load_sd(&a0[1])));
		vec0 = _mm_sub_sd(vec0, _mm_shuffle_pd(vec0, vec0, 0x01));
		_mm_store_sd(&r2[2], _mm_mul_sd(vec0, vecDetA));
	}
	*det1 = detA;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
