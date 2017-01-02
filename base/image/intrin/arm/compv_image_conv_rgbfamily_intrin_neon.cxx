/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_conv_rgbfamily_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

// TODO(dmi): RGB -> RGBA conversion is done twice (Y plane then UV plane)
// Neon intrinsics: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0491h/BABDFJCI.html
// Neon instructions: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489c/CJAJIIGG.html

COMPV_NAMESPACE_BEGIN()

void CompVImageConvRgb24family_to_y_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGB = padY * 3;

	uint8x8x3_t vec0RGB, vec1RGB; // contains [R, G and B] samples each component on its own lane
	uint16x8_t vec0, vec1;

	const uint16x8_t vec2048 = vdupq_n_u16(2048);
	// The order in which the coeffs appears depends on the format (RGB, BGR, GRB...)
	const uint8x8x4_t vecCoeffs = vld4_u8(reinterpret_cast<uint8_t const *>(kRGBfamilyToYUV_YCoeffs8));
	const uint8x8_t vecCoeff0 = vecCoeffs.val[0]; // should be 33
	const uint8x8_t vecCoeff1 = vecCoeffs.val[1]; // should be 65
	const uint8x8_t vecCoeff2 = vecCoeffs.val[2]; // should be 13

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			vec0RGB = vld3_u8(rgb24Ptr + 0); // 8 RGB samples, [R, G and B] samples each component on its own lane
			vec1RGB = vld3_u8(rgb24Ptr + 24); // 8 RGB samples, [R, G and B] samples each component on its own lane		
			vec0 = vmull_u8(vecCoeff0, vec0RGB.val[0]); // vec0 = (33 * R)
			vec1 = vmull_u8(vecCoeff0, vec1RGB.val[0]); // vec1 = (33 * R)
			vec0 = vmlal_u8(vec0, vecCoeff1, vec0RGB.val[1]); // vec0 += (65 * G)
			vec1 = vmlal_u8(vec1, vecCoeff1, vec1RGB.val[1]); // vec1 += (65 * G)
			vec0 = vmlal_u8(vec0, vecCoeff2, vec0RGB.val[2]); // vec0 += (13 * B)			
			vec1 = vmlal_u8(vec1, vecCoeff2, vec1RGB.val[2]); // vec1 += (13 * B)
			// ((r >> 7) + 16) = (r + 2048) >> 7
			vec0 = vaddq_u16(vec0, vec2048);
			vec1 = vaddq_u16(vec1, vec2048);
			vst1q_u8(outYPtr, vcombine_u8(vqshrn_n_u16(vec0, 7), vqshrn_n_u16(vec1, 7))); // shift, saturate, narrow, concat			
			outYPtr += 16;
			rgb24Ptr += 48;
		}
		outYPtr += padY;
		rgb24Ptr += padRGB;
	}
}

void CompVImageConvRgb565lefamily_to_y_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* rgb565Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGB = padY << 1;

	uint16x8_t vec0R, vec1R, vec0G, vec1G, vec0B, vec1B;
	uint16x8_t vec0, vec1;

	const uint16x8_t vec2048 = vdupq_n_u16(2048);
	const uint16x8_t vecMaskR = vdupq_n_u16(0xF800);
	const uint16x8_t vecMaskG = vdupq_n_u16(0x07E0);
	const uint16x8_t vecMaskB = vdupq_n_u16(0x001F);
	// The order in which the coeffs appears depends on the format (RGB, BGR, GRB...)
	const uint16x8_t vecCoeff0 = vdupq_n_u16(static_cast<uint16_t>(kRGBfamilyToYUV_YCoeffs8[0])); // should be 33
	const uint16x8_t vecCoeff1 = vdupq_n_u16(static_cast<uint16_t>(kRGBfamilyToYUV_YCoeffs8[1])); // should be 65
	const uint16x8_t vecCoeff2 = vdupq_n_u16(static_cast<uint16_t>(kRGBfamilyToYUV_YCoeffs8[2])); // should be 13

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			vec0 = vld1q_u16(reinterpret_cast<uint16_t const *>(rgb565Ptr + 0));
			vec1 = vld1q_u16(reinterpret_cast<uint16_t const *>(rgb565Ptr + 16));
			vec0R = vshrq_n_u16(vandq_u16(vec0, vecMaskR), 8);
			vec1R = vshrq_n_u16(vandq_u16(vec1, vecMaskR), 8);
			vec0G = vshrq_n_u16(vandq_u16(vec0, vecMaskG), 3);
			vec1G = vshrq_n_u16(vandq_u16(vec1, vecMaskG), 3);
			vec0B = vshlq_n_u16(vandq_u16(vec0, vecMaskB), 3);
			vec1B = vshlq_n_u16(vandq_u16(vec1, vecMaskB), 3);
			vec0R = vsraq_n_u16(vec0R, vec0R, 5);
			vec1R = vsraq_n_u16(vec1R, vec1R, 5);
			vec0G = vsraq_n_u16(vec0G, vec0G, 6);
			vec1G = vsraq_n_u16(vec1G, vec1G, 6);
			vec0B = vsraq_n_u16(vec0B, vec0B, 5);
			vec1B = vsraq_n_u16(vec1B, vec1B, 5);
			vec0 = vmulq_u16(vecCoeff0, vec0R);
			vec1 = vmulq_u16(vecCoeff0, vec1R);
			vec0 = vmlaq_u16(vec0, vecCoeff1, vec0G);
			vec1 = vmlaq_u16(vec1, vecCoeff1, vec1G);
			vec0 = vmlaq_u16(vec0, vecCoeff2, vec0B);
			vec1 = vmlaq_u16(vec1, vecCoeff2, vec1B);
			// ((r >> 7) + 16) = (r + 2048) >> 7
			vec0 = vaddq_u16(vec0, vec2048);
			vec1 = vaddq_u16(vec1, vec2048);
			vst1q_u8(outYPtr, vcombine_u8(vqshrn_n_u16(vec0, 7), vqshrn_n_u16(vec1, 7))); // shift, saturate, narrow, concat			
			outYPtr += 16;
			rgb565Ptr += 32;
		}
		outYPtr += padY;
		rgb565Ptr += padRGB;
	}
}

void CompVImageConvRgb32family_to_y_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* rgb32Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGBA = padY << 2;
	
	uint8x8x4_t vec0RGBA, vec1RGBA; // contains [R, G, B and A] samples each component on its own lane
	uint16x8_t vec0, vec1;

	const uint16x8_t vec2048 = vdupq_n_u16(2048);
	// The order in which the coeffs appears depends on the format (RGBA, BGRA, ARGB...)
	const uint8x8x4_t vecCoeffs = vld4_u8(reinterpret_cast<uint8_t const *>(kRGBAfamilyToYUV_YCoeffs8));
	const uint8x8_t vecCoeff0 = vecCoeffs.val[0]; // should be 33
	const uint8x8_t vecCoeff1 = vecCoeffs.val[1]; // should be 65
	const uint8x8_t vecCoeff2 = vecCoeffs.val[2]; // should be 13
	const uint8x8_t vecCoeff3 = vecCoeffs.val[3]; // should be 0

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			vec0RGBA = vld4_u8(rgb32Ptr + 0); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane
			vec1RGBA = vld4_u8(rgb32Ptr + 32); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane		
			vec0 = vmull_u8(vecCoeff0, vec0RGBA.val[0]); // vec0 = (33 * R)
			vec1 = vmull_u8(vecCoeff0, vec1RGBA.val[0]); // vec1 = (33 * R)
			vec0 = vmlal_u8(vec0, vecCoeff1, vec0RGBA.val[1]); // vec0 += (65 * G)
			vec1 = vmlal_u8(vec1, vecCoeff1, vec1RGBA.val[1]); // vec1 += (65 * G)
			vec0 = vmlal_u8(vec0, vecCoeff2, vec0RGBA.val[2]); // vec0 += (13 * B)			
			vec1 = vmlal_u8(vec1, vecCoeff2, vec1RGBA.val[2]); // vec1 += (13 * B)
			vec0 = vmlal_u8(vec0, vecCoeff3, vec0RGBA.val[3]); // vec0 += (0 * A)			
			vec1 = vmlal_u8(vec1, vecCoeff3, vec1RGBA.val[3]); // vec1 += (0 * A)
			// ((r >> 7) + 16) = (r + 2048) >> 7
			vec0 = vaddq_u16(vec0, vec2048);
			vec1 = vaddq_u16(vec1, vec2048);
			vst1q_u8(outYPtr, vcombine_u8(vqshrn_n_u16(vec0, 7), vqshrn_n_u16(vec1, 7))); // shift, saturate, narrow, concat	
			outYPtr += 16;
			rgb32Ptr += 64;
		}
		outYPtr += padY;
		rgb32Ptr += padRGBA;
	}
}

void CompVImageConvRgb24family_to_uv_planar_11_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr, COMPV_ALIGNED(NEON) uint8_t* outUPtr, COMPV_ALIGNED(NEON) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padUV = (stride - maxI), padRGB = padUV * 3;

	uint8x8x3_t vec0RGB, vec1RGB; // contains [R, G and B] samples each component on its own lane
	int16x8_t vec0, vec1, vec0U, vec1U, vec0V, vec1V;

	const uint16x8_t vec128 = vld1q_u16(reinterpret_cast<const uint16_t*>(k128_i16));
	// The order in which the coeffs appears depends on the format (RGB, BGR, GRB...)
	const int8x8x4_t vecCoeffsU = vld4_s8(reinterpret_cast<int8_t const *>(kRGBfamilyToYUV_UCoeffs8));
	const int8x8x4_t vecCoeffsV = vld4_s8(reinterpret_cast<int8_t const *>(kRGBfamilyToYUV_VCoeffs8));
	const int16x8_t vecCoeffU0 = vmovl_s8(vecCoeffsU.val[0]); // should be -38
	const int16x8_t vecCoeffU1 = vmovl_s8(vecCoeffsU.val[1]); // should be -74
	const int16x8_t vecCoeffU2 = vmovl_s8(vecCoeffsU.val[2]); // should be 112
	const int16x8_t vecCoeffV0 = vmovl_s8(vecCoeffsV.val[0]); // should be 112
	const int16x8_t vecCoeffV1 = vmovl_s8(vecCoeffsV.val[1]); // should be -94
	const int16x8_t vecCoeffV2 = vmovl_s8(vecCoeffsV.val[2]); // should be -18

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			vec0RGB = vld3_u8(rgb24Ptr + 0); // 8 RGB samples, [R, G and B] samples each component on its own lane
			vec1RGB = vld3_u8(rgb24Ptr + 24); // 8 RGB samples, [R, G and B] samples each component on its own lane

			// Here (compared to "RGBA->Y" conversion) it's not possible to use 'vmull_u8' to perform 'u8' multiplication to get an 'u16' result
			// because the pixels are 'unsigned' while the coeffs are 'signed'

			// First part(R)
			vec0 = vreinterpretq_s16_u16(vmovl_u8(vec0RGB.val[0])); // Convert from u8 to s16
			vec1 = vreinterpretq_s16_u16(vmovl_u8(vec1RGB.val[0])); // Convert from u8 to s16
			vec0U = vmulq_s16(vecCoeffU0, vec0); // vec0U = (-38 * R)
			vec0V = vmulq_s16(vecCoeffV0, vec0); // vec0V = (112 * R)
			vec1U = vmulq_s16(vecCoeffU0, vec1); // vec1U = (-38 * R)
			vec1V = vmulq_s16(vecCoeffV0, vec1); // vec1V = (112 * R)
			// Second part(G)
			vec0 = vreinterpretq_s16_u16(vmovl_u8(vec0RGB.val[1]));
			vec1 = vreinterpretq_s16_u16(vmovl_u8(vec1RGB.val[1]));
			vec0U = vmlaq_s16(vec0U, vecCoeffU1, vec0); // vec0U += (-74 * G)
			vec0V = vmlaq_s16(vec0V, vecCoeffV1, vec0); // vec0V += (-94 * G)
			vec1U = vmlaq_s16(vec1U, vecCoeffU1, vec1); // vec1U += (-74 * G)
			vec1V = vmlaq_s16(vec1V, vecCoeffV1, vec1); // vec1V += (-94 * G)
			// Third part(B)
			vec0 = vreinterpretq_s16_u16(vmovl_u8(vec0RGB.val[2]));
			vec1 = vreinterpretq_s16_u16(vmovl_u8(vec1RGB.val[2]));
			vec0U = vmlaq_s16(vec0U, vecCoeffU2, vec0); // vec0U += (112 * G)
			vec0V = vmlaq_s16(vec0V, vecCoeffV2, vec0); // vec0V += (-18 * G)
			vec1U = vmlaq_s16(vec1U, vecCoeffU2, vec1); // vec1U += (112 * G)
			vec1V = vmlaq_s16(vec1V, vecCoeffV2, vec1); // vec1V += (-18 * G)
			// >> 8
			vec0U = vshrq_n_s16(vec0U, 8); // vec0U >>= 8
			vec0V = vshrq_n_s16(vec0V, 8); // vec0V >>= 8
			vec1U = vshrq_n_s16(vec1U, 8); // vec1U >>= 8
			vec1V = vshrq_n_s16(vec1V, 8); // vec1V >>= 8
			// + 128
			vec0U = vaddq_s16(vec0U, vec128); // vec0U += 128
			vec0V = vaddq_s16(vec0V, vec128); // vec0V += 128
			vec1U = vaddq_s16(vec1U, vec128); // vec1U += 128
			vec1V = vaddq_s16(vec1V, vec128); // vec1V += 128
			// outPtr = concat(saturate(vec0), saturate(vec1))
			vst1q_u8(outUPtr, vcombine_u8(vqmovun_s16(vec0U), vqmovun_s16(vec1U)));
			vst1q_u8(outVPtr, vcombine_u8(vqmovun_s16(vec0V), vqmovun_s16(vec1V)));
			outUPtr += 16;
			outVPtr += 16;
			rgb24Ptr += 48;
		}
		rgb24Ptr += padRGB;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

void CompVImageConvRgb32family_to_uv_planar_11_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* rgb32Ptr, COMPV_ALIGNED(NEON) uint8_t* outUPtr, COMPV_ALIGNED(NEON) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_VCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padUV = (stride - maxI), padRGBA = padUV << 2;

	uint8x8x4_t vec0RGBA, vec1RGBA; // contains [R, G, B and A] samples each component on its own lane
	int16x8_t vec0, vec1, vec0U, vec1U, vec0V, vec1V;

	const uint16x8_t vec128 = vld1q_u16(reinterpret_cast<const uint16_t*>(k128_i16));
	// The order in which the coeffs appears depends on the format (RGBA, BGRA, ARGB...)
	const int8x8x4_t vecCoeffsU = vld4_s8(reinterpret_cast<int8_t const *>(kRGBAfamilyToYUV_UCoeffs8));
	const int8x8x4_t vecCoeffsV = vld4_s8(reinterpret_cast<int8_t const *>(kRGBAfamilyToYUV_VCoeffs8));
	const int16x8_t vecCoeffU0 = vmovl_s8(vecCoeffsU.val[0]); // should be -38
	const int16x8_t vecCoeffU1 = vmovl_s8(vecCoeffsU.val[1]); // should be -74
	const int16x8_t vecCoeffU2 = vmovl_s8(vecCoeffsU.val[2]); // should be 112
	const int16x8_t vecCoeffU3 = vmovl_s8(vecCoeffsU.val[3]); // should be 0
	const int16x8_t vecCoeffV0 = vmovl_s8(vecCoeffsV.val[0]); // should be 112
	const int16x8_t vecCoeffV1 = vmovl_s8(vecCoeffsV.val[1]); // should be -94
	const int16x8_t vecCoeffV2 = vmovl_s8(vecCoeffsV.val[2]); // should be -18
	const int16x8_t vecCoeffV3 = vmovl_s8(vecCoeffsV.val[3]); // should be 0

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			vec0RGBA = vld4_u8(rgb32Ptr + 0); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane
			vec1RGBA = vld4_u8(rgb32Ptr + 32); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane
			
			// Here (compared to "RGBA->Y" conversion) it's not possible to use 'vmull_u8' to perform 'u8' multiplication to get an 'u16' result
			// because the pixels are 'unsigned' while the coeffs are 'signed'
			
			// First part(R)
			vec0 = vreinterpretq_s16_u16(vmovl_u8(vec0RGBA.val[0])); // Convert from u8 to s16
			vec1 = vreinterpretq_s16_u16(vmovl_u8(vec1RGBA.val[0])); // Convert from u8 to s16
			vec0U = vmulq_s16(vecCoeffU0, vec0); // vec0U = (-38 * R)
			vec0V = vmulq_s16(vecCoeffV0, vec0); // vec0V = (112 * R)
			vec1U = vmulq_s16(vecCoeffU0, vec1); // vec1U = (-38 * R)
			vec1V = vmulq_s16(vecCoeffV0, vec1); // vec1V = (112 * R)
			// Second part(G)
			vec0 = vreinterpretq_s16_u16(vmovl_u8(vec0RGBA.val[1]));
			vec1 = vreinterpretq_s16_u16(vmovl_u8(vec1RGBA.val[1]));
			vec0U = vmlaq_s16(vec0U, vecCoeffU1, vec0); // vec0U += (-74 * G)
			vec0V = vmlaq_s16(vec0V, vecCoeffV1, vec0); // vec0V += (-94 * G)
			vec1U = vmlaq_s16(vec1U, vecCoeffU1, vec1); // vec1U += (-74 * G)
			vec1V = vmlaq_s16(vec1V, vecCoeffV1, vec1); // vec1V += (-94 * G)
			// Third part(B)
			vec0 = vreinterpretq_s16_u16(vmovl_u8(vec0RGBA.val[2]));
			vec1 = vreinterpretq_s16_u16(vmovl_u8(vec1RGBA.val[2]));
			vec0U = vmlaq_s16(vec0U, vecCoeffU2, vec0); // vec0U += (112 * G)
			vec0V = vmlaq_s16(vec0V, vecCoeffV2, vec0); // vec0V += (-18 * G)
			vec1U = vmlaq_s16(vec1U, vecCoeffU2, vec1); // vec1U += (112 * G)
			vec1V = vmlaq_s16(vec1V, vecCoeffV2, vec1); // vec1V += (-18 * G)
			// Fourth part(B)
			vec0 = vreinterpretq_s16_u16(vmovl_u8(vec0RGBA.val[3]));
			vec1 = vreinterpretq_s16_u16(vmovl_u8(vec1RGBA.val[3]));
			vec0U = vmlaq_s16(vec0U, vecCoeffU3, vec0); // vec0U += (0 * A)
			vec0V = vmlaq_s16(vec0V, vecCoeffV3, vec0); // vec0V += (0 * A)
			vec1U = vmlaq_s16(vec1U, vecCoeffU3, vec1); // vec1U += (0 * A)
			vec1V = vmlaq_s16(vec1V, vecCoeffV3, vec1); // vec1V += (0 * A)
			// >> 8
			vec0U = vshrq_n_s16(vec0U, 8); // vec0U >>= 8
			vec0V = vshrq_n_s16(vec0V, 8); // vec0V >>= 8
			vec1U = vshrq_n_s16(vec1U, 8); // vec1U >>= 8
			vec1V = vshrq_n_s16(vec1V, 8); // vec1V >>= 8
			// + 128
			vec0U = vaddq_s16(vec0U, vec128); // vec0U += 128
			vec0V = vaddq_s16(vec0V, vec128); // vec0V += 128
			vec1U = vaddq_s16(vec1U, vec128); // vec1U += 128
			vec1V = vaddq_s16(vec1V, vec128); // vec1V += 128
			// outPtr = concat(saturate(vec0), saturate(vec1))
			vst1q_u8(outUPtr, vcombine_u8(vqmovun_s16(vec0U), vqmovun_s16(vec1U)));
			vst1q_u8(outVPtr, vcombine_u8(vqmovun_s16(vec0V), vqmovun_s16(vec1V)));
			outUPtr += 16;
			outVPtr += 16;
			rgb32Ptr += 64;
		}
		rgb32Ptr += padRGBA;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
