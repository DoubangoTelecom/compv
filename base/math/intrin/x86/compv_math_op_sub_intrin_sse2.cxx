/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_op_sub_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathOpSubSub_32f32f32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float32_t* Aptr,
	COMPV_ALIGNED(SSE) const compv_float32_t* Bptr,
	COMPV_ALIGNED(SSE) compv_float32_t* Rptr,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Astride,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_uscalar_t width16 = width & -16;
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			_mm_store_ps(&Rptr[i], _mm_sub_ps(_mm_load_ps(&Aptr[i]), _mm_load_ps(&Bptr[i])));
			_mm_store_ps(&Rptr[i + 4], _mm_sub_ps(_mm_load_ps(&Aptr[i + 4]), _mm_load_ps(&Bptr[i + 4])));
			_mm_store_ps(&Rptr[i + 8], _mm_sub_ps(_mm_load_ps(&Aptr[i + 8]), _mm_load_ps(&Bptr[i + 8])));
			_mm_store_ps(&Rptr[i + 12], _mm_sub_ps(_mm_load_ps(&Aptr[i + 12]), _mm_load_ps(&Bptr[i + 12])));
		}
		for (; i < width; i += 4) {
			_mm_store_ps(&Rptr[i], _mm_sub_ps(_mm_load_ps(&Aptr[i]), _mm_load_ps(&Bptr[i])));
		}
		Aptr += Astride;
		Bptr += Bstride;
		Rptr += Rstride;
	}
}

void CompVMathOpSubSubMul_32f32f32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float32_t* Aptr,
	const compv_float32_t* subVal1,
	const compv_float32_t* mulVal1,
	COMPV_ALIGNED(SSE) compv_float32_t* Rptr,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Astride,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_uscalar_t width16 = width & -16;
	const __m128 vecSubVal = _mm_load1_ps(subVal1);
	const __m128 vecMulVal = _mm_load1_ps(mulVal1);
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			_mm_store_ps(&Rptr[i], _mm_mul_ps(_mm_sub_ps(_mm_load_ps(&Aptr[i]), vecSubVal), vecMulVal));
			_mm_store_ps(&Rptr[i + 4], _mm_mul_ps(_mm_sub_ps(_mm_load_ps(&Aptr[i + 4]), vecSubVal), vecMulVal));
			_mm_store_ps(&Rptr[i + 8], _mm_mul_ps(_mm_sub_ps(_mm_load_ps(&Aptr[i + 8]), vecSubVal), vecMulVal));
			_mm_store_ps(&Rptr[i + 12], _mm_mul_ps(_mm_sub_ps(_mm_load_ps(&Aptr[i + 12]), vecSubVal), vecMulVal));
		}
		for (; i < width; i += 4) {
			_mm_store_ps(&Rptr[i], _mm_mul_ps(_mm_sub_ps(_mm_load_ps(&Aptr[i]), vecSubVal), vecMulVal));
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

void CompVMathOpSubSubVal_32f32f32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float32_t* Aptr,
	const compv_float32_t* subVal1,
	COMPV_ALIGNED(SSE) compv_float32_t* Rptr,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Astride,
	COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_uscalar_t width16 = width & -16;
	const __m128 vecSubVal = _mm_load1_ps(subVal1);
	compv_uscalar_t i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width16; i += 16) {
			_mm_store_ps(&Rptr[i], _mm_sub_ps(_mm_load_ps(&Aptr[i]), vecSubVal));
			_mm_store_ps(&Rptr[i + 4], _mm_sub_ps(_mm_load_ps(&Aptr[i + 4]), vecSubVal));
			_mm_store_ps(&Rptr[i + 8], _mm_sub_ps(_mm_load_ps(&Aptr[i + 8]), vecSubVal));
			_mm_store_ps(&Rptr[i + 12], _mm_sub_ps(_mm_load_ps(&Aptr[i + 12]), vecSubVal));
		}
		for (; i < width; i += 4) {
			_mm_store_ps(&Rptr[i], _mm_sub_ps(_mm_load_ps(&Aptr[i]), vecSubVal));
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
