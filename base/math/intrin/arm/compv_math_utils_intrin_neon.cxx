/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/intrin/arm/compv_math_utils_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMathUtilsMax_16u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint16_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint16_t *max)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_scalar_t widthSigned = static_cast<compv_scalar_t>(width);
	compv_scalar_t i, orphans = (widthSigned & 7); // in short
	uint16x8_t vec0, vec1, vec2, vec3;
	uint16x8_t vecMax = vdupq_n_u16(0);
	uint16x8_t vecOrphansSuppress = vdupq_n_u16(0); // not needed, just to stop compiler warnings about unset variable
	if (orphans) {
		compv_scalar_t orphansInBytes = orphans << 1; // convert to bytes
		// When the width isn't multiple of #16 then we set the padding bytes to #0 using a mask
		// Not an issue reading beyond width because the data is strided and aligned on NEON (#16 bytes)
		vecOrphansSuppress = vceqq_u8(vecOrphansSuppress, vecOrphansSuppress); // all bits to #1 (all bytes to #0xff)
		orphansInBytes = ((orphansInBytes - 16) << 3); // convert form bytes to bits and negate (negative means shift right)
		vecOrphansSuppress = vshlq_u64(vecOrphansSuppress, (int64x2_t) { orphansInBytes < -64 ? (orphansInBytes + 64) : 0, orphansInBytes });
	}
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < widthSigned - 31; i += 32) {
			vec0 = vld1q_u16(&data[i + 0]);
			vec1 = vld1q_u16(&data[i + 8]);
			vec2 = vld1q_u16(&data[i + 16]);
			vec3 = vld1q_u16(&data[i + 24]);
			vec0 = vmaxq_u16(vec0, vec1);
			vec2 = vmaxq_u16(vec2, vec3);
			vecMax = vmaxq_u16(vecMax, vec0);
			vecMax = vmaxq_u16(vecMax, vec2);
		}
		for (; i < widthSigned - 7; i += 8) {
			vec0 = vld1q_u16(&data[i + 0]);
			vecMax = vmaxq_u16(vecMax, vec0);
		}
		if (orphans) {
			vec0 = vld1q_u16(&data[i + 0]);
			vec0 = vandq_u16(vec0, vecOrphansSuppress);
			vecMax = vmaxq_u16(vecMax, vec0);
		}
		data += stride;
	}

    
	// Paiwise / hz max
	uint16x4_t vecMaxn = vpmax_u16(vget_low_u16(vecMax), vget_high_u16(vecMax));
	vecMaxn = vpmax_u16(vecMaxn, vecMaxn);
    vecMaxn = vpmax_u16(vecMaxn, vecMaxn);
	*max = vget_lane_u16(vecMaxn, 0);
}

void CompVMathUtilsSum_8u32u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* data, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, uint32_t *sum1)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	uint8x16_t vecSuml = vdupq_n_u32(0), vecSumh = vdupq_n_u32(0), vec0, vec1, vec2, vec3, vecOrphansSuppress = vdupq_n_u32(0);
	compv_scalar_t widthSigned = static_cast<compv_scalar_t>(width), i;
	compv_scalar_t orphans = (widthSigned & 15); // in bytes
	if (orphans) {
		compv_scalar_t orphansInBytes = orphans << 0;
		vecOrphansSuppress = vceqq_u8(vecOrphansSuppress, vecOrphansSuppress);
		orphansInBytes = ((orphansInBytes - 16) << 3);
		vecOrphansSuppress = vshlq_u64(vecOrphansSuppress, (int64x2_t) { orphansInBytes < -64 ? (orphansInBytes + 64) : 0, orphansInBytes });
	}

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < widthSigned - 63; i += 64) {
			vec0 = vld1q_u8(&data[i]);
			vec1 = vld1q_u8(&data[i + 16]);
			vec2 = vld1q_u8(&data[i + 32]);
			vec3 = vld1q_u8(&data[i + 48]);
			vec0 = vaddl_u8(vget_low_u8(vec0), vget_high_u8(vec0));
			vec1 = vaddl_u8(vget_low_u8(vec1), vget_high_u8(vec1));
			vec2 = vaddl_u8(vget_low_u8(vec2), vget_high_u8(vec2));
			vec3 = vaddl_u8(vget_low_u8(vec3), vget_high_u8(vec3));
			vec0 = vaddq_u16(vec0, vec1);
			vec2 = vaddq_u16(vec2, vec3);
			// use vecSuml and vecSumh to break dependency
			vecSuml = vaddq_u32(vecSuml, vaddl_u16(vget_low_u16(vec0), vget_high_u16(vec0)));
			vecSumh = vaddq_u32(vecSumh, vaddl_u16(vget_low_u16(vec2), vget_high_u16(vec2)));
		}
		for (; i < widthSigned - 15; i += 16) {
			vec0 = vld1q_u8(&data[i]);
			vec0 = vaddl_u8(vget_low_u8(vec0), vget_high_u8(vec0));
			vecSuml = vaddq_u32(vecSuml, vaddl_u16(vget_low_u16(vec0), vget_high_u16(vec0)));
		}
		if (orphans) {
			vec0 = vld1q_u8(&data[i]);
			vec0 = vandq_u16(vec0, vecOrphansSuppress);
			vec0 = vaddl_u8(vget_low_u8(vec0), vget_high_u8(vec0));
			vecSuml = vaddq_u32(vecSuml, vaddl_u16(vget_low_u16(vec0), vget_high_u16(vec0)));
		}
		data += stride;
	}

	vecSuml = vaddq_u32(vecSuml, vecSumh);
	uint32x2_t vecSumln = vadd_u32(vget_low_u32(vecSuml), vget_high_u32(vecSuml));
	vecSumln = vpadd_u32(vecSumln, vecSumln);

	*sum1 = vget_lane_u32(vecSumln, 0);
}

#define __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(i) \
	vst1q_s32(&s[i], vaddq_s32(vld1q_s32(&a[i]), vld1q_s32(&b[i]))); \
	vst1q_s32(&s[i + 4], vaddq_s32(vld1q_s32(&a[i + 4]), vld1q_s32(&b[i + 4]))); \
	vst1q_s32(&s[i + 8], vaddq_s32(vld1q_s32(&a[i + 8]), vld1q_s32(&b[i + 8]))); \
	vst1q_s32(&s[i + 12], vaddq_s32(vld1q_s32(&a[i + 12]), vld1q_s32(&b[i + 12])))


void CompVMathUtilsSum2_32s32s_Intrin_NEON(COMPV_ALIGNED(NEON) const int32_t* a, COMPV_ALIGNED(NEON) const int32_t* b, COMPV_ALIGNED(NEON) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t j;
	compv_scalar_t i;
	compv_scalar_t width_ = static_cast<compv_scalar_t>(width);

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width_ - 15; i += 16) {
			__CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(i);
		}
		for (; i < width_; i += 4) {
			vst1q_s32(&s[i], vaddq_s32(vld1q_s32(&a[i]), vld1q_s32(&b[i])));
		}
		a += stride;
		b += stride;
		s += stride;
	}
}

// width = 256, height = 1: common size (histogram 8u)
void CompVMathUtilsSum2_32s32s_256x1_Intrin_NEON(COMPV_ALIGNED(NEON) const int32_t* a, COMPV_ALIGNED(NEON) const int32_t* b, COMPV_ALIGNED(NEON) int32_t* s, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(0); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(16); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(32); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(48);
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(64); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(80); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(96); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(112);
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(128); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(144); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(160); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(176);
	__CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(192); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(208); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(224); __CompVMathUtilsSum2_16x1_32s32s_Intrin_NEON(240);
}

// "strideInBytes" must be NEON-aligned
void CompVMathUtilsSumAbs_16s16u_Intrin_NEON(const COMPV_ALIGNED(NEON) int16_t* a, const COMPV_ALIGNED(NEON) int16_t* b, COMPV_ALIGNED(NEON) uint16_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t j;
	int16x8_t vec0, vec1, vec2, vec3;
	compv_scalar_t i, widthSigned = static_cast<compv_scalar_t>(width);

	for (j = 0; j < height; ++j) {
		for (i = 0; i < widthSigned - 31; i += 32) {
			vec0 = vqaddq_u16(vabsq_s16(vld1q_s16(&a[i])), vabsq_s16(vld1q_s16(&b[i])));
			vec1 = vqaddq_u16(vabsq_s16(vld1q_s16(&a[i + 8])), vabsq_s16(vld1q_s16(&b[i + 8])));
			vec2 = vqaddq_u16(vabsq_s16(vld1q_s16(&a[i + 16])), vabsq_s16(vld1q_s16(&b[i + 16])));
			vec3 = vqaddq_u16(vabsq_s16(vld1q_s16(&a[i + 24])), vabsq_s16(vld1q_s16(&b[i + 24])));
			vst1q_u16(&r[i], vec0);
			vst1q_u16(&r[i + 8], vec1);
			vst1q_u16(&r[i + 16], vec2);
			vst1q_u16(&r[i + 24], vec3);
		}
		for (; i < widthSigned; i += 8) { // can read beyond width as data is strided
			vst1q_u16(&r[i], vqaddq_u16(vabsq_s16(vld1q_s16(&a[i])), vabsq_s16(vld1q_s16(&b[i]))));
		}

		r += stride;
		a += stride;
		b += stride;
	}
}

