/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/arm/compv_image_conv_to_rgbx_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/intrin/arm/compv_intrin_neon.h"
#include "compv/base/image/compv_image_conv_common.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_cpu.h"

COMPV_NAMESPACE_BEGIN()

static const uint8x8_t vec16n = vdup_n_u8(16); // half-vector
static const uint8x8_t vec127n = vdup_n_u8(127); // half-vector

static const int16x8_t vec37 = vdupq_n_s16(37);
static const int16x8_t vec51 = vdupq_n_s16(51);
static const int16x8_t vec65 = vdupq_n_s16(65);
static const int16x8_t vec13 = vdupq_n_s16(13);
static const int16x8_t vec26 = vdupq_n_s16(26);

#define rgb24_store(ptr, vecResult)		vst3q_u8((ptr), vecResult)
#define rgba32_store(ptr, vecResult)	vst4q_u8((ptr), vecResult)

#define rgb24_declVecResult				uint8x16x3_t vecResult;
#define rgba32_declVecResult			uint8x16x4_t vecResult; vecResult.val[3] = vceqq_u8(vec37, vec37)

#define rgb24_step						48 /* (16 * 3) */
#define rgba32_step						64 /* (16 * 4) */

#define rgb24_bytes_per_sample			3
#define rgba32_bytes_per_sample			4

#define yuv420p_uv_step					8
#define yuv422p_uv_step					8
#define yuv444p_uv_step					16
#define nv12_uv_step					16
#define nv21_uv_step					16

#define yuv420p_uv_stride				(stride >> 1) /* no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes) */
#define yuv422p_uv_stride				(stride >> 1) /* no need for "((stride + 1) >> 1)" because stride is even (aligned on #16 bytes) */
#define yuv444p_uv_stride				(stride)
#define nv12_uv_stride					(stride)
#define nv21_uv_stride					(stride)

#define yuv420p_uv_prefetch_read(index)	__compv_builtin_prefetch_read(&uPtr[(index)]); __compv_builtin_prefetch_read(&vPtr[(index)])
#define yuv422p_uv_prefetch_read(index)	__compv_builtin_prefetch_read(&uPtr[(index)]); __compv_builtin_prefetch_read(&vPtr[(index)])
#define yuv444p_uv_prefetch_read(index)	__compv_builtin_prefetch_read(&uPtr[(index)]); __compv_builtin_prefetch_read(&vPtr[(index)])
#define nv12_uv_prefetch_read(index)	__compv_builtin_prefetch_read(&uvPtr[(index)])
#define nv21_uv_prefetch_read(index)	__compv_builtin_prefetch_read(&uvPtr[(index)])


#define yuv420p_u_load					vecUn = vld1_u8(&uPtr[l])
#define yuv422p_u_load					vecUn = vld1_u8(&uPtr[l])
#define yuv444p_u_load					vecUlo = vld1q_u8(&uPtr[l]); (void)(vecUn)
#define nv12_u_load						vecUV = vld2_u8(&uvPtr[l]); vecUn = vecUV.val[0]; (void)(vecUn)
#define nv21_u_load						vecUV = vld2_u8(&uvPtr[l]); vecUn = vecUV.val[1]; (void)(vecUn)

#define yuv420p_v_load					vecVn = vld1_u8(&vPtr[l])
#define yuv422p_v_load					vecVn = vld1_u8(&vPtr[l])
#define yuv444p_v_load					vecVlo = vld1q_u8(&vPtr[l]); (void)(vecVn)
#define nv12_v_load						vecVn = vecUV.val[1]; (void)(vecVn)
#define nv21_v_load						vecVn = vecUV.val[0]; (void)(vecVn)

#define yuv420p_uv_inc_check			if (j & 1)
#define yuv422p_uv_inc_check 
#define yuv444p_uv_inc_check
#define nv12_uv_inc_check				if (j & 1)
#define nv21_uv_inc_check				if (j & 1)

#define yuv420p_uv_inc					(uPtr) += strideUV; (vPtr) += strideUV
#define yuv422p_uv_inc					(uPtr) += strideUV; (vPtr) += strideUV
#define yuv444p_uv_inc					(uPtr) += strideUV; (vPtr) += strideUV
#define nv12_uv_inc						(uvPtr) += strideUV
#define nv21_uv_inc						(uvPtr) += strideUV

#define yuv420p_u_primelo				vecUlo = vsubl_u8(vecUn, vec127n)
#define yuv422p_u_primelo				vecUlo = vsubl_u8(vecUn, vec127n)
#define yuv444p_u_primelo				vecUlo = vsubl_u8(vget_low_u8(vecUlo), vec127n)
#define nv12_u_primelo					vecUlo = vsubl_u8(vecUn, vec127n)
#define nv21_u_primelo					vecUlo = vsubl_u8(vecUn, vec127n)

#define yuv420p_v_primelo				vecVlo = vsubl_u8(vecVn, vec127n)
#define yuv422p_v_primelo				vecVlo = vsubl_u8(vecVn, vec127n)
#define yuv444p_v_primelo				vecVlo = vsubl_u8(vget_low_u8(vecVlo), vec127n)
#define nv12_v_primelo					vecVlo = vsubl_u8(vecVn, vec127n)
#define nv21_v_primelo					vecVlo = vsubl_u8(vecVn, vec127n)

#define yuv420p_u_primehi				(void)(vecUhi)
#define yuv422p_u_primehi				(void)(vecUhi)
#define yuv444p_u_primehi				vecUhi = vsubl_u8(vget_high_u8(vecUlo), vec127n)
#define nv12_u_primehi					(void)(vecUhi)
#define nv21_u_primehi					(void)(vecUhi)

#define yuv420p_v_primehi				(void)(vecVhi)
#define yuv422p_v_primehi				(void)(vecVhi)
#define yuv444p_v_primehi				vecVhi = vsubl_u8(vget_high_u8(vecVlo), vec127n)
#define nv12_v_primehi					(void)(vecVhi)
#define nv21_v_primehi					(void)(vecVhi)

#define yuv420p_u_primehi65				(void)(vec1hi)
#define yuv422p_u_primehi65				(void)(vec1hi)
#define yuv444p_u_primehi65				vec1hi = vmulq_s16(vecUhi, vec65)
#define nv12_u_primehi65				(void)(vec1hi)
#define nv21_u_primehi65				(void)(vec1hi)

#define yuv420p_v_primehi51				(void)(vec0hi)
#define yuv422p_v_primehi51				(void)(vec0hi)
#define yuv444p_v_primehi51				vec0hi = vmulq_s16(vecVhi, vec51)
#define nv12_v_primehi51				(void)(vec0hi)
#define nv21_v_primehi51				(void)(vec0hi)

#define yuv420p_final_vec(vec)			vec2 = vzipq_s16(vec##lo, vec##lo)
#define yuv422p_final_vec(vec)			vec2 = vzipq_s16(vec##lo, vec##lo)
#define yuv444p_final_vec(vec)			vec2.val[0] = vec##lo; vec2.val[1] = vec##hi
#define nv12_final_vec(vec)				vec2 = vzipq_s16(vec##lo, vec##lo)
#define nv21_final_vec(vec)				vec2 = vzipq_s16(vec##lo, vec##lo)

#define yuv420p_g_high					(void)(vec0hi)
#define yuv422p_g_high					(void)(vec0hi)
#define yuv444p_g_high					vec0hi = vmulq_s16(vecUhi, vec13); vec0hi = vmlaq_s16(vec0hi, vecVhi, vec26)
#define nv12_g_high						(void)(vec0hi)
#define nv21_g_high						(void)(vec0hi)

