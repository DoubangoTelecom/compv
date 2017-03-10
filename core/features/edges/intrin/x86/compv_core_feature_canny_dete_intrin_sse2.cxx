/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): no ASM implementation
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 8mpw -> minpack 8 for words (int16)
void CompVCannyHysteresisRow_8mpw_Intrin_SSE2(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t col, mi;
	__m128i vecG, vecP, vecGrad, vecE;
	const __m128i vecTLow = _mm_set1_epi16(tLow);
	const __m128i vecTHigh = _mm_set1_epi16(tHigh);
	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vecMaskFF = _mm_setr_epi16(-1, -1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
	static const __m128i vecMaskFFF = _mm_setr_epi16(-1, -1, -1, 0x0, 0x0, 0x0, 0x0, 0x0);
	int m0, m1, mf;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	CompVMatIndex edge;
	std::vector<CompVMatIndex> edges;

	for (col = colStart; col < width - 7; col += 8) { // width is alredy >=8 (checked by the caller)
		vecGrad = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&grad[col]));
		vecE = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&e[col]));
		m0 = _mm_movemask_epi8(_mm_and_si128(_mm_cmpeq_epi16(_mm_unpacklo_epi8(vecE, vecE), vecZero), _mm_cmpgt_epi16(vecGrad, vecTHigh)));
		if (m0) {
			mi = 0, mf = 3;
			do {
				if (m0 & mf) {
					e[col + mi] = 0xff;
					edges.push_back(CompVMatIndex(row, col + mi));
					while (!edges.empty()) {
						edge = edges.back();
						edges.pop_back();
						c = edge.col;
						r = edge.row;
						if (r && c && r < height && c < width) {
							s = (r * stride) + c;
							p = e0 + s;
							g = g0 + s;
							pb = p + stride;
							pt = p - stride;
							gb = g + stride;
							gt = g - stride;
							vecG = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&g[-1])), 0x8), vecMaskFF),
								_mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gt[-1])), vecMaskFFF), 4)),
								_mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gb[-1])), vecMaskFFF), 10));
							vecP = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&p[-1])), vecZero), 0x8), vecMaskFF),
								_mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pt[-1])), vecZero), vecMaskFFF), 4)),
								_mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pb[-1])), vecZero), vecMaskFFF), 10));
							m1 = _mm_movemask_epi8(_mm_and_si128(_mm_cmpeq_epi16(vecP, vecZero), _mm_cmpgt_epi16(vecG, vecTLow)));
							if (m1) {
								if (m1 & 0x00ff) {
									if (m1 & 0x0003) { // left
										p[-1] = 0xff;
										edges.push_back(CompVMatIndex(r, c - 1));
									}
									if (m1 & 0x000c) { // right
										p[1] = 0xff;
										edges.push_back(CompVMatIndex(r, c + 1));
									}
									if (m1 & 0x0030) { // top-left
										pt[-1] = 0xff;
										edges.push_back(CompVMatIndex(r - 1, c - 1));
									}
									if (m1 & 0x00c0) { // top-center
										*pt = 0xff;
										edges.push_back(CompVMatIndex(r - 1, c));
									}
								}
								if (m1 & 0xff00) {
									if (m1 & 0x0300) { // top-right
										pt[1] = 0xff;
										edges.push_back(CompVMatIndex(r - 1, c + 1));
									}
									if (m1 & 0x0c00) { // bottom-left
										pb[-1] = 0xff;
										edges.push_back(CompVMatIndex(r + 1, c - 1));
									}
									if (m1 & 0x3000) { // bottom-center
										*pb = 0xff;
										edges.push_back(CompVMatIndex(r + 1, c));
									}
									if (m1 & 0xc000) { // bottom-right
										pb[1] = 0xff;
										edges.push_back(CompVMatIndex(r + 1, c + 1));
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
#if 0
	CompVMatIndex edge;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	uint32_t cmp32;
	std::vector<CompVMatIndex> edges;

	for (size_t col = colStart; col < width; ++col) {
		if (grad[col] > tHigh && !e[col]) { // strong edge and not connected yet
			e[col] = 0xff;
			edges.push_back(CompVMatIndex(row, col));
			while (!edges.empty()) {
				edge = edges.back();
				edges.pop_back();
				c = edge.col;
				r = edge.row;
				if (r && c && r < height && c < width) {
					s = (r * stride) + c;
					p = e0 + s;
					g = g0 + s;
					pb = p + stride;
					pt = p - stride;
					gb = g + stride;
					gt = g - stride;
					if (g[-1] > tLow && !p[-1]) { // left
						p[-1] = 0xff;
						edges.push_back(CompVMatIndex(r, c - 1));
					}
					if (g[1] > tLow && !p[1]) { // right
						p[1] = 0xff;
						edges.push_back(CompVMatIndex(r, c + 1));
					}
					/* TOP */
					cmp32 = *reinterpret_cast<const uint32_t*>(&pt[-1]) ^ 0xffffff;
					if (cmp32) {
						if (cmp32 & 0xff && gt[-1] > tLow) { // left
							pt[-1] = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c - 1));
						}
						if (cmp32 & 0xff00 && gt[0] > tLow) { // center
							*pt = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c));
						}
						if (cmp32 & 0xff0000 && gt[1] > tLow && !pt[1]) { // right
							pt[1] = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c + 1));
						}
					}
					/* BOTTOM */
					cmp32 = *reinterpret_cast<const uint32_t*>(&pb[-1]) ^ 0xffffff;
					if (cmp32) {
						if (cmp32 & 0xff && gb[-1] > tLow) { // left
							pb[-1] = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c - 1));
						}
						if (cmp32 & 0xff00 && gb[0] > tLow) { // center
							*pb = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c));
						}
						if (cmp32 & 0xff0000 && gb[1] > tLow) { // right
							pb[1] = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c + 1));
						}
					}
				}
			}
		}
	}
#endif
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */