/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define SET_RLC_1(mm, ii) \
		if ((mm)) { \
			RLCi[er] = ((ii) - b); \
			b ^= 1;  \
			++er; \
		} \
		ERi[(ii)] = er

#define SET_RLC_1_SSE2(mm, ii, offset) \
		if ((mm & (1 << offset))) { \
			RLCi[er] = (((ii) + offset) - b); \
			b ^= 1;  \
			++er; \
		} \
		ERi[((ii) + offset)] = er

#define SET_RLC_16_SSE2(mm, ii) \
		SET_RLC_1_SSE2(mm, ii, 0); SET_RLC_1_SSE2(mm, ii, 1); SET_RLC_1_SSE2(mm, ii, 2); SET_RLC_1_SSE2(mm, ii, 3); \
		SET_RLC_1_SSE2(mm, ii, 4); SET_RLC_1_SSE2(mm, ii, 5); SET_RLC_1_SSE2(mm, ii, 6); SET_RLC_1_SSE2(mm, ii, 7); \
		SET_RLC_1_SSE2(mm, ii, 8); SET_RLC_1_SSE2(mm, ii, 9); SET_RLC_1_SSE2(mm, ii, 10); SET_RLC_1_SSE2(mm, ii, 11); \
		SET_RLC_1_SSE2(mm, ii, 12); SET_RLC_1_SSE2(mm, ii, 13); SET_RLC_1_SSE2(mm, ii, 14); SET_RLC_1_SSE2(mm, ii, 15)

#define SET_RLC_ZERO_1_SSE2(ii, offset) ERi[((ii) + offset)] = er

#define SET_RLC_ZERO_16_SSE2(ii) \
		SET_RLC_ZERO_1_SSE2(ii, 0); SET_RLC_ZERO_1_SSE2(ii, 1); SET_RLC_ZERO_1_SSE2(ii, 2); SET_RLC_ZERO_1_SSE2(ii, 3); \
		SET_RLC_ZERO_1_SSE2(ii, 4); SET_RLC_ZERO_1_SSE2(ii, 5); SET_RLC_ZERO_1_SSE2(ii, 6); SET_RLC_ZERO_1_SSE2(ii, 7); \
		SET_RLC_ZERO_1_SSE2(ii, 8); SET_RLC_ZERO_1_SSE2(ii, 9); SET_RLC_ZERO_1_SSE2(ii, 10); SET_RLC_ZERO_1_SSE2(ii, 11); \
		SET_RLC_ZERO_1_SSE2(ii, 12); SET_RLC_ZERO_1_SSE2(ii, 13); SET_RLC_ZERO_1_SSE2(ii, 14); SET_RLC_ZERO_1_SSE2(ii, 15)

// TODO(dmi): Function not used -> remove
void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_8u32s_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Xi, int32_t* RLCi, int32_t* ERi, int32_t* b1, int32_t* er1, const int32_t width)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Not faster than serial code");
	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vecFF = _mm_cmpeq_epi8(vecZero, vecZero);
	const int32_t w64 = width & -64;
	int32_t b = *b1, er = *er1;
	int32_t i;
	int mask0, mask1, mask2, mask3;
	for (i = 1; i < w64; i += 64) {
		mask0 = _mm_movemask_epi8(_mm_cmpnot_epu8_SSE2(
			_mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Xi[i - 1])),
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&Xi[i]))
			),
			vecZero, vecFF)
		);
		mask1 = _mm_movemask_epi8(_mm_cmpnot_epu8_SSE2(
			_mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Xi[i + 15])),
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&Xi[i + 16]))
			),
			vecZero, vecFF)
		);
		mask2 = _mm_movemask_epi8(_mm_cmpnot_epu8_SSE2(
			_mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Xi[i + 31])),
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&Xi[i + 32]))
			),
			vecZero, vecFF)
		);
		mask3 = _mm_movemask_epi8(_mm_cmpnot_epu8_SSE2(
			_mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Xi[i + 47])),
				_mm_loadu_si128(reinterpret_cast<const __m128i*>(&Xi[i + 48]))
			),
			vecZero, vecFF)
		);

		if (mask0) { SET_RLC_16_SSE2(mask0, (i + 0)); }
		else { SET_RLC_ZERO_16_SSE2((i + 0)); }

		if (mask1) { SET_RLC_16_SSE2(mask1, (i + 16)); }
		else { SET_RLC_ZERO_16_SSE2((i + 16)); }

		if (mask2) { SET_RLC_16_SSE2(mask2, (i + 32)); }
		else { SET_RLC_ZERO_16_SSE2((i + 32)); }

		if (mask3) { SET_RLC_16_SSE2(mask3, (i + 48)); }
		else { SET_RLC_ZERO_16_SSE2((i + 48)); }
	}
	for (; i < width; ++i) {
		SET_RLC_1(Xi[i - 1] ^ Xi[i], i);
	}
	*b1 = b;
	*er1 = er;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */