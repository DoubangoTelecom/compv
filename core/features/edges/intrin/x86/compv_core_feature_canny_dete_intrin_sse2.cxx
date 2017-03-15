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

#define CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, pp, rr, cc, mm, ii) \
	if ((mask & mm) && ((cc) + ii) < width) { \
		(pp)[ii] = 0xff; \
		edges.push_back(CompVMatIndex(rr, (cc) + ii)); \
	}

#define CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(gg, pp, rr, cc) \
	vecgg = _mm_loadu_si128(reinterpret_cast<const __m128i*>((gg))); \
	vecpp = _mm_loadl_epi64(reinterpret_cast<const __m128i*>((pp))); \
	vecWeak = _mm_and_si128(_mm_cmpeq_epi16(_mm_unpacklo_epi8(vecpp, vecpp), vecZero), _mm_cmpgt_epi16(vecgg, vecTLow)); \
	vecWeak = _mm_and_si128(vecWeak, vecp); \
	mask = _mm_movemask_epi8(vecWeak); \
	if (mask) { \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x1, 0); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x4, 1); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x10, 2); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x40, 3); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x100, 4); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x400, 5); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x1000, 6); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_SSE2(mask, (pp), (rr), (cc), 0x4000, 7); \
	}

// TODO(dmi): no ASM implementation
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 8mpw -> minpack 8 for words (int16)
void CompVCannyHysteresisRow_8mpw_Intrin_SSE2(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
#if 1
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
	// std::vector is faster than std::list, std::dequeue and std::stack (perf. done using Intel VTune on core i7)
	// also, check https://baptiste-wicht.com/posts/2012/11/cpp-benchmark-vector-vs-list.html
	std::vector<CompVMatIndex> edges;

	for (col = colStart; col < width - 7; col += 8) { // width is alredy >=8 (checked by the caller)
		vecGrad = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&grad[col]));
		vecE = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&e[col])); // high 64bits set to zero
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
#if 0
							vecG = _mm_setr_epi16(g[-1], g[1], gt[-1], gt[0], gt[1], gb[-1], gb[0], gb[1]);
							vecP = _mm_setr_epi16(p[-1], p[1], pt[-1], pt[0], pt[1], pb[-1], pb[0], pb[1]);
#else
							vecG = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&g[-1])), 0x8), vecMaskFF),
								_mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gt[-1])), vecMaskFFF), 4)),
								_mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gb[-1])), vecMaskFFF), 10));
							vecP = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&p[-1])), vecZero), 0x8), vecMaskFF),
								_mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pt[-1])), vecZero), vecMaskFFF), 4)),
								_mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pb[-1])), vecZero), vecMaskFFF), 10));
#endif
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
#else
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Slower than above code");
	__m128i vecGrad, vecStrong, vecWeak, vecgg, vecpp, vecp;
	const __m128i vecTLow = _mm_set1_epi16(tLow);
	const __m128i vecTHigh = _mm_set1_epi16(tHigh);
	const __m128i vecZero = _mm_setzero_si128();
	int mask;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	CompVMatIndex edge;
	bool lookingForStringEdges;
	// std::vector is faster than std::list, std::dequeue and std::stack (perf. done using Intel VTune on core i7)
	// also, check https://baptiste-wicht.com/posts/2012/11/cpp-benchmark-vector-vs-list.html
	std::vector<CompVMatIndex> edges;
	while (colStart < width - 7 || !edges.empty()) { // width is already >=8 (checked by the caller)
		if ((lookingForStringEdges = edges.empty())) { // looking for string edges if there is no pending edge in the vector
			c = colStart;
			r = row;
			p = &e[colStart];
			g = &grad[colStart];
			colStart += 8; // move to next samples
		}
		else {
			edge = edges.back(); // use saved edges
			edges.pop_back();
			c = edge.col;
			r = edge.row;
			s = (r * stride) + c;
			p = e0 + s;
			g = g0 + s;
		}
		if (r && c && r < height && c < width) {
			vecp = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p)); // high 64bits set to zero
			if (lookingForStringEdges) {
				vecGrad = _mm_loadu_si128(reinterpret_cast<const __m128i*>(g));
				vecStrong = _mm_and_si128(_mm_cmpeq_epi16(_mm_unpacklo_epi8(vecp, vecp), vecZero), _mm_cmpgt_epi16(vecGrad, vecTHigh));
				if (_mm_movemask_epi8(vecStrong)) {
					// write strong edges and update vecp
					vecp = _mm_or_si128(vecp, _mm_packs_epi16(vecStrong, vecStrong));
					_mm_storel_epi64(reinterpret_cast<__m128i*>(p), vecp);
				}
				else {
					continue;
				}
			}
			pb = p + stride;
			pt = p - stride;
			gb = g + stride;
			gt = g - stride;
			vecp = _mm_unpacklo_epi8(vecp, vecp); // 64bits -> 128bits
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&g[-1], &p[-1], r, c - 1); // left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&g[1], &p[1], r, c + 1); // right
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&gt[-1], &pt[-1], r - 1, c - 1); // top-left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&gt[0], &pt[0], r - 1, c); // top-center
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&gt[1], &pt[1], r - 1, c + 1); // top-right
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&gb[-1], &pb[-1], r + 1, c - 1); // bottom-left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&gb[0], &pb[0], r + 1, c); // bottom-center
			CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(&gb[1], &pb[1], r + 1, c + 1); // bottom-right
		}
	}
#endif
}

void CompVCannyNMSApply_Intrin_SSE2(COMPV_ALIGNED(SSE) uint16_t* grad, COMPV_ALIGNED(SSE) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	__m128i vec0;
	compv_uscalar_t col_, row_;
	static const __m128i vecZero = _mm_setzero_si128();
	for (row_ = 1; row_ < height; ++row_) { // row starts to #1 and ends at (heigth = imageHeigth - 1)
		for (col_ = 0; col_ < width; col_ += 8) { // SIMD, starts at 0 (instead of 1) to have memory aligned, reading beyong width which means data must be strided
			vec0 = _mm_cmpeq_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&nms[col_])), vecZero);
			if (_mm_movemask_epi8(vec0) ^ 0xffff) { // arm neon -> NotAllZeros(_mm_cmpgt_epu8(nms, zero))
				vec0 = _mm_and_si128(_mm_unpacklo_epi8(vec0, vec0), _mm_load_si128(reinterpret_cast<const __m128i*>(&grad[col_])));
				_mm_storel_epi64(reinterpret_cast<__m128i*>(&nms[col_]), vecZero);
				_mm_store_si128(reinterpret_cast<__m128i*>(&grad[col_]), vec0);
			}
		}
		nms += stride;
		grad += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */