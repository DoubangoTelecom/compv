/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/core/features/edges/compv_core_feature_canny_dete.h" /* kCannyTangentPiOver8Int and kCannyTangentPiTimes3Over8Int */
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#define CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, pp, rr, cc, mm, ii) \
	if (mask & mm && ((cc) + ii) < width) { \
		(pp)[ii] = 0xff; \
		edges.push_back(CompVMatIndex(rr, (cc) + ii)); \
	}

#define CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(gg, pp, rr, cc) \
	vecgg = _mm256_loadu_si256(reinterpret_cast<const __m256i*>((gg))); \
	vecppn = _mm_loadu_si128(reinterpret_cast<const __m128i*>((pp))); \
	vecpp = _mm256_broadcastsi128_si256(vecppn); \
	vecpp = _mm256_permute4x64_epi64(vecpp, 0xD8); \
	vecWeak = _mm256_and_si256(_mm256_cmpeq_epi16(_mm256_unpacklo_epi8(vecpp, vecpp), vecZero), _mm256_cmpgt_epi16(vecgg, vecTLow)); \
	vecWeak = _mm256_and_si256(vecWeak, vecp); \
	mask = _mm256_movemask_epi8(vecWeak); \
	if (mask) { \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x1, 0); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x4, 1); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x10, 2); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x40, 3); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x100, 4); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x400, 5); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x1000, 6); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x4000, 7); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x10000, 8); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x40000, 9); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x100000, 10); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x400000, 11); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x1000000, 12); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x4000000, 13); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x10000000, 14); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_AVX2(mask, (pp), (rr), (cc), 0x40000000, 15); \
	}


