/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"
#include "compv/base/compv_bits.h"

COMPV_NAMESPACE_BEGIN()

// xPtr, yPtr and distPtr must be strided so that we can read/write up to align_forward(count)
void CompVMathDistanceLine_32f_Intrin_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* xPtr, COMPV_ALIGNED(SSE) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(SSE) compv_float32_t* distPtr, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	const compv_uscalar_t count16 = count & -16;
	compv_uscalar_t i;
	__m128 vec0, vec1, vec2, vec3;
	const __m128 vecA = _mm_set1_ps(*Ascaled1);
	const __m128 vecB = _mm_set1_ps(*Bscaled1);
	const __m128 vecC = _mm_set1_ps(*Cscaled1);
	const __m128 vecMask = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)); // mask used for '_mm_abs_ps'

	for (i = 0; i < count16; i += 16) {
		vec0 = _mm_mul_ps(vecA, _mm_load_ps(&xPtr[i]));
		vec1 = _mm_mul_ps(vecA, _mm_load_ps(&xPtr[i + 4]));
		vec2 = _mm_mul_ps(vecA, _mm_load_ps(&xPtr[i + 8]));
		vec3 = _mm_mul_ps(vecA, _mm_load_ps(&xPtr[i + 12]));
		vec0 = _mm_add_ps(_mm_add_ps(vec0, vecC), _mm_mul_ps(vecB, _mm_load_ps(&yPtr[i])));
		vec1 = _mm_add_ps(_mm_add_ps(vec1, vecC), _mm_mul_ps(vecB, _mm_load_ps(&yPtr[i + 4])));
		vec2 = _mm_add_ps(_mm_add_ps(vec2, vecC), _mm_mul_ps(vecB, _mm_load_ps(&yPtr[i + 8])));
		vec3 = _mm_add_ps(_mm_add_ps(vec3, vecC), _mm_mul_ps(vecB, _mm_load_ps(&yPtr[i + 12])));
		_mm_store_ps(&distPtr[i], _mm_and_ps(vecMask, vec0));
		_mm_store_ps(&distPtr[i + 4], _mm_and_ps(vecMask, vec1));
		_mm_store_ps(&distPtr[i + 8], _mm_and_ps(vecMask, vec2));
		_mm_store_ps(&distPtr[i + 12], _mm_and_ps(vecMask, vec3));
	}
	for (; i < count; i += 4) { // can read beyond count and up to align_forward(count) - data strided
		vec0 = _mm_mul_ps(vecA, _mm_load_ps(&xPtr[i]));
		vec0 = _mm_add_ps(_mm_add_ps(vec0, vecC), _mm_mul_ps(vecB, _mm_load_ps(&yPtr[i])));
		_mm_store_ps(&distPtr[i], _mm_and_ps(vecMask, vec0));
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
