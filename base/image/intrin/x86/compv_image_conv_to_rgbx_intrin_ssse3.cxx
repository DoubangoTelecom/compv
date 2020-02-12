/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_SSEx(yuv420p, rgb24, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
#	undef yuv420p_u_load_SSSE3
}

void CompVImageConvYuv422p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define yuv422p_u_load_SSSE3 yuv422p_u_load_SSE2
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_SSEx(yuv422p, rgb24, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
#	undef yuv422p_u_load_SSSE3
}

void CompVImageConvYuv444p_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define yuv444p_u_load_SSSE3 yuv444p_u_load_SSE2
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_SSEx(yuv444p, rgb24, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
#	undef yuv444p_u_load_SSSE3
}

void CompVImageConvNv12_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_SSEx(nv12, rgb24, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
}

void CompVImageConvNv12_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define rgba32_store_SSSE3 rgba32_store_SSE2
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_SSEx(nv12, rgba32, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
#	undef rgba32_store_SSSE3
}

void CompVImageConvNv21_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_SSEx(nv21, rgb24, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
}

void CompVImageConvNv21_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define rgba32_store_SSSE3 rgba32_store_SSE2
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_SSEx(nv21, rgba32, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
#	undef rgba32_store_SSSE3
}

void CompVImageConvYuyv422_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPacked_to_Rgbx_Intrin_SSEx(yuyv422, rgb24, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
}

void CompVImageConvYuyv422_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define rgba32_store_SSSE3 rgba32_store_SSE2
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPacked_to_Rgbx_Intrin_SSEx(yuyv422, rgba32, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
#	undef rgba32_store_SSSE3
}

void CompVImageConvUyvy422_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPacked_to_Rgbx_Intrin_SSEx(uyvy422, rgb24, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
}

void CompVImageConvUyvy422_to_Rgba32_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();
#	define rgba32_store_SSSE3 rgba32_store_SSE2
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-variable")
	CompVImageConvYuvPacked_to_Rgbx_Intrin_SSEx(uyvy422, rgba32, SSSE3);
	COMPV_GCC_DISABLE_WARNINGS_END()
#	undef rgba32_store_SSSE3
}

void CompVImageConvRgba32_to_Rgb24_Intrin_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgba32Ptr, COMPV_ALIGNED(SSE) uint8_t* rgb24Ptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
	__m128i vecLaneR, vecLaneG, vecLaneB, vecLaneA, vectmp0, vectmp1;
	const compv_uint8x4_t* rgba32Ptr_ = reinterpret_cast<const compv_uint8x4_t*>(rgba32Ptr);
	compv_uint8x3_t* rgb24Ptr_ = reinterpret_cast<compv_uint8x3_t*>(rgb24Ptr);
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) { // strided/SSE-aligned -> can write beyond width
			COMPV_VLD4_U8_SSSE3(&rgba32Ptr_[i], vecLaneR, vecLaneG, vecLaneB, vecLaneA, vectmp0, vectmp1);
			COMPV_VST3_U8_SSSE3(&rgb24Ptr_[i], vecLaneR, vecLaneG, vecLaneB, vectmp0, vectmp1);
		}
		rgba32Ptr_ += stride;
		rgb24Ptr_ += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
