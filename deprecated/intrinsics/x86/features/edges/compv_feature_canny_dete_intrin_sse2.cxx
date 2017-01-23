/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/intrinsics/x86/features/edges/compv_feature_canny_dete_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/features/edges/compv_feature_canny_dete.h"
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void CannyNMSApply_Intrin_SSE2(COMPV_ALIGNED(SSE) uint16_t* grad, COMPV_ALIGNED(SSE) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
    COMPV_DEBUG_INFO_CHECK_SSE2();

    __m128i xmm0;
    compv_uscalar_t col_, row_;
    static const __m128i xmmZero = _mm_setzero_si128();
    for (row_ = 1; row_ < height; ++row_) {
        for (col_ = 0; col_ < width; col_ += 8) { // SIMD, starts at 0 (instead of 1) to have memory aligned
            xmm0 = _mm_cmpeq_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&nms[col_])), xmmZero);
            if (_mm_movemask_epi8(xmm0) ^ 0xffff) {
                xmm0 = _mm_and_si128(_mm_unpacklo_epi8(xmm0, xmm0), _mm_load_si128(reinterpret_cast<const __m128i*>(&grad[col_])));
                _mm_store_si128(reinterpret_cast<__m128i*>(&grad[col_]), xmm0);
                _mm_storel_epi64(reinterpret_cast<__m128i*>(&nms[col_]), xmmZero);
            }
        }
        nms += stride;
        grad += stride;
    }
}

// TODO(dmi): add ASM
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
void CannyHysteresis_Intrin_SSE2(compv_uscalar_t row, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0, CompVPtr<CompVBox<CompVIndex>* >& candidates)
{
    COMPV_DEBUG_INFO_CHECK_SSE2();
    compv_uscalar_t col, mi;
    __m128i xmmG, xmmP, xmmGrad, xmmE;
    const __m128i xmmTLow = _mm_set1_epi16(tLow);
    const __m128i xmmTHigh = _mm_set1_epi16(tHigh);
    static const __m128i xmmZero = _mm_setzero_si128();
    static const __m128i xmmMaskFF = _mm_setr_epi16(-1, -1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
    static const __m128i xmmMaskFFF = _mm_setr_epi16(-1, -1, -1, 0x0, 0x0, 0x0, 0x0, 0x0);
    int m0, m1, mf;
    const CompVIndex* edge;
    uint8_t* p;
    const uint16_t *g, *gb, *gt;
    size_t c, r, s;
    uint8_t *pb, *pt;
    CompVIndex* ne;

    for (col = 1; col < width - 7; col += 8) {
        xmmGrad = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&grad[col]));
        xmmE = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&e[col]));
        m0 = _mm_movemask_epi8(_mm_and_si128(_mm_cmpeq_epi16(_mm_unpacklo_epi8(xmmE, xmmE), xmmZero), _mm_cmpgt_epi16(xmmGrad, xmmTHigh)));
        if (m0) {
            mi = 0, mf = 3;
            do {
                if (m0 & mf) {
                    e[col + mi] = 0xff;
                    COMPV_CANNY_PUSH_CANDIDATE(candidates, row, col + mi);
                    while ((edge = candidates->pop_back())) {
                        c = edge->col;
                        r = edge->row;
                        if (r && c && r < height && c < width) {
                            s = (r * stride) + c;
                            p = e0 + s;
                            g = g0 + s;
                            pb = p + stride;
                            pt = p - stride;
                            gb = g + stride;
                            gt = g - stride;
                            xmmG = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&g[-1])), 0x8), xmmMaskFF),
                                                             _mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gt[-1])), xmmMaskFFF), 4)),
                                                _mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gb[-1])), xmmMaskFFF), 10));
                            xmmP = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&p[-1])), xmmZero), 0x8), xmmMaskFF),
                                                             _mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pt[-1])), xmmZero), xmmMaskFFF), 4)),
                                                _mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pb[-1])), xmmZero), xmmMaskFFF), 10));
                            m1 = _mm_movemask_epi8(_mm_and_si128(_mm_cmpeq_epi16(xmmP, xmmZero), _mm_cmpgt_epi16(xmmG, xmmTLow)));
                            if (m1) {
                                if (m1 & 0x00ff) {
                                    if (m1 & 0x0003) { // left
                                        p[-1] = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r, c - 1);
                                    }
                                    if (m1 & 0x000c) { // right
                                        p[1] = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r, c + 1);
                                    }
                                    if (m1 & 0x0030) { // top-left
                                        pt[-1] = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r - 1, c - 1);
                                    }
                                    if (m1 & 0x00c0) { // top-center
                                        *pt = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r - 1, c);
                                    }
                                }
                                if (m1 & 0xff00) {
                                    if (m1 & 0x0300) { // top-right
                                        pt[1] = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r - 1, c + 1);
                                    }
                                    if (m1 & 0x0c00) { // bottom-left
                                        pb[-1] = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r + 1, c - 1);
                                    }
                                    if (m1 & 0x3000) { // bottom-center
                                        *pb = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r + 1, c);
                                    }
                                    if (m1 & 0xc000) { // bottom-right
                                        pb[1] = 0xff;
                                        COMPV_CANNY_PUSH_CANDIDATE(candidates, r + 1, c + 1);
                                    }
                                }
                            }
                        }
                    }
                    m0 ^= mf;
                }
                mf <<= 2, ++mi;
            }
            while (m0);
        }
    }

    if (col < width) {
        CannyHysteresisRow_C(row, col, width, height, stride, tLow, tHigh, grad, g0, e, e0, candidates);
    }
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
