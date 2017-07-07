/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_TO_RGBX_INTRIN_SSSE3_H_)
#define _COMPV_BASE_IMAGE_CONV_TO_RGBX_INTRIN_SSSE3_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageConvYuv420p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVImageConvYuv422p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVImageConvYuv444p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

void CompVImageConvNv12_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVImageConvNv12_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVImageConvNv21_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
void CompVImageConvNv21_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_CONV_TO_RGBX_INTRIN_SSSE3_H_ */
