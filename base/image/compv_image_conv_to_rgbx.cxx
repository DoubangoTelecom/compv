/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_conv_to_rgbx.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/image/intrin/x86/compv_image_conv_to_rgbx_intrin_sse2.h"
#include "compv/base/image/intrin/x86/compv_image_conv_to_rgbx_intrin_ssse3.h"
#include "compv/base/image/intrin/x86/compv_image_conv_to_rgbx_intrin_avx2.h"
#include "compv/base/image/intrin/arm/compv_image_conv_to_rgbx_intrin_neon.h"

#define COMPV_THIS_CLASSNAME	"CompVImageConvToRGBx"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgb24_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgba32_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgb24_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgba32_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uPtr, COMPV_ALIGNED(SSE) const uint8_t* vPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv12_to_Rgba32_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv12_to_Rgb24_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv21_to_Rgba32_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv21_to_Rgb24_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgb24_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgba32_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgb24_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgba32_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* yuvPtr, COMPV_ALIGNED(SSE) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgb24_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgba32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgb24_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgba32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uPtr, COMPV_ALIGNED(AVX) const uint8_t* vPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv12_to_Rgba32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv12_to_Rgb24_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv21_to_Rgba32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvNv21_to_Rgb24_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yPtr, COMPV_ALIGNED(AVX) const uint8_t* uvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgb24_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yuvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgba32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yuvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgb24_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yuvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgba32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* yuvPtr, COMPV_ALIGNED(AVX) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvRgba32_to_Rgb24_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgba32Ptr, COMPV_ALIGNED(SSE) uint8_t* rgb24Ptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
#	elif COMPV_ARCH_ARM32
	COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgb24_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgba32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgb24_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgba32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv12_to_Rgba32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv12_to_Rgb24_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv21_to_Rgba32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv21_to_Rgb24_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgb24_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgba32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgb24_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgba32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvRgba32_to_Rgb24_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* rgba32Ptr, COMPV_ALIGNED(NEON) uint8_t* rgb24Ptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#	elif COMPV_ARCH_ARM64
	COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgb24_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuv420p_to_Rgba32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgb24_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuv422p_to_Rgba32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv12_to_Rgba32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv12_to_Rgb24_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv21_to_Rgba32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvNv21_to_Rgb24_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(SSE) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgb24_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvYuyv422_to_Rgba32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgb24_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
    COMPV_EXTERNC void CompVImageConvUyvy422_to_Rgba32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
	COMPV_EXTERNC void CompVImageConvRgba32_to_Rgb24_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* rgba32Ptr, COMPV_ALIGNED(NEON) uint8_t* rgb24Ptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#	endif /* COMPV_ARCH_X64 */
#endif /* COMPV_ASM */

static void yuv420p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv420p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv422p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv422p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv444p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuv444p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv12_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv12_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv21_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void nv21_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuyv422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void yuyv422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void uyvy422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void uyvy422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void rgba32_to_rgb24_C(const uint8_t* rgba32Ptr, uint8_t* rgb24Ptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

COMPV_ERROR_CODE CompVImageConvToRGBx::process(const CompVMatPtr& imageIn, const COMPV_SUBTYPE rgbxFormat, CompVMatPtrPtr imageRGBx)
{
	// Internal function, do not check input parameters (already done)
	//	-> rgbxFormat must be RGBA32 or RGB24

	CompVMatPtr imageOut = (*imageRGBx == imageIn) ? nullptr : *imageRGBx;
	COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&imageOut, rgbxFormat, imageIn->cols(), imageIn->rows(), imageIn->stride()));

	const COMPV_SUBTYPE inSubType = (imageIn->planeCount() == 1 && imageIn->elmtInBytes() == sizeof(uint8_t))
		? COMPV_SUBTYPE_PIXELS_Y : imageIn->subType();

	switch (inSubType) {
	case COMPV_SUBTYPE_PIXELS_RGBA32:
	case COMPV_SUBTYPE_PIXELS_BGRA32:
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::rgbx(imageIn, imageOut));
		break;

	case COMPV_SUBTYPE_PIXELS_YUV420P:
	case COMPV_SUBTYPE_PIXELS_YUV422P:
	case COMPV_SUBTYPE_PIXELS_YUV444P:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::yuvPlanar(imageIn, imageOut));
		break;

	case COMPV_SUBTYPE_PIXELS_NV12:
	case COMPV_SUBTYPE_PIXELS_NV21:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::yuvSemiPlanar(imageIn, imageOut));
		break;

	case COMPV_SUBTYPE_PIXELS_YUYV422:
	case COMPV_SUBTYPE_PIXELS_UYVY422:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::yuvPacked(imageIn, imageOut));
		break;

	case COMPV_SUBTYPE_PIXELS_Y:
		COMPV_CHECK_CODE_RETURN(CompVImageConvToRGBx::yGrayscale(imageIn, imageOut));
		break;

	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> %s not supported", CompVGetSubtypeString(imageIn->subType()), CompVGetSubtypeString(rgbxFormat));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	*imageRGBx = imageOut;

	return COMPV_ERROR_CODE_S_OK;
}

// YUV420P, YVU420P, YUV422P, YUV444P
COMPV_ERROR_CODE CompVImageConvToRGBx::yuvPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBx)
{
	// Internal function, do not check input parameters (already done)
	void(*fptr_planar_to_rgbx)(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	const COMPV_SUBTYPE outPixelFormat = imageRGBx->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUV420P:
		fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? yuv420p_to_rgba32_C : yuv420p_to_rgb24_C;
#if COMPV_ARCH_X86
		if (imageRGBx->isAlignedSSE(0) && imageIn->isAlignedSSE(0) && imageIn->isAlignedSSE(1) && imageIn->isAlignedSSE(2)) {
			if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
				if (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) {
					COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = CompVImageConvYuv420p_to_Rgba32_Intrin_SSE2);
					COMPV_EXEC_IFDEF_ASM_X64(fptr_planar_to_rgbx = CompVImageConvYuv420p_to_Rgba32_Asm_X64_SSE2);
				}
			}
			if (CompVCpu::isEnabled(kCpuFlagSSSE3)) {
				if (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGB24) {
					COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = CompVImageConvYuv420p_to_Rgb24_Intrin_SSSE3);
					COMPV_EXEC_IFDEF_ASM_X64(fptr_planar_to_rgbx = CompVImageConvYuv420p_to_Rgb24_Asm_X64_SSSE3);
				}
			}
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX(0) && imageIn->isAlignedAVX(0) && imageIn->isAlignedAVX(1) && imageIn->isAlignedAVX(2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv420p_to_Rgba32_Intrin_AVX2 : CompVImageConvYuv420p_to_Rgb24_Intrin_AVX2);			
			COMPV_EXEC_IFDEF_ASM_X64(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv420p_to_Rgba32_Asm_X64_AVX2 : CompVImageConvYuv420p_to_Rgb24_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageRGBx->isAlignedNEON(0) && imageIn->isAlignedNEON(0) && imageIn->isAlignedNEON(1) && imageIn->isAlignedNEON(2)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv420p_to_Rgba32_Intrin_NEON : CompVImageConvYuv420p_to_Rgb24_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv420p_to_Rgba32_Asm_NEON32 : CompVImageConvYuv420p_to_Rgb24_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv420p_to_Rgba32_Asm_NEON64 : CompVImageConvYuv420p_to_Rgb24_Asm_NEON64);
		}
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_YUV422P:
		fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? yuv422p_to_rgba32_C : yuv422p_to_rgb24_C;
#if COMPV_ARCH_X86
		if (imageRGBx->isAlignedSSE(0) && imageIn->isAlignedSSE(0) && imageIn->isAlignedSSE(1) && imageIn->isAlignedSSE(2)) {
			if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
				if (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) {
					COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = CompVImageConvYuv422p_to_Rgba32_Intrin_SSE2);
					COMPV_EXEC_IFDEF_ASM_X64(fptr_planar_to_rgbx = CompVImageConvYuv422p_to_Rgba32_Asm_X64_SSE2);
				}
			}
			if (CompVCpu::isEnabled(kCpuFlagSSSE3)) {
				if (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGB24) {
					COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = CompVImageConvYuv422p_to_Rgb24_Intrin_SSSE3);
					COMPV_EXEC_IFDEF_ASM_X64(fptr_planar_to_rgbx = CompVImageConvYuv422p_to_Rgb24_Asm_X64_SSSE3);
				}
			}
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX(0) && imageIn->isAlignedAVX(0) && imageIn->isAlignedAVX(1) && imageIn->isAlignedAVX(2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv422p_to_Rgba32_Intrin_AVX2 : CompVImageConvYuv422p_to_Rgb24_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv422p_to_Rgba32_Asm_X64_AVX2 : CompVImageConvYuv422p_to_Rgb24_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageRGBx->isAlignedNEON(0) && imageIn->isAlignedNEON(0) && imageIn->isAlignedNEON(1) && imageIn->isAlignedNEON(2)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv422p_to_Rgba32_Intrin_NEON : CompVImageConvYuv422p_to_Rgb24_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv422p_to_Rgba32_Asm_NEON32 : CompVImageConvYuv422p_to_Rgb24_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv422p_to_Rgba32_Asm_NEON64 : CompVImageConvYuv422p_to_Rgb24_Asm_NEON64);
		}
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_YUV444P:
		fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? yuv444p_to_rgba32_C : yuv444p_to_rgb24_C;
#if COMPV_ARCH_X86
		if (imageRGBx->isAlignedSSE(0) && imageIn->isAlignedSSE(0) && imageIn->isAlignedSSE(1) && imageIn->isAlignedSSE(2)) {
			if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
				if (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) {
					COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = CompVImageConvYuv444p_to_Rgba32_Intrin_SSE2);
				}
			}
			if (CompVCpu::isEnabled(kCpuFlagSSSE3)) {
				if (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGB24) {
					COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = CompVImageConvYuv444p_to_Rgb24_Intrin_SSSE3);
				}
			}
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX(0) && imageIn->isAlignedAVX(0) && imageIn->isAlignedAVX(1) && imageIn->isAlignedAVX(2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv444p_to_Rgba32_Intrin_AVX2 : CompVImageConvYuv444p_to_Rgb24_Intrin_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageRGBx->isAlignedNEON(0) && imageIn->isAlignedNEON(0) && imageIn->isAlignedNEON(1) && imageIn->isAlignedNEON(2)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_planar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuv444p_to_Rgba32_Intrin_NEON : CompVImageConvYuv444p_to_Rgb24_Intrin_NEON);
		}
#endif
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> %s not supported", CompVGetSubtypeString(inPixelFormat), CompVGetSubtypeString(outPixelFormat));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

	const size_t widthInSamples = imageRGBx->cols();
	const size_t heightInSamples = imageRGBx->rows();
	const size_t strideInSamples = imageRGBx->stride();

	const uint8_t* yPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y);
	const uint8_t* uPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_U);
	const uint8_t* vPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_V);
	uint8_t* rgbxPtr = imageRGBx->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heights + (heightInSamples % heights);
		size_t yPtrPaddingInBytes, uPtrPaddingInBytes, vPtrPaddingInBytes, rgbxPtrPaddingInBytes;
		size_t uPtrHeights, vPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yPtr_, const uint8_t* uPtr_, const uint8_t* vPtr_, uint8_t* rgbxPtr_, compv_uscalar_t heightInSamples_) -> void {
			fptr_planar_to_rgbx(
				yPtr_, uPtr_, vPtr_, rgbxPtr_,
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_U, 1, heights, &tmpWidth, &uPtrHeights));
		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_V, 1, heights, &tmpWidth, &vPtrHeights));

		yPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_Y) * heights;
		uPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_U) * uPtrHeights;
		vPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_U) * vPtrHeights;
		rgbxPtrPaddingInBytes = imageRGBx->strideInBytes() * heights;

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uPtr, vPtr, rgbxPtr, heights), taskIds), "Dispatching task failed");
			yPtr += yPtrPaddingInBytes;
			uPtr += uPtrPaddingInBytes;
			vPtr += vPtrPaddingInBytes;
			rgbxPtr += rgbxPtrPaddingInBytes;
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uPtr, vPtr, rgbxPtr, lastHeight), taskIds), "Dispatching task failed");
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		fptr_planar_to_rgbx(
			yPtr, uPtr, vPtr, rgbxPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// NV12, NV21
COMPV_ERROR_CODE CompVImageConvToRGBx::yuvSemiPlanar(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBx)
{
	// Internal function, do not check input parameters (already done)
	void(*fptr_semiplanar_to_rgbx)(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	const COMPV_SUBTYPE outPixelFormat = imageRGBx->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_NV12:
		fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? nv12_to_rgba32_C : nv12_to_rgb24_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE(0) && imageIn->isAlignedSSE(0) && imageIn->isAlignedSSE(1)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv12_to_Rgba32_Intrin_SSSE3: CompVImageConvNv12_to_Rgb24_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv12_to_Rgba32_Asm_X64_SSSE3 : CompVImageConvNv12_to_Rgb24_Asm_X64_SSSE3);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX(0) && imageIn->isAlignedAVX(0) && imageIn->isAlignedAVX(1)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv12_to_Rgba32_Intrin_AVX2 : CompVImageConvNv12_to_Rgb24_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv12_to_Rgba32_Asm_X64_AVX2 : CompVImageConvNv12_to_Rgb24_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageRGBx->isAlignedNEON(0) && imageIn->isAlignedNEON(0) && imageIn->isAlignedNEON(1)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv12_to_Rgba32_Intrin_NEON : CompVImageConvNv12_to_Rgb24_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv12_to_Rgba32_Asm_NEON32 : CompVImageConvNv12_to_Rgb24_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv12_to_Rgba32_Asm_NEON64 : CompVImageConvNv12_to_Rgb24_Asm_NEON64);
		}
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_NV21:
		fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? nv21_to_rgba32_C : nv21_to_rgb24_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE(0) && imageIn->isAlignedSSE(0) && imageIn->isAlignedSSE(1)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv21_to_Rgba32_Intrin_SSSE3 : CompVImageConvNv21_to_Rgb24_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv21_to_Rgba32_Asm_X64_SSSE3 : CompVImageConvNv21_to_Rgb24_Asm_X64_SSSE3);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX(0) && imageIn->isAlignedAVX(0) && imageIn->isAlignedAVX(1)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv21_to_Rgba32_Intrin_AVX2 : CompVImageConvNv21_to_Rgb24_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv21_to_Rgba32_Asm_X64_AVX2 : CompVImageConvNv21_to_Rgb24_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageRGBx->isAlignedNEON(0) && imageIn->isAlignedNEON(0) && imageIn->isAlignedNEON(1)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv21_to_Rgba32_Intrin_NEON : CompVImageConvNv21_to_Rgb24_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv21_to_Rgba32_Asm_NEON32 : CompVImageConvNv21_to_Rgb24_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(fptr_semiplanar_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvNv21_to_Rgba32_Asm_NEON64 : CompVImageConvNv21_to_Rgb24_Asm_NEON64);
		}
#endif
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> %s not supported", CompVGetSubtypeString(inPixelFormat), CompVGetSubtypeString(outPixelFormat));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGBx->cols();
	const size_t heightInSamples = imageRGBx->rows();
	const size_t strideInSamples = imageRGBx->stride();

	const uint8_t* yPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_Y);
	const uint8_t* uvPtr = imageIn->ptr<const uint8_t>(0, 0, COMPV_PLANE_UV);
	uint8_t* rgbxPtr = imageRGBx->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heights + (heightInSamples % heights);
		size_t yPtrPaddingInBytes, uvPtrPaddingInBytes, rgbxPtrPaddingInBytes;
		size_t uvPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yPtr_, const uint8_t* uvPtr_, uint8_t* rgbxPtr_, compv_uscalar_t heightInSamples_) -> void {
			fptr_semiplanar_to_rgbx(
				yPtr_, uvPtr_, rgbxPtr_,
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, COMPV_PLANE_UV, 1, heights, &tmpWidth, &uvPtrHeights));

		yPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_Y) * heights;
		uvPtrPaddingInBytes = imageIn->strideInBytes(COMPV_PLANE_UV) * uvPtrHeights;
		rgbxPtrPaddingInBytes = imageRGBx->strideInBytes() * heights;

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uvPtr, rgbxPtr, heights), taskIds), "Dispatching task failed");
			yPtr += yPtrPaddingInBytes;
			uvPtr += uvPtrPaddingInBytes;
			rgbxPtr += rgbxPtrPaddingInBytes;
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yPtr, uvPtr, rgbxPtr, lastHeight), taskIds), "Dispatching task failed");
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		fptr_semiplanar_to_rgbx(
			yPtr, uvPtr, rgbxPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// YUYV422, UYVY422
COMPV_ERROR_CODE CompVImageConvToRGBx::yuvPacked(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBx)
{
	// Internal function, do not check input parameters (already done)
	void(*fptr_packed_to_rgbx)(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	const COMPV_SUBTYPE outPixelFormat = imageRGBx->subType();
	switch (inPixelFormat) {
	case COMPV_SUBTYPE_PIXELS_YUYV422:
		fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? yuyv422_to_rgba32_C : yuyv422_to_rgb24_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE(0) && imageIn->isAlignedSSE(0)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuyv422_to_Rgba32_Intrin_SSSE3 : CompVImageConvYuyv422_to_Rgb24_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuyv422_to_Rgba32_Asm_X64_SSSE3 : CompVImageConvYuyv422_to_Rgb24_Asm_X64_SSSE3);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX(0) && imageIn->isAlignedAVX(0)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuyv422_to_Rgba32_Intrin_AVX2 : CompVImageConvYuyv422_to_Rgb24_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuyv422_to_Rgba32_Asm_X64_AVX2 : CompVImageConvYuyv422_to_Rgb24_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageRGBx->isAlignedNEON(0) && imageIn->isAlignedNEON(0)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuyv422_to_Rgba32_Intrin_NEON : CompVImageConvYuyv422_to_Rgb24_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuyv422_to_Rgba32_Asm_NEON32 : CompVImageConvYuyv422_to_Rgb24_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvYuyv422_to_Rgba32_Asm_NEON64 : CompVImageConvYuyv422_to_Rgb24_Asm_NEON64);
		}
#endif
		break;
	case COMPV_SUBTYPE_PIXELS_UYVY422:
		fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? uyvy422_to_rgba32_C : uyvy422_to_rgb24_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE(0) && imageIn->isAlignedSSE(0)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvUyvy422_to_Rgba32_Intrin_SSSE3 : CompVImageConvUyvy422_to_Rgb24_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvUyvy422_to_Rgba32_Asm_X64_SSSE3 : CompVImageConvUyvy422_to_Rgb24_Asm_X64_SSSE3);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && imageRGBx->isAlignedAVX(0) && imageIn->isAlignedAVX(0)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvUyvy422_to_Rgba32_Intrin_AVX2 : CompVImageConvUyvy422_to_Rgb24_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvUyvy422_to_Rgba32_Asm_X64_AVX2 : CompVImageConvUyvy422_to_Rgb24_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && imageRGBx->isAlignedNEON(0) && imageIn->isAlignedNEON(0)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvUyvy422_to_Rgba32_Intrin_NEON : CompVImageConvUyvy422_to_Rgb24_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvUyvy422_to_Rgba32_Asm_NEON32 : CompVImageConvUyvy422_to_Rgb24_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(fptr_packed_to_rgbx = (outPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32) ? CompVImageConvUyvy422_to_Rgba32_Asm_NEON64 : CompVImageConvUyvy422_to_Rgb24_Asm_NEON64);
		}
#endif
		break;
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> %s not supported", CompVGetSubtypeString(inPixelFormat), CompVGetSubtypeString(outPixelFormat));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	const size_t widthInSamples = imageRGBx->cols();
	const size_t heightInSamples = imageRGBx->rows();
	const size_t strideInSamples = imageRGBx->stride();

	const uint8_t* yuvPtr = imageIn->ptr<const uint8_t>();
	uint8_t* rgbxPtr = imageRGBx->ptr<uint8_t>();

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;

	// Compute number of threads
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(strideInSamples, heightInSamples, maxThreads, COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD)
		: 1;

	if (threadsCount > 1) {
		const size_t heights = (heightInSamples / threadsCount) & -2; //!\\ must be even number
		const size_t lastHeight = heights + (heightInSamples % heights);
		size_t yuvPtrPaddingInBytes, rgbxPtrPaddingInBytes;
		size_t yuvPtrHeights, tmpWidth;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](const uint8_t* yuvPtr_, uint8_t* rgbxPtr_, compv_uscalar_t heightInSamples_) -> void {
			fptr_packed_to_rgbx(
				yuvPtr_, rgbxPtr_,
				widthInSamples, heightInSamples_, strideInSamples
			);
		};

		COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(inPixelFormat, 0, 1, heights, &tmpWidth, &yuvPtrHeights));

		yuvPtrPaddingInBytes = imageIn->strideInBytes() * yuvPtrHeights;
		rgbxPtrPaddingInBytes = imageRGBx->strideInBytes() * heights;

		for (size_t threadIdx = 0; threadIdx < threadsCount - 1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yuvPtr, rgbxPtr, heights), taskIds), "Dispatching task failed");
			yuvPtr += yuvPtrPaddingInBytes;
			rgbxPtr += rgbxPtrPaddingInBytes;
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, yuvPtr, rgbxPtr, lastHeight), taskIds), "Dispatching task failed");
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		fptr_packed_to_rgbx(
			yuvPtr, rgbxPtr,
			widthInSamples, heightInSamples, strideInSamples
		);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Y -> RGBx
COMPV_ERROR_CODE CompVImageConvToRGBx::yGrayscale(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBx)
{
	COMPV_ASSERT(imageIn->planeCount() == 1 && imageIn->elmtInBytes() == sizeof(uint8_t) && imageRGBx->cols() == imageIn->cols() && imageRGBx->rows() == imageIn->rows() && imageRGBx->stride() == imageIn->stride());

	switch (imageRGBx->subType()) {
	case COMPV_SUBTYPE_PIXELS_RGB24:
	case COMPV_SUBTYPE_PIXELS_BGR24: {
		CompVMatPtrVector vecRGB(3, imageIn); // duplicate reference (not a copy operation)
		COMPV_CHECK_CODE_RETURN(CompVImage::pack(vecRGB, imageRGBx->subType(), &imageRGBx));
		break;
	}
	case COMPV_SUBTYPE_PIXELS_ARGB32:
	case COMPV_SUBTYPE_PIXELS_ABGR32:
	case COMPV_SUBTYPE_PIXELS_BGRA32:
	case COMPV_SUBTYPE_PIXELS_RGBA32: {
		CompVMatPtrVector vecRGB(4, imageIn); // duplicate reference (not a copy operation)
		const size_t alphaIndex = (imageRGBx->subType() == COMPV_SUBTYPE_PIXELS_ARGB32 || imageRGBx->subType() == COMPV_SUBTYPE_PIXELS_ABGR32) ? 0 : 3;
		CompVMatPtr alphatMat;
		COMPV_CHECK_CODE_RETURN(CompVImage::newObj8u(&alphatMat, COMPV_SUBTYPE_PIXELS_Y, imageIn->cols(), imageIn->rows(), imageIn->stride()));
		COMPV_CHECK_CODE_RETURN(CompVMem::set(alphatMat->ptr<void>(), 0xff, alphatMat->dataSizeInBytes(), sizeof(uint8_t)));
		vecRGB[alphaIndex] = alphatMat;
		COMPV_CHECK_CODE_RETURN(CompVImage::pack(vecRGB, imageRGBx->subType(), &imageRGBx));
		break;
	}
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> %s not supported", CompVGetSubtypeString(imageIn->subType()), CompVGetSubtypeString(imageRGBx->subType()));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// RGBx -> RGBx
COMPV_ERROR_CODE CompVImageConvToRGBx::rgbx(const CompVMatPtr& imageIn, CompVMatPtr& imageRGBx)
{
	// Internal function, do not check input parameters (already done) ... but I want to feel good
	COMPV_ASSERT(imageRGBx->cols() == imageIn->cols() && imageRGBx->rows() == imageIn->rows() && imageRGBx->stride() == imageIn->stride());
	const COMPV_SUBTYPE inPixelFormat = imageIn->subType();
	const COMPV_SUBTYPE outPixelFormat = imageRGBx->subType();

	if (inPixelFormat == outPixelFormat) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Source and destination formats are the same. Useless, cloning.");
		COMPV_CHECK_CODE_RETURN(CompVImage::clone(imageIn, &imageRGBx));
		return COMPV_ERROR_CODE_S_OK;
	}

	// BGRA32/BGR24 -> RGB24 is very important because many deep learning functions expect RGB_888 as input.
	if ((inPixelFormat == COMPV_SUBTYPE_PIXELS_BGR24 || inPixelFormat == COMPV_SUBTYPE_PIXELS_BGRA32) && outPixelFormat == COMPV_SUBTYPE_PIXELS_RGB24) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No in-place conversion found for BGRA32/BGR24 -> RGB24. You should consider using RGBA32/RGB24 instead of BGRA32/BGR24");
		CompVMatPtrVector bgra32Or24Vec;
		COMPV_CHECK_CODE_RETURN(CompVImage::unpack(imageIn, bgra32Or24Vec));
		CompVMatPtr R = bgra32Or24Vec[2], G = bgra32Or24Vec[0];
		bgra32Or24Vec[0] = R, bgra32Or24Vec[2] = G; // Switch R<->B to convert from BGRA32 to RGBA32
		bgra32Or24Vec.resize(3); // Remove alpha lane
		COMPV_CHECK_CODE_RETURN(CompVImage::pack(bgra32Or24Vec, COMPV_SUBTYPE_PIXELS_RGB24, &imageRGBx));
		return COMPV_ERROR_CODE_S_OK;
	}

	/* Other SIMD in-place conversions */

	void(*fptr_rgbx_to_rgbx)(const uint8_t* rgbxInPtr, uint8_t* rgbxOutPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= nullptr;

	const size_t width = imageRGBx->cols();
	const size_t height = imageRGBx->rows();
	const size_t stride = imageRGBx->stride();
	
	// Support for RGBA32 -> RGB24 is very important because many deep learning functions expect RGB_888 as input.
	if (inPixelFormat == COMPV_SUBTYPE_PIXELS_RGBA32 && outPixelFormat == COMPV_SUBTYPE_PIXELS_RGB24) {
		fptr_rgbx_to_rgbx = rgba32_to_rgb24_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSSE3) && imageRGBx->isAlignedSSE() && imageIn->isAlignedSSE() && COMPV_IS_ALIGNED_SSE(stride)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(fptr_rgbx_to_rgbx = CompVImageConvRgba32_to_Rgb24_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(fptr_rgbx_to_rgbx = CompVImageConvRgba32_to_Rgb24_Asm_X64_SSSE3);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(stride)) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(fptr_rgbx_to_rgbx = CompVImageConvRgba32_to_Rgb24_Intrin_NEON);
			if (imageRGBx->isAlignedNEON() && imageIn->isAlignedNEON()) { // ASM requires mem to be aligned
				COMPV_EXEC_IFDEF_ASM_ARM32(fptr_rgbx_to_rgbx = CompVImageConvRgba32_to_Rgb24_Asm_NEON32);
				COMPV_EXEC_IFDEF_ASM_ARM64(fptr_rgbx_to_rgbx = CompVImageConvRgba32_to_Rgb24_Asm_NEON64);
			}
		}
#endif
	}

	// Check if the function is correctly hooked
	if (!fptr_rgbx_to_rgbx) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "%s -> %s not supported", CompVGetSubtypeString(inPixelFormat), CompVGetSubtypeString(outPixelFormat));
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}

	// Processing
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		fptr_rgbx_to_rgbx(
			imageIn->ptr<const uint8_t>(ystart),
			imageRGBx->ptr<uint8_t>(ystart),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride)
		);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_IMAGE_CONV_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

// R = (37Y' + 0U' + 51V') >> 5
// G = (37Y' - 13U' - 26V') >> 5
// B = (37Y' + 65U' + 0V') >> 5
// A = 255
// where Y'=(Y - 16), U' = (U - 128), V'=(V - 128)
#define yuv_to_rgb(yp, up, vp, r, g, b) \
	r = CompVMathUtils::clampPixel8((37 * yp + 51 * vp) >> 5); \
	g = CompVMathUtils::clampPixel8((37 * yp - 13 * up - 26 * vp) >> 5); \
	b = CompVMathUtils::clampPixel8((37 * yp + 65 * up) >> 5)

#define yuv_to_4rgb(yp, up, vp, r, g, b, a) /*RGBA32*/\
	yuv_to_rgb(yp, up, vp, r, g, b); \
	a = 0xff
#define yuv_to_3rgb(yp, up, vp, r, g, b, a) /*RGB24*/\
	yuv_to_rgb(yp, up, vp, r, g, b)

#define yuv420p_to_rgbx_strideUV() const compv_uscalar_t strideUV = ((stride + 1) >> 1);
#define yuv422p_to_rgbx_strideUV() const compv_uscalar_t strideUV = ((stride + 1) >> 1);
#define yuv444p_to_rgbx_strideUV() const compv_uscalar_t strideUV = stride;

#define yuv420p_to_rgbx_indexUV() (i >> 1)
#define yuv422p_to_rgbx_indexUV() (i >> 1)
#define yuv444p_to_rgbx_indexUV() (i)

#define yuv420p_to_rgbx_checkAddStrideUV() if (j & 1)
#define yuv422p_to_rgbx_checkAddStrideUV() 
#define yuv444p_to_rgbx_checkAddStrideUV() 

#define planar_to_rgbx(name, rgbxw, yPtr, uPtr, vPtr, rgbxPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGBx = stride * rgbxw; \
		const compv_uscalar_t strideY = stride; \
		name##_to_rgbx_strideUV(); \
		int16_t Yp, Up, Vp; \
		compv_uscalar_t i, j, k; \
		for (j = 0; j < height; ++j) { \
			for (i = 0, k = 0; i < width; ++i, k += rgbxw) { \
				Yp = (yPtr[i] - 16); \
				Up = (uPtr[name##_to_rgbx_indexUV()] - 127); \
				Vp = (vPtr[name##_to_rgbx_indexUV()] - 127); \
				yuv_to_##rgbxw##rgb(Yp, Up, Vp, rgbxPtr[k + 0], rgbxPtr[k + 1], rgbxPtr[k + 2], rgbxPtr[k + 3]); \
			} \
			rgbxPtr += strideRGBx; \
			yPtr += strideY; \
			name##_to_rgbx_checkAddStrideUV() { \
				uPtr += strideUV; \
				vPtr += strideUV; \
			} \
		} \
	}

static void yuv420p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgbx(yuv420p, 4, yPtr, uPtr, vPtr, rgbxPtr, width, height, stride);
}

static void yuv420p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgbx(yuv420p, 3, yPtr, uPtr, vPtr, rgbxPtr, width, height, stride);
}

static void yuv422p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgbx(yuv422p, 4, yPtr, uPtr, vPtr, rgbxPtr, width, height, stride);
}

static void yuv422p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgbx(yuv422p, 3, yPtr, uPtr, vPtr, rgbxPtr, width, height, stride);
}

static void yuv444p_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgbx(yuv444p, 4, yPtr, uPtr, vPtr, rgbxPtr, width, height, stride);
}

static void yuv444p_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	planar_to_rgbx(yuv444p, 3, yPtr, uPtr, vPtr, rgbxPtr, width, height, stride);
}


#define nv12_to_rgbx_indexU() 0
#define nv12_to_rgbx_indexV() 1

