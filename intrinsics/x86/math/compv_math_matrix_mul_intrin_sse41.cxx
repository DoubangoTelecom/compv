/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_matrix_mul_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void MatrixMulGA_float64_minpack2_Intrin_SSE41(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // SSE2 faster

	__m128d xmmCS, xmmMSC, xmmRI, xmmRJ, xmmLow, xmmHigh;
	compv_uscalar_t i;

	i = 0;
	count -= 2; // up to the caller to check that  count is >= 2

	xmmCS = _mm_set_pd(*s1, *c1); // C S
	xmmMSC = _mm_set_pd(*c1, -*s1); // -S C

	do {
		xmmRI = _mm_load_pd(&ri[i]);
		xmmRJ = _mm_load_pd(&rj[i]);

		xmmLow = _mm_unpacklo_pd(xmmRI, xmmRJ);
		xmmHigh = _mm_unpackhi_pd(xmmRI, xmmRJ);

		_mm_store_pd(&ri[i], _mm_unpacklo_pd(_mm_dp_pd(xmmLow, xmmCS, 0xff), _mm_dp_pd(xmmHigh, xmmCS, 0xff)));
		_mm_store_pd(&rj[i], _mm_unpacklo_pd(_mm_dp_pd(xmmLow, xmmMSC, 0xff), _mm_dp_pd(xmmHigh, xmmMSC, 0xff)));
	} while ((i += 2) < count);
}

// aRows == bRows == bCols == 3
void MatrixMulABt_float64_3x3_Intrin_SSE41(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM, FMA3
	const uint8_t* a = reinterpret_cast<const uint8_t*>(A);
	const uint8_t* b = reinterpret_cast<const uint8_t*>(B);
	uint8_t* r = reinterpret_cast<uint8_t*>(R);

	__m128d xmmB00 = _mm_load_pd(B);
	__m128d xmmB01 = _mm_load_sd(&B[2]);
	__m128d xmmB10 = _mm_load_pd(reinterpret_cast<const compv_float64_t*>(&b[bStrideInBytes]));
	__m128d xmmB11 = _mm_load_sd(reinterpret_cast<const compv_float64_t*>(&b[bStrideInBytes + 16]));
	__m128d xmmB20 = _mm_load_pd(reinterpret_cast<const compv_float64_t*>(&b[bStrideInBytes << 1]));
	__m128d xmmB21 = _mm_load_sd(reinterpret_cast<const compv_float64_t*>(&b[(bStrideInBytes << 1) + 16]));

	__m128d xmmA00 = _mm_load_pd(A);
	__m128d xmmA01 = _mm_load_sd(&A[2]);
	__m128d xmm0 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB00, 0xff), _mm_mul_pd(xmmA01, xmmB01)); // FMA3
	__m128d xmm1 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB10, 0xff), _mm_mul_pd(xmmA01, xmmB11)); // FMA3
	__m128d xmm2 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB20, 0xff), _mm_mul_pd(xmmA01, xmmB21)); // FMA3
	__m128d xmm3 = _mm_shuffle_pd(xmm0, xmm1, 0x0);
	_mm_store_pd(&R[0], xmm3);
	_mm_store_sd(&R[2], xmm2);

	a += aStrideInBytes;
	r += rStrideInBytes;
	xmmA00 = _mm_load_pd(reinterpret_cast<const compv_float64_t*>(a));
	xmmA01 = _mm_load_sd(reinterpret_cast<const compv_float64_t*>(a) + 2);
	xmm0 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB00, 0xff), _mm_mul_pd(xmmA01, xmmB01)); // FMA3
	xmm1 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB10, 0xff), _mm_mul_pd(xmmA01, xmmB11)); // FMA3
	xmm2 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB20, 0xff), _mm_mul_pd(xmmA01, xmmB21)); // FMA3
	xmm3 = _mm_shuffle_pd(xmm0, xmm1, 0x0);
	_mm_store_pd(reinterpret_cast<compv_float64_t*>(r), xmm3);
	_mm_store_sd(reinterpret_cast<compv_float64_t*>(r) + 2, xmm2);

	a += aStrideInBytes;
	r += rStrideInBytes;
	xmmA00 = _mm_load_pd(reinterpret_cast<const compv_float64_t*>(a));
	xmmA01 = _mm_load_sd(reinterpret_cast<const compv_float64_t*>(a) + 2);
	xmm0 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB00, 0xff), _mm_mul_pd(xmmA01, xmmB01)); // FMA3
	xmm1 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB10, 0xff), _mm_mul_pd(xmmA01, xmmB11)); // FMA3
	xmm2 = _mm_add_pd(_mm_dp_pd(xmmA00, xmmB20, 0xff), _mm_mul_pd(xmmA01, xmmB21)); // FMA3
	xmm3 = _mm_shuffle_pd(xmm0, xmm1, 0x0);
	_mm_store_pd(reinterpret_cast<compv_float64_t*>(r), xmm3);
	_mm_store_sd(reinterpret_cast<compv_float64_t*>(r)+2, xmm2);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
