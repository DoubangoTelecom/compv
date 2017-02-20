/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_INTRIN_SSE_H_)
#define _COMPV_BASE_INTRIN_SSE_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

// -0: Sign bit (bit-63) to 1 (https://en.wikipedia.org/wiki/Double-precision_floating-point_format) and all other bites to 0
// not(-0.) = 0x7fffffffffffffff. r=0x7fffffffffffffff doesn't fill in 32bit register and this is why we use not(-0) instead of the result.
// we can also use: _mm_and_pd(a, _mm_load_pd(reinterpret_cast<const double*>(kAVXFloat64MaskAbs)))) which doesn't override the mask in asm and is faster
#define _mm_abs_pd_SSE2(a) _mm_andnot_pd(_mm_set1_pd(-0.), a)

static COMPV_INLINE __m128i _mm_mullo_epi32_SSE2(const __m128i &a, const __m128i &b)
{
    const __m128i x = _mm_mul_epu32(a, b);
	const __m128i y = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4));
    return _mm_unpacklo_epi32(_mm_shuffle_epi32(x, 0x8), _mm_shuffle_epi32(y, 0x8));
}

#define _mm_cvtepi16_epi32_low_SSE2(a) _mm_srai_epi32(_mm_unpacklo_epi16(a, a), 16)
#define _mm_cvtepi16_epi32_hi_SSE2(a) _mm_srai_epi32(_mm_unpackhi_epi16(a, a), 16)

// algorithm: return (vecX[i] != vecY[i]) ? vecPlaceholder[i] : 0x00; 
// e.g. to test vec not zero: _mm_cmpnot_epu8_SSE2(vec, 0x00, 0xff) - mask is used to set value
#define _mm_cmpnot_epu8_SSE2(vecX, vecY, vecPlaceholder) _mm_andnot_si128(_mm_cmpeq_epi8(vecX, vecY), vecPlaceholder)
// algorithm: return (vecX[i] > vecY[i]) ? vecPlaceholder[i] : 0x00; 
#define _mm_cmpgt_epu8_SSE2(vecX, vecY, vecZero, vecPlaceholder) _mm_cmpnot_epu8_SSE2(_mm_subs_epu8(vecX, vecY), vecZero, vecPlaceholder)
#define _mm_cmplt_epu8_SSE2(vecX, vecY, vecZero, vecPlaceholder) _mm_cmpgt_epu8_SSE2(vecY, vecX, vecZero, vecPlaceholder)

#define _mm_cmple_epu8_SSE2(x, y) _mm_cmpeq_epi8(_mm_min_epu8(x, y), x)
#define _mm_cmpge_epu8_SSE2(x, y) _mm_cmple_epu8_SSE2(y, x)

// Compute the minimum, set the min a the first position and clear all other values
#define _mm_minhz_epu8_SSE2(vec) /*SSE2 use _mm_minpos_epu16 on SSE41 */\
	vec = _mm_min_epu8(vec, _mm_srli_si128(vec, 8)); /* >> 64b */ \
	vec = _mm_min_epu8(vec, _mm_srli_si128(vec, 4)); /* >> 32b */ \
	vec = _mm_min_epu8(vec, _mm_srli_si128(vec, 2)); /* >> 16b */ \
	vec = _mm_min_epu8(vec, _mm_srli_si128(vec, 1)) /* >> 1b */

/*
Interleaves two 128bits vectors.
From:
0 0 0 0 0 0 . . . .
1 1 1 1 1 1 . . . .
To:
0 1 0 1 0 1 . . . .
*/
#define COMPV_INTERLEAVE_I8_SSE2(_m0, _m1, _tmp) \
	_tmp = _mm_unpackhi_epi8(_m0, _m1); \
	_m0 = _mm_unpacklo_epi8(_m0, _m1); \
	_m1 = _tmp;

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

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_INTRIN_SSE_H_ */