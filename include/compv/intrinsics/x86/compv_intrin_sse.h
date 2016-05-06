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
#if !defined(_COMPV_INTRIN_SSE_H_)
#define _COMPV_INTRIN_SSE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

static COMPV_INLINE __m128i _mm_mullo_epi32_SSE2(const __m128i &a, const __m128i &b)
{
	__m128i x, y;
	_mm_store_si128(&x, _mm_mul_epu32(a, b));
	_mm_store_si128(&y, _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4)));
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(x, 0x8), _mm_shuffle_epi32(y, 0x8));
}

#define _mm_cvtepi16_epi32_low_SSE2(a) _mm_srai_epi32(_mm_unpacklo_epi16(a, a), 16)
#define _mm_cvtepi16_epi32_hi_SSE2(a) _mm_srai_epi32(_mm_unpackhi_epi16(a, a), 16)

/*
Macro used to convert 3x16RGB to 4x16RGBA samples
*/
#define COMPV_3RGB_TO_4RGBA_SSSE3(rgbaPtr_, rgbPtr_, xmm0_, xmm1_, xmmMaskRgbToRgba_) \
	/* RGBA0 = Convert(RGB0) -> 4RGBAs which means we used 4RGBs = 12bytes and lost 4bytes from RGB0 */ \
	/* RGBA1 = Convert(ALIGN(RGB0, RGB1, 12)) -> we used 4bytes from RGB0 and 8bytes from RGB1 = 12bytes RGB = 16bytes RGBA -> lost 12bytes from RGB1 */ \
	/* RGBA2 = Convert(ALIGN(RGB1, RGB2, 8)) -> we used 8bytes from RGB1 and 4bytes from RGB2 = 12bytes RGB = 16bytes RGBA -> lost 12bytes from RGB2 */ \
	/* RGBA3 = Convert(ALIGN(RGB2, RGB2, 4)) -> used 12bytes from RGB2 = 12bytes RGB = 16bytes RGBA */ \
	_mm_store_si128(&xmm0_, _mm_load_si128((__m128i*)rgbPtr_)); /* load first 16 samples */ \
	_mm_store_si128(&xmm1_, _mm_load_si128((__m128i*)(rgbPtr_ + 16))); /* load next 16 samples */ \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[0], _mm_shuffle_epi8(xmm0_, xmmMaskRgbToRgba_)); \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[1], _mm_shuffle_epi8(_mm_alignr_epi8(xmm1_, xmm0_, 12), xmmMaskRgbToRgba_)); \
	_mm_store_si128(&xmm0_, _mm_load_si128((__m128i*)(rgbPtr_ + 32))); /* load next 16 samples */ \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[2], _mm_shuffle_epi8(_mm_alignr_epi8(xmm0_, xmm1_, 8), xmmMaskRgbToRgba_)); \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[3], _mm_shuffle_epi8(_mm_alignr_epi8(xmm0_, xmm0_, 4), xmmMaskRgbToRgba_)); \
 
/*
Interleaves two 128bits vectors.
From:
0 0 0 0 0 0 . . . .
1 1 1 1 1 1 . . . .
To:
0 1 0 1 0 1 . . . .
*/
#define COMPV_INTERLEAVE_I8_SSE2(_m0, _m1, _tmp) \
	_mm_store_si128(&_tmp, _mm_unpackhi_epi8(_m0, _m1)); \
	_mm_store_si128(&_m0, _mm_unpacklo_epi8(_m0, _m1)); \
	_mm_store_si128(&_m1, _tmp);

/*
Transpose a 4x16 matrix containing u8/i8 values.
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
#define COMPV_TRANSPOSE_I8_4X16_SSE2(_x0, _x1, _x2, _x3, _tmp) \
	COMPV_INTERLEAVE_I8_SSE2(_x0, _x2, _tmp) \
	COMPV_INTERLEAVE_I8_SSE2(_x1, _x3, _tmp) \
	COMPV_INTERLEAVE_I8_SSE2(_x0, _x1, _tmp) \
	COMPV_INTERLEAVE_I8_SSE2(_x2, _x3, _tmp)


/*
Transpose a 16x16 matrix containing u8/i8 values.
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
#define COMPV_TRANSPOSE_I8_16X16_SSE2(_x0, _x1, _x2, _x3, _x4, _x5, _x6, _x7, _x8, _x9, _x10, _x11, _x12, _x13, _x14, _x15, _tmp) \
	/* 1 * 5 * 9 * d */ \
	COMPV_TRANSPOSE_I8_4X16_SSE2(_x1, _x5, _x9, _x13, _tmp); \
	/* 3 * 7 * b * f */ \
	COMPV_TRANSPOSE_I8_4X16_SSE2(_x3, _x7, _x11, _x15, _tmp); \
	/* 0 * 4 * 8 * c */ \
	COMPV_TRANSPOSE_I8_4X16_SSE2(_x0, _x4, _x8, _x12, _tmp); \
	/* 2 * 6 * a * e */ \
	COMPV_TRANSPOSE_I8_4X16_SSE2(_x2, _x6, _x10, _x14, _tmp); \
	/* 1 * 3 * 5 * 7 * 9 * b * d * f */ \
	COMPV_INTERLEAVE_I8_SSE2(_x1, _x3, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x5, _x7, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x9, _x11, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x13, _x15, _tmp); \
	/* 0 * 2 * 4 * 6 * 8 * a * c * e */ \
	COMPV_INTERLEAVE_I8_SSE2(_x0, _x2, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x4, _x6, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x8, _x10, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x12, _x14, _tmp); \
	/* 0 * 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * a * b * c * d * e * f */ \
	COMPV_INTERLEAVE_I8_SSE2(_x0, _x1, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x2, _x3, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x4, _x5, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x6, _x7, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x8, _x9, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x10, _x11, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x12, _x13, _tmp); \
	COMPV_INTERLEAVE_I8_SSE2(_x14, _x15, _tmp);

COMPV_NAMESPACE_END()

#endif /* _COMPV_INTRIN_SSE_H_ */