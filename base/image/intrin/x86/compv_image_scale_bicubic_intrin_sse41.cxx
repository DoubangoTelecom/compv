/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

static const __m128 vecCoeff0 = _mm_setr_ps(-0.5f, 1.f, -0.5f, 0.0f);
static const __m128 vecCoeff1 = _mm_setr_ps(1.5f, -2.5f, 0.f, 1.0f);
static const __m128 vecCoeff2 = _mm_setr_ps(-1.5f, 2.0f, 0.5f, 0.0f);
static const __m128 vecCoeff3 = _mm_setr_ps(0.5f, -0.5f, 0.0f, 0.0f);

// Hermite
static compv_float32_t __hermite_32f(const compv_float32_t A, const compv_float32_t B, const compv_float32_t C, const compv_float32_t D, const compv_float32_t t, const compv_float32_t t2, compv_float32_t t3)
{
	__m128 vec0 = _mm_mul_ps(_mm_set1_ps(A), vecCoeff0);
	__m128 vec1 = _mm_mul_ps(_mm_set1_ps(B), vecCoeff1);
	__m128 vec2 = _mm_mul_ps(_mm_set1_ps(C), vecCoeff2);
	__m128 vec3 = _mm_mul_ps(_mm_set1_ps(D), vecCoeff3);
	vec0 = _mm_add_ps(vec0, vec1);
	vec2 = _mm_add_ps(vec2, vec3);
	vec0 = _mm_add_ps(vec0, vec2);
	vec0 = _mm_mul_ps(vec0, _mm_setr_ps(t3, t2, t, 1.f));

	// ARM: vpadd_f32
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E));
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x01));

	return _mm_cvtss_f32(vec0);

#if 0
	const compv_float32_t a = (A*(-0.5f)) + (B*(1.5f)) + (C*(-1.5f)) + (D*(0.5f));
	const compv_float32_t b = A + (B*(-2.5f)) + (C*(2.0f)) + (D*(-0.5f));
	const compv_float32_t c = (A*(-0.5f)) + (C * 0.5f);
	const compv_float32_t d = B;

	return a*t3 + b*t2 + c*t + d;
#endif
}

void CompVImageScaleBicubicHermite_8u32f_Intrin_SSE41(
	uint8_t* outPtr,
	const uint8_t* inPtr,
	const int32_t* xint1,
	const compv_float32_t* xfract1,
	const int32_t* yint1,
	const compv_float32_t* yfract1,
	const compv_uscalar_t inWidth,
	const compv_uscalar_t inHeight,
	const compv_uscalar_t inStride
)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("AVX using Gather is faster");

	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vecOffset = _mm_setr_epi32(-1, 0, 1, 2);

	// Add offsets (-1, 0, 1, 2)
	__m128i vecX = _mm_add_epi32(_mm_set1_epi32(*xint1), vecOffset);
	__m128i vecY = _mm_add_epi32(_mm_set1_epi32(*yint1), vecOffset);

	// a = COMPV_MATH_CLIP3(0, size-1, a)
	vecX = _mm_max_epi32(vecZero, _mm_min_epi32(vecX, _mm_set1_epi32(static_cast<int32_t>(inWidth - 1))));
	vecY = _mm_max_epi32(vecZero, _mm_min_epi32(vecY, _mm_set1_epi32(static_cast<int32_t>(inHeight - 1))));

	// Y = Y * stride
	vecY = _mm_mullo_epi32(vecY, _mm_set1_epi32(static_cast<int32_t>(inStride)));

	// Index[i] = Y[i] + X
	const __m128i vecIdx0 = _mm_add_epi32(_mm_shuffle_epi32(vecY, 0x00), vecX);
	const __m128i vecIdx1 = _mm_add_epi32(_mm_shuffle_epi32(vecY, 0x55), vecX);
	const __m128i vecIdx2 = _mm_add_epi32(_mm_shuffle_epi32(vecY, 0xAA), vecX);
	const __m128i vecIdx3 = _mm_add_epi32(_mm_shuffle_epi32(vecY, 0xFF), vecX);

	// TODO(dmi): AVX - use gather
	COMPV_ALIGN_SSE() int32_t vecIdx0_mem[4 * 4];
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[0]), vecIdx0);
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[4]), vecIdx1);
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[8]), vecIdx2);
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[12]), vecIdx3);

	const compv_float32_t& xfract = *xfract1;
	const compv_float32_t& yfract = *yfract1;
	const compv_float32_t xfract2 = (xfract * xfract);
	const compv_float32_t xfract3 = (xfract2 * xfract);
	const compv_float32_t yfract2 = (yfract * yfract);
	const compv_float32_t yfract3 = (yfract2 * yfract);

	const compv_float32_t c0 = __hermite_32f(inPtr[vecIdx0_mem[0]], inPtr[vecIdx0_mem[1]], inPtr[vecIdx0_mem[2]], inPtr[vecIdx0_mem[3]], xfract, xfract2, xfract3); // TODO(dmi): AVX - use gather
	const compv_float32_t c1 = __hermite_32f(inPtr[vecIdx0_mem[4]], inPtr[vecIdx0_mem[5]], inPtr[vecIdx0_mem[6]], inPtr[vecIdx0_mem[7]], xfract, xfract2, xfract3);
	const compv_float32_t c2 = __hermite_32f(inPtr[vecIdx0_mem[8]], inPtr[vecIdx0_mem[9]], inPtr[vecIdx0_mem[10]], inPtr[vecIdx0_mem[11]], xfract, xfract2, xfract3);
	const compv_float32_t c3 = __hermite_32f(inPtr[vecIdx0_mem[12]], inPtr[vecIdx0_mem[13]], inPtr[vecIdx0_mem[14]], inPtr[vecIdx0_mem[15]], xfract, xfract2, xfract3);
	const compv_float32_t value = __hermite_32f(c0, c1, c2, c3, yfract, yfract2, yfract3);

	*outPtr = static_cast<uint8_t>(COMPV_MATH_CLIP3(0, 255.f, value)); // SIMD(dmi): saturation (no need to clip)
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
