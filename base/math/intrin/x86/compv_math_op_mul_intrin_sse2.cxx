/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_op_mul_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathOpMulMulABt_32f32f32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float32_t* Aptr,
	COMPV_ALIGNED(SSE) const compv_float32_t* Bptr,
	COMPV_ALIGNED(SSE) compv_float32_t* Rptr,
	const compv_uscalar_t Bcols,
	const compv_uscalar_t Arows,
	const compv_uscalar_t Brows,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Astride,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Use ASM code with support for AVX and FMA");
	const compv_uscalar_t Bcols16 = Bcols & -16;
	const compv_uscalar_t Bcols4 = Bcols & -4;
	compv_uscalar_t k;
	for (compv_uscalar_t i = 0; i < Arows; ++i) {
		const compv_float32_t* B0ptr = Bptr;
		for (compv_uscalar_t j = 0; j < Brows; ++j) {
			__m128 vec0 = _mm_setzero_ps();
			__m128 vec1 = _mm_setzero_ps();
			__m128 vec2 = _mm_setzero_ps();
			__m128 vec3 = _mm_setzero_ps();
			for (k = 0; k < Bcols16; k += 16) {
				vec0 = _mm_add_ps(vec0, _mm_mul_ps(_mm_load_ps(&Aptr[k]), _mm_load_ps(&B0ptr[k])));
				vec1 = _mm_add_ps(vec1, _mm_mul_ps(_mm_load_ps(&Aptr[k + 4]), _mm_load_ps(&B0ptr[k + 4])));
				vec2 = _mm_add_ps(vec2, _mm_mul_ps(_mm_load_ps(&Aptr[k + 8]), _mm_load_ps(&B0ptr[k + 8])));
				vec3 = _mm_add_ps(vec3, _mm_mul_ps(_mm_load_ps(&Aptr[k + 12]), _mm_load_ps(&B0ptr[k + 12])));
			}
			for (; k < Bcols4; k += 4) {
				vec0 = _mm_add_ps(vec0, _mm_mul_ps(_mm_load_ps(&Aptr[k]), _mm_load_ps(&B0ptr[k])));
			}
			vec0 = _mm_add_ps(vec0, vec1);
			vec2 = _mm_add_ps(vec2, vec3);
			vec0 = _mm_add_ps(vec0, vec2);
			vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E));
			vec0 = _mm_add_ss(vec0, _mm_shuffle_ps(vec0, vec0, 0x01));
			for (; k < Bcols; k += 1) {
				vec0 = _mm_add_ss(vec0, _mm_mul_ss(_mm_load_ss(&Aptr[k]), _mm_load_ss(&B0ptr[k])));
			}
			_mm_store_ss(&Rptr[j], vec0);
			B0ptr += Bstride;
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
