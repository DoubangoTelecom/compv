/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_simd_globals.h"
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/math/compv_math.h"

#if COMPV_ARCH_X86

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k0_8u[] = { //!\\ You should use 'pxor' instruction which is faster than movdqa to load zeros in xmm register 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k_0_0_0_255_8u[] = {
    0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
    0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k1_8s[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k2_8s[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k3_8s[] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k5_8s[] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k15_8s[] = {
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k16_8s[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k85_8s[] = {
	85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
	85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k127_8s[] = {
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k128_8u[] = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k171_8u[] = {
	171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171,
	171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k254_8u[] = { // 254 = FE = 11111110, not(254) = 00000001 -> useful to select first or last bit, there is no shift_epi8(7) in SSE
    254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
    254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k255_8u[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k13_26_16s[] = {
	13, 26, 13, 26, 13, 26, 13, 26,
	13, 26, 13, 26, 13, 26, 13, 26,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k16_16s[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 
	16, 16, 16, 16, 16, 16, 16, 16,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k37_16s[] = {
	37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k51_16s[] = {
	51, 51, 51, 51, 51, 51, 51, 51,
	51, 51, 51, 51, 51, 51, 51, 51,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k65_16s[] = {
	65, 65, 65, 65, 65, 65, 65, 65,
	65, 65, 65, 65, 65, 65, 65, 65,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k127_16s[] = {
	127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127,
};


COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k128_16s[] = {
    128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k255_16s[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k7120_16s[] = {
    7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120,
    7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k8912_16s[] = {
    8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912,
    8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k4400_16s[] = {
    4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400,
    4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float32_t k43_32f[] = {
	43.f, 43.f, 43.f, 43.f,
	43.f, 43.f, 43.f, 43.f,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float32_t k255_32f[] = {
	255.f, 255.f, 255.f, 255.f,
	255.f, 255.f, 255.f, 255.f,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t ksqrt2_64f[] = {
    COMPV_MATH_SQRT_2, COMPV_MATH_SQRT_2, // SSE
    COMPV_MATH_SQRT_2, COMPV_MATH_SQRT_2 // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t km1_64f[] = {
	-1., -1., // SSE
	-1., -1., // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t km1_0_64f[] = {
	-1., 0., // SSE
	-1., 0. // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t k1_64f[] = {
	1., 1., // SSE
	1., 1., // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskstore_0_64u[] = { // use with _mm256_maskload
    0xF000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskstore_0_1_64u[] = { // use with _mm256_maskload
    0xF000000000000000, 0xF000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint32_t kAVXMaskstore_0_32u[] = { // use with _mm256_maskload
    0xF0000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskzero_3_64u[] = { // mask to zero the last 64bits - use with _mm256_and
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskzero_2_3_64u[] = { // mask to zero the last two 64bits (128) - use with _mm256_and
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskzero_1_2_3_64u[] = { // mask to zero the last three 64bits (192) - use with _mm256_and
    0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_AEBFCGDH_32s[] = { // = k_0_4_1_5_2_6_3_7
    COMPV_AVX_A64, COMPV_AVX_E64, COMPV_AVX_B64, COMPV_AVX_F64, COMPV_AVX_C64, COMPV_AVX_G64, COMPV_AVX_D64, COMPV_AVX_H64
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_ABCDDEFG_32s[] = { // = k_0_1_2_3_3_4_5_6
    COMPV_AVX_A64, COMPV_AVX_B64, COMPV_AVX_C64, COMPV_AVX_D64, COMPV_AVX_D64, COMPV_AVX_E64, COMPV_AVX_F64, COMPV_AVX_G64
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_CDEFFGHX_32s[] = { // = k_2_3_4_5_5_6_7_X
    COMPV_AVX_C64, COMPV_AVX_D64, COMPV_AVX_E64, COMPV_AVX_F64, COMPV_AVX_F64, COMPV_AVX_G64, COMPV_AVX_H64, 0xFF
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_XXABBCDE_32s[] = { // = k_X_X_0_1_1_2_3_4
    0xFF, 0xFF, COMPV_AVX_A64, COMPV_AVX_B64, COMPV_AVX_B64, COMPV_AVX_C64, COMPV_AVX_D64, COMPV_AVX_E64
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint32_t kAVXFloat64MaskAbs[] = { // Mask used for operations to compute the absolute value -> and(xmm, mask)
    0xffffffff, 0x7fffffff, 0xffffffff, 0x7fffffff, // SSE
    0xffffffff, 0x7fffffff, 0xffffffff, 0x7fffffff // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint32_t kAVXFloat64MaskNegate[] = { // Mask used for operations to negate values -> xor(xmm, mask)
    0x00000000, 0x80000000, 0x00000000, 0x80000000, // SSE
    0x00000000, 0x80000000, 0x00000000, 0x80000000 // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Popcnt_32s[] = { // To be used with _mm_shuffle_epi8 for popcnt computation
	COMPV_MM_SHUFFLE_EPI8(2, 1, 1, 0), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(4, 3, 3, 2), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(2, 1, 1, 0), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(4, 3, 3, 2), // 256bits AVX register
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Deinterleave8uL2_32s[] = { // To be used with _mm_shuffle_epi8, use vld2.u8/vst2.u8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(6, 4, 2, 0), COMPV_MM_SHUFFLE_EPI8(14, 12, 10, 8), COMPV_MM_SHUFFLE_EPI8(7, 5, 3, 1), COMPV_MM_SHUFFLE_EPI8(15, 13, 11, 9), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(6, 4, 2, 0), COMPV_MM_SHUFFLE_EPI8(14, 12, 10, 8), COMPV_MM_SHUFFLE_EPI8(7, 5, 3, 1), COMPV_MM_SHUFFLE_EPI8(15, 13, 11, 9), // 256bits AVX register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Deinterleave16uL2_32s[] = { // To be used with _mm_shuffle_epi8, use vld2.u16/vst2.u16 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(5, 4, 1, 0), COMPV_MM_SHUFFLE_EPI8(13, 12, 9, 8), COMPV_MM_SHUFFLE_EPI8(7, 6, 3, 2), COMPV_MM_SHUFFLE_EPI8(15, 14, 11, 10), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(5, 4, 1, 0), COMPV_MM_SHUFFLE_EPI8(13, 12, 9, 8), COMPV_MM_SHUFFLE_EPI8(7, 6, 3, 2), COMPV_MM_SHUFFLE_EPI8(15, 14, 11, 10), // 256bits AVX register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Deinterleave8uL3_32s[] = { // To be used with _mm_shuffle_epi8, use vld3.u8/vst3.u8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(9, 6, 3, 0), COMPV_MM_SHUFFLE_EPI8(4, 1, 15, 12), COMPV_MM_SHUFFLE_EPI8(2, 13, 10, 7), COMPV_MM_SHUFFLE_EPI8(14, 11, 8, 5), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(9, 6, 3, 0), COMPV_MM_SHUFFLE_EPI8(4, 1, 15, 12), COMPV_MM_SHUFFLE_EPI8(2, 13, 10, 7), COMPV_MM_SHUFFLE_EPI8(14, 11, 8, 5), // 256bits AVX register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Interleave8uL3_Step0_s32[] = { // To be used with _mm_shuffle_epi8, use vld3.u8/vst3.u8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(2, 11, 1, 0), COMPV_MM_SHUFFLE_EPI8(5, 4, 12, 3), COMPV_MM_SHUFFLE_EPI8(14, 7, 6, 13), COMPV_MM_SHUFFLE_EPI8(10, 15, 9, 8), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(2, 11, 1, 0), COMPV_MM_SHUFFLE_EPI8(5, 4, 12, 3), COMPV_MM_SHUFFLE_EPI8(14, 7, 6, 13), COMPV_MM_SHUFFLE_EPI8(10, 15, 9, 8), // 256bits AVX register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Interleave8uL3_Step1_s32[] = { // To be used with _mm_shuffle_epi8, use vld3.u8/vst3.u8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(2, 1, 11, 0), COMPV_MM_SHUFFLE_EPI8(13, 4, 3, 12), COMPV_MM_SHUFFLE_EPI8(7, 14, 6, 5), COMPV_MM_SHUFFLE_EPI8(10, 9, 15, 8), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(2, 1, 11, 0), COMPV_MM_SHUFFLE_EPI8(13, 4, 3, 12), COMPV_MM_SHUFFLE_EPI8(7, 14, 6, 5), COMPV_MM_SHUFFLE_EPI8(10, 9, 15, 8), // 256bits AVX register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Interleave8uL3_Step2_s32[] = { // To be used with _mm_shuffle_epi8, use vld3.u8/vst3.u8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(11, 1, 0, 10), COMPV_MM_SHUFFLE_EPI8(4, 12, 3, 2), COMPV_MM_SHUFFLE_EPI8(7, 6, 13, 5), COMPV_MM_SHUFFLE_EPI8(15, 9, 8, 14), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(11, 1, 0, 10), COMPV_MM_SHUFFLE_EPI8(4, 12, 3, 2), COMPV_MM_SHUFFLE_EPI8(7, 6, 13, 5), COMPV_MM_SHUFFLE_EPI8(15, 9, 8, 14), // 256bits AVX register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Deinterleave8uL4_32s[] = { // To be used with _mm_shuffle_epi8, use vld4.u8/vst4.u8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(14, 10, 12, 8), COMPV_MM_SHUFFLE_EPI8(15, 11, 13, 9), COMPV_MM_SHUFFLE_EPI8(6, 2, 4, 0), COMPV_MM_SHUFFLE_EPI8(7, 3, 5, 1), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(14, 10, 12, 8), COMPV_MM_SHUFFLE_EPI8(15, 11, 13, 9), COMPV_MM_SHUFFLE_EPI8(6, 2, 4, 0), COMPV_MM_SHUFFLE_EPI8(7, 3, 5, 1), // 256bits AVX register
};

COMPV_EXTERNC COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_16s7_32s[] = { // To be used with _mm_shuffle_epi8 to duplicate the last (7th) element in epi16, use vdup.u16 q0, d1[] for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(15, 14, 15, 14), COMPV_MM_SHUFFLE_EPI8(15, 14, 15, 14), COMPV_MM_SHUFFLE_EPI8(15, 14, 15, 14), COMPV_MM_SHUFFLE_EPI8(15, 14, 15, 14)
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL0_8u_32s[] { // To be used with _mm_shuffle_epi8, use vdup.u8[0]/vshl.u8#0 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), COMPV_MM_SHUFFLE_EPI8(0, 0, 0, 0), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL1_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[1]/vshl.u8#1 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 0x80), COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 1), COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 1), COMPV_MM_SHUFFLE_EPI8(1, 1, 1, 1), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL2_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[2]/vshl.u8#2 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(2, 2, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(2, 2, 2, 2), COMPV_MM_SHUFFLE_EPI8(2, 2, 2, 2), COMPV_MM_SHUFFLE_EPI8(2, 2, 2, 2), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL3_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[3]/vshl.u8#3 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(3, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(3, 3, 3, 3), COMPV_MM_SHUFFLE_EPI8(3, 3, 3, 3), COMPV_MM_SHUFFLE_EPI8(3, 3, 3, 3), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL4_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[4]/vshl.u8#4 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(4, 4, 4, 4), COMPV_MM_SHUFFLE_EPI8(4, 4, 4, 4), COMPV_MM_SHUFFLE_EPI8(4, 4, 4, 4), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL5_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[5]/vshl.u8#5 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(5, 5, 5, 0x80), COMPV_MM_SHUFFLE_EPI8(5, 5, 5, 5), COMPV_MM_SHUFFLE_EPI8(5, 5, 5, 5), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL6_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[6]/vshl.u8#6 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(6, 6, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(6, 6, 6, 6), COMPV_MM_SHUFFLE_EPI8(6, 6, 6, 6), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL7_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[7]/vshl.u8#7 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(7, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(7, 7, 7, 7), COMPV_MM_SHUFFLE_EPI8(7, 7, 7, 7), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL8_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[8]/vshl.u8#8 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(8, 8, 8, 8), COMPV_MM_SHUFFLE_EPI8(8, 8, 8, 8), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL9_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[9]/vshl.u8#9 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(9, 9, 9, 0x80), COMPV_MM_SHUFFLE_EPI8(9, 9, 9, 9), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL10_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[10]/vshl.u8#10 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(10, 10, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(10, 10, 10, 10), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL11_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[11]/vshl.u8#11 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(11, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(11, 11, 11, 11), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL12_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[12]/vshl.u8#12 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(12, 12, 12, 12), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL13_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[13]/vshl.u8#13 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(13, 13, 13, 0x80), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL14_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[14]/vshl.u8#14 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(14, 14, 0x80, 0x80), // 128bits SSE register
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_DUP_SHL15_8u_32s[] = { // To be used with _mm_shuffle_epi8, use vdup.u8[15]/vshl.u8#15 for ARM NEON
	COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(0x80, 0x80, 0x80, 0x80), COMPV_MM_SHUFFLE_EPI8(15, 0x80, 0x80, 0x80), // 128bits SSE register
};

#endif /* COMPV_ARCH_X86 */