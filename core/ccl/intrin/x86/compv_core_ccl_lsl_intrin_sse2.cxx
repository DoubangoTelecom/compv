/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Function requires width > 16 (not ">=" but ">")
void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Intrin_SSE2(
	const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	const compv_uscalar_t width, const compv_uscalar_t height
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

	const int16_t width1 = static_cast<int16_t>(width);
	const int16_t width16 = (width1 - 1) & -16; // width > 16 (at least 17) which means never equal to zero
	int16_t er, i;
	__m128i vec0, vec1;
	int mask, m;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		er = (Xi[0] & 1);
		RLCi[0] = 0;

		// In asm code, no need to test "width16 != 0" because "width1" > 16 (at least 17)
		for (i = 1; i < width16; i += 16) {
			vec0 = _mm_cmpeq_epi16(
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&ERi[i - 1])),
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&ERi[i]))
			);
			vec1 = _mm_cmpeq_epi16(
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&ERi[i + 7])),
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&ERi[i + 8]))
			);
			vec0 = _mm_packs_epi16(vec0, vec1);
			mask = _mm_movemask_epi8(vec0) ^ 0xffff;
			if (mask) {
				m = i;
				do {
					if (mask & 1) {
						RLCi[er++] = m;
					}
					++m;
				} while (mask >>= 1);
			}
		}

		for (; i < width1; ++i) {
			if (ERi[i - 1] != ERi[i]) {
				RLCi[er++] = i;
			}
		}

		RLCi[er] = width1 - ((Xi[width1 - 1] & 1) ^ 1);

		/* next */
		Xi += Xi_stride;
		RLCi += RLCi_stride;
		ERi += ERi_stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */