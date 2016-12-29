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

	uint8x8x3_t xmm0RGB, xmm1RGB; // contains [R, G and B] samples each component on its own lane
	uint16x8_t xmm0, xmm1;

	const uint16x8_t xmm2048 = vdupq_n_u16(2048);
	// The order in which the coeffs appears depends on the format (RGB, BGR, GRB...)
	const uint8x8x4_t xmmCoeffs = vld4_u8(reinterpret_cast<uint8_t const *>(kRGBfamilyToYUV_YCoeffs8));
	const uint8x8_t xmmCoeff0 = xmmCoeffs.val[0]; // should be 33
	const uint8x8_t xmmCoeff1 = xmmCoeffs.val[1]; // should be 65
	const uint8x8_t xmmCoeff2 = xmmCoeffs.val[2]; // should be 13

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			xmm0RGB = vld3_u8(rgb24Ptr + 0); // 8 RGB samples, [R, G and B] samples each component on its own lane
			xmm1RGB = vld3_u8(rgb24Ptr + 24); // 8 RGB samples, [R, G and B] samples each component on its own lane		
			xmm0 = vmull_u8(xmmCoeff0, xmm0RGB.val[0]); // xmm0 = (33 * R)
			xmm1 = vmull_u8(xmmCoeff0, xmm1RGB.val[0]); // xmm1 = (33 * R)
			xmm0 = vmlal_u8(xmm0, xmmCoeff1, xmm0RGB.val[1]); // xmm0 += (65 * G)
			xmm1 = vmlal_u8(xmm1, xmmCoeff1, xmm1RGB.val[1]); // xmm1 += (65 * G)
			xmm0 = vmlal_u8(xmm0, xmmCoeff2, xmm0RGB.val[2]); // xmm0 += (13 * B)			
			xmm1 = vmlal_u8(xmm1, xmmCoeff2, xmm1RGB.val[2]); // xmm1 += (13 * B)
			// ((r >> 7) + 16) = (r + 2048) >> 7
			xmm0 = vaddq_u16(xmm0, xmm2048);
			xmm1 = vaddq_u16(xmm1, xmm2048);
			vst1q_u8(outYPtr, vcombine_u8(vqshrn_n_u16(xmm0, 7), vqshrn_n_u16(xmm1, 7))); // shift, saturate, narrow, concat			
			outYPtr += 16;
			rgb24Ptr += 48;
		}
		outYPtr += padY;
		rgb24Ptr += padRGB;
	}
}

