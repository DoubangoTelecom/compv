/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse41.h"
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

void CompVImageScaleBicubicHermite_32f32s_Intrin_SSE41(
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
	COMPV_DEBUG_INFO_CHECK_SSE41();
#if 0
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("AVX using Gather is faster");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation");
#endif

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

	// TODO(dmi): AVX - use gather
	COMPV_ALIGN_SSE() int32_t vecIdx0_mem[4 * 4];
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[0]), vecIdx0);
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[4]), vecIdx1);
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[8]), vecIdx2);
	_mm_store_si128(reinterpret_cast<__m128i*>(&vecIdx0_mem[12]), vecIdx3);

	const __m128 xfract = _mm_set1_ps(*xfract1);
	const __m128 xfract2 = _mm_mul_ps(xfract, xfract);
	const __m128 xfract3 = _mm_mul_ps(xfract2, xfract);

	const compv_float32_t& yfract1_ = *yfract1;
	const compv_float32_t yfract2_ = yfract1_ * yfract1_;
	const __m128 yfract = _mm_setr_ps((yfract2_ * yfract1_), yfract2_, yfract1_, 1.f);

	const __m128 AA = _mm_setr_ps(inPtr[vecIdx0_mem[0]], inPtr[vecIdx0_mem[1]], inPtr[vecIdx0_mem[2]], inPtr[vecIdx0_mem[3]]);
	const __m128 BB = _mm_setr_ps(inPtr[vecIdx0_mem[4]], inPtr[vecIdx0_mem[5]], inPtr[vecIdx0_mem[6]], inPtr[vecIdx0_mem[7]]);
	const __m128 CC = _mm_setr_ps(inPtr[vecIdx0_mem[8]], inPtr[vecIdx0_mem[9]], inPtr[vecIdx0_mem[10]], inPtr[vecIdx0_mem[11]]);
	const __m128 DD = _mm_setr_ps(inPtr[vecIdx0_mem[12]], inPtr[vecIdx0_mem[13]], inPtr[vecIdx0_mem[14]], inPtr[vecIdx0_mem[15]]);
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
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
