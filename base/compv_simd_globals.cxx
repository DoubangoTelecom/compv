/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_simd_globals.h"
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/math/compv_math.h"

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k0_u8[] = { //!\\ You should use 'pxor' instruction which is faster than movdqa to load zeros in xmm register 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k_0_0_0_255_u8[] = {
    0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
    0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k1_i8[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t km1_f64[] = {
    -1., -1., // SSE
    -1., -1., // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t km1_0_f64[] = {
    -1., 0., // SSE
    -1., 0. // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t k1_f64[] = {
    1., 1., // SSE
    1., 1., // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k2_i8[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k3_i8[] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k5_i8[] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k15_i8[] = {
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k16_i8[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int8_t k127_i8[] = {
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k128_u8[] = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k254_u8[] = { // 254 = FE = 11111110, not(254) = 00000001 -> useful to select first or last bit, there is no shift_epi8(7) in SSE
    254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
    254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint8_t k255_u8[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k13_26_i16[] = {
	13, 26, 13, 26, 13, 26, 13, 26,
	13, 26, 13, 26, 13, 26, 13, 26,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k16_i16[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 
	16, 16, 16, 16, 16, 16, 16, 16,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k37_i16[] = {
	37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k51_i16[] = {
	51, 51, 51, 51, 51, 51, 51, 51,
	51, 51, 51, 51, 51, 51, 51, 51,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k65_i16[] = {
	65, 65, 65, 65, 65, 65, 65, 65,
	65, 65, 65, 65, 65, 65, 65, 65,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k127_i16[] = {
	127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127,
};


COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k128_i16[] = {
    128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128, 128, 128, 128, 128, 128, 128,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k255_i16[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k7120_i16[] = {
    7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120,
    7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k8912_i16[] = {
    8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912,
    8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int16_t k4400_i16[] = {
    4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400,
    4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() COMPV_NAMESPACE::compv_float64_t ksqrt2_f64[] = {
    COMPV_MATH_SQRT_2, COMPV_MATH_SQRT_2, // SSE
    COMPV_MATH_SQRT_2, COMPV_MATH_SQRT_2 // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskstore_0_u64[] = { // use with _mm256_maskload
    0xF000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskstore_0_1_u64[] = { // use with _mm256_maskload
    0xF000000000000000, 0xF000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint32_t kAVXMaskstore_0_u32[] = { // use with _mm256_maskload
    0xF0000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskzero_3_u64[] = { // mask to zero the last 64bits - use with _mm256_and
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskzero_2_3_u64[] = { // mask to zero the last two 64bits (128) - use with _mm256_and
    0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000,
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint64_t kAVXMaskzero_1_2_3_u64[] = { // mask to zero the last three 64bits (192) - use with _mm256_and
    0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
};

#if COMPV_ARCH_X86
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_AEBFCGDH_i32[] = { // = k_0_4_1_5_2_6_3_7
    COMPV_AVX_A64, COMPV_AVX_E64, COMPV_AVX_B64, COMPV_AVX_F64, COMPV_AVX_C64, COMPV_AVX_G64, COMPV_AVX_D64, COMPV_AVX_H64
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_ABCDDEFG_i32[] = { // = k_0_1_2_3_3_4_5_6
    COMPV_AVX_A64, COMPV_AVX_B64, COMPV_AVX_C64, COMPV_AVX_D64, COMPV_AVX_D64, COMPV_AVX_E64, COMPV_AVX_F64, COMPV_AVX_G64
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_CDEFFGHX_i32[] = { // = k_2_3_4_5_5_6_7_X
    COMPV_AVX_C64, COMPV_AVX_D64, COMPV_AVX_E64, COMPV_AVX_F64, COMPV_AVX_F64, COMPV_AVX_G64, COMPV_AVX_H64, 0xFF
};
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_XXABBCDE_i32[] = { // = k_X_X_0_1_1_2_3_4
    0xFF, 0xFF, COMPV_AVX_A64, COMPV_AVX_B64, COMPV_AVX_B64, COMPV_AVX_C64, COMPV_AVX_D64, COMPV_AVX_E64
};
#endif /* COMPV_ARCH_X86 */

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint32_t kAVXFloat64MaskAbs[] = { // Mask used for operations to compute the absolute value -> and(xmm, mask)
    0xffffffff, 0x7fffffff, 0xffffffff, 0x7fffffff, // SSE
    0xffffffff, 0x7fffffff, 0xffffffff, 0x7fffffff // AVX
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() uint32_t kAVXFloat64MaskNegate[] = { // Mask used for operations to negate values -> xor(xmm, mask)
    0x00000000, 0x80000000, 0x00000000, 0x80000000, // SSE
    0x00000000, 0x80000000, 0x00000000, 0x80000000 // AVX
};

// Deinterleaves bytes [a,b,a,b,b,a,b,a,b] to [a,a,a,a,a,b,b,b,b,b]
COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Deinterleave_i32[] = { // To be used with _mm_shuffle_epi8
	COMPV_MM_SHUFFLE_EPI8(6, 4, 2, 0), COMPV_MM_SHUFFLE_EPI8(14, 12, 10, 8), COMPV_MM_SHUFFLE_EPI8(7, 5, 3, 1), COMPV_MM_SHUFFLE_EPI8(15, 13, 11, 9), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(6, 4, 2, 0), COMPV_MM_SHUFFLE_EPI8(14, 12, 10, 8), COMPV_MM_SHUFFLE_EPI8(7, 5, 3, 1), COMPV_MM_SHUFFLE_EPI8(15, 13, 11, 9), // 256bits AVX register
};

COMPV_BASE_API COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Popcnt_i32[] = { // To be used with _mm_shuffle_epi8 for popcnt computation
	COMPV_MM_SHUFFLE_EPI8(2, 1, 1, 0), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(4, 3, 3, 2), // 128bits SSE register
	COMPV_MM_SHUFFLE_EPI8(2, 1, 1, 0), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(3, 2, 2, 1), COMPV_MM_SHUFFLE_EPI8(4, 3, 3, 2), // 256bits AVX register
};