/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_bits.h"

COMPV_NAMESPACE_BEGIN()

// xPtr, yPtr and distPtr must be strided so that we can read/write up to align_forward(count)
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathDistanceLine_32f_Intrin_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* xPtr, COMPV_ALIGNED(AVX) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(AVX) compv_float32_t* distPtr, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const compv_uscalar_t count32 = count & -32;
	compv_uscalar_t i;
	__m256 vec0, vec1, vec2, vec3;
	const __m256 vecA = _mm256_set1_ps(*Ascaled1);
	const __m256 vecB = _mm256_set1_ps(*Bscaled1);
	const __m256 vecC = _mm256_set1_ps(*Cscaled1);
	const __m256 vecMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff)); // mask used for '_mm256_abs_ps'

	for (i = 0; i < count32; i += 32) {
		vec0 = _mm256_mul_ps(vecA, _mm256_load_ps(&xPtr[i]));
		vec1 = _mm256_mul_ps(vecA, _mm256_load_ps(&xPtr[i + 8]));
		vec2 = _mm256_mul_ps(vecA, _mm256_load_ps(&xPtr[i + 16]));
		vec3 = _mm256_mul_ps(vecA, _mm256_load_ps(&xPtr[i + 24]));
		vec0 = _mm256_add_ps(_mm256_add_ps(vec0, vecC), _mm256_mul_ps(vecB, _mm256_load_ps(&yPtr[i])));
		vec1 = _mm256_add_ps(_mm256_add_ps(vec1, vecC), _mm256_mul_ps(vecB, _mm256_load_ps(&yPtr[i + 8])));
		vec2 = _mm256_add_ps(_mm256_add_ps(vec2, vecC), _mm256_mul_ps(vecB, _mm256_load_ps(&yPtr[i + 16])));
		vec3 = _mm256_add_ps(_mm256_add_ps(vec3, vecC), _mm256_mul_ps(vecB, _mm256_load_ps(&yPtr[i + 24])));
		_mm256_store_ps(&distPtr[i], _mm256_and_ps(vecMask, vec0));
		_mm256_store_ps(&distPtr[i + 8], _mm256_and_ps(vecMask, vec1));
		_mm256_store_ps(&distPtr[i + 16], _mm256_and_ps(vecMask, vec2));
		_mm256_store_ps(&distPtr[i + 24], _mm256_and_ps(vecMask, vec3));
	}
	for (; i < count; i += 8) { // can read beyond count and up to align_forward(count) - data strided
		vec0 = _mm256_mul_ps(vecA, _mm256_load_ps(&xPtr[i]));
		vec0 = _mm256_add_ps(_mm256_add_ps(vec0, vecC), _mm256_mul_ps(vecB, _mm256_load_ps(&yPtr[i])));
		_mm256_store_ps(&distPtr[i], _mm256_and_ps(vecMask, vec0));
	}
	_mm256_zeroupper();
}

// xPtr, yPtr and distPtr must be strided so that we can read/write up to align_forward(count)
// TODO(dmi): asm code faster by faar
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx
#endif
void CompVMathDistanceParabola_32f_Intrin_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* xPtr, COMPV_ALIGNED(AVX) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(AVX) compv_float32_t* distPtr, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CHECK_AVX();
	_mm256_zeroupper();
	const compv_uscalar_t count32 = count & -32;
	compv_uscalar_t i;
	__m256 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
	const __m256 vecA = _mm256_set1_ps(*A1);
	const __m256 vecB = _mm256_set1_ps(*B1);
	const __m256 vecC = _mm256_set1_ps(*C1);
	const __m256 vecMask = _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff)); // mask used for '_mm256_abs_ps'

	for (i = 0; i < count32; i += 32) {
		vec4 = _mm256_load_ps(&xPtr[i]);
		vec5 = _mm256_load_ps(&xPtr[i + 8]);
		vec6 = _mm256_load_ps(&xPtr[i + 16]);
		vec7 = _mm256_load_ps(&xPtr[i + 24]);
		vec0 = _mm256_mul_ps(vecA, _mm256_mul_ps(vec4, vec4));
		vec1 = _mm256_mul_ps(vecA, _mm256_mul_ps(vec5, vec5));
		vec2 = _mm256_mul_ps(vecA, _mm256_mul_ps(vec6, vec6));
		vec3 = _mm256_mul_ps(vecA, _mm256_mul_ps(vec7, vec7));
		vec0 = _mm256_add_ps(_mm256_add_ps(vec0, vecC), _mm256_mul_ps(vecB, vec4));
		vec1 = _mm256_add_ps(_mm256_add_ps(vec1, vecC), _mm256_mul_ps(vecB, vec5));
		vec2 = _mm256_add_ps(_mm256_add_ps(vec2, vecC), _mm256_mul_ps(vecB, vec6));
		vec3 = _mm256_add_ps(_mm256_add_ps(vec3, vecC), _mm256_mul_ps(vecB, vec7));
		_mm256_store_ps(&distPtr[i], _mm256_and_ps(vecMask, _mm256_sub_ps(vec0, _mm256_load_ps(&yPtr[i]))));
		_mm256_store_ps(&distPtr[i + 8], _mm256_and_ps(vecMask, _mm256_sub_ps(vec1, _mm256_load_ps(&yPtr[i + 8]))));
		_mm256_store_ps(&distPtr[i + 16], _mm256_and_ps(vecMask, _mm256_sub_ps(vec2, _mm256_load_ps(&yPtr[i + 16]))));
		_mm256_store_ps(&distPtr[i + 24], _mm256_and_ps(vecMask, _mm256_sub_ps(vec3, _mm256_load_ps(&yPtr[i + 24]))));
	}
	for (; i < count; i += 8) { // can read beyond count and up to align_forward(count) - data strided
		vec4 = _mm256_load_ps(&xPtr[i]);
		vec0 = _mm256_mul_ps(vecA, _mm256_mul_ps(vec4, vec4));
		vec0 = _mm256_add_ps(_mm256_add_ps(vec0, vecC), _mm256_mul_ps(vecB, vec4));
		_mm256_store_ps(&distPtr[i], _mm256_and_ps(vecMask, _mm256_sub_ps(vec0, _mm256_load_ps(&yPtr[i]))));
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
