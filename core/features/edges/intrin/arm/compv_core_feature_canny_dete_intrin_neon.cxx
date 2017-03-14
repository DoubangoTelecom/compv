/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/intrin/arm/compv_core_feature_canny_dete_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): no ASM implementation
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 8mpw -> minpack 8 for words (int16)
void CompVCannyHysteresisRow_8mpw_Intrin_NEON(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t col, mi;
	uint16x8_t vecG, vecP, vecGrad, vec0;
	uint8x8_t vec0n;
	const uint16x8_t vecZero = vdupq_n_u8(0);
	const uint16x8_t vecTLow = vdupq_n_u16(tLow);
	const uint16x8_t vecTHigh = vdupq_n_u16(tHigh);
	uint16_t m0, mf;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	CompVMatIndex edge;
	// std::vector is faster than std::list, std::dequeue and std::stack (perf. done using Intel VTune on core i7)
	// also, check https://baptiste-wicht.com/posts/2012/11/cpp-benchmark-vector-vs-list.html
	std::vector<CompVMatIndex> edges;

	// mask for _mm_movemask_epi8
#if COMPV_ARCH_ARM32
	static const uint8x16_t vecMask = (uint32x4_t) { 0x8040201, 0x80402010, 0x8040201, 0x80402010 };
#else
	static const uint8x16_t vecMask = (uint64x2_t) { 9241421688590303745ULL, 9241421688590303745ULL };
#endif

	for (col = colStart; col < width - 7; col += 8) { // width is alredy >=8 (checked by the caller)
		vecGrad = vld1q_u16(&grad[col]); // unaligned load
		vec0 = vceqq_u16(vmovl_u8(vld1_u8(&e[col])), vecZero); // high 64bits then extend to 128bits (unaligned load)
		vec0 = vandq_u16(vec0, vcgtq_u16(vecGrad, vecTHigh));
		if (COMPV_ARM_NEON_EQ_ZERO(vec0)) {
			continue;
		}
		// _mm_movemask_epi8
		vec0 = vandq_u8(vec0, vecMask);
		vec0n = vpadd_u8(vget_low_u8(vec0), vget_high_u8(vec0));
		vec0n = vpadd_u8(vec0n, vec0n);
		vec0n = vpadd_u8(vec0n, vec0n);
		m0 = vget_lane_u16(vec0n, 0);
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
						vecG = (uint16x8_t) { g[-1], g[1], gt[-1], gt[0], gt[1], gb[-1], gb[0], gb[1] };
						vecP = (uint16x8_t) { p[-1], p[1], pt[-1], pt[0], pt[1], pb[-1], pb[0], pb[1] };
						vec0 = vceqq_u16(vecP, vecZero);
						vec0 = vandq_u16(vec0, vcgtq_u16(vecG, vecTLow));
						if (COMPV_ARM_NEON_NEQ_ZERO(vec0)) {
							if (vgetq_lane_u16(vec0, 0)) { // left
								p[-1] = 0xff;
								edges.push_back(CompVMatIndex(r, c - 1));
							}
							if (vgetq_lane_u16(vec0, 1)) { // right
								p[1] = 0xff;
								edges.push_back(CompVMatIndex(r, c + 1));
							}
							if (vgetq_lane_u16(vec0, 2)) { // top-left
								pt[-1] = 0xff;
								edges.push_back(CompVMatIndex(r - 1, c - 1));
							}
							if (vgetq_lane_u16(vec0, 3)) { // top-center
								*pt = 0xff;
								edges.push_back(CompVMatIndex(r - 1, c));
							}
							if (vgetq_lane_u16(vec0, 4)) { // top-right
								pt[1] = 0xff;
								edges.push_back(CompVMatIndex(r - 1, c + 1));
							}
							if (vgetq_lane_u16(vec0, 5)) { // bottom-left
								pb[-1] = 0xff;
								edges.push_back(CompVMatIndex(r + 1, c - 1));
							}
							if (vgetq_lane_u16(vec0, 6)) { // bottom-center
								*pb = 0xff;
								edges.push_back(CompVMatIndex(r + 1, c));
							}
							if (vgetq_lane_u16(vec0, 7)) { // bottom-right
								pb[1] = 0xff;
								edges.push_back(CompVMatIndex(r + 1, c + 1));
							}
						}
					}
				}
				m0 ^= mf;
			}
			mf <<= 2, ++mi;
		} while (m0);
	}
}

void CompVCannyNMSApply_Intrin_NEON(COMPV_ALIGNED(NEON) uint16_t* grad, COMPV_ALIGNED(NEON) uint8_t* nms, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	uint16x8_t vec0;
	uint8x8_t vec0n;
	compv_uscalar_t col_, row_;
	static const uint8x8_t vecZero = vdup_n_u8(0);
	for (row_ = 1; row_ < height; ++row_) { // row starts to #1 and ends at (heigth = imageHeigth - 1)
		for (col_ = 0; col_ < width; col_ += 8) { // SIMD, starts at 0 (instead of 1) to have memory aligned, reading beyong width which means data must be strided
			vec0n = vcgt_u8(vld1_u8(&nms[col_]), vecZero);
#if COMPV_ARCH_ARM64
			if (vgetq_lane_u64(vec0n, 0)) {
#else
			if (vget_lane_u32(vec0n, 0) || vget_lane_u32(vec0n, 1)) {
#endif
				vec0 = vmovl_u8(vec0n); // FIXME: hyst use this instead of vmovl_u8 abov
				vec0 = vsliq_n_u16(vec0, vec0, 8);
				vst1_u8(&nms[col_], vecZero);
				vst1q_u16(&grad[col_], vbicq_u16(vld1q_u16(&grad[col_]), vec0)); // suppress
			}
		}
		nms += stride;
		grad += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */