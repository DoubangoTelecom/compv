/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_rgbfamily.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Supports RGB24, BGR24...family
// Single-threaded function
void CompVImageConvRGBfamily::rgb24family_to_y(const uint8_t* rgbPtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, 
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8)
{
	// internal function, no need to check result or input parameters
	// up to the caller to use multi-threading

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	compv_uscalar_t padSample = (stride - width);
	compv_uscalar_t padRGB = padSample * 3;
	compv_uscalar_t padY = padSample;
	// Convert coeffs from int8 to int16 to avoid math ops overflow
	const int16_t c0 = static_cast<int16_t>(kRGBfamilyToYUV_YCoeffs8[0]);
	const int16_t c1 = static_cast<int16_t>(kRGBfamilyToYUV_YCoeffs8[1]);
	const int16_t c2 = static_cast<int16_t>(kRGBfamilyToYUV_YCoeffs8[2]);
	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			*outYPtr++ = CompVMathUtils::clampPixel8((((c0 * rgbPtr[0]) + (c1 * rgbPtr[1]) + (c2 * rgbPtr[2])) >> 7) + 16);
			rgbPtr += 3;
		}
		rgbPtr += padRGB;
		outYPtr += padY;
	}
}

void CompVImageConvRGBfamily::rgb24_to_y(const uint8_t* rgb24Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	// internal function, no need to check result or input parameters
	CompVImageConvRGBfamily::rgb24family_to_y(rgb24Ptr, outYPtr, width, height, stride, kRGBAToYUV_YCoeffs8);
}

void CompVImageConvRGBfamily::bgr24_to_y(const uint8_t* bgr24Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	// internal function, no need to check result or input parameters
	CompVImageConvRGBfamily::rgb24family_to_y(bgr24Ptr, outYPtr, width, height, stride, kBGRAToYUV_YCoeffs8);
}

// Supports RGB24, BGR24...family
// Single-threaded function
// U and V subsampled 1x1
void CompVImageConvRGBfamily::rgb24family_to_uv_planar_11(const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8)
{
	// internal function, no need to check result or input parameters
	// up to the caller to use multi-threading

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	compv_uscalar_t i, j, padUV = (stride - width), padRGB = (padUV * 3);
	// Convert coeffs from int8 to int16 to avoid math ops overflow
	const int16_t c0u = kRGBfamilyToYUV_UCoeffs8[0], c0v = kRGBfamilyToYUV_VCoeffs8[0];
	const int16_t c1u = kRGBfamilyToYUV_UCoeffs8[1], c1v = kRGBfamilyToYUV_VCoeffs8[1];
	const int16_t c2u = kRGBfamilyToYUV_UCoeffs8[2], c2v = kRGBfamilyToYUV_VCoeffs8[2];
	// U = (((-38 * R) + (-74 * G) + (112 * B))) >> 8 + 128
	// V = (((112 * R) + (-94 * G) + (-18 * B))) >> 8 + 128
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; ++i) {
			*outUPtr++ = CompVMathUtils::clampPixel8((((c0u* rgbPtr[0]) + (c1u * rgbPtr[1]) + (c2u * rgbPtr[2])) >> 8) + 128);
			*outVPtr++ = CompVMathUtils::clampPixel8(((((c0v * rgbPtr[0]) + (c1v* rgbPtr[1]) + (c2v * rgbPtr[2]))) >> 8) + 128);
			rgbPtr += 3;
		}
		rgbPtr += padRGB;
		outUPtr += padUV;
		outVPtr += padUV;
	}
}

void CompVImageConvRGBfamily::rgb24_to_uv_planar_11(const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	// internal function, no need to check result or input parameters
	CompVImageConvRGBfamily::rgb24family_to_uv_planar_11(rgbPtr, outUPtr, outVPtr, width, height, stride, kRGBAToYUV_UCoeffs8, kRGBAToYUV_VCoeffs8);
}

void CompVImageConvRGBfamily::bgr24_to_uv_planar_11(const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	// internal function, no need to check result or input parameters
	CompVImageConvRGBfamily::rgb24family_to_uv_planar_11(rgbPtr, outUPtr, outVPtr, width, height, stride, kBGRAToYUV_UCoeffs8, kBGRAToYUV_VCoeffs8);
}

COMPV_NAMESPACE_END()
