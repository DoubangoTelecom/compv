/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#if !defined(_COMPV_INTRIN_AVX_H_)
#define _COMPV_INTRIN_AVX_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

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
Macro used to convert 3x32RGB to 4x32RGBA samples
*/
#define COMPV_3RGB_TO_4RGBA_AVX2(rgbaPtr_, rgbPtr_, ymm0_, ymm1_, ymmLost_, ymmMaskRgbToRgba_, ymmABCDDEFG_, ymmXXABBCDE_, ymmCDEFFGHX_) \
	/* TODO(dmi): */ \
	/* This section is marked as not optimized because VS2013 generate SSE code which produce AVX/SSE transition penalities issue. */ \
	/* Intrin code:*/ \
	/* 		_mm256_store_si256(&ymmLost_, _mm256_broadcastsi128_si256(_mm256_extractf128_si256(ymm1_, 1)));*/ \
	/* VS2013 ASM:*/ \
	/* 		vextractf128 xmm0,ymm0,1*/ \
	/* 		movdqa xmmword ptr [rbp+5A0h],xmm0*/ \
	/* 		movdqa xmm0,xmmword ptr [rbp+5A0h]*/ \
	/* 		movdqa xmmword ptr [rbp+9D0h],xmm0*/ \
	/* 		lea rax,[rbp+9D0h]*/ \
	/* 		vbroadcasti128 ymm0,oword ptr [rax]*/ \
	/* DOUBANGO ASM (file: compv_imageconv_macros_avx.s):*/ \
	/* 		vextractf128 [%3 + 0], ymm6, 0x1*/ \
	/* 		vbroadcasti128 ymm5, [%3 + 0] */ \
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); \
	/************ Line-0 ***********/ \
	_mm256_store_si256(&ymm0_, _mm256_load_si256((__m256i*)(rgbPtr + 0))); /* load first 32 samples */  \
	_mm256_store_si256(&ymm1_, _mm256_permutevar8x32_epi32(ymm0_, ymmABCDDEFG_)); /* move the last 4bytes in the first 128-lane to the second 128-lane */ \
	_mm256_store_si256(&((__m256i*)rgbaPtr_)[0], _mm256_shuffle_epi8(ymm1, ymmMaskRgbToRgba_)); /* RGB -> RGBA */ \
	/************ Line-1 ***********/ \
	_mm256_store_si256(&ymm1_, _mm256_load_si256((__m256i*)(rgbPtr + 32))); /* load next 32 samples */ \
	_mm256_store_si256(&ymm0_, _mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 3, 3, 3))); /* duplicate lost0 */ \
	_mm256_store_si256(&ymmLost_, _mm256_broadcastsi128_si256(_mm256_extractf128_si256(ymm1_, 1))); /* high-128 = low-lost = lost0 || lost1 */ \
	_mm256_store_si256(&ymm1_, _mm256_permutevar8x32_epi32(ymm1_, ymmXXABBCDE_)); \
	_mm256_store_si256(&ymm1_, _mm256_blend_epi32(ymm1_, ymm0_, 0x03)); /* ymm0(64bits)||ymm1(192bits) */ \
	_mm256_store_si256(&((__m256i*)rgbaPtr_)[1], _mm256_shuffle_epi8(ymm1_, ymmMaskRgbToRgba_)); /* RGB -> RGBA	*/ \
	/************ Line-2 ***********/ \
	_mm256_store_si256(&ymm0_, _mm256_load_si256((__m256i*)(rgbPtr + 64))); /* load next 32 samples */ \
	_mm256_store_si256(&ymm1_, _mm256_permutevar8x32_epi32(ymm0_, ymmCDEFFGHX_)); /* lost0 || lost1 || lost2 || garbage */ \
	_mm256_store_si256(&ymmLost_, _mm256_inserti128_si256(ymmLost_, _mm256_extractf128_si256(ymm0_, 0), 1)); /* lost0 || lost1 || 0 || 1 */ \
	_mm256_store_si256(&ymm0_, _mm256_permutevar8x32_epi32(ymmLost_, ymmABCDDEFG_)); \
	_mm256_store_si256(&((__m256i*)rgbaPtr_)[2], _mm256_shuffle_epi8(ymm0_, ymmMaskRgbToRgba_)); /* RGB -> RGBA */ \
	/************ Line-3 ***********/ \
	_mm256_store_si256(&((__m256i*)rgbaPtr_)[3], _mm256_shuffle_epi8(ymm1_, ymmMaskRgbToRgba_)); \
 

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

#endif /* _COMPV_INTRIN_AVX_H_ */
