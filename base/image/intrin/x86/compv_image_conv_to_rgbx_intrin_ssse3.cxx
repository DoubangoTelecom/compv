/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_conv_to_rgbx_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/image/intrin/x86/compv_image_conv_to_rgbx_intrin_ssex.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVImageConvYuv420p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define yuv420p_u_load_SSSE3 yuv420p_u_load_SSE2
	CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(yuv420p, rgb24, SSSE3);
#	undef yuv420p_u_load_SSSE3
}

void CompVImageConvYuv422p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define yuv422p_u_load_SSSE3 yuv422p_u_load_SSE2
	CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(yuv422p, rgb24, SSSE3);
#	undef yuv422p_u_load_SSSE3
}

void CompVImageConvYuv444p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define yuv444p_u_load_SSSE3 yuv444p_u_load_SSE2
	CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(yuv444p, rgb24, SSSE3);
#	undef yuv444p_u_load_SSSE3
}

void CompVImageConvNv12_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(nv12, rgb24, SSSE3);
}

void CompVImageConvNv12_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define rgba32_store_SSSE3 rgba32_store_SSE2
	CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(nv12, rgba32, SSSE3);
#	undef rgba32_store_SSSE3
}

void CompVImageConvNv21_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(nv21, rgb24, SSSE3);
}

void CompVImageConvNv21_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define rgba32_store_SSSE3 rgba32_store_SSE2
	CompVImageConvPlanar_to_Rgbx_Intrin_SSEx(nv21, rgba32, SSSE3);
#	undef rgba32_store_SSSE3
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
