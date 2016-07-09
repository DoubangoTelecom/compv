/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/edges/compv_feature_canny_dete_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void CannyNMSApply_Intrin_SSE2(COMPV_ALIGNED(SSE) uint16_t* grad, COMPV_ALIGNED(SSE) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM

	__m128i xmm0;
	compv_uscalar_t col_, row_;
	const __m128i xmmZero = _mm_setzero_si128();
	for (row_ = 1; row_ < height; ++row_) {
		for (col_ = 0; col_ < width; col_ += 8) { // SIMD, starts at 0 (instead of 1) to have memory aligned
			xmm0 = _mm_cmpeq_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&nms[col_])), xmmZero);
			if (_mm_movemask_epi8(xmm0) ^ 0xFFFF) {
				xmm0 = _mm_and_si128(_mm_unpacklo_epi8(xmm0, xmm0), _mm_load_si128(reinterpret_cast<const __m128i*>(&grad[col_])));
				_mm_store_si128(reinterpret_cast<__m128i*>(&grad[col_]), xmm0);
				_mm_storel_epi64(reinterpret_cast<__m128i*>(&nms[col_]), xmmZero);
			}
		}
		nms += stride;
		grad += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
