/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/intrin/arm/compv_core_feature_canny_dete_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/core/features/edges/compv_core_feature_canny_dete.h" /* kCannyTangentPiOver8Int and kCannyTangentPiTimes3Over8Int */
#include "compv/base/compv_debug.h"
#include "compv/base/intrin/arm/compv_intrin_neon.h"

COMPV_NAMESPACE_BEGIN()

#define CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, pp, rr, cc, mm, ii) \
	if ((mask & mm) && ((cc) + ii) < width) { \
		(pp)[ii] = 0xff; \
		edges.push_back(CompVMatIndex(rr, (cc) + ii)); \
	}

#define CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(gg, pp, rr, cc) \
	vecgg = vld1q_u16((gg)); \
	vecppn = vld1_u8((pp)); \
	vecWeakn = vand_u8(vceq_u8(vecppn, vecZeron), vqmovn_u16(vcgtq_u16(vecgg, vecTLow))); \
	vecWeakn = vand_u8(vecWeakn, vecpn); \
	if ((mask = vget_lane_u32(vecWeakn, 0))) { \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0x000000ff, 0); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0x0000ff00, 1); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0x00ff0000, 2); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0xff000000, 3); \
	} \
	if ((mask = vget_lane_u32(vecWeakn, 1))) { \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0x000000ff, 4); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0x0000ff00, 5); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0x00ff0000, 6); \
		CompVCannyHysteresisRowMask_8mpw_Intrin_NEON(mask, (pp), (rr), (cc), 0xff000000, 7); \
	}

// TODO(dmi): no ASM implementation
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 8mpw -> minpack 8 for words (int16)
void CompVCannyHysteresisRow_8mpw_Intrin_NEON(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("CompVCannyHysteresisRow_16mpw_Intrin_NEON is faster");
	uint16x8_t vecGrad, vecgg;
	uint8x8_t vecpn, vecppn, vecStrongn, vecWeakn;
	const uint16x8_t vecTLow = vdupq_n_u16(tLow);
	const uint16x8_t vecTHigh = vdupq_n_u16(tHigh);
	const uint8x8_t vecZeron = vdup_n_u16(0);
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	uint32_t mask;
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
			vecpn = vld1_u8(p); // high 64bits set to zero
			if (lookingForStringEdges) {
				vecGrad = vld1q_u16(g);
				vecStrongn = vand_u8(vceq_u8(vecpn, vecZeron), vqmovn_u16(vcgtq_u16(vecGrad, vecTHigh)));
				if (vget_lane_u32(vecStrongn, 0) || vget_lane_u32(vecStrongn, 1)) {
					// write strong edges and update vecp
					vecpn = vorr_u8(vecpn, vecStrongn);
					vst1_u8(p, vecpn);
				}
				else {
					continue;
				}
			}
			pb = p + stride;
			pt = p - stride;
			gb = g + stride;
			gt = g - stride;
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&g[-1], &p[-1], r, c - 1); // left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&g[1], &p[1], r, c + 1); // right
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&gt[-1], &pt[-1], r - 1, c - 1); // top-left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&gt[0], &pt[0], r - 1, c); // top-center
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&gt[1], &pt[1], r - 1, c + 1); // top-right
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&gb[-1], &pb[-1], r + 1, c - 1); // bottom-left
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&gb[0], &pb[0], r + 1, c); // bottom-center
			CompVCannyHysteresisRowWeak_8mpw_Intrin_NEON(&gb[1], &pb[1], r + 1, c + 1); // bottom-right
		}
	}
}

