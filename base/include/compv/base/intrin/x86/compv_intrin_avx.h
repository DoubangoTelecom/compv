/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_INTRIN_AVX_H_)
#define _COMPV_BASE_INTRIN_AVX_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
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
#define compv_avx2_shuffle_epi8(ymm0_, ymm1_)		_mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(ymm0_, kAVXPermutevar8x32_ABCDDEFG_i32), ymm1_) // Intentionally not correct to avoid performance issues, load 'kAVXPermutevar8x32_ABCDDEFG_i32' yourself *once*


// algorithm: return (vecX[i] != vecY[i]) ? vecPlaceholder[i] : 0x00; 
// e.g. to test vec not zero: _mm_cmpnot_epu8_SSE2(vec, 0x00, 0xff) - mask is used to set value
#define _mm256_cmpnot_epu8_AVX2(vecX, vecY, vecPlaceholder) _mm256_andnot_si256(_mm256_cmpeq_epi8(vecX, vecY), vecPlaceholder)
// algorithm: return (vecX[i] > vecY[i]) ? vecPlaceholder[i] : 0x00; 
#define _mm256_cmpgt_epu8_AVX2(vecX, vecY, vecZero, vecPlaceholder) _mm256_cmpnot_epu8_AVX2(_mm256_subs_epu8(vecX, vecY), vecZero, vecPlaceholder)
#define _mm256_cmplt_epu8_AVX2(vecX, vecY, vecZero, vecPlaceholder) _mm256_cmpgt_epu8_AVX2(vecY, vecX, vecZero, vecPlaceholder)

#define _mm256_cmple_epu8_AVX2(x, y) _mm256_cmpeq_epi8(_mm256_min_epu8(x, y), x)
#define _mm256_cmpge_epu8_AVX2(x, y) _mm256_cmple_epu8_AVX2(y, x)

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

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_INTRIN_AVX_H_ */
