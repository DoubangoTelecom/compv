/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_avx2.h"
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

#define HERMITE1_32F_INTRIN_FMA3_AVX2(A, B, C, D, ttt, ret) { \
	static const __m128 vecCoeff0 = _mm_setr_ps(-0.5f, 1.f, -0.5f, 0.0f); \
	static const __m128 vecCoeff1 = _mm_setr_ps(1.5f, -2.5f, 0.f, 1.0f); \
	static const __m128 vecCoeff2 = _mm_setr_ps(-1.5f, 2.0f, 0.5f, 0.0f); \
	static const __m128 vecCoeff3 = _mm_setr_ps(0.5f, -0.5f, 0.0f, 0.0f); \
	const __m128 vec1 = _mm_mul_ps(B, vecCoeff1); \
	const __m128 vec3 = _mm_mul_ps(D, vecCoeff3); \
	__m128 vec0 = _mm_fmadd_ps(A, vecCoeff0, vec1); \
	const __m128 vec2 = _mm_fmadd_ps(C, vecCoeff2, vec3); \
	vec0 = _mm_add_ps(vec0, vec2); \
	vec0 = _mm_mul_ps(vec0, ttt); \
	/* ARM: vpadd_f32 */  \
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x0E)); \
	vec0 = _mm_add_ps(vec0, _mm_shuffle_ps(vec0, vec0, 0x01)); \
	_mm_store_ss(&ret, vec0); \
}

