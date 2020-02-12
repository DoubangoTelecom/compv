/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bicubic_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVImageScaleBicubicPostProcessRow_32f32s_Intrin_SSE2(
	compv_float32_t* outPtr,
	const compv_float32_t* inPtr,
	COMPV_ALIGNED(SSE) const int32_t* xint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* xfract4,
	COMPV_ALIGNED(SSE) const int32_t* yint4,
	COMPV_ALIGNED(SSE) const compv_float32_t* yfract4,
	const compv_uscalar_t rowCount
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("AVX using Gather is faster");

	// TODO(dmi): No ASM code

	const compv_float32_t* p0 = &inPtr[yint4[0]];
	const compv_float32_t* p1 = &inPtr[yint4[1]];
	const compv_float32_t* p2 = &inPtr[yint4[2]];
	const compv_float32_t* p3 = &inPtr[yint4[3]];

	__m128 AA, BB, CC, DD, EE;
	const __m128 yfract = _mm_load_ps(yfract4);

	for (compv_uscalar_t i = 0; i < rowCount; ++i, xint4 += 4, xfract4 += 4) {
		const int32_t& x0 = xint4[0];
		const int32_t& x3 = xint4[3];		
		
		if ((x3 - x0) == 3) {
			AA = _mm_loadu_ps(&p0[x0]);
			BB = _mm_loadu_ps(&p1[x0]);
			CC = _mm_loadu_ps(&p2[x0]);
			DD = _mm_loadu_ps(&p3[x0]);
			_MM_TRANSPOSE4_PS(AA, BB, CC, DD);
		}
		else {
			// TODO(dmi): use shufle
			const int32_t& x1 = xint4[1];
			const int32_t& x2 = xint4[2];
			AA = _mm_setr_ps(p0[x0], p1[x0], p2[x0], p3[x0]);
			BB = _mm_setr_ps(p0[x1], p1[x1], p2[x1], p3[x1]);
			CC = _mm_setr_ps(p0[x2], p1[x2], p2[x2], p3[x2]);
			DD = _mm_setr_ps(p0[x3], p1[x3], p2[x3], p3[x3]);
		}

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
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