// TODO(dmi): no ASM implementation
// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 16mpw -> minpack 16 for words (int16)
void CompVCannyHysteresisRow_16mpw_Intrin_NEON(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t col;
	size_t i, j;
	uint16x8_t vecG, vec0;
	uint8x8_t vec0n, vecPn;
	const uint8x8_t vecZeron = vdup_n_u8(0);
	const uint16x8_t vecZero = vdupq_n_u8(0);
	const uint16x8_t vecTLow = vdupq_n_u16(tLow);
	const uint16x8_t vecTHigh = vdupq_n_u16(tHigh);
	uint32_t mask = 0, mask0;
	uint8_t* p;
	const uint16_t *g, *gb, *gt;
	size_t c, r, s;
	uint8_t *pb, *pt;
	CompVMatIndex edge;
	// std::vector is faster than std::list, std::dequeue and std::stack (perf. done using Intel VTune on core i7)
	// also, check https://baptiste-wicht.com/posts/2012/11/cpp-benchmark-vector-vs-list.html
	std::vector<CompVMatIndex> edges;

	for (col = colStart; col < width - 15; col += 16) { // width is alredy >=8 (checked by the caller)
		vec0 = vceqq_u8(vld1q_u8(&e[col]), vecZero); // high 64bits then extend to 128bits (unaligned load)
		vec0 = vandq_u8(
			vec0, 
			vcombine_u8(
				vqmovn_u16(vcgtq_u16(vld1q_u16(&grad[col]), vecTHigh)), 
				vqmovn_u16(vcgtq_u16(vld1q_u16(&grad[col + 8]), vecTHigh))
			)
		);
		if (COMPV_ARM_NEON_EQ_ZERO(vec0)) {
			continue;
		}
		for (i = 0, j = 0; i < 16; ++i, j += 8) {
			switch (i) {
				case 0:	if (!(mask = vgetq_lane_u32(vec0, 0))) { i += 3; continue; } j = 0; break;
				case 4: if (!(mask = vgetq_lane_u32(vec0, 1))) { i += 3; continue; } j = 0; break;
				case 8: if (!(mask = vgetq_lane_u32(vec0, 2))) { i += 3; continue; } j = 0;  break;
				case 12: if (!(mask = vgetq_lane_u32(vec0, 3))) { i += 3; continue; } j = 0; break;
				default: break;
			}
			if (!(mask & (0xff << j))) {
				continue;
			}
			e[col + i] = 0xff;
			edges.push_back(CompVMatIndex(row, col + i));
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
					vecPn = (uint8x8_t) { p[-1], p[1], pt[-1], pt[0], pt[1], pb[-1], pb[0], pb[1] };
					vec0n = vceq_u8(vecPn, vecZeron);
					vec0n = vand_u8(vec0n, vqmovn_u16(vcgtq_u16(vecG, vecTLow)));
					mask0 = vget_lane_u32(vec0n, 0);
					if (mask0) {
						if (mask0 & 0x000000ff) { // left
							p[-1] = 0xff;
							edges.push_back(CompVMatIndex(r, c - 1));
						}
						if (mask0 & 0x0000ff00) { // right
							p[1] = 0xff;
							edges.push_back(CompVMatIndex(r, c + 1));
						}
						if (mask0 & 0x00ff0000) { // top-left
							pt[-1] = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c - 1));
						}
						if (mask0 & 0xff000000) { // top-center
							*pt = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c));
						}
					}
					mask0 = vget_lane_u32(vec0n, 1);
					if (mask0) {
						if (mask0 & 0x000000ff) { // top-right
							pt[1] = 0xff;
							edges.push_back(CompVMatIndex(r - 1, c + 1));
						}
						if (mask0 & 0x0000ff00) { // bottom-left
							pb[-1] = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c - 1));
						}
						if (mask0 & 0x00ff0000) { // bottom-center
							*pb = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c));
						}
						if (mask0 & 0xff000000) { // bottom-right
							pb[1] = 0xff;
							edges.push_back(CompVMatIndex(r + 1, c + 1));
						}
					}
				}
			}
		}
	}
}

void CompVCannyNMSGatherRow_8mpw_Intrin_NEON(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, const uint16_t* tLow1, compv_uscalar_t width, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	uint8x16_t vec0, vec1, vec2, vec3, vec4, vec5, vec6;
	int16x8_t vecGX, vecGY;
	uint16x8_t vecG;
	uint8x8_t vecNMS;
	uint32x4_t vecAbsGY0, vecAbsGY1;
	uint16x8_t vecAbsGX;
	const int16x8_t vecZero = vdupq_n_s16(0);
	const uint16x8_t vecTLow = vdupq_n_u16(*tLow1);
	static const uint16x4_t vecTangentPiOver8Int = vdup_n_u16(static_cast<uint16_t>(kCannyTangentPiOver8Int)); // 27145 -> can be stored in uint16_t then using long multiply
	static const int32x4_t vecTangentPiTimes3Over8Int = vdupq_n_s32(kCannyTangentPiTimes3Over8Int); // 158217 -> *cannot* be stored in uint16_t
	compv_uscalar_t col;
	const int stride_ = static_cast<const int>(stride);
	const int c0 = 1 - stride_, c1 = 1 + stride_;

	for (col = 1; col < width - 7; col += 8) { // up to the caller to check that width is >= 8
		vecG = vld1q_u16(&g[col]);
		vec0 = vcgtq_u16(vecG, vecTLow);
		if (COMPV_ARM_NEON_NEQ_ZERO(vec0)) {
			vecNMS = vdup_n_u8(0);
			vecGX = vld1q_s16(&gx[col]);
			vecGY = vld1q_s16(&gy[col]);

			vec1 = vabsq_s16(vecGY);
			vecAbsGX = vabsq_s16(vecGX);

			vecAbsGY0 = vshll_n_u16(vget_low_u16(vec1), 16); // convert from epi16 to epi32 then  "<< 16"
			vecAbsGY1 = vshll_n_u16(vget_high_u16(vec1), 16); // convert from epi16 to epi32 then  "<< 16"

            // angle = "0° / 180°"
			vec1 = vcgtq_u32(vmull_u16(vecTangentPiOver8Int, vget_low_u16(vecAbsGX)), vecAbsGY0); // convert from epi16 to epi32 then multiply
			vec2 = vcgtq_u32(vmull_u16(vecTangentPiOver8Int, vget_high_u16(vecAbsGX)), vecAbsGY1); // convert from epi16 to epi32 then multiply
			vec3 = vandq_u16(vec0, vcombine_u16(vqmovn_u32(vec1), vqmovn_u32(vec2)));
			if (COMPV_ARM_NEON_NEQ_ZERO(vec3)) {
				vec1 = vcgtq_u16(vld1q_u16(&g[col - 1]), vecG); // aligned load
				vec2 = vcgtq_u16(vld1q_u16(&g[col + 1]), vecG); // unaligned load
				vec1 = vandq_s16(vec3, vorrq_s16(vec1, vec2));
				vecNMS = vorr_u8(vqmovn_u16(vec1), vecNMS);
			}

			// angle = "45° / 225°" or "135 / 315"
			vec4 = vbicq_s16(vec0, vec3);
			if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) {
				vec1 = vcgtq_u32(vmulq_u32(vecTangentPiTimes3Over8Int, vmovl_u16(vget_low_u16(vecAbsGX))), vecAbsGY0); // multiply without pre-conversion
				vec2 = vcgtq_u32(vmulq_u32(vecTangentPiTimes3Over8Int, vmovl_u16(vget_high_u16(vecAbsGX))), vecAbsGY1); // multiply without pre-conversion
				vec4 = vandq_u16(vec4, vcombine_u16(vqmovn_u32(vec1), vqmovn_u32(vec2)));
				if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) {
					vec1 = vcgtq_s16(vecZero, veorq_s16(vecGX, vecGY)); // todo(asm): compare on signed numbers (different than other compare in this function)
					vec1 = vandq_u16(vec1, vec4);
					vec2 = vbicq_u16(vec4, vec1);
					if (COMPV_ARM_NEON_NEQ_ZERO(vec1)) {
						vec5 = vcgtq_u16(vld1q_u16(&g[col - c0]), vecG); // aligned load
						vec6 = vcgtq_u16(vld1q_u16(&g[col + c0]), vecG); // unaligned load
						vec1 = vandq_s16(vec1, vorrq_s16(vec5, vec6));
					}
					if (COMPV_ARM_NEON_NEQ_ZERO(vec2)) {
						vec5 = vcgtq_u16(vld1q_u16(&g[col - c1]), vecG); // aligned load
						vec6 = vcgtq_u16(vld1q_u16(&g[col + c1]), vecG); // unaligned load
						vec2 = vandq_u16(vec2, vorrq_s16(vec5, vec6));
					}
					vec1 = vorrq_u16(vec1, vec2);
					vecNMS = vorr_u8(vecNMS, vqmovn_u16(vec1));
				} // if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) - 1
			} // if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) - 0

			  // angle = "90° / 270°"
			vec5 = vbicq_s16(vbicq_s16(vec0, vec4), vec3);
			if (COMPV_ARM_NEON_NEQ_ZERO(vec5)) {
				vec1 = vcgtq_u16(vld1q_u16(&g[col - stride]), vecG); // unaligned load
				vec2 = vcgtq_u16(vld1q_u16(&g[col + stride]), vecG); // unaligned load
				vec5 = vandq_u16(vec5, vorrq_u16(vec1, vec2));
				vecNMS = vorr_u8(vqmovn_u16(vec5), vecNMS);
			}

			// update NMS
			vst1_u8(&nms[col], vecNMS);
		} // if (COMPV_ARM_NEON_NEQ_ZERO(vec0))
	} // for (col = 1...
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
			if (vget_lane_u64(vec0n, 0)) {
#else
			if (vget_lane_u32(vec0n, 0) || vget_lane_u32(vec0n, 1)) {
#endif
				vec0 = vmovl_u8(vec0n);
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
