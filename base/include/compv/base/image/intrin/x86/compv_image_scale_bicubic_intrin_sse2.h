/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_SSE2_H_)
#define _COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#define HERMITE1_32F_INTRIN_SSE2(A, B, C, D, ttt, ret) { \
	static const __m128 vecCoeff0 = _mm_setr_ps(-0.5f, 1.f, -0.5f, 0.0f); \
	static const __m128 vecCoeff1 = _mm_setr_ps(1.5f, -2.5f, 0.f, 1.0f); \
	static const __m128 vecCoeff2 = _mm_setr_ps(-1.5f, 2.0f, 0.5f, 0.0f); \
	static const __m128 vecCoeff3 = _mm_setr_ps(0.5f, -0.5f, 0.0f, 0.0f); \
	__m128 vec0 = _mm_mul_ps(A, vecCoeff0); \
	const __m128 vec1 = _mm_mul_ps(B, vecCoeff1); \
	__m128 vec2 = _mm_mul_ps(C, vecCoeff2); \
	const __m128 vec3 = _mm_mul_ps(D, vecCoeff3); \
	vec0 = _mm_add_ps(vec0, vec1); \
	vec2 = _mm_add_ps(vec2, vec3); \
	vec0 = _mm_add_ps(vec0, vec2); \
	vec0 = _mm_mul_ps(vec0, ttt); \
	/* ARM: vpadd_f32 */ \
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E)); \
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x01)); \
	_mm_store_ss(&ret, vec0); \
}

#define HERMITE4_32F_INTRIN_SSE2(A, B, C, D, t, t2, t3, ret) { \
	static const __m128 vec05m = _mm_set1_ps(-0.5f); \
	static const __m128 vec15 = _mm_set1_ps(1.5f); \
	static const __m128 vec20 = _mm_set1_ps(2.0f); \
	static const __m128 vec25m = _mm_set1_ps(-2.5f); \
	ret = _mm_mul_ps(_mm_sub_ps(A, D), vec05m); \
	ret = _mm_add_ps(ret, _mm_mul_ps(_mm_sub_ps(B, C), vec15)); \
	__m128 vec1 = _mm_add_ps(A, _mm_mul_ps(B, vec25m)); \
	vec1 = _mm_add_ps(vec1, _mm_mul_ps(C, vec20)); \
	vec1 = _mm_add_ps(_mm_mul_ps(D, vec05m), vec1); \
	__m128 vec2 = _mm_mul_ps(_mm_sub_ps(A, C), vec05m); \
	ret = _mm_mul_ps(ret, t3); \
	vec1 = _mm_mul_ps(vec1, t2); \
	vec2 = _mm_mul_ps(vec2, t); \
	ret = _mm_add_ps(ret, vec2); \
	vec1 = _mm_add_ps(vec1, B); \
	ret = _mm_add_ps(ret, vec1); \
}

void CompVImageScaleBicubicPostProcessRow_32f32s_Intrin_SSE2(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	COMPV_ALIGNED(SSE) const int32_t* xint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* xfract4,
	COMPV_ALIGNED(SSE) const int32_t* yint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* yfract4,
	const compv_uscalar_t rowCount
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_SCALE_BICUBIC_INTRIN_SSE2_H_ */