#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVCannyHysteresisRow_16mpw_Intrin_AVX2(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("SSE implementation is faster");
	_mm256_zeroupper();
	__m256i vecGrad, vecStrong, vecWeak, vecgg, vecpp, vecp;
	__m128i vecpn, vecppn;
	const __m256i vecTLow = _mm256_set1_epi16(tLow);
	const __m256i vecTHigh = _mm256_set1_epi16(tHigh);
	const __m256i vecZero = _mm256_setzero_si256();
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
			vecpn = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p)); // high 128bits set to zero
			if (lookingForStringEdges) {
				vecGrad = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(g));
				vecp = _mm256_broadcastsi128_si256(vecpn);
				vecp = _mm256_permute4x64_epi64(vecp, 0xD8);
				vecStrong = _mm256_and_si256(_mm256_cmpeq_epi16(_mm256_unpacklo_epi8(vecp, vecp), vecZero), _mm256_cmpgt_epi16(vecGrad, vecTHigh));
				if (_mm256_movemask_epi8(vecStrong)) {
					// write strong edges and update vecp
					vecpn = _mm_or_si128(vecpn, _mm256_castsi256_si128(compv_avx2_packs_epi16(vecStrong, vecStrong)));
					_mm_storeu_si128(reinterpret_cast<__m128i*>(p), vecpn);
				}
				else {
					continue;
				}
			}
			pb = p + stride;
			pt = p - stride;
			gb = g + stride;
			gt = g - stride;
			vecp = _mm256_broadcastsi128_si256(vecpn);
			vecp = _mm256_permute4x64_epi64(vecp, 0xD8);
			vecp = _mm256_unpacklo_epi8(vecp, vecp); // 64bits -> 128bits
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&g[-1], &p[-1], r, c - 1); // left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&g[1], &p[1], r, c + 1); // right
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&gt[-1], &pt[-1], r - 1, c - 1); // top-left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&gt[0], &pt[0], r - 1, c); // top-center
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&gt[1], &pt[1], r - 1, c + 1); // top-right
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&gb[-1], &pb[-1], r + 1, c - 1); // bottom-left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&gb[0], &pb[0], r + 1, c); // bottom-center
			CompVCannyHysteresisRowWeak_8mpw_Intrin_AVX2(&gb[1], &pb[1], r + 1, c + 1); // bottom-right
		}
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVCannyNMSApply_Intrin_AVX2(COMPV_ALIGNED(AVX) uint16_t* grad, COMPV_ALIGNED(AVX) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	_mm256_zeroupper();
	__m128i vec0n;
	__m256i vec0;
	compv_uscalar_t col_, row_;
	static const __m256i vecZero = _mm256_setzero_si256();
	for (row_ = 1; row_ < height; ++row_) { // row starts to #1 and ends at (heigth = imageHeigth - 1)
		for (col_ = 0; col_ < width; col_ += 16) { // SIMD, starts at 0 (instead of 1) to have memory aligned, reading beyong width which means data must be strided
			vec0n = _mm_cmpeq_epi8(_mm_load_si128(reinterpret_cast<const __m128i*>(&nms[col_])), _mm256_castsi256_si128(vecZero));
			if (_mm_movemask_epi8(vec0n) ^ 0xffff) { // arm neon -> _mm_cmpgt_epu8(nms, zero)
				vec0 = _mm256_broadcastsi128_si256(vec0n);
				vec0 = _mm256_permute4x64_epi64(vec0, 0xD8);
				vec0 = _mm256_and_si256(_mm256_unpacklo_epi8(vec0, vec0), _mm256_load_si256(reinterpret_cast<const __m256i*>(&grad[col_])));
				_mm256_store_si256(reinterpret_cast<__m256i*>(&grad[col_]), vec0);
				_mm_store_si128(reinterpret_cast<__m128i*>(&nms[col_]), _mm256_castsi256_si128(vecZero));
			}
		}
		nms += stride;
		grad += stride;
	}
	_mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 16mpw -> minpack 16 for words (int16)
// TODO(dmi): not optiz -> Visual studio 2015 core i7 x64, run 1k times: asm: 548ms, Intrin: 730ms
void CompVCannyNMSGatherRow_16mpw_Intrin_AVX2(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, const uint16_t* tLow1, compv_uscalar_t width, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_AVX2(); // AVX/SSE transition issues
	_mm256_zeroupper();
	__m256i vecG, vecGX, vecAbsGX0, vecAbsGX1, vecGY, vecAbsGY0, vecAbsGY1, vec0, vec1, vec2, vec3, vec4, vec5, vec6;
	__m128i vecNMS;
	const __m256i vecTLow = _mm256_set1_epi16(*tLow1);
	static const __m256i vecZero = _mm256_setzero_si256();
	static const __m256i vecTangentPiOver8Int = _mm256_set1_epi32(kCannyTangentPiOver8Int);
	static const __m256i vecTangentPiTimes3Over8Int = _mm256_set1_epi32(kCannyTangentPiTimes3Over8Int);
	compv_uscalar_t col;
	const int stride_ = static_cast<const int>(stride);
	const int c0 = 1 - stride_, c1 = 1 + stride_;

	for (col = 1; col < width - 15; col += 16) { // up to the caller to check that width is >= 16
		vecG = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&g[col]));
		vec0 = _mm256_cmpgt_epi16(vecG, vecTLow);
		if (!_mm256_testz_si256(vec0, vec0)) {
			vecNMS = _mm_setzero_si128();
			vecGX = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&gx[col]));
			vecGY = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&gy[col]));

			vec1 = _mm256_abs_epi16(vecGY);
			vec2 = _mm256_abs_epi16(vecGX);
			vec1 = _mm256_permute4x64_epi64(vec1, 0xD8);

			vecAbsGY0 = _mm256_unpacklo_epi16(vecZero, vec1); // convert from epi16 to epi32 the  "<< 16"
			vecAbsGY1 = _mm256_unpackhi_epi16(vecZero, vec1); // convert from epi16 to epi32 the  "<< 16"
			vecAbsGX0 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(vec2)); // convert from epi16 to epi32
			vecAbsGX1 = _mm256_cvtepu16_epi32(_mm256_extractf128_si256(vec2, 1)); // convert from epi16 to epi32

			// angle = "0° / 180°"
			vec1 = _mm256_cmpgt_epi32(_mm256_mullo_epi32(vecTangentPiOver8Int, vecAbsGX0), vecAbsGY0);
			vec2 = _mm256_cmpgt_epi32(_mm256_mullo_epi32(vecTangentPiOver8Int, vecAbsGX1), vecAbsGY1);
			vec3 = _mm256_and_si256(_mm256_permute4x64_epi64(vec0, 0xD8), _mm256_packs_epi32(vec1, vec2));
			if (!_mm256_testz_si256(vec3, vec3)) {
				vec3 = _mm256_permute4x64_epi64(vec3, 0xD8);
				vec1 = _mm256_cmpgt_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(&g[col - 1])), vecG);
				vec2 = _mm256_cmpgt_epi16(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&g[col + 1])), vecG);
				vec1 = _mm256_and_si256(vec3, _mm256_or_si256(vec1, vec2));
				vec1 = _mm256_permute4x64_epi64(_mm256_packs_epi16(vec1, vec1), 0xD8);
				vecNMS = _mm_or_si128(vecNMS, _mm256_castsi256_si128(vec1));
			}

			// angle = "45° / 225°" or "135 / 315"
			vec4 = _mm256_andnot_si256(vec3, vec0);
			if (!_mm256_testz_si256(vec4, vec4)) {
				vec1 = _mm256_cmpgt_epi32(_mm256_mullo_epi32(vecTangentPiTimes3Over8Int, vecAbsGX0), vecAbsGY0);
				vec2 = _mm256_cmpgt_epi32(_mm256_mullo_epi32(vecTangentPiTimes3Over8Int, vecAbsGX1), vecAbsGY1);
				vec4 = _mm256_and_si256(_mm256_permute4x64_epi64(vec4, 0xD8), _mm256_packs_epi32(vec1, vec2));
				if (!_mm256_testz_si256(vec4, vec4)) {
					vec4 = _mm256_permute4x64_epi64(vec4, 0xD8);
					vec1 = _mm256_cmpgt_epi16(vecZero, _mm256_xor_si256(vecGX, vecGY));
					vec1 = _mm256_and_si256(vec4, vec1);
					vec2 = _mm256_andnot_si256(vec1, vec4);
					if (!_mm256_testz_si256(vec1, vec1)) {
						vec5 = _mm256_cmpgt_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(&g[col - c0])), vecG);
						vec6 = _mm256_cmpgt_epi16(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&g[col + c0])), vecG);
						vec1 = _mm256_and_si256(vec1, _mm256_or_si256(vec5, vec6));
					}
					if (!_mm256_testz_si256(vec2, vec2)) {
						vec5 = _mm256_cmpgt_epi16(_mm256_load_si256(reinterpret_cast<const __m256i*>(&g[col - c1])), vecG);
						vec6 = _mm256_cmpgt_epi16(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&g[col + c1])), vecG);
						vec2 = _mm256_and_si256(vec2, _mm256_or_si256(vec5, vec6));
					}
					vec1 = _mm256_or_si256(vec1, vec2);
					vec1 = _mm256_permute4x64_epi64(_mm256_packs_epi16(vec1, vec1), 0xD8);
					vecNMS = _mm_or_si128(vecNMS, _mm256_castsi256_si128(vec1));
				}
			}

			// angle = "90° / 270°"
			vec5 = _mm256_andnot_si256(vec3, _mm256_andnot_si256(vec4, vec0));
			if (!_mm256_testz_si256(vec5, vec5)) {
				vec1 = _mm256_cmpgt_epi16(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&g[col - stride])), vecG);
				vec2 = _mm256_cmpgt_epi16(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&g[col + stride])), vecG);
				vec1 = _mm256_and_si256(vec5, _mm256_or_si256(vec1, vec2));
				vec1 = _mm256_permute4x64_epi64(_mm256_packs_epi16(vec1, vec1), 0xD8);
				vecNMS = _mm_or_si128(vecNMS, _mm256_castsi256_si128(vec1));
			}

			_mm_storeu_si128(reinterpret_cast<__m128i*>(&nms[col]), vecNMS);
		}
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */