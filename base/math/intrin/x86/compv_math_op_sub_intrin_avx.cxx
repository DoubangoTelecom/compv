/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_op_sub_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathOpSubSubMul_32f32f32f_Intrin_AVX(
	COMPV_ALIGNED(AVX) const compv_float32_t* Aptr,
	const compv_float32_t* subVal1,
	const compv_float32_t* mulVal1,
	COMPV_ALIGNED(AVX) compv_float32_t* Rptr,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(AVX) const compv_uscalar_t Astride,
	COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride
)
{
#if 0 // TODO(dmi): No asm code yet
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
#endif
	COMPV_DEBUG_INFO_CHECK_AVX();
	const compv_uscalar_t width32 = width & -32;
	const __m256 vecSubVal = _mm256_broadcast_ss(subVal1);
	const __m256 vecMulVal = _mm256_broadcast_ss(mulVal1);
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width32; i += 32) {
			_mm256_store_ps(&Rptr[i], _mm256_mul_ps(_mm256_sub_ps(_mm256_load_ps(&Aptr[i]), vecSubVal), vecMulVal));
			_mm256_store_ps(&Rptr[i + 8], _mm256_mul_ps(_mm256_sub_ps(_mm256_load_ps(&Aptr[i + 8]), vecSubVal), vecMulVal));
			_mm256_store_ps(&Rptr[i + 16], _mm256_mul_ps(_mm256_sub_ps(_mm256_load_ps(&Aptr[i + 16]), vecSubVal), vecMulVal));
			_mm256_store_ps(&Rptr[i + 24], _mm256_mul_ps(_mm256_sub_ps(_mm256_load_ps(&Aptr[i + 24]), vecSubVal), vecMulVal));
		}
		for (; i < width; i += 8) {
			_mm256_store_ps(&Rptr[i], _mm256_mul_ps(_mm256_sub_ps(_mm256_load_ps(&Aptr[i]), vecSubVal), vecMulVal));
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