#define nv21_to_rgbx_indexU() 1
#define nv21_to_rgbx_indexV() 0

#define semiplanar_to_rgbx(name, rgbxw, yPtr, uvPtr, rgbxPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGBx = stride * rgbxw; \
		int16_t Yp, Up, Vp; \
		for (compv_uscalar_t j = 0; j < height; ++j) { \
			for (compv_uscalar_t i = 0, k = 0; i < width; ++i, k += rgbxw) { \
				Yp = (yPtr[i] - 16); \
				Up = (uvPtr[(i & -2) + name##_to_rgbx_indexU()] - 127); \
				Vp = (uvPtr[(i & -2) + name##_to_rgbx_indexV()] - 127); \
				yuv_to_##rgbxw##rgb(Yp, Up, Vp, rgbxPtr[k + 0], rgbxPtr[k + 1], rgbxPtr[k + 2], rgbxPtr[k + 3]); \
			} \
			rgbxPtr += strideRGBx; \
			yPtr += stride; \
			if (j & 1) { \
				uvPtr += stride; \
			} \
		} \
	}

static void nv12_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgbx(nv12, 4, yPtr, uvPtr, rgbxPtr, width, height, stride);
}

static void nv12_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgbx(nv12, 3, yPtr, uvPtr, rgbxPtr, width, height, stride);
}

static void nv21_to_rgba32_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgbx(nv21, 4, yPtr, uvPtr, rgbxPtr, width, height, stride);
}

static void nv21_to_rgb24_C(const uint8_t* yPtr, const uint8_t* uvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	semiplanar_to_rgbx(nv21, 3, yPtr, uvPtr, rgbxPtr, width, height, stride);
}

#define yuyv422_to_rgbx_indexY() 0
#define yuyv422_to_rgbx_indexU() 1
#define yuyv422_to_rgbx_indexV() 3

#define uyvy422_to_rgbx_indexY() 1
#define uyvy422_to_rgbx_indexU() 0
#define uyvy422_to_rgbx_indexV() 2

#define packed_to_rgbx(name, rgbxw, yuyvPtr, rgbxPtr, width, height, stride) { \
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); \
		const compv_uscalar_t strideRGBx = (stride * rgbxw); \
		const compv_uscalar_t strideYUV = (stride << 1); \
		const compv_uscalar_t maxWidth = (width << 1); \
		int16_t Yp, Up, Vp; \
		for (compv_uscalar_t j = 0; j < height; ++j) { \
			for (compv_uscalar_t i = 0, k = 0; i < maxWidth; i += 2, k += rgbxw) { \
				Yp = (yuyvPtr[i + name##_to_rgbx_indexY()] - 16); \
				Up = (yuyvPtr[((i >> 2) << 2) + name##_to_rgbx_indexU()] - 127); \
				Vp = (yuyvPtr[((i >> 2) << 2) + name##_to_rgbx_indexV()] - 127); \
				yuv_to_##rgbxw##rgb(Yp, Up, Vp, rgbxPtr[k + 0], rgbxPtr[k + 1], rgbxPtr[k + 2], rgbxPtr[k + 3]); \
			} \
			rgbxPtr += strideRGBx; \
			yuyvPtr += strideYUV; \
		} \
	}

static void yuyv422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgbx(yuyv422, 4, yuyvPtr, rgbxPtr, width, height, stride);
}

static void yuyv422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgbx(yuyv422, 3, yuyvPtr, rgbxPtr, width, height, stride);
}

static void uyvy422_to_rgba32_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgbx(uyvy422, 4, yuyvPtr, rgbxPtr, width, height, stride);
}

static void uyvy422_to_rgb24_C(const uint8_t* yuyvPtr, uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	packed_to_rgbx(uyvy422, 3, yuyvPtr, rgbxPtr, width, height, stride);
}

static void rgba32_to_rgb24_C(const uint8_t* rgba32Ptr, uint8_t* rgb24Ptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");

	const compv_uint8x4_t* rgba32Ptr_ = reinterpret_cast<const compv_uint8x4_t*>(rgba32Ptr);
	compv_uint8x3_t* rgb24Ptr_ = reinterpret_cast<compv_uint8x3_t*>(rgb24Ptr);
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const compv_uint8x4_t& rgba = rgba32Ptr_[i];
			compv_uint8x3_t& rgb = rgb24Ptr_[i];
			rgb[0] = rgba[0];
			rgb[1] = rgba[1];
			rgb[2] = rgba[2];
		}
		rgba32Ptr_ += stride;
		rgb24Ptr_ += stride;
	}
}

COMPV_NAMESPACE_END()