#define CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(nameYuv, nameRgbx) { \
	COMPV_DEBUG_INFO_CHECK_NEON(); \
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Please use ASM code which is faaaster"); \
	 \
	compv_uscalar_t i, j, k, l; \
	const compv_uscalar_t strideUV = nameYuv##_uv_stride;  \
	const compv_uscalar_t strideRGBx = (stride * nameRgbx##_bytes_per_sample); \
	int16x8_t vecYlo, vecYhi, vecUlo, vecUhi, vecVlo, vecVhi, vec0lo, vec0hi, vec1lo, vec1hi; \
	nameRgbx##_declVecResult; \
	uint8x8_t vecUn, vecVn; \
	int16x8x2_t vec2; \
	uint8x8x2_t vecUV; (void)(vecUV); \
	 \
	__compv_builtin_prefetch_read(&yPtr[COMPV_CACHE1_LINE_SIZE * 0]); \
	__compv_builtin_prefetch_read(&yPtr[COMPV_CACHE1_LINE_SIZE * 1]); \
	__compv_builtin_prefetch_read(&yPtr[COMPV_CACHE1_LINE_SIZE * 2]); \
	nameYuv##_uv_prefetch_read(COMPV_CACHE1_LINE_SIZE * 0); \
	nameYuv##_uv_prefetch_read(COMPV_CACHE1_LINE_SIZE * 1); \
	nameYuv##_uv_prefetch_read(COMPV_CACHE1_LINE_SIZE * 2); \
	 \
	for (j = 0; j < height; ++j) { \
		for (i = 0, k = 0, l = 0; i < width; i += 16, k += nameRgbx##_step, l += nameYuv##_uv_step) { \
			/* Load samples */ \
			__compv_builtin_prefetch_read(&yPtr[i + (COMPV_CACHE1_LINE_SIZE * 3)]); \
			nameYuv##_uv_prefetch_read(l + (COMPV_CACHE1_LINE_SIZE * 3)); \
			vecYlo = vld1q_u8(&yPtr[i]); /* #16 Y samples */ \
			nameYuv##_u_load; /* #8 or #16 U samples, low mem */ \
			nameYuv##_v_load; /* #8 or #16 V samples, low mem */ \
			 \
			/* Compute Y', U', V': substract and convert to I16 */ \
			vecYhi = vsubl_u8(vget_high_u8(vecYlo), vec16n); \
			vecYlo = vsubl_u8(vget_low_u8(vecYlo), vec16n); \
			nameYuv##_u_primehi; \
			nameYuv##_u_primelo; \
			nameYuv##_v_primehi; \
			nameYuv##_v_primelo; \
			 \
			/* Compute (37Y'), (51V') and (65U') */ \
			vecYlo = vmulq_s16(vecYlo, vec37); \
			vecYhi = vmulq_s16(vecYhi, vec37); \
			vec0lo = vmulq_s16(vecVlo, vec51); \
			nameYuv##_v_primehi51; \
			vec1lo = vmulq_s16(vecUlo, vec65); \
			nameYuv##_u_primehi65; \
			 \
			/* Compute R = (37Y' + 0U' + 51V') >> 5 */ \
			nameYuv##_final_vec(vec0); \
			vecResult.val[0] = vcombine_u8( \
				vqshrun_n_s16(vaddq_s16(vecYlo, vec2.val[0]), 5), \
				vqshrun_n_s16(vaddq_s16(vecYhi, vec2.val[1]), 5) \
			); \
			 \
			/* B = (37Y' + 65U' + 0V') >> 5 */ \
			nameYuv##_final_vec(vec1); \
			vecResult.val[2] = vcombine_u8( /*/!\\ Notice the indice: #2 (B) */ \
				vqshrun_n_s16(vaddq_s16(vecYlo, vec2.val[0]), 5), \
				vqshrun_n_s16(vaddq_s16(vecYhi, vec2.val[1]), 5) \
			); \
			 \
			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */ \
			vec0lo = vmulq_s16(vecUlo, vec13); \
			vec0lo = vmlaq_s16(vec0lo, vecVlo, vec26); \
			nameYuv##_g_high; \
			nameYuv##_final_vec(vec0); \
			vecResult.val[1] = vcombine_u8( \
				vqshrun_n_s16(vsubq_s16(vecYlo, vec2.val[0]), 5), \
				vqshrun_n_s16(vsubq_s16(vecYhi, vec2.val[1]), 5) \
			); \
			 \
			/* Store result */ \
			nameRgbx##_store(&rgbxPtr[k], vecResult); \
			 \
		} /* End_Of for (i = 0; i < width; i += 16) */ \
		yPtr += stride; \
		rgbxPtr += strideRGBx; \
		nameYuv##_uv_inc_check { \
			nameYuv##_uv_inc; \
		} \
	} /* End_Of for (j = 0; j < height; ++j) */ \
}