#define HERMITE4_32F_INTRIN_FMA3_AVX2(A, B, C, D, t, t2, t3, ret) { \
	static __m128 vec05m = _mm_set1_ps(-0.5f); \
	static __m128 vec15 = _mm_set1_ps(1.5f); \
	static __m128 vec20 = _mm_set1_ps(2.0f); \
	static __m128 vec25m = _mm_set1_ps(-2.5f); \
	ret = _mm_mul_ps(_mm_sub_ps(A, D), vec05m); \
	ret = _mm_fmadd_ps(_mm_sub_ps(B, C), vec15, ret); \
	__m128 vec1 = _mm_fmadd_ps(B, vec25m, A); \
	vec1 = _mm_fmadd_ps(C, vec20, vec1); \
	vec1 = _mm_fmadd_ps(D, vec05m, vec1); \
	__m128 vec2 = _mm_mul_ps(_mm_sub_ps(A, C), vec05m); \
	vec2 = _mm_mul_ps(vec2, t); \
	ret = _mm_fmadd_ps(ret, t3, vec2); \
	vec1 = _mm_fmadd_ps(vec1, t2, B); \
	ret = _mm_add_ps(ret, vec1); \
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageScaleBicubicHermite_32f32s_Intrin_AVX2(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	const int32_t* xint1,
	const compv_float32_t* xfract1,
	const int32_t* yint1,
	const compv_float32_t* yfract1,
	const compv_uscalar_t inWidthMinus1,
	const compv_uscalar_t inHeightMinus1,
	const compv_uscalar_t inStride
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
#if 0
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster (FMA3)");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation");
#endif

	// TODO(dmi): No ASM code

	_mm256_zeroupper();

	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vecOffset = _mm_setr_epi32(-1, 0, 1, 2);

	// Add offsets (-1, 0, 1, 2)
	__m128i vecX = _mm_add_epi32(_mm_set1_epi32(*xint1), vecOffset);
	__m128i vecY = _mm_add_epi32(_mm_set1_epi32(*yint1), vecOffset);

	// a = COMPV_MATH_CLIP3(0, size-1, a)
	vecX = _mm_max_epi32(vecZero, _mm_min_epi32(vecX, _mm_set1_epi32(static_cast<int32_t>(inWidthMinus1))));
	vecY = _mm_max_epi32(vecZero, _mm_min_epi32(vecY, _mm_set1_epi32(static_cast<int32_t>(inHeightMinus1))));

	// Y = Y * stride
	vecY = _mm_mullo_epi32(vecY, _mm_set1_epi32(static_cast<int32_t>(inStride)));

	// Index[i] = Y[i] + X
	const __m128i vecIdx0 = _mm_add_epi32(vecY, _mm_shuffle_epi32(vecX, 0x00));
	const __m128i vecIdx1 = _mm_add_epi32(vecY, _mm_shuffle_epi32(vecX, 0x55));
	const __m128i vecIdx2 = _mm_add_epi32(vecY, _mm_shuffle_epi32(vecX, 0xAA));
	const __m128i vecIdx3 = _mm_add_epi32(vecY, _mm_shuffle_epi32(vecX, 0xFF));

	const __m128 xfract = _mm_set1_ps(*xfract1);
	const __m128 xfract2 = _mm_mul_ps(xfract, xfract);
	const __m128 xfract3 = _mm_mul_ps(xfract2, xfract);

	const compv_float32_t& yfract1_ = *yfract1;
	const compv_float32_t yfract2_ = yfract1_ * yfract1_;
	const __m128 yfract = _mm_setr_ps((yfract2_ * yfract1_), yfract2_, yfract1_, 1.f);

	const __m128 AA = _mm_i32gather_ps(inPtr, vecIdx0, sizeof(compv_float32_t));
	const __m128 BB = _mm_i32gather_ps(inPtr, vecIdx1, sizeof(compv_float32_t));
	const __m128 CC = _mm_i32gather_ps(inPtr, vecIdx2, sizeof(compv_float32_t));
	const __m128 DD = _mm_i32gather_ps(inPtr, vecIdx3, sizeof(compv_float32_t));
	__m128 EE;
	HERMITE4_32F_INTRIN_SSE2(
		AA, BB, CC, DD,
		xfract, xfract2, xfract3,
		EE
	);
	HERMITE1_32F_INTRIN_SSE2(
		_mm_shuffle_ps(EE, EE, 0x00),
		_mm_shuffle_ps(EE, EE, 0x55),
		_mm_shuffle_ps(EE, EE, 0xAA),
		_mm_shuffle_ps(EE, EE, 0xFF),
		yfract,
		*outPtr
	);

	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageScaleBicubicPostProcessRow_32f32s_Intrin_AVX2(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	COMPV_ALIGNED(SSE) const int32_t* xint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* xfract4,
	COMPV_ALIGNED(SSE) const int32_t* yint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* yfract4,
	const compv_uscalar_t rowCount
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
#if 0
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation");
#endif

	_mm256_zeroupper();

	const compv_float32_t* p0 = &inPtr[yint4[0]];
	const compv_float32_t* p1 = &inPtr[yint4[1]];
	const compv_float32_t* p2 = &inPtr[yint4[2]];
	const compv_float32_t* p3 = &inPtr[yint4[3]];

	__m128 AA, BB, CC, DD, EE;
	const __m128 yfract = _mm_load_ps(yfract4);

	for (compv_uscalar_t i = 0; i < rowCount; ++i, xint4 += 4, xfract4 += 4) {
		const int32_t& x0 = xint4[0];
		if ((xint4[3] - x0) == 3) {
			AA = _mm_loadu_ps(&p0[x0]);
			BB = _mm_loadu_ps(&p1[x0]);
			CC = _mm_loadu_ps(&p2[x0]);
			DD = _mm_loadu_ps(&p3[x0]);
		}
		else {
			const __m128i vecXint4 = _mm_load_si128(reinterpret_cast<const __m128i*>(xint4));
			AA = _mm_i32gather_ps(p0, vecXint4, sizeof(compv_float32_t));
			BB = _mm_i32gather_ps(p1, vecXint4, sizeof(compv_float32_t));
			CC = _mm_i32gather_ps(p2, vecXint4, sizeof(compv_float32_t));
			DD = _mm_i32gather_ps(p3, vecXint4, sizeof(compv_float32_t));
		}
		_MM_TRANSPOSE4_PS(AA, BB, CC, DD);

		const __m128 xfract = _mm_load_ps(xfract4);
		const __m128 xfract3 = _mm_shuffle_ps(xfract, xfract, 0x00);
		const __m128 xfract2 = _mm_shuffle_ps(xfract, xfract, 0x55);
		const __m128 xfract1 = _mm_shuffle_ps(xfract, xfract, 0xAA);
		HERMITE4_32F_INTRIN_SSE2(
			AA, BB, CC, DD,
			xfract1, xfract2, xfract3,
			EE
		);
		HERMITE1_32F_INTRIN_SSE2(
			_mm_shuffle_ps(EE, EE, 0x00),
			_mm_shuffle_ps(EE, EE, 0x55),
			_mm_shuffle_ps(EE, EE, 0xAA),
			_mm_shuffle_ps(EE, EE, 0xFF),
			yfract,
			outPtr[i]
		);
	}

	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
