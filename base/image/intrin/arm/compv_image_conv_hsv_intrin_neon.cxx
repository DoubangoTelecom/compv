/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_conv_hsv_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

static const int8x8_t vec43n = vdup_n_s8(43); // half-vector

static const int32x4_t vecZero = vdupq_n_s32(0);
static const int8x16_t vec85 = vdupq_n_s8(85);
static const uint8x16_t vec171 = vdupq_n_u8(171);
static const uint8x16_t vecFF = vceqq_u8(vec85, vec85);
static const float32x4_t vec255f = vdupq_n_f32(255.f);
static const float32x4_t vecHalf = vdupq_n_f32(0.5f);

void CompVImageConvRgb24ToHsv_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr, COMPV_ALIGNED(NEON) uint8_t* hsvPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
    
	compv_uscalar_t i, j;
	int32x4_t vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9;
	float32x4_t vec0f, vec1f, vec2f, vec3f;
	uint8x16x3_t vecLanes;

	width += (width << 1); // from samples to bytes (width * 3)
	stride += (stride << 1); // from samples to bytes (stride * 3)

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 48) { // 48 = (16 * 3)
			vecLanes = vld3q_u8(&rgb24Ptr[i]);

			vec3 = vminq_u8(vecLanes.val[2], vminq_u8(vecLanes.val[0], vecLanes.val[1])); // vec3 = minVal
			vec4 = vmaxq_u8(vecLanes.val[2], vmaxq_u8(vecLanes.val[0], vecLanes.val[1])); // vec4 = maxVal = hsv[2].u8
			vec1 = vsubl_u8(vget_low_u8(vec4), vget_low_u8(vec3)); // vec1 = minus.low
			vec3 = vsubl_u8(vget_high_u8(vec4), vget_high_u8(vec3)); // vec3 = minus.high

			vec5 = vceqq_u8(vec4, vecLanes.val[0]); // m0 = (maxVal == r)
			vec6 = vbicq_u8(vceqq_u8(vec4, vecLanes.val[1]), vec5); // vec6 = m1 = (maxVal == g) & ~m0
			vec7 = vbicq_u8(vecFF, vorrq_u8(vec5, vec6)); // vec7 = m2 = ~(m0 | m1)

			vec9 = vandq_s8(vec7, vsubq_s8(vecLanes.val[0], vecLanes.val[1]));
			vec5 = vandq_s8(vec5, vsubq_s8(vecLanes.val[1], vecLanes.val[2]));
			vec8 = vandq_s8(vec6, vsubq_s8(vecLanes.val[2], vecLanes.val[0]));
			
			vec5 = vorrq_s8(vec5, vec8);
			vec5 = vorrq_s8(vec5, vec9); // vec5 = diff

			// minus = ToFloat32(ToUInt32(ToUInt16(vec4)))
			vec0 = vmovl_u16(vget_low_u16(vec1));
			vec1 = vmovl_u16(vget_high_u16(vec1));
			vec2 = vmovl_u16(vget_low_u16(vec3));
			vec3 = vmovl_u16(vget_high_u16(vec3));
			vec0 = vcvtq_f32_u32(vec0);
			vec1 = vcvtq_f32_u32(vec1);
			vec2 = vcvtq_f32_u32(vec2);
			vec3 = vcvtq_f32_u32(vec3);

			// maxVal = ToFloat32(ToUInt32(ToUInt16(ToUInt8(vec4))))
			vec1f = vmovl_u8(vget_low_u8(vec4));
			vec3f = vmovl_u8(vget_high_u8(vec4));
			vec0f = vmovl_u16(vget_low_u16(vec1f));
			vec1f = vmovl_u16(vget_high_u16(vec1f));
			vec2f = vmovl_u16(vget_low_u16(vec3f));
			vec3f = vmovl_u16(vget_high_u16(vec3f));
			vec0f = vcvtq_f32_u32(vec0f);
			vec1f = vcvtq_f32_u32(vec1f);
			vec2f = vcvtq_f32_u32(vec2f);
			vec3f = vcvtq_f32_u32(vec3f);

			// scale = maxVal ? (1.f / maxVal) : 0.f
			vec0f = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec0f), vceqq_s32(vec0f, vecZero));
			vec1f = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec1f), vceqq_s32(vec1f, vecZero));
			vec2f = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec2f), vceqq_s32(vec2f, vecZero));
			vec3f = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec3f), vceqq_s32(vec3f, vecZero));

			// scales255 = (255 * scale)
			vec0f = vmulq_f32(vec0f, vec255f);
			vec1f = vmulq_f32(vec1f, vec255f);
			vec2f = vmulq_f32(vec2f, vec255f);
			vec3f = vmulq_f32(vec3f, vec255f);

			// hsv[1].float = static_cast<uint8_t>((scales255 * minus)) - unsigned
			vec0f = vmulq_f32(vec0f, vec0);
			vec1f = vmulq_f32(vec1f, vec1);
			vec2f = vmulq_f32(vec2f, vec2);
			vec3f = vmulq_f32(vec3f, vec3);
			vec0f = vcvtq_u32_f32(vec0f);
			vec1f = vcvtq_u32_f32(vec1f);
			vec2f = vcvtq_u32_f32(vec2f);
			vec3f = vcvtq_u32_f32(vec3f);
			vec0f = vcombine_u16(vmovn_s32(vec0f), vmovn_s32(vec1f));
			vec2f = vcombine_u16(vmovn_s32(vec2f), vmovn_s32(vec3f));
			vec8 = vcombine_u8(vqmovun_s16(vec0f), vqmovun_s16(vec2f)); // vec8 = hsv[1].u8

			// B = ToFloat32(ToInt32(ToInt16(diff * 43)))
			vec1f = vmull_s8(vget_low_s8(vec5), vec43n);
			vec3f = vmull_s8(vget_high_s8(vec5), vec43n);
			vec0f = vmovl_s16(vget_low_s16(vec1f));
			vec1f = vmovl_s16(vget_high_s16(vec1f));
			vec2f = vmovl_s16(vget_low_s16(vec3f));
			vec3f = vmovl_s16(vget_high_s16(vec3f));
			vec0f = vcvtq_f32_s32(vec0f);
			vec1f = vcvtq_f32_s32(vec1f);
			vec2f = vcvtq_f32_s32(vec2f);
			vec3f = vcvtq_f32_s32(vec3f);

			// scale = minus ? (1.f / minus) : 0.f
			vec0 = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec0), vceqq_s32(vec0, vecZero));
			vec1 = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec1), vceqq_s32(vec1, vecZero));
			vec2 = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec2), vceqq_s32(vec2, vecZero));
			vec3 = vbicq_u32(COMPV_ARM_NEON_RECIPROCAL(vec3), vceqq_s32(vec3, vecZero));

			// compute static_cast<uint8_t>(round(B * scale) + ((85 & m1) | (171 & m2))
			vec0f = vmulq_f32(vec0f, vec0);
			vec1f = vmulq_f32(vec1f, vec1);
			vec2f = vmulq_f32(vec2f, vec2);
			vec3f = vmulq_f32(vec3f, vec3);
			vec0f = COMPV_ARM_NEON_MATH_ROUNDF_2_NEAREST_INT(vec0f); //!\\ **MUST NOT*** use vcvtq_s32_f32
			vec1f = COMPV_ARM_NEON_MATH_ROUNDF_2_NEAREST_INT(vec1f);
			vec2f = COMPV_ARM_NEON_MATH_ROUNDF_2_NEAREST_INT(vec2f);
			vec3f = COMPV_ARM_NEON_MATH_ROUNDF_2_NEAREST_INT(vec3f);
			vec0f = vcombine_s16(vmovn_s32(vec0f), vmovn_s32(vec1f));
			vec2f = vcombine_s16(vmovn_s32(vec2f), vmovn_s32(vec3f));
			vec9 = vcombine_s8(vqmovn_s16(vec0f), vqmovn_s16(vec2f)); //!\\ 'vqmovn_s16' instead of 'vqmovun_s16' because the values are signed
			vec6 = vandq_s8(vec6, vec85); // (85 & m1)
			vec7 = vandq_s8(vec7, vec171); // (171 & m2)
			vec6 = vorrq_s8(vec6, vec7); // (85 & m1) | (171 & m2)
			vec9 = vqaddq_s8(vec9, vec6); // // vec9 = hsv[0].u8

			// Store the result
			vecLanes = (uint8x16x3_t) { { vec9, vec8, vec4 } };
			vst3q_u8(&hsvPtr[i], vecLanes);

		} // End_Of for (i = 0; i < width; i += 48)
		rgb24Ptr += stride;
		hsvPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