// TODO(dmi): Optiz issues. ASM code is by far faster (all cases):
// - ARM32 Galaxy Tab A6/1 thread/1k loop: 2037.ms vs 3182.ms
// - ARM32 MediaPad2/1 thread/1k loop: 1749.ms vs 2606.ms
// - ARM32 iPhone5/1 thread/1k loop: 1960.ms vs 3354.ms
// - ARM64 iPad Air2/1 thread/10k loop: 6157.ms vs 7438.ms
// - ARM64 MediaPad2/1 thread/1k loop: 1590.ms vs 2409.ms
void CompVImageConvYuv420p_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(yuv420p, rgb24);
}

void CompVImageConvYuv420p_to_Rgba32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(yuv420p, rgba32);
}

void CompVImageConvYuv422p_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(yuv422p, rgb24);
}

void CompVImageConvYuv422p_to_Rgba32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(yuv422p, rgba32);
}

void CompVImageConvYuv444p_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(yuv444p, rgb24);
}

void CompVImageConvYuv444p_to_Rgba32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uPtr, COMPV_ALIGNED(NEON) const uint8_t* vPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(yuv444p, rgba32);
}

void CompVImageConvNv12_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(nv12, rgb24);
}

void CompVImageConvNv12_to_Rgba32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(nv12, rgba32);
}

void CompVImageConvNv21_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(nv21, rgb24);
}

void CompVImageConvNv21_to_Rgba32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yPtr, COMPV_ALIGNED(NEON) const uint8_t* uvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPlanar_to_Rgbx_Intrin_NEON(nv21, rgba32);
}

#define yuyv422_yi		0
#define uyvy422_yi		1

#define yuyv422_uvi		1
#define uyvy422_uvi		0

