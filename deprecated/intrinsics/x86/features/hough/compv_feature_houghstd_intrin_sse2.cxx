/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/hough/compv_feature_houghstd_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/features/hough/compv_feature_houghstd.h"
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM
void HoughStdNmsGatherRow_Intrin_SSE2(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, int32_t nThreshold, compv_uscalar_t width)
{
    COMPV_DEBUG_INFO_CHECK_SSE2();
    compv_uscalar_t col, maxColsSSE = width - 3; // must not go beyond the stride as "acc" and "nms" have different ones
    const __m128i xmmThreshold = _mm_set1_epi32(nThreshold);
    int stride = static_cast<int>(nAccStride);
    __m128i xmmAcc, xmm0, xmm1;
    const int32_t *curr, *top, *bottom;

    for (col = 1; col < maxColsSSE; col += 4) {
        xmmAcc = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&pAcc[col]));
        xmm0 = _mm_cmpgt_epi32(xmmAcc, xmmThreshold);
        if (_mm_movemask_epi8(xmm0)) {
            curr = &pAcc[col];
            top = &pAcc[col - stride];
            bottom = &pAcc[col + stride];
            xmm1 = _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&curr[-1])), xmmAcc);
            xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&curr[+1])), xmmAcc));
            xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[-1])), xmmAcc));
            xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[0])), xmmAcc));
            xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&top[+1])), xmmAcc));
            xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[-1])), xmmAcc));
            xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[0])), xmmAcc));
            xmm1 = _mm_or_si128(xmm1, _mm_cmpgt_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&bottom[+1])), xmmAcc));
            xmm0 = _mm_and_si128(xmm0, xmm1);
            *reinterpret_cast<uint32_t*>(&pNms[col]) = static_cast<uint32_t>(_mm_cvtsi128_si32(_mm_packs_epi16(_mm_packs_epi32(xmm0, xmm0), xmm0))); // ASM: second argument for packs could be anything, we only need the first 4 Bytes
        }
    }
    if (col < width) {
        HoughStdNmsGatherRow_C(pAcc, nAccStride, pNms, nThreshold, col, width);
    }
}

// TODO(dmi): add ASM
void HoughStdNmsApplyRow_Intrin_SSE2(COMPV_ALIGNED(SSE) int32_t* pACC, COMPV_ALIGNED(SSE) uint8_t* pNMS, int32_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t maxCols, CompVPtrBox(CompVCoordPolar2f)& coords)
{
    COMPV_DEBUG_INFO_CHECK_SSE2();
    compv_uscalar_t col, maxColsSSE = maxCols - 15; // must not go beyond the stride as "acc" and "nms" have different ones
    const __m128i xmmThreshold = _mm_set1_epi32(threshold);
    __m128i xmm0, xmm1, xmm2;
    static const __m128i xmmZero = _mm_setzero_si128();
    int m0, mi;
    CompVCoordPolar2f* coord;

    for (col = 0; col < maxColsSSE; col += 16) { // start at zero to have alignment
        xmm0 = _mm_cmpeq_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(&pNMS[col])), xmmZero);
        xmm1 = _mm_unpacklo_epi8(xmm0, xmm0);
        xmm2 = _mm_unpackhi_epi8(xmm0, xmm0);
        m0 = _mm_movemask_epi8(
                 _mm_packs_epi16(_mm_packs_epi32(_mm_and_si128(_mm_unpacklo_epi16(xmm1, xmm1), _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[col])), xmmThreshold)), _mm_and_si128(_mm_unpackhi_epi16(xmm1, xmm1), _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[col + 4])), xmmThreshold))),
                                 _mm_packs_epi32(_mm_and_si128(_mm_unpacklo_epi16(xmm2, xmm2), _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[col + 8])), xmmThreshold)), _mm_and_si128(_mm_unpackhi_epi16(xmm2, xmm2), _mm_cmpgt_epi32(_mm_load_si128(reinterpret_cast<const __m128i*>(&pACC[col + 12])), xmmThreshold)))));
        if (m0) {
            for (mi = 0; mi < 16 && m0; ++mi, m0 >>= 1) {
                if (m0 & 1) {
                    coords->new_item(&coord);
                    coord->count = pACC[col + mi];
                    coord->rho = static_cast<compv_float32_t>(barrier - row);
                    coord->theta = (col + mi) * theta;
                }
            }
        }
        _mm_store_si128(reinterpret_cast<__m128i*>(&pNMS[col]), xmmZero);
    }
    if (col < maxCols) {
        HoughStdNmsApplyRow_C(pACC, pNMS, threshold, theta, barrier, row, col, maxCols, coords);
    }
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