void CompVImageConvRgb32family_to_y_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* rgb32Ptr, COMPV_ALIGNED(NEON) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBAfamilyToYUV_YCoeffs8)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t i, j, maxI = ((width + 15) & -16), padY = (stride - maxI), padRGBA = padY << 2;
	
	uint8x8x4_t xmm0RGBA, xmm1RGBA; // contains [R, G, B and A] samples each component on its own lane
	uint16x8_t xmm0, xmm1;

	const uint16x8_t xmm16 = vld1q_u16(reinterpret_cast<const uint16_t*>(k16_i16));
	// The order in which the coeffs appears depends on the format (RGBA, BGRA, ARGB...)
	const uint8x8x4_t xmmCoeffs = vld4_u8(reinterpret_cast<uint8_t const *>(kRGBAfamilyToYUV_YCoeffs8));
	const uint8x8_t xmmCoeff0 = xmmCoeffs.val[0]; // should be 33
	const uint8x8_t xmmCoeff1 = xmmCoeffs.val[1]; // should be 65
	const uint8x8_t xmmCoeff2 = xmmCoeffs.val[2]; // should be 13

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			xmm0RGBA = vld4_u8(rgb32Ptr + 0); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane
			xmm1RGBA = vld4_u8(rgb32Ptr + 32); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane		
			xmm0 = vmull_u8(xmmCoeff0, xmm0RGBA.val[0]); // xmm0 = (33 * R)
			xmm1 = vmull_u8(xmmCoeff0, xmm1RGBA.val[0]); // xmm1 = (33 * R)
			xmm0 = vmlal_u8(xmm0, xmmCoeff1, xmm0RGBA.val[1]); // xmm0 += (65 * G)
			xmm1 = vmlal_u8(xmm1, xmmCoeff1, xmm1RGBA.val[1]); // xmm1 += (65 * G)
			xmm0 = vmlal_u8(xmm0, xmmCoeff2, xmm0RGBA.val[2]); // xmm0 += (13 * B)			
			xmm1 = vmlal_u8(xmm1, xmmCoeff2, xmm1RGBA.val[2]); // xmm1 += (13 * B)
			xmm0 = vshrq_n_u16(xmm0, 7); // xmm0 >>= 7
			xmm1 = vshrq_n_u16(xmm1, 7); // xmm1 >>= 7
			xmm0 = vaddq_u16(xmm0, xmm16); // xmm0 += 16
			xmm1 = vaddq_u16(xmm1, xmm16); // xmm1 += 16
			vst1q_u8(outYPtr, vcombine_u8(vqmovun_s16(xmm0), vqmovun_s16(xmm1))); // outYPtr = concat(saturate(xmm0), saturate(xmm1))
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

	uint8x8x3_t xmm0RGB, xmm1RGB; // contains [R, G and B] samples each component on its own lane
	int16x8_t xmm0, xmm1, xmm0U, xmm1U, xmm0V, xmm1V;

	const uint16x8_t xmm128 = vld1q_u16(reinterpret_cast<const uint16_t*>(k128_i16));
	// The order in which the coeffs appears depends on the format (RGB, BGR, GRB...)
	const int8x8x4_t xmmCoeffsU = vld4_s8(reinterpret_cast<int8_t const *>(kRGBfamilyToYUV_UCoeffs8));
	const int8x8x4_t xmmCoeffsV = vld4_s8(reinterpret_cast<int8_t const *>(kRGBfamilyToYUV_VCoeffs8));
	const int16x8_t xmmCoeffU0 = vmovl_s8(xmmCoeffsU.val[0]); // should be -38
	const int16x8_t xmmCoeffU1 = vmovl_s8(xmmCoeffsU.val[1]); // should be -74
	const int16x8_t xmmCoeffU2 = vmovl_s8(xmmCoeffsU.val[2]); // should be 112
	const int16x8_t xmmCoeffV0 = vmovl_s8(xmmCoeffsV.val[0]); // should be 112
	const int16x8_t xmmCoeffV1 = vmovl_s8(xmmCoeffsV.val[1]); // should be -94
	const int16x8_t xmmCoeffV2 = vmovl_s8(xmmCoeffsV.val[2]); // should be -18

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			xmm0RGB = vld3_u8(rgb24Ptr + 0); // 8 RGB samples, [R, G and B] samples each component on its own lane
			xmm1RGB = vld3_u8(rgb24Ptr + 24); // 8 RGB samples, [R, G and B] samples each component on its own lane

			// Here (compared to "RGBA->Y" conversion) it's not possible to use 'vmull_u8' to perform 'u8' multiplication to get an 'u16' result
			// because the pixels are 'unsigned' while the coeffs are 'signed'

			// First part(R)
			xmm0 = vreinterpretq_s16_u16(vmovl_u8(xmm0RGB.val[0])); // Convert from u8 to s16
			xmm1 = vreinterpretq_s16_u16(vmovl_u8(xmm1RGB.val[0])); // Convert from u8 to s16
			xmm0U = vmulq_s16(xmmCoeffU0, xmm0); // xmm0U = (-38 * R)
			xmm0V = vmulq_s16(xmmCoeffV0, xmm0); // xmm0V = (112 * R)
			xmm1U = vmulq_s16(xmmCoeffU0, xmm1); // xmm1U = (-38 * R)
			xmm1V = vmulq_s16(xmmCoeffV0, xmm1); // xmm1V = (112 * R)
			// Second part(G)
			xmm0 = vreinterpretq_s16_u16(vmovl_u8(xmm0RGB.val[1]));
			xmm1 = vreinterpretq_s16_u16(vmovl_u8(xmm1RGB.val[1]));
			xmm0U = vmlaq_s16(xmm0U, xmmCoeffU1, xmm0); // xmm0U += (-74 * G)
			xmm0V = vmlaq_s16(xmm0V, xmmCoeffV1, xmm0); // xmm0V += (-94 * G)
			xmm1U = vmlaq_s16(xmm1U, xmmCoeffU1, xmm1); // xmm1U += (-74 * G)
			xmm1V = vmlaq_s16(xmm1V, xmmCoeffV1, xmm1); // xmm1V += (-94 * G)
			// Third part(B)
			xmm0 = vreinterpretq_s16_u16(vmovl_u8(xmm0RGB.val[2]));
			xmm1 = vreinterpretq_s16_u16(vmovl_u8(xmm1RGB.val[2]));
			xmm0U = vmlaq_s16(xmm0U, xmmCoeffU2, xmm0); // xmm0U += (112 * G)
			xmm0V = vmlaq_s16(xmm0V, xmmCoeffV2, xmm0); // xmm0V += (-18 * G)
			xmm1U = vmlaq_s16(xmm1U, xmmCoeffU2, xmm1); // xmm1U += (112 * G)
			xmm1V = vmlaq_s16(xmm1V, xmmCoeffV2, xmm1); // xmm1V += (-18 * G)
			// >> 8
			xmm0U = vshrq_n_s16(xmm0U, 8); // xmm0U >>= 8
			xmm0V = vshrq_n_s16(xmm0V, 8); // xmm0V >>= 8
			xmm1U = vshrq_n_s16(xmm1U, 8); // xmm1U >>= 8
			xmm1V = vshrq_n_s16(xmm1V, 8); // xmm1V >>= 8
			// + 128
			xmm0U = vaddq_s16(xmm0U, xmm128); // xmm0U += 128
			xmm0V = vaddq_s16(xmm0V, xmm128); // xmm0V += 128
			xmm1U = vaddq_s16(xmm1U, xmm128); // xmm1U += 128
			xmm1V = vaddq_s16(xmm1V, xmm128); // xmm1V += 128
			// outPtr = concat(saturate(xmm0), saturate(xmm1))
			vst1q_u8(outUPtr, vcombine_u8(vqmovun_s16(xmm0U), vqmovun_s16(xmm1U)));
			vst1q_u8(outVPtr, vcombine_u8(vqmovun_s16(xmm0V), vqmovun_s16(xmm1V)));
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

	uint8x8x4_t xmm0RGBA, xmm1RGBA; // contains [R, G, B and A] samples each component on its own lane
	int16x8_t xmm0, xmm1, xmm0U, xmm1U, xmm0V, xmm1V;

	const uint16x8_t xmm128 = vld1q_u16(reinterpret_cast<const uint16_t*>(k128_i16));
	// The order in which the coeffs appears depends on the format (RGBA, BGRA, ARGB...)
	const int8x8x4_t xmmCoeffsU = vld4_s8(reinterpret_cast<int8_t const *>(kRGBAfamilyToYUV_UCoeffs8));
	const int8x8x4_t xmmCoeffsV = vld4_s8(reinterpret_cast<int8_t const *>(kRGBAfamilyToYUV_VCoeffs8));
	const int16x8_t xmmCoeffU0 = vmovl_s8(xmmCoeffsU.val[0]); // should be -38
	const int16x8_t xmmCoeffU1 = vmovl_s8(xmmCoeffsU.val[1]); // should be -74
	const int16x8_t xmmCoeffU2 = vmovl_s8(xmmCoeffsU.val[2]); // should be 112
	const int16x8_t xmmCoeffV0 = vmovl_s8(xmmCoeffsV.val[0]); // should be 112
	const int16x8_t xmmCoeffV1 = vmovl_s8(xmmCoeffsV.val[1]); // should be -94
	const int16x8_t xmmCoeffV2 = vmovl_s8(xmmCoeffsV.val[2]); // should be -18

	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 16) {
			xmm0RGBA = vld4_u8(rgb32Ptr + 0); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane
			xmm1RGBA = vld4_u8(rgb32Ptr + 32); // 8 RGBA samples, [R, G, B and A] samples each component on its own lane
			
			// Here (compared to "RGBA->Y" conversion) it's not possible to use 'vmull_u8' to perform 'u8' multiplication to get an 'u16' result
			// because the pixels are 'unsigned' while the coeffs are 'signed'
			
			// First part(R)
			xmm0 = vreinterpretq_s16_u16(vmovl_u8(xmm0RGBA.val[0])); // Convert from u8 to s16
			xmm1 = vreinterpretq_s16_u16(vmovl_u8(xmm1RGBA.val[0])); // Convert from u8 to s16
			xmm0U = vmulq_s16(xmmCoeffU0, xmm0); // xmm0U = (-38 * R)
			xmm0V = vmulq_s16(xmmCoeffV0, xmm0); // xmm0V = (112 * R)
			xmm1U = vmulq_s16(xmmCoeffU0, xmm1); // xmm1U = (-38 * R)
			xmm1V = vmulq_s16(xmmCoeffV0, xmm1); // xmm1V = (112 * R)
			// Second part(G)
			xmm0 = vreinterpretq_s16_u16(vmovl_u8(xmm0RGBA.val[1]));
			xmm1 = vreinterpretq_s16_u16(vmovl_u8(xmm1RGBA.val[1]));
			xmm0U = vmlaq_s16(xmm0U, xmmCoeffU1, xmm0); // xmm0U += (-74 * G)
			xmm0V = vmlaq_s16(xmm0V, xmmCoeffV1, xmm0); // xmm0V += (-94 * G)
			xmm1U = vmlaq_s16(xmm1U, xmmCoeffU1, xmm1); // xmm1U += (-74 * G)
			xmm1V = vmlaq_s16(xmm1V, xmmCoeffV1, xmm1); // xmm1V += (-94 * G)
			// Third part(B)
			xmm0 = vreinterpretq_s16_u16(vmovl_u8(xmm0RGBA.val[2]));
			xmm1 = vreinterpretq_s16_u16(vmovl_u8(xmm1RGBA.val[2]));
			xmm0U = vmlaq_s16(xmm0U, xmmCoeffU2, xmm0); // xmm0U += (112 * G)
			xmm0V = vmlaq_s16(xmm0V, xmmCoeffV2, xmm0); // xmm0V += (-18 * G)
			xmm1U = vmlaq_s16(xmm1U, xmmCoeffU2, xmm1); // xmm1U += (112 * G)
			xmm1V = vmlaq_s16(xmm1V, xmmCoeffV2, xmm1); // xmm1V += (-18 * G)
			// >> 8
			xmm0U = vshrq_n_s16(xmm0U, 8); // xmm0U >>= 8
			xmm0V = vshrq_n_s16(xmm0V, 8); // xmm0V >>= 8
			xmm1U = vshrq_n_s16(xmm1U, 8); // xmm1U >>= 8
			xmm1V = vshrq_n_s16(xmm1V, 8); // xmm1V >>= 8
			// + 128
			xmm0U = vaddq_s16(xmm0U, xmm128); // xmm0U += 128
			xmm0V = vaddq_s16(xmm0V, xmm128); // xmm0V += 128
			xmm1U = vaddq_s16(xmm1U, xmm128); // xmm1U += 128
			xmm1V = vaddq_s16(xmm1V, xmm128); // xmm1V += 128
			// outPtr = concat(saturate(xmm0), saturate(xmm1))
			vst1q_u8(outUPtr, vcombine_u8(vqmovun_s16(xmm0U), vqmovun_s16(xmm1U)));
			vst1q_u8(outVPtr, vcombine_u8(vqmovun_s16(xmm0V), vqmovun_s16(xmm1V)));
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
