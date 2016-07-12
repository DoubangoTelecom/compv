/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/edges/compv_feature_canny_dete_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/features/edges/compv_feature_canny_dete.h"
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/math/compv_math_utils.h"
#include "compv/compv_simd_globals.h"
#include "compv/compv_mem.h"


COMPV_NAMESPACE_BEGIN()

void CannyNMSGatherRow_Intrin_SSSE3(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, const uint16_t* tLow1, compv_uscalar_t width, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
	COMPV_DEBUG_INFO_CHECK_SSSE3();

	__m128i xmmNMS, xmmG, xmmGX, xmmAbsGX0, xmmAbsGX1, xmmGY, xmmAbsGY0, xmmAbsGY1, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
	const __m128i xmmTLow = _mm_set1_epi16(*tLow1);
	static const __m128i xmmZero = _mm_setzero_si128();
	static const __m128i xmmTangentPiOver8Int = _mm_set1_epi32(kTangentPiOver8Int);
	static const __m128i xmmTangentPiTimes3Over8Int = _mm_set1_epi32(kTangentPiTimes3Over8Int);
	compv_uscalar_t col;
	const int stride_ = static_cast<const int>(stride);
	const int c0 = 1 - stride_, c1 = 1 + stride_;
	compv_uscalar_t maxColsSSE = COMPV_MATH_CLIP3(width - 7, stride - 7, width);

	for (col = 1; col < maxColsSSE; col += 8) {
		xmmG = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col]));
		xmm0 = _mm_cmpgt_epi16(xmmG, xmmTLow);
		if (_mm_movemask_epi8(xmm0)) {
			xmmNMS = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&nms[col]));
			xmmGX = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&gx[col]));
			xmmGY = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&gy[col]));

			xmm1 = _mm_abs_epi16(xmmGY);
			xmm2 = _mm_abs_epi16(xmmGX);

			xmmAbsGY0 = _mm_unpacklo_epi16(xmmZero, xmm1);
			xmmAbsGX0 = _mm_unpacklo_epi16(xmm2, xmmZero);
			xmmAbsGY1 = _mm_unpackhi_epi16(xmmZero, xmm1);
			xmmAbsGX1 = _mm_unpackhi_epi16(xmm2, xmmZero);

			// angle = "0° / 180°"
			xmm1 = _mm_cmplt_epi32(xmmAbsGY0, _mm_mullo_epi32(xmmTangentPiOver8Int, xmmAbsGX0));
			xmm2 = _mm_cmplt_epi32(xmmAbsGY1, _mm_mullo_epi32(xmmTangentPiOver8Int, xmmAbsGX1));
			xmm3 = _mm_and_si128(xmm0, _mm_packs_epi32(xmm1, xmm2));
			if (_mm_movemask_epi8(xmm3)) {
				xmm1 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col - 1])), xmmG);
				xmm2 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + 1])), xmmG);
				xmm1 = _mm_and_si128(xmm3, _mm_or_si128(xmm1, xmm2));
				xmmNMS = _mm_or_si128(_mm_packs_epi16(xmm1, xmm1), xmmNMS);
			}

			// angle = "45° / 225°" or "135 / 315"
			xmm4 = _mm_andnot_si128(xmm3, xmm0);
			if (_mm_movemask_epi8(xmm4)) {
				xmm1 = _mm_cmplt_epi32(xmmAbsGY0, _mm_mullo_epi32(xmmTangentPiTimes3Over8Int, xmmAbsGX0));
				xmm2 = _mm_cmplt_epi32(xmmAbsGY1, _mm_mullo_epi32(xmmTangentPiTimes3Over8Int, xmmAbsGX1));
				xmm4 = _mm_and_si128(xmm4, _mm_packs_epi32(xmm1, xmm2));
				if (_mm_movemask_epi8(xmm4)) {
					xmm1 = _mm_cmplt_epi16(_mm_xor_si128(xmmGX, xmmGY), xmmZero);
					xmm1 = _mm_and_si128(xmm4, xmm1);
					xmm2 = _mm_andnot_si128(xmm1, xmm4);
					if (_mm_movemask_epi8(xmm1)) {
						xmm5 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col - c0])), xmmG);
						xmm6 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + c0])), xmmG);
						xmm1 = _mm_and_si128(xmm1, _mm_or_si128(xmm5, xmm6));
					}
					if (_mm_movemask_epi8(xmm2)) {
						xmm5 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col - c1])), xmmG);
						xmm6 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + c1])), xmmG);
						xmm2 = _mm_and_si128(xmm2, _mm_or_si128(xmm5, xmm6));
					}
					xmm1 = _mm_or_si128(xmm1, xmm2);
					xmmNMS = _mm_or_si128(xmmNMS, _mm_packs_epi16(xmm1, xmm1));
				}
			}

			// angle = "90° / 270°"
			xmm5 = _mm_andnot_si128(xmm3, _mm_andnot_si128(xmm4, xmm0));
			if (_mm_movemask_epi8(xmm5)) {
				xmm1 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col - stride])), xmmG);
				xmm2 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + stride])), xmmG);
				xmm1 = _mm_and_si128(xmm5, _mm_or_si128(xmm1, xmm2));
				xmmNMS = _mm_or_si128(_mm_packs_epi16(xmm1, xmm1), xmmNMS);
			}

			_mm_storel_epi64(reinterpret_cast<__m128i*>(&nms[col]), xmmNMS);
		}
	}
	if (col < width) {
		nms_gather_row_C(nms, g, gx, gy, *tLow1, col, width, stride);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