#define CompVImageConvYuvPacked_to_Rgbx_Intrin_NEON(nameYuv, nameRgbx) { \
	COMPV_DEBUG_INFO_CHECK_NEON(); \
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Please use ASM code which is faaaster"); \
	 \
	compv_uscalar_t i, j, k; \
	const compv_uscalar_t strideRGBx = (stride * nameRgbx##_bytes_per_sample); \
	int16x8_t vecYlow, vecYhigh, vecU, vecV, vec0, vec1; \
	nameRgbx##_declVecResult; \
	uint8x8_t vecUn, vecVn; \
	int16x8x2_t vec2; \
	uint8x16x2_t vec3; \
	uint8x8x2_t vec4; \
	 \
	stride <<= 1; \
	width <<= 1; \
	 \
	__compv_builtin_prefetch_read(&yuvPtr[COMPV_CACHE1_LINE_SIZE * 0]); \
	__compv_builtin_prefetch_read(&yuvPtr[COMPV_CACHE1_LINE_SIZE * 1]); \
	__compv_builtin_prefetch_read(&yuvPtr[COMPV_CACHE1_LINE_SIZE * 2]); \
	 \
	for (j = 0; j < height; ++j) { \
		for (i = 0, k = 0; i < width; i += 32, k += nameRgbx##_step) { \
			/* Load samples */ \
			__compv_builtin_prefetch_read(&yuvPtr[i + (COMPV_CACHE1_LINE_SIZE * 3)]); \
			vec3 = vld2q_u8(&yuvPtr[i]); \
			vec4 = vuzp_u8(vget_low_u8(vec3.val[nameYuv##_uvi]), vget_high_u8(vec3.val[nameYuv##_uvi])); \
			vecYlow = vec3.val[nameYuv##_yi]; /* #16 Y samples */ \
			vecUn = vec4.val[0]; /* #8 U samples, low mem */ \
			vecVn = vec4.val[1]; /* #8 V samples, low mem */ \
			 \
			/* Compute Y', U', V': substract and convert to I16 */ \
			vecYhigh = vsubl_u8(vget_high_u8(vecYlow), vec16n); \
			vecYlow = vsubl_u8(vget_low_u8(vecYlow), vec16n); \
			vecU = vsubl_u8(vecUn, vec127n); \
			vecV = vsubl_u8(vecVn, vec127n); \
			 \
			/* Compute (37Y') */ \
			vecYlow = vmulq_s16(vecYlow, vec37); \
			vecYhigh = vmulq_s16(vecYhigh, vec37); \
			 \
			/* Compute R = (37Y' + 0U' + 51V') >> 5, Instead of (#2 'vmlaq_s16' + #2 'vzipq_s16') use (#1 'vmulq_s16' + #1 'vzipq_s16' and #2 'vaddq_s16') */ \
			vec0 = vmulq_s16(vecV, vec51); \
			vec2 = vzipq_s16(vec0, vec0); /* UV sampled 1/2 -> duplicate to have same size as Y*/ \
			vecResult.val[0] = vcombine_u8( \
				vqshrun_n_s16(vaddq_s16(vecYlow, vec2.val[0]), 5), \
				vqshrun_n_s16(vaddq_s16(vecYhigh, vec2.val[1]), 5) \
			); \
			 \
			/* B = (37Y' + 65U' + 0V') >> 5, Instead of (#2 'vmlaq_s16' + #2 'vzipq_s16') use (#1 'vmulq_s16' + #1 'vzipq_s16' and #2 'vaddq_s16') */ \
			vec1 = vmulq_s16(vecU, vec65); \
			vec2 = vzipq_s16(vec1, vec1); /* UV sampled 1/2 -> duplicate to have same size as Y */ \
			vecResult.val[2] = vcombine_u8( /*/!\\ Notice the indice: #2 (B) */ \
				vqshrun_n_s16(vaddq_s16(vecYlow, vec2.val[0]), 5), \
				vqshrun_n_s16(vaddq_s16(vecYhigh, vec2.val[1]), 5) \
			); \
			 \
			/* Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 */ \
			vec0 = vmulq_s16(vecU, vec13); \
			vec1 = vmlaq_s16(vec0, vecV, vec26); \
			vec2 = vzipq_s16(vec1, vec1); \
			vecResult.val[1] = vcombine_u8( \
				vqshrun_n_s16(vsubq_s16(vecYlow, vec2.val[0]), 5), \
				vqshrun_n_s16(vsubq_s16(vecYhigh, vec2.val[1]), 5) \
			); \
			 \
			/* Store result */ \
			nameRgbx##_store(&rgbxPtr[k], vecResult); \
			 \
		} /* End_Of for (i = 0; i < width; i += 16) */ \
		yuvPtr += stride; \
		rgbxPtr += strideRGBx; \
	} /* End_Of for (j = 0; j < height; ++j) */ \
}

// TODO(dmi): Optiz issues. ASM code is by far faster (all cases):
// - ARM32 iPhone5/1 thread/1k loop: 2624.ms vs 4526.ms
void CompVImageConvYuyv422_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPacked_to_Rgbx_Intrin_NEON(yuyv422, rgb24);
}

void CompVImageConvYuyv422_to_Rgba32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPacked_to_Rgbx_Intrin_NEON(yuyv422, rgba32);
}

void CompVImageConvUyvy422_to_Rgb24_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPacked_to_Rgbx_Intrin_NEON(uyvy422, rgb24);
}

void CompVImageConvUyvy422_to_Rgba32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* yuvPtr, COMPV_ALIGNED(NEON) uint8_t* rgbxPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	CompVImageConvYuvPacked_to_Rgbx_Intrin_NEON(uyvy422, rgba32);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */
