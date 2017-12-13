/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_scale_bilinear_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

#define _neon_bilinear_extract_then_insert(vecNeareastX, neareastIndex0, neareastIndex1, vecNeighbA, vecNeighbB, neighbIndex) \
			/* Extract indices(neareastIndex0, neareastIndex1) */ \
			nearestX0 = vgetq_lane_u32(vecNeareastX, neareastIndex0); \
			nearestX1 = vgetq_lane_u32(vecNeareastX, neareastIndex1); \
			/* Insert in vecNeighbA(neighbIndex) */ \
			vecNeighbA = vsetq_lane_u32(*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16), vecNeighbA, neighbIndex); /* vecNeighbA  -> 0,1,0,1,0,1,0,1,0,1,0,1 */ \
			/* Insert in vecNeighbB(neighbIndex) */ \
			vecNeighbB = vsetq_lane_u32(*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16), vecNeighbB, neighbIndex); /* vecNeighbB -> 2,3,2,3,2,3,2,3 */

#define _neon_bilinear_set_neighbs(vecNeareastX, vecNeighbA, vecNeighbB, neighbIndex0, neighbIndex1) \
			_neon_bilinear_extract_then_insert(vecNeareastX, 0, 1, vecNeighbA, vecNeighbB, neighbIndex0); \
			_neon_bilinear_extract_then_insert(vecNeareastX, 2, 3, vecNeighbA, vecNeighbB, neighbIndex1)

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): Not optiz -> on Android (Huawei MediaPad2, ARM64, HD image, single threaded, #1000 times), asm code: 2862.ms, intrin code: 4349.ms
void CompVImageScaleBilinear_Intrin_NEON(
	const uint8_t* inPtr, compv_uscalar_t inStride,
	COMPV_ALIGNED(NEON) uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outYStart, compv_uscalar_t outYEnd, COMPV_ALIGNED(NEON) compv_uscalar_t outStride,
	compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CHECK_NEON();

	compv_uscalar_t i, nearestY;
	const uint8_t* inPtr_;
	uint32x4_t vec0, vec1, vec2, vec3, vecX0, vecX1, vecX2, vecX3;
	uint16x8_t vecy0, vecy1, vec4, vec5, vec6, vec7;
    uint8x16_t vecNeighb0 = {}, vecNeighb1= {}, vecNeighb2 = {}, vecNeighb3 = {};
	uint32_t sf_x_ = static_cast<uint32_t>(sf_x);
	const uint32x4_t vecSfxTimes16 = vdupq_n_u32(sf_x_ << 4);
	const uint32x4_t vecSfxTimes4 = vdupq_n_u32(sf_x_ << 2);
	COMPV_ALIGN_NEON() const uint32_t SFX0[4] = { sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 };
	const uint32x4_t vecSFX0 = vld1q_u32(SFX0);
	const uint32x4_t vecSFX1 = vaddq_u32(vecSFX0, vecSfxTimes4);
	const uint32x4_t vecSFX2 = vaddq_u32(vecSFX1, vecSfxTimes4);
	const uint32x4_t vecSFX3 = vaddq_u32(vecSFX2, vecSfxTimes4);
	static const uint32x4_t vec0xff_epi32 = vdupq_n_u32(0xff);
	static const uint16x8_t vec0xff_epi16 = vdupq_n_u16(0xff);
	static const uint8x16_t vecMask = vld1q_u8(reinterpret_cast<const uint8_t*>(kShuffleEpi8_Deinterleave8uL2_32s)); // FIXME: remove
	uint32_t nearestX0, nearestX1;

	do {
		nearestY = (outYStart >> 8); // nearest y-point
		inPtr_ = &inPtr[nearestY * inStride];
		vecy0 = vdupq_n_u16(static_cast<uint16_t>(outYStart & 0xff));
		vecy1 = vbicq_u16(vec0xff_epi16, vecy0);
		vecX0 = vecSFX0;
		vecX1 = vecSFX1;
		vecX2 = vecSFX2;
		vecX3 = vecSFX3;
		for (i = 0; i < outWidth; i += 16) {
			// nearest x-point
			vec0 = vshrq_n_u32(vecX0, 8);
			vec1 = vshrq_n_u32(vecX1, 8);
			vec2 = vshrq_n_u32(vecX2, 8);
			vec3 = vshrq_n_u32(vecX3, 8);


			/* write memNeighbs */
			_neon_bilinear_set_neighbs(vec0, vecNeighb0, vecNeighb2, 0, 1);
			_neon_bilinear_set_neighbs(vec1, vecNeighb0, vecNeighb2, 2, 3);
			_neon_bilinear_set_neighbs(vec2, vecNeighb1, vecNeighb3, 0, 1);
			_neon_bilinear_set_neighbs(vec3, vecNeighb1, vecNeighb3, 2, 3);
            

			/* Deinterleave neighbs	*/
			// Assembler code uses vuzp and vswp and the result is very faaast. Same code in intrinsic is very slooow (GCC 4.1, Android, Huawei MediaPad2). So, we keep using
			// vtables in intrin.
			vec0 = vcombine_u8(vtbx2_u8(vget_low_u8(vecNeighb0), (uint8x8x2_t&)vecNeighb0, vget_low_u8(vecMask)),
				vtbx2_u8(vget_high_u8(vecNeighb0), (uint8x8x2_t&)vecNeighb0, vget_high_u8(vecMask))); // 0,0,0,0,1,1,1,1
			vec1 = vcombine_u8(vtbx2_u8(vget_low_u8(vecNeighb1), (uint8x8x2_t&)vecNeighb1, vget_low_u8(vecMask)),
				vtbx2_u8(vget_high_u8(vecNeighb1), (uint8x8x2_t&)vecNeighb1, vget_high_u8(vecMask))); // 0,0,0,0,1,1,1,1
			vec2 = vcombine_u8(vtbx2_u8(vget_low_u8(vecNeighb2), (uint8x8x2_t&)vecNeighb2, vget_low_u8(vecMask)),
				vtbx2_u8(vget_high_u8(vecNeighb2), (uint8x8x2_t&)vecNeighb2, vget_high_u8(vecMask))); // 2,2,2,2,3,3,3,3
			vec3 = vcombine_u8(vtbx2_u8(vget_low_u8(vecNeighb3), (uint8x8x2_t&)vecNeighb3, vget_low_u8(vecMask)),
				vtbx2_u8(vget_high_u8(vecNeighb3), (uint8x8x2_t&)vecNeighb3, vget_high_u8(vecMask))); // 2,2,2,2,3,3,3,3
			vecNeighb0 = vcombine_u8(vget_low_u8(vec0), vget_low_u8(vec1));   // 0,0,0,0,0,0 (remark: using asm code this is a nop operation, just use the dx registers to ref vecNeighbx)
			vecNeighb1 = vcombine_u8(vget_high_u8(vec0), vget_high_u8(vec1)); // 1,1,1,1,1,1
			vecNeighb2 = vcombine_u8(vget_low_u8(vec2), vget_low_u8(vec3));   // 2,2,2,2,2,2
			vecNeighb3 = vcombine_u8(vget_high_u8(vec2), vget_high_u8(vec3)); // 3,3,3,3,3,3
			
			/* compute x0 and x1 (first #8) and convert from epi32 and epi16 */
			vec0 = vcombine_u16(vmovn_u32(vandq_u32(vecX0, vec0xff_epi32)), vmovn_u32(vandq_u32(vecX1, vec0xff_epi32))); // epi16
			vec1 = vbicq_u16(vec0xff_epi16, vec0);
			// compute vec4 = (neighb0 * x1) + (neighb1 * x0) -> #8 epi16
			vec4 = vmulq_u16(vmovl_u8(vget_low_u8(vecNeighb0)), vec1);
			vec4 = vmlaq_u16(vec4, vmovl_u8(vget_low_u8(vecNeighb1)), vec0);
			// compute vec5 = (neighb2 * x1) + (neighb3 * x0) -> #8 epi16
			vec5 = vmulq_u16(vmovl_u8(vget_low_u8(vecNeighb2)), vec1);
			vec5 = vmlaq_u16(vec5, vmovl_u8(vget_low_u8(vecNeighb3)), vec0);

			/* compute x0 and x1 (second #8) and convert from epi32 and epi16 */
			vec0 = vcombine_u16(vmovn_u32(vandq_u32(vecX2, vec0xff_epi32)), vmovn_u32(vandq_u32(vecX3, vec0xff_epi32))); // epi16
			vec1 = vbicq_u16(vec0xff_epi16, vec0);
			// compute vec6 = (neighb0 * x1) + (neighb1 * x0) -> #8 epi16
			vec6 = vmulq_u16(vmovl_u8(vget_high_u8(vecNeighb0)), vec1);
			vec6 = vmlaq_u16(vec6, vmovl_u8(vget_high_u8(vecNeighb1)), vec0);
			// compute vec7 = (neighb2 * x1) + (neighb3 * x0) -> #8 epi16
			vec7 = vmulq_u16(vmovl_u8(vget_high_u8(vecNeighb2)), vec1);
			vec7 = vmlaq_u16(vec7, vmovl_u8(vget_high_u8(vecNeighb3)), vec0);

			// Let's say:
			//		A = ((neighb0 * x1) + (neighb1 * x0))
			//		B = ((neighb2 * x1) + (neighb3 * x0))
			// Then:
			//		A = vec4, vec6
			//		B = vec5, vec7
			//
			// We cannot use vshrq_n_s16(vqdmulhq_s16(a, b), 1) to compute C and D because it operates on epi16 while A and B contain epu16 values

			/* compute C = (y1 * A) >> 16 */
			vec0 = vcombine_u16(vshrn_n_u32(vmull_u16(vget_low_u16(vecy1), vget_low_u16(vec4)), 16),
				vshrn_n_u32(vmull_u16(vget_high_u16(vecy1), vget_high_u16(vec4)), 16));
			vec1 = vcombine_u16(vshrn_n_u32(vmull_u16(vget_low_u16(vecy1), vget_low_u16(vec6)), 16),
				vshrn_n_u32(vmull_u16(vget_high_u16(vecy1), vget_high_u16(vec6)), 16));

			/* compute D = (y0 * B) >> 16 */
			vec2 = vcombine_u16(vshrn_n_u32(vmull_u16(vget_low_u16(vecy0), vget_low_u16(vec5)), 16),
				vshrn_n_u32(vmull_u16(vget_high_u16(vecy0), vget_high_u16(vec5)), 16));
			vec3 = vcombine_u16(vshrn_n_u32(vmull_u16(vget_low_u16(vecy0), vget_low_u16(vec7)), 16),
				vshrn_n_u32(vmull_u16(vget_high_u16(vecy0), vget_high_u16(vec7)), 16));

			/* Compute R = (C + D) */
			vec0 = vqaddq_u16(vec0, vec2);
			vec1 = vqaddq_u16(vec1, vec3);

			/* Store the result */
			vst1q_u8(&outPtr[i], vcombine_u8(vmovn_u16(vec0), vmovn_u16(vec1)));

			// move to next indices
			vecX0 = vaddq_u32(vecX0, vecSfxTimes16);
			vecX1 = vaddq_u32(vecX1, vecSfxTimes16);
			vecX2 = vaddq_u32(vecX2, vecSfxTimes16);
			vecX3 = vaddq_u32(vecX3, vecSfxTimes16);
		}
		
		outPtr += outStride;
		outYStart += sf_y;
	} while (outYStart < outYEnd);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
