/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_sse41.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"
#include "compv/math/compv_math_utils.h"
#include "compv/compv_bits.h"
#include "compv/compv_cpu.h"

#include <algorithm>

extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast9Arcs[16][16];
extern "C" const COMPV_ALIGN_DEFAULT() uint8_t kFast12Arcs[16][16];
extern "C" const COMPV_ALIGN_DEFAULT() uint16_t Fast9Flags[16];
extern "C" const COMPV_ALIGN_DEFAULT() uint16_t Fast12Flags[16];

extern "C" void FastStrengths16(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x16, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);

COMPV_NAMESPACE_BEGIN()

void FastStrengths16_Intrin_SSE41(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x16, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv_scalar_t N)
{
    COMPV_DEBUG_INFO_CHECK_SSE41();
    __m128i xmm0, xmm1, xmmFastXFlagsLow, xmmFastXFlagsHigh, xmmFdarkers, xmmFbrighters;
    __m128i xmmZeros;
    int r0 = 0, r1 = 0, maxn;
    int lowMin, highMin;
    uint16_t rb = (uint16_t)rbrighters, rd = (uint16_t)rdarkers;
    const uint16_t(&FastXFlags)[16] = N == 9 ? Fast9Flags : Fast12Flags;
    const uint8_t(&kFastArcs)[16][16] = (N == 9 ? kFast9Arcs : kFast12Arcs);

    // FAST hard-coded flags
    _mm_store_si128(&xmmFastXFlagsLow, _mm_load_si128((__m128i*)(FastXFlags + 0)));
    _mm_store_si128(&xmmFastXFlagsHigh, _mm_load_si128((__m128i*)(FastXFlags + 8)));

    _mm_store_si128(&xmmZeros, _mm_setzero_si128());

    // xmm0 contains the u8 values
    // xmm1 is used as temp register and will be trashed
#define COMPV_HORIZ_MIN(r, i, maxn_) \
	if (r & (1 << i)) { \
		_mm_store_si128(&xmm1, _mm_shuffle_epi8(xmm0, _mm_load_si128((__m128i*)kFastArcs[i]))); /* eliminate zeros and duplicate first matching non-zero */ \
		lowMin = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_unpacklo_epi8(xmm1, xmmZeros))); \
		highMin = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_unpackhi_epi8(xmm1, xmmZeros))); \
		/* clear the index bits [16:18] */ \
		lowMin &= 0xFFFF; \
		highMin &= 0xFFFF; \
		maxn_ = std::max(std::min(lowMin, highMin), (int)maxn_); \
	}

    for (unsigned p = 0; p < 16; ++p) {
        maxn = 0;
        // Brighters
        if (rb & (1 << p)) {
            // brighters flags
            _mm_store_si128(&xmmFbrighters, _mm_set1_epi16((short)(*fbrighters16)[p]));

            _mm_store_si128(&xmm0, _mm_and_si128(xmmFbrighters, xmmFastXFlagsLow));
            _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
            _mm_store_si128(&xmm1, _mm_and_si128(xmmFbrighters, xmmFastXFlagsHigh));
            _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
            r0 = _mm_movemask_epi8(_mm_packs_epi16(xmm0, xmm1)); // r0's popcnt is equal to N as FastXFlags contains values with popcnt==N
            if (r0) {
                _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)dbrighters16x16));
                // Compute minimum hz
                COMPV_HORIZ_MIN(r0, 0, maxn) COMPV_HORIZ_MIN(r0, 1, maxn) COMPV_HORIZ_MIN(r0, 2, maxn) COMPV_HORIZ_MIN(r0, 3, maxn)
                COMPV_HORIZ_MIN(r0, 4, maxn) COMPV_HORIZ_MIN(r0, 5, maxn) COMPV_HORIZ_MIN(r0, 6, maxn) COMPV_HORIZ_MIN(r0, 7, maxn)
                COMPV_HORIZ_MIN(r0, 8, maxn) COMPV_HORIZ_MIN(r0, 9, maxn) COMPV_HORIZ_MIN(r0, 10, maxn) COMPV_HORIZ_MIN(r0, 11, maxn)
                COMPV_HORIZ_MIN(r0, 12, maxn) COMPV_HORIZ_MIN(r0, 13, maxn) COMPV_HORIZ_MIN(r0, 14, maxn) COMPV_HORIZ_MIN(r0, 15, maxn)
            }
        }

        // Darkers
        if (rd & (1 << p)) {
            // darkers flags
            _mm_store_si128(&xmmFdarkers, _mm_set1_epi16((short)(*fdarkers16)[p]));

            _mm_store_si128(&xmm0, _mm_and_si128(xmmFdarkers, xmmFastXFlagsLow));
            _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
            _mm_store_si128(&xmm1, _mm_and_si128(xmmFdarkers, xmmFastXFlagsHigh));
            _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
            r1 = _mm_movemask_epi8(_mm_packs_epi16(xmm0, xmm1)); // r1's popcnt is equal to N as FastXFlags contains values with popcnt==N
            if (r1) {
                _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)ddarkers16x16));

                // Compute minimum hz
                COMPV_HORIZ_MIN(r1, 0, maxn) COMPV_HORIZ_MIN(r1, 1, maxn) COMPV_HORIZ_MIN(r1, 2, maxn) COMPV_HORIZ_MIN(r1, 3, maxn)
                COMPV_HORIZ_MIN(r1, 4, maxn) COMPV_HORIZ_MIN(r1, 5, maxn) COMPV_HORIZ_MIN(r1, 6, maxn) COMPV_HORIZ_MIN(r1, 7, maxn)
                COMPV_HORIZ_MIN(r1, 8, maxn) COMPV_HORIZ_MIN(r1, 9, maxn) COMPV_HORIZ_MIN(r1, 10, maxn) COMPV_HORIZ_MIN(r1, 11, maxn)
                COMPV_HORIZ_MIN(r1, 12, maxn) COMPV_HORIZ_MIN(r1, 13, maxn) COMPV_HORIZ_MIN(r1, 14, maxn) COMPV_HORIZ_MIN(r1, 15, maxn)
            }
        }

        strengths16[p] = (uint8_t)maxn;

        dbrighters16x16 += 16;
        ddarkers16x16 += 16;
    }

