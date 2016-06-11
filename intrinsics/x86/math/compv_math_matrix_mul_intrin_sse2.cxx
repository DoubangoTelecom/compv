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

void MatrixMulGA_float64_minpack2_Intrin_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM

	__m128d xmmC, xmmS, xmmRI, xmmRJ;
	compv_uscalar_t i;
	
	i = 0;
	count -= 2; // up to the caller to check that  count is >= 2

	xmmC = _mm_load1_pd(c1);
	xmmS = _mm_load1_pd(s1);

	do {
		xmmRI = _mm_load_pd(&ri[i]);
		xmmRJ = _mm_load_pd(&rj[i]);
		_mm_store_pd(&ri[i], _mm_add_pd(_mm_mul_pd(xmmRI, xmmC), _mm_mul_pd(xmmRJ, xmmS)));
		_mm_store_pd(&rj[i], _mm_sub_pd(_mm_mul_pd(xmmRJ, xmmC), _mm_mul_pd(xmmRI, xmmS)));
	} while ((i += 2) < count);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
