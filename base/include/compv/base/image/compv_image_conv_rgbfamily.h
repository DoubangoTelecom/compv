/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_RGBFAMILY_H_)
#define _COMPV_BASE_IMAGE_CONV_RGBFAMILY_H_

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

// CompVImageConvRGBfamily::functions are internals and no need to check result or input parameters
class CompVImageConvRGBfamily
{
public:
	static void rgb24_to_y(const uint8_t* rgb24Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgr24_to_y(const uint8_t* bgr24Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void rgb24_to_uv_planar_11(const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgr24_to_uv_planar_11(const uint8_t* rgbPtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

	static void rgba32_to_y(const uint8_t* rgba32Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void argb32_to_y(const uint8_t* argb32Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgra32_to_y(const uint8_t* bgra32Ptr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void rgba32_to_uv_planar_11(const uint8_t* rgba32Ptr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void argb32_to_uv_planar_11(const uint8_t* argb32Ptr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgra32_to_uv_planar_11(const uint8_t* bgra32Ptr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

	static void rgb565le_to_y(const uint8_t* rgb565lePtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void rgb565be_to_y(const uint8_t* rgb565bePtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgr565le_to_y(const uint8_t* rgb565lePtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgr565be_to_y(const uint8_t* rgb565bePtr, uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void rgb565le_to_uv_planar_11(const uint8_t* rgb565lePtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void rgb565be_to_uv_planar_11(const uint8_t* rgb565bePtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgr565le_to_uv_planar_11(const uint8_t* rgb565lePtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
	static void bgr565be_to_uv_planar_11(const uint8_t* rgb565bePtr, uint8_t* outUPtr, uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_CONV_RGBFAMILY_H_ */