void CompVMathUtilsScaleAndClipPixel8_16u32f_Intrin_NEON(COMPV_ALIGNED(NEON) const uint16_t* in, const compv_float32_t* scale1, COMPV_ALIGNED(NEON) uint8_t* out, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_scalar_t i, widthSigned = static_cast<compv_scalar_t>(width);
	uint16x8_t vec0, vec1, vec2, vec3;
	float32x4_t vec0f, vec1f, vec2f, vec3f, vec4f, vec5f, vec6f, vec7f;
	const compv_float32_t scale = *scale1;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < widthSigned - 31; i += 32) {
			vec0 = vld1q_u16(&in[i]);
			vec1 = vld1q_u16(&in[i + 8]);
			vec2 = vld1q_u16(&in[i + 16]);
			vec3 = vld1q_u16(&in[i + 24]);
			vec0f = vcvtq_f32_u32(vmovl_u16(vget_low_u16(vec0)));
			vec1f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(vec0)));
			vec2f = vcvtq_f32_u32(vmovl_u16(vget_low_u16(vec1)));
			vec3f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(vec1)));
			vec4f = vcvtq_f32_u32(vmovl_u16(vget_low_u16(vec2)));
			vec5f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(vec2)));
			vec6f = vcvtq_f32_u32(vmovl_u16(vget_low_u16(vec3))); 
			vec7f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(vec3)));
			vec0f = vmulq_n_f32(vec0f, scale);
			vec1f = vmulq_n_f32(vec1f, scale);
			vec2f = vmulq_n_f32(vec2f, scale);
			vec3f = vmulq_n_f32(vec3f, scale);
			vec4f = vmulq_n_f32(vec4f, scale);
			vec5f = vmulq_n_f32(vec5f, scale);
			vec6f = vmulq_n_f32(vec6f, scale);
			vec7f = vmulq_n_f32(vec7f, scale);
			vec0 = vcombine_u16(vqmovn_u32(vcvtq_u32_f32(vec0f)), vqmovn_u32(vcvtq_u32_f32(vec1f)));
			vec1 = vcombine_u16(vqmovn_u32(vcvtq_u32_f32(vec2f)), vqmovn_u32(vcvtq_u32_f32(vec3f)));
			vec2 = vcombine_u16(vqmovn_u32(vcvtq_u32_f32(vec4f)), vqmovn_u32(vcvtq_u32_f32(vec5f)));
			vec3 = vcombine_u16(vqmovn_u32(vcvtq_u32_f32(vec6f)), vqmovn_u32(vcvtq_u32_f32(vec7f)));
			vec0 = vcombine_u8(vqmovn_u16(vec0), vqmovn_u16(vec1));
			vec1 = vcombine_u8(vqmovn_u16(vec2), vqmovn_u16(vec3));
			vst1q_u8(&out[i], vec0);
			vst1q_u8(&out[i + 16], vec1);
		}
		for (; i < widthSigned; i += 8) { // reading beyond width which means in and out must be strided
			vec0 = vld1q_u16(&in[i]);
			vec0f = vcvtq_f32_u32(vmovl_u16(vget_low_u16(vec0)));
			vec1f = vcvtq_f32_u32(vmovl_u16(vget_high_u16(vec0)));
			vec0f = vmulq_n_f32(vec0f, scale);
			vec1f = vmulq_n_f32(vec1f, scale);
			vec0 = vcombine_u16(vqmovn_u32(vcvtq_u32_f32(vec0f)), vqmovn_u32(vcvtq_u32_f32(vec1f)));
			vst1_u8(&out[i], vqmovn_u16(vec0));
		}
		out += stride;
		in += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
