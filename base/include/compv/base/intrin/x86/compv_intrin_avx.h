/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_INTRIN_AVX_H_)
#define _COMPV_BASE_INTRIN_AVX_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

/*
AVX uses #2 128bits SSE registers to emulate a 256bits register.
Let's say we have #2 SSE registers XMM-low=m1m2 and XMM-high=m3m4, then YMM=m1m2m3m4.
avx_shuffle_lo(YMM, YMM)=shuffle(m1,m3)||shuffle(m1,m3) not shuffle(m1,m2)||shuffle(m1,m2). AVX is performing #2 sse_shuffle(XMM-low, XMM-low).
The next macros change the behavior to consider the AVX registers as a single 256bits
/!\ These macros are time-consuming and should be used carrefully.
*/
#define compv_avx2_unpacklo_epi8(ymm0_, ymm1_)		_mm256_unpacklo_epi8(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define compv_avx2_unpackhi_epi8(ymm0_, ymm1_)		_mm256_unpackhi_epi8(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define compv_avx2_hadd_epi16(ymm0_, ymm1_)			_mm256_permute4x64_epi64(_mm256_hadd_epi16(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_hadd_epi32(ymm0_, ymm1_)			_mm256_permute4x64_epi64(_mm256_hadd_epi32(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_hadd_pd(ymm0_, ymm1_)			_mm256_permute4x64_pd(_mm256_hadd_pd(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_packs_epi32(ymm0_, ymm1_)		_mm256_permute4x64_epi64(_mm256_packs_epi32(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_packs_epi32(ymm0_, ymm1_)		_mm256_permute4x64_epi64(_mm256_packs_epi32(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_packus_epi16(ymm0_, ymm1_)		_mm256_permute4x64_epi64(_mm256_packus_epi16(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_packs_epi16(ymm0_, ymm1_)		_mm256_permute4x64_epi64(_mm256_packs_epi16(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_unpacklo_epi16(ymm0_, ymm1_)		_mm256_unpacklo_epi16(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define compv_avx2_unpackhi_epi16(ymm0_, ymm1_)		_mm256_unpackhi_epi16(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))

/*
Index for the 64bit packed values
*/
#define COMPV_AVX_A64 0
#define COMPV_AVX_B64 1
#define COMPV_AVX_C64 2
#define COMPV_AVX_D64 3
#define COMPV_AVX_E64 4
#define COMPV_AVX_F64 5
#define COMPV_AVX_G64 6
#define COMPV_AVX_H64 7

/*
Macro used to convert 32 RGB to 32 RGBA samples
32 RGB samples requires 96 Bytes(3 YMM registers), will be converted to 32 RGBA samples
requiring 128 Bytes (4 YMM registers)
The aplha channel will contain garbage instead of 0xff because this macro is used to fetch samples in place
*/
#define COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgbPtr_, ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_, ymmABCDDEFG_, ymmCDEFFGHX_, ymmXXABBCDE_, ymmMaskRgbToRgba_) \
	_mm256_store_si256(&ymmRGBA3_, _mm256_load_si256(reinterpret_cast<const __m256i*>(rgbPtr_ + 32))); \
	_mm256_store_si256(&ymmRGBA1_, _mm256_permute2x128_si256(ymmRGBA3_, ymmRGBA3_, 0x11)); \
	_mm256_store_si256(&ymmRGBA3_, _mm256_permutevar8x32_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgbPtr_ + 64)), ymmCDEFFGHX_)); \
	_mm256_store_si256(&ymmRGBA1_, _mm256_permute2x128_si256(ymmRGBA1_, _mm256_load_si256(reinterpret_cast<const __m256i*>(rgbPtr_ + 64)), 0x20)); \
	_mm256_store_si256(&ymmRGBA2_, _mm256_permutevar8x32_epi32(ymmRGBA1_, ymmABCDDEFG_)); \
	_mm256_store_si256(&ymmRGBA2_, _mm256_shuffle_epi8(ymmRGBA2_, ymmMaskRgbToRgba_)); \
	_mm256_store_si256(&ymmRGBA3_, _mm256_shuffle_epi8(ymmRGBA3_, ymmMaskRgbToRgba_)); \
	_mm256_store_si256(&ymmRGBA0_, _mm256_permute4x64_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgbPtr_ + 0)), 0xff)); \
	_mm256_store_si256(&ymmRGBA1_, _mm256_permutevar8x32_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgbPtr_ + 32)), ymmXXABBCDE_)); \
	_mm256_store_si256(&ymmRGBA1_, _mm256_blend_epi32(ymmRGBA1_, ymmRGBA0_, 0x03)); \
	_mm256_store_si256(&ymmRGBA1_, _mm256_shuffle_epi8(ymmRGBA1_, ymmMaskRgbToRgba_)); \
	_mm256_store_si256(&ymmRGBA0_, _mm256_permutevar8x32_epi32(_mm256_load_si256(reinterpret_cast<const __m256i*>(rgbPtr_ + 0)), ymmABCDDEFG_)); \
	_mm256_store_si256(&ymmRGBA0_, _mm256_shuffle_epi8(ymmRGBA0_, ymmMaskRgbToRgba_));
// Next version not optimized as we load the masks for each call, use above version and load masks once
#define COMPV_32xRGB_TO_32xRGBA_AVX2_SLOW(rgbPtr_, ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_) \
	COMPV_32xRGB_TO_32xRGBA_AVX2_FAST(rgbPtr_, ymmRGBA0_, ymmRGBA1_, ymmRGBA2_, ymmRGBA3_, \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_ABCDDEFG_i32)), \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_CDEFFGHX_i32)), \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kAVXPermutevar8x32_XXABBCDE_i32)), \
		_mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_RgbToRgba_i32)) \
	)


 

/*
Interleaves two 256bits vectors without crossing the 128-lane.
From:
0 0 0 0 0 0 . . . .
1 1 1 1 1 1 . . . .
To:
0 1 0 1 0 1 . . . .
*/
#define COMPV_INTERLEAVE_I8_AVX2(_m0, _m1, _tmp) \
	_mm256_store_si256(&_tmp, _mm256_unpackhi_epi8(_m0, _m1)); \
	_mm256_store_si256(&_m0, _mm256_unpacklo_epi8(_m0, _m1)); \
	_mm256_store_si256(&_m1, _tmp);

/*
Transpose a 4x32 matrix containing u8/i8 values without crossing the 128-lane.
From:
0 0 0 0 . .
1 1 1 1 . .
2 2 2 2 . .
3 3 3 3 . .
To:
0 1 2 3 . .
0 1 2 3 . .
0 1 2 3 . .
*/
#define COMPV_TRANSPOSE_I8_4X32_AVX2(_x0, _x1, _x2, _x3, _tmp) \
	COMPV_INTERLEAVE_I8_AVX2(_x0, _x2, _tmp) \
	COMPV_INTERLEAVE_I8_AVX2(_x1, _x3, _tmp) \
	COMPV_INTERLEAVE_I8_AVX2(_x0, _x1, _tmp) \
	COMPV_INTERLEAVE_I8_AVX2(_x2, _x3, _tmp)

/*
Transpose a 16x32 matrix containing u8/i8 values without crossing the 128-lane.
From:
0 0 0 0 . .
1 1 1 1 . .
2 2 2 2 . .
3 3 3 3 . .
To:
0 1 2 3 . .
0 1 2 3 . .
0 1 2 3 . .
*/
#define COMPV_TRANSPOSE_I8_16X32_AVX2(_x0, _x1, _x2, _x3, _x4, _x5, _x6, _x7, _x8, _x9, _x10, _x11, _x12, _x13, _x14, _x15, _tmp) \
	/* 1 * 5 * 9 * d */ \
	COMPV_TRANSPOSE_I8_4X32_AVX2(_x1, _x5, _x9, _x13, _tmp); \
	/* 3 * 7 * b * f */ \
	COMPV_TRANSPOSE_I8_4X32_AVX2(_x3, _x7, _x11, _x15, _tmp); \
	/* 0 * 4 * 8 * c */ \
	COMPV_TRANSPOSE_I8_4X32_AVX2(_x0, _x4, _x8, _x12, _tmp); \
	/* 2 * 6 * a * e */ \
	COMPV_TRANSPOSE_I8_4X32_AVX2(_x2, _x6, _x10, _x14, _tmp); \
	/* 1 * 3 * 5 * 7 * 9 * b * d * f */ \
	COMPV_INTERLEAVE_I8_AVX2(_x1, _x3, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x5, _x7, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x9, _x11, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x13, _x15, _tmp); \
	/* 0 * 2 * 4 * 6 * 8 * a * c * e */ \
	COMPV_INTERLEAVE_I8_AVX2(_x0, _x2, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x4, _x6, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x8, _x10, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x12, _x14, _tmp); \
	/* 0 * 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * a * b * c * d * e * f */ \
	COMPV_INTERLEAVE_I8_AVX2(_x0, _x1, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x2, _x3, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x4, _x5, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x6, _x7, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x8, _x9, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x10, _x11, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x12, _x13, _tmp); \
	COMPV_INTERLEAVE_I8_AVX2(_x14, _x15, _tmp);


COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_INTRIN_AVX_H_ */