#undef COMPV_HORIZ_MIN
}

// TODO(dmi): add ASM version
// TODO(dmi) this is a temp function to replace the AVX version until we found a non SSE/AVX mixing implementation
void FastStrengths32_Intrin_SSE41(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x32, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x32, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv_scalar_t N)
{
    COMPV_DEBUG_INFO_CHECK_SSE41();
    __m128i xmm0, xmm1, xmmFastXFlagsLow, xmmFastXFlagsHigh, xmmFX;
    __m128i xmmZeros;
    int r0 , r1, g = 0, maxn;
    int lowMin, highMin;
    uint32_t rb = (uint32_t)rbrighters, rd = (uint32_t)rdarkers;
    const uint16_t(&FastXFlags)[16] = N == 9 ? Fast9Flags : Fast12Flags;
    const uint8_t(&kFastArcs)[16][16] = (N == 9 ? kFast9Arcs : kFast12Arcs);
    bool bHighPartDone = false;

    // FAST hard-coded flags
    _mm_store_si128(&xmmFastXFlagsLow, _mm_load_si128((__m128i*)(FastXFlags + 0)));
    _mm_store_si128(&xmmFastXFlagsHigh, _mm_load_si128((__m128i*)(FastXFlags + 8)));

    _mm_store_si128(&xmmZeros, _mm_setzero_si128());

    // xmm0 contains the u8 values
    // xmm1 is used as temp register and will be trashed
#define COMPV_HORIZ_MIN(r, i, maxn_) \
	if (r & (1 << i)) { \
		_mm_store_si128(&xmm1, _mm_shuffle_epi8(xmm0, _mm_load_si128((__m128i*)kFastArcs[i]))); /* eliminate zeros and duplicate first matching non-zero */ \
		lowMin = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_unpacklo_epi8(xmm1, xmmZeros))); \
		highMin = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_unpackhi_epi8(xmm1, xmmZeros))); \
		/* clear the index (bits [16:18]) */ \
		lowMin &= 0xFFFF; \
		highMin &= 0xFFFF; \
		maxn_ = (uint8_t)std::max(std::min(lowMin, highMin), (int)maxn_); \
		}

process16:
    for (unsigned p = 0, v = 0; p < 16; ++p, v += 32) {
        maxn = 0;
        // Brighters
        if (rb & (1 << p)) {
            // brighters flags
            _mm_store_si128(&xmmFX, _mm_set1_epi16((short)((*fbrighters16)[p] >> g)));

            _mm_store_si128(&xmm0, _mm_and_si128(xmmFX, xmmFastXFlagsLow));
            _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
            _mm_store_si128(&xmm1, _mm_and_si128(xmmFX, xmmFastXFlagsHigh));
            _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
            r0 = _mm_movemask_epi8(_mm_packs_epi16(xmm0, xmm1)); // r0's popcnt is equal to N as FastXFlags contains values with popcnt==N
            if (r0) {
                _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)&dbrighters16x32[v]));
                // Compute minimum hz
                COMPV_HORIZ_MIN(r0, 0, maxn) COMPV_HORIZ_MIN(r0, 1, maxn) COMPV_HORIZ_MIN(r0, 2, maxn) COMPV_HORIZ_MIN(r0, 3, maxn)
                COMPV_HORIZ_MIN(r0, 4, maxn) COMPV_HORIZ_MIN(r0, 5, maxn) COMPV_HORIZ_MIN(r0, 6, maxn) COMPV_HORIZ_MIN(r0, 7, maxn)
                COMPV_HORIZ_MIN(r0, 8, maxn) COMPV_HORIZ_MIN(r0, 9, maxn) COMPV_HORIZ_MIN(r0, 10, maxn) COMPV_HORIZ_MIN(r0, 11, maxn)
                COMPV_HORIZ_MIN(r0, 12, maxn) COMPV_HORIZ_MIN(r0, 13, maxn) COMPV_HORIZ_MIN(r0, 14, maxn) COMPV_HORIZ_MIN(r0, 15, maxn)
            }
        }

        // Darkers
        if (rd & (1 << p)) {
            // darkers flags
            _mm_store_si128(&xmmFX, _mm_set1_epi16((short)((*fdarkers16)[p] >> g)));

            _mm_store_si128(&xmm0, _mm_and_si128(xmmFX, xmmFastXFlagsLow));
            _mm_store_si128(&xmm0, _mm_cmpeq_epi16(xmm0, xmmFastXFlagsLow));
            _mm_store_si128(&xmm1, _mm_and_si128(xmmFX, xmmFastXFlagsHigh));
            _mm_store_si128(&xmm1, _mm_cmpeq_epi16(xmm1, xmmFastXFlagsHigh));
            r1 = _mm_movemask_epi8(_mm_packs_epi16(xmm0, xmm1)); // r1's popcnt is equal to N as FastXFlags contains values with popcnt==N
            if (r1) {
                _mm_store_si128(&xmm0, _mm_load_si128((__m128i*)&ddarkers16x32[v]));

                // Compute minimum hz
                COMPV_HORIZ_MIN(r1, 0, maxn) COMPV_HORIZ_MIN(r1, 1, maxn) COMPV_HORIZ_MIN(r1, 2, maxn) COMPV_HORIZ_MIN(r1, 3, maxn)
                COMPV_HORIZ_MIN(r1, 4, maxn) COMPV_HORIZ_MIN(r1, 5, maxn) COMPV_HORIZ_MIN(r1, 6, maxn) COMPV_HORIZ_MIN(r1, 7, maxn)
                COMPV_HORIZ_MIN(r1, 8, maxn) COMPV_HORIZ_MIN(r1, 9, maxn) COMPV_HORIZ_MIN(r1, 10, maxn) COMPV_HORIZ_MIN(r1, 11, maxn)
                COMPV_HORIZ_MIN(r1, 12, maxn) COMPV_HORIZ_MIN(r1, 13, maxn) COMPV_HORIZ_MIN(r1, 14, maxn) COMPV_HORIZ_MIN(r1, 15, maxn)
            }
        }

        strengths32[p] = maxn;
    }

    if (!bHighPartDone) {
        rb = (rb >> 16) & 0xFFFF;
        rd = (rd >> 16) & 0xFFFF;
        if (rb || rd) {
            g = 16;
            bHighPartDone = true;
            dbrighters16x32 += 16;
            ddarkers16x32 += 16;
            strengths32 += 16;
            goto process16;
        }
        else {
            _mm_storeu_si128((__m128i*)(strengths32 + 16), xmmZeros);
        }
    }
#undef COMPV_HORIZ_MIN
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
