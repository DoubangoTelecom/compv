/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_bits.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define CompVCannyHysteresisRowWeak_8mpw_Intrin_SSE2(gg, pp, rr, cc) \
	vecgg = _mm_loadu_si128(reinterpret_cast<const __m128i*>((gg))); \
	vecpp = _mm_loadl_epi64(reinterpret_cast<const __m128i*>((pp))); \
	vecWeak = _mm_and_si128(_mm_cmpeq_epi16(_mm_unpacklo_epi8(vecpp, vecpp), vecZero), _mm_cmpgt_epi16(vecgg, vecTLow)); \
	vecWeak = _mm_and_si128(vecWeak, vecp); \
	mask = _mm_movemask_epi8(_mm_packs_epi16(vecWeak, vecWeak)); \
	if (mask) { \
		mask &= 0xff; /*vecWeak duplicated because of packs(vecWeak, vecWeak) -> clear high and keep low */\
		compv_bsf_t ii; \
		do { \
			compv_bsf(mask, &ii); \
			const size_t cc_ii = cc + ii; \
			if (cc_ii >= width) break; \
			mask ^= (1 << ii); \
			(pp)[ii] = 0xff; \
			edges.push_back(CompVMatIndex(rr, cc_ii)); \
		} while (mask); \
	}


// TODO(dmi): no ASM implementation
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 8mpw -> minpack 8 for words (int16)
void CompVCannyHysteresisRow_8mpw_Intrin_SSE2(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("CompVCannyHysteresisRow_16mpw_Intrin_SSE2 is faster");
	__m128i vecGrad, vecStrong, vecWeak, vecgg, vecpp, vecp;
	const __m128i vecTLow = _mm_set1_epi16(tLow);
	const __m128i vecTHigh = _mm_set1_epi16(tHigh);
	const __m128i vecZero = _mm_setzero_si128();
	compv_bsf_t mask;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	CompVMatIndex edge;
	bool lookingForStrongEdges;
	// std::vector is faster than std::list, std::dequeue and std::stack (perf. done using Intel VTune on core i7)
	// also, check https://baptiste-wicht.com/posts/2012/11/cpp-benchmark-vector-vs-list.html
	std::vector<CompVMatIndex> edges;
	while (colStart < width - 7 || !edges.empty()) { // width is already >=8 (checked by the caller)
		if ((lookingForStrongEdges = edges.empty())) { // looking for strong edges if there is no pending edge in the vector
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
			if (lookingForStrongEdges) {
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
}

// TODO(dmi): no ASM implementation
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 16mpw -> minpack 16 for words (int16)
void CompVCannyHysteresisRow_16mpw_Intrin_SSE2(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	int16_t col;
	__m128i vecG, vecP, vec0, vec1, vec2;
	const __m128i vecTLow = _mm_set1_epi16(tLow);
	const __m128i vecTHigh = _mm_set1_epi16(tHigh);
	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vecMaskFF = _mm_setr_epi16(-1, -1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
	static const __m128i vecMaskFFF = _mm_setr_epi16(-1, -1, -1, 0x0, 0x0, 0x0, 0x0, 0x0);
	compv_bsf_t m0, m1, mi;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	int16_t c, r;
	size_t s;
	uint8_t *pb, *pt;
	uint32_t edge;
	const int16_t width_ = static_cast<int16_t>(width);
	const int16_t height_ = static_cast<int16_t>(height);
	const int16_t maxWidth = static_cast<int16_t>(width - 15);
	const int32_t rowlsl16 = static_cast<int32_t>(row << 16);
	// std::vector is faster than std::list, std::dequeue and std::stack (perf. done using Intel VTune on core i7)
	// also, check https://baptiste-wicht.com/posts/2012/11/cpp-benchmark-vector-vs-list.html
	std::vector<uint32_t> edges;

	for (col = static_cast<int16_t>(colStart); col < maxWidth; col += 16) { // width is alredy >=8 (checked by the caller)
		vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&e[col]));
		vec1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&grad[col]));
		vec2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&grad[col + 8]));
		vec1 = _mm_packs_epi16(_mm_cmpgt_epi16(vec1, vecTHigh), _mm_cmpgt_epi16(vec2, vecTHigh));
		m0 = _mm_movemask_epi8(_mm_and_si128(_mm_cmpeq_epi8(vec0, vecZero), vec1));
		if (!m0) {
			continue;
		}

		do {
			compv_bsf(m0, &mi);
			m0 ^= (1 << mi); // asm: "btr" (http://www.felixcloutier.com/x86/BTR.html) - Visual Studio: "_bittestandreset"
			e[col + mi] = 0xff;
			edges.push_back(rowlsl16 | (col + mi));
			while (!edges.empty()) {
				edge = edges.back();
				edges.pop_back();
				c = edge & 0xffff;
				r = edge >> 16;
				if (r && c && r < height_ && c < width_) {
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
#endif
					vecG = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&g[-1])), 0x8), vecMaskFF),
						_mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gt[-1])), vecMaskFFF), 4)),
						_mm_slli_si128(_mm_and_si128(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&gb[-1])), vecMaskFFF), 10));
					vec0 = _mm_cmpgt_epi16(vecG, vecTLow);
					if (_mm_movemask_epi8(vec0)) {
						vecP = _mm_or_si128(_mm_or_si128(_mm_and_si128(_mm_shufflelo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&p[-1])), vecZero), 0x8), vecMaskFF),
							_mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pt[-1])), vecZero), vecMaskFFF), 4)),
							_mm_slli_si128(_mm_and_si128(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*reinterpret_cast<const int32_t*>(&pb[-1])), vecZero), vecMaskFFF), 10));

						vec1 = _mm_and_si128(_mm_cmpeq_epi16(vecP, vecZero), vec0);
						m1 = _mm_movemask_epi8(_mm_packs_epi16(vec1, vec1));
						if (m1) {
							m1 &= 0xff; /* vec0 duplicated because of packs(vec1, vec1) -> clear high and keep low */
							do {
								compv_bsf(m1, &mi);
								m1 ^= (1 << mi);
								switch (mi) {
								case 0: p[-1] = 0xff; edges.push_back((r << 16) | (c - 1)); break; // left	
								case 1: p[1] = 0xff; edges.push_back((r << 16) | (c + 1)); break; // right
								case 2: pt[-1] = 0xff; edges.push_back(((r - 1) << 16) | (c - 1)); break; // top-left
								case 3: *pt = 0xff; edges.push_back(((r - 1) << 16) | (c)); break; // top-center
								case 4: pt[1] = 0xff;  edges.push_back(((r - 1) << 16) | (c + 1)); break; // top-right
								case 5: pb[-1] = 0xff;  edges.push_back(((r + 1)  << 16) | (c - 1)); break; // bottom-left
								case 6: *pb = 0xff;  edges.push_back(((r + 1) << 16) | (c)); break; // bottom-center
								case 7: pb[1] = 0xff;  edges.push_back(((r + 1) << 16) | (c + 1)); break; // bottom-right
								}
							} while (m1);
						}
					}
				}
			}
		} while (m0);
	}
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