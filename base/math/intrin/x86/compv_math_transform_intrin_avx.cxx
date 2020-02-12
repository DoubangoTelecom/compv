/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_transform_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathTransformHomogeneousToCartesian2D_4_64f_Intrin_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* srcX, const COMPV_ALIGNED(AVX) compv_float64_t* srcY, const COMPV_ALIGNED(AVX) compv_float64_t* srcZ, COMPV_ALIGNED(AVX) compv_float64_t* dstX, COMPV_ALIGNED(AVX) compv_float64_t* dstY, compv_uscalar_t numPoints)
{
	COMPV_DEBUG_INFO_CHECK_AVX();

	_mm256_zeroupper();
	const __m256d vec0 = _mm256_div_pd(_mm256_load_pd(k1_64f), _mm256_load_pd(&srcZ[0]));
	_mm256_store_pd(dstX, _mm256_mul_pd(_mm256_load_pd(srcX), vec0));
	_mm256_store_pd(dstY, _mm256_mul_pd(_mm256_load_pd(srcY), vec0));
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
