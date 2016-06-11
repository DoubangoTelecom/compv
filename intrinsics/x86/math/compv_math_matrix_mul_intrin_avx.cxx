/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/math/compv_math_matrix_mul_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void MatrixMulGA_float64_minpack4_Intrin_AVX(COMPV_ALIGNED(AVX2) compv_float64_t* ri, COMPV_ALIGNED(AVX2) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Use ASM which support FMA3
#if !defined(__AVX__)
	COMPV_DEBUG_INFO_CODE_AVX_SSE_MIX();
#endif
	_mm256_zeroupper();

	__m256d ymmC, ymmS, ymmRI, ymmRJ;
	compv_uscalar_t i;

	i = 0;
	count -= 4; // up to the caller to check that  count is >= 4

	ymmC = _mm256_broadcast_sd(c1);
	ymmS = _mm256_broadcast_sd(s1);

	do {
		ymmRI = _mm256_load_pd(&ri[i]);
		ymmRJ = _mm256_load_pd(&rj[i]);

#if 1
		_mm256_store_pd(&ri[i], _mm256_add_pd(_mm256_mul_pd(ymmRI, ymmC), _mm256_mul_pd(ymmRJ, ymmS)));
		_mm256_store_pd(&rj[i], _mm256_sub_pd(_mm256_mul_pd(ymmRJ, ymmC), _mm256_mul_pd(ymmRI, ymmS)));
#else // FMA3 disable (different MD5 result and not faster)
		COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // FMA3
		_mm256_store_pd(&ri[i], _mm256_fmadd_pd(ymmC, ymmRI, _mm256_mul_pd(ymmS, ymmRJ)));
		_mm256_store_pd(&rj[i], _mm256_fmsub_pd(ymmC, ymmRJ, _mm256_mul_pd(ymmS, ymmRI)));
#endif
	} while ((i += 4) < count);
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
