/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_CONV_RGBFAMILY_INTRIN_SSE2_H_)
#define _COMPV_BASE_IMAGE_CONV_RGBFAMILY_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVImageConvRgb565lefamily_to_y_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565lePtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8);
void CompVImageConvRgb565befamily_to_y_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565bePtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8);
void CompVImageConvRgb565lefamily_to_uv_planar_11_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565lePtr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8);
void CompVImageConvRgb565befamily_to_uv_planar_11_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* rgb565lePtr, COMPV_ALIGNED(SSE) uint8_t* outUPtr, COMPV_ALIGNED(SSE) uint8_t* outVPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride,
	COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8, COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_IMAGE_CONV_RGBFAMILY_INTRIN_SSE2_H_ */
