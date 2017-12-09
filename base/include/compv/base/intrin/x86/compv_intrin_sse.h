/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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

// zero must be equal to _mm_setzero_si128()
#define _mm_cmple_epu16_SSE2(a, b, zero) _mm_cmpeq_epi16(_mm_subs_epu16(a, b), _mm_setzero_si128())

// zero must be equal to _mm_setzero_si128()
#define _mm_cmpge_epu16_SSE(a, b, zero) _mm_cmple_epu16_SSE(b, a, zero)

// -0: Sign bit (bit-63) to 1 (https://en.wikipedia.org/wiki/Double-precision_floating-point_format) and all other bites to 0
// not(-0.) = 0x7fffffffffffffff. r=0x7fffffffffffffff doesn't fill in 32bit register and this is why we use not(-0) instead of the result.
// we can also use: _mm_and_pd(a, _mm_load_pd(reinterpret_cast<const double*>(kAVXFloat64MaskAbs)))) which doesn't override the mask in asm and is faster
#define _mm_abs_pd_SSE2(a) _mm_andnot_pd(_mm_set1_pd(-0.), a)

// mask must be equal to _mm_set1_epi16(0x8000)
#define _mm_max_epu16_SS2(a, b, mask) _mm_xor_si128(_mm_max_epi16(_mm_xor_si128(a, mask), _mm_xor_si128(b, mask)), mask)

// zero must be equal to _mm_setzero_si128()
#define _mm_blendv_epi8_SS2(a, b, mask, zero) _mm_blendv_si128(a, b, _mm_cmplt_epi8(mask, zero))

// zero must be equal to _mm_setzero_si128()
#define _mm_min_epu16_SSE2(a, b, zero) _mm_blendv_si128(b, a, _mm_cmple_epu16_SSE2(a, b, zero))

#if 0
// zero must be equal to _mm_setzero_si128()
#define _mm_max_epu16_SSE2(a, b, zero) _mm_blendv_si128(a, b, _mm_cmple_epu16_SSE2(a, b, zero))
#endif


// algorithm: return (vecX[i] != vecY[i]) ? vecPlaceholder[i] : 0x00; 
// e.g. to test vec not zero: _mm_cmpnot_epu8_SSE2(vec, 0x00, 0xff) - mask is used to set value
#define _mm_cmpnot_epu8_SSE2(vecX, vecY, vecPlaceholder) _mm_andnot_si128(_mm_cmpeq_epi8(vecX, vecY), vecPlaceholder)
#define _mm_cmpnot_epi32_SSE2(vecX, vecY, vecPlaceholder) _mm_andnot_si128(_mm_cmpeq_epi32(vecX, vecY), vecPlaceholder)
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

#define _mm_cvtepi8_epi16_low_SSE2(a) _mm_srai_epi16(_mm_unpacklo_epi8(a, a), 8) // Convert from I8 to I16 while shifting in sign bits, ASM: use '_mm_cvtepi8_epi16' which is SSE4.1
#define _mm_cvtepi8_epi16_hi_SSE2(a) _mm_srai_epi16(_mm_unpackhi_epi8(a, a), 8) // Convert from I8 to I16 while shifting in sign bits, ASM: use '_mm_cvtepi8_epi16' which is SSE4.1
#define _mm_cvtepi16_epi32_low_SSE2(a) _mm_srai_epi32(_mm_unpacklo_epi16(a, a), 16) // Convert from I16 to I32 while shifting in sign bits, ASM: use '_mm_cvtepi16_epi32' which is SSE4.1
#define _mm_cvtepi16_epi32_hi_SSE2(a) _mm_srai_epi32(_mm_unpackhi_epi16(a, a), 16) // Convert from I16 to I32 while shifting in sign bits, ASM: use '_mm_cvtepi16_epi32' which is SSE4.1

static COMPV_INLINE __m128i _mm_mullo_epi32_SSE2(const __m128i &a, const __m128i &b) {
	const __m128i x = _mm_mul_epu32(a, b);
	const __m128i y = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4));
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(x, 0x8), _mm_shuffle_epi32(y, 0x8));
}

// De-Interleave "ptr" into  "vecLane0", "vecLane1" and "vecLane2"
// e.g. RGBRGBRGB -> [RRRR], [GGGG], [BBBB]
//!\\ You should not need to use this function -> FASTER: convert to RGBX then process (more info: see RGB24 -> YUV)
#define COMPV_VLD3_I8_SSSE3(ptr, vecLane0, vecLane1, vecLane2, vectmp0, vectmp1) { \
		static const __m128i vecMask = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Deinterleave8uL3_i32)); \
		vecLane0 = _mm_load_si128(reinterpret_cast<const __m128i*>((ptr))); \
		vecLane1 = _mm_load_si128(reinterpret_cast<const __m128i*>((ptr)) + 1); \
		vecLane2 = _mm_load_si128(reinterpret_cast<const __m128i*>((ptr)) + 2); \
		vectmp0 = _mm_shuffle_epi8(vecLane0, vecMask); \
		vectmp1 = _mm_shuffle_epi8(vecLane1, vecMask); \
		vecLane2 = _mm_shuffle_epi8(vecLane2, vecMask); \
		vecLane0 = _mm_alignr_epi8(_mm_srli_si128(vecLane2, 6), vectmp1, 11); \
		vecLane1 = _mm_slli_si128(vectmp1, 10); \
		vectmp1 = _mm_srli_si128(vectmp1, 6); \
		vectmp1 = _mm_alignr_epi8(vectmp1, vectmp0, 11); \
		vecLane1 = _mm_alignr_epi8(_mm_srli_si128(vecLane2, 11), vecLane1, 10); \
		vectmp0 = _mm_slli_si128(vectmp0, 5); \
		vecLane1 = _mm_alignr_epi8(vecLane1, vectmp0, 11); \
		vectmp0 = _mm_slli_si128(vectmp0, 5); \
		vecLane0 = _mm_alignr_epi8(vecLane0, vectmp0, 10); \
		vectmp1 = _mm_slli_si128(vectmp1, 6); \
		vecLane2 = _mm_alignr_epi8(vecLane2, vectmp1, 6); \
	}

#define COMPV_VLD3_U8_SSSE3 COMPV_VLD3_I8_SSSE3

// Interleave "vecLane0", "vecLane1" and "vecLane3" then store into "ptr"
// !!! "vecLane0", "vecLane1" and "vecLane3" ARE modified !!!
// e.g. [RRRR], [GGGG], [BBBB] -> RGBRGBRGB
#define COMPV_VST3_I8_SSSE3(ptr, vecLane0, vecLane1, vecLane2, vectmp0, vectmp1) { \
		static const __m128i vecMask0 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Interleave8uL3_Step0_i32)); \
		static const __m128i vecMask1 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Interleave8uL3_Step1_i32)); \
		static const __m128i vecMask2 = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Interleave8uL3_Step2_i32)); \
		vectmp0 = _mm_unpacklo_epi8(vecLane0, vecLane1); /* RG RG RG...*/ \
		vectmp1 = _mm_unpackhi_epi8(vecLane0, vecLane1); /* RG RG RG... */ \
		/* First = vecLane0 */ \
		vecLane0 = _mm_alignr_epi8(vecLane2, _mm_slli_si128(vectmp0, 5), 5); /* RG RG RG ...BBBBB */ \
		vecLane0 = _mm_shuffle_epi8(vecLane0, vecMask0); /* RGB RGB...R */ \
		/* Second = vecLane1 */ \
		vecLane1 = _mm_alignr_epi8(vectmp1, vectmp0, 11); /* GR GR GR ... */ \
		vecLane2 = _mm_srli_si128(vecLane2, 5); \
		vecLane1 = _mm_slli_si128(vecLane1, 5); \
		vecLane1 = _mm_alignr_epi8(vecLane2, vecLane1, 5); /* GR GR GR ...BBBBB */ \
		vecLane1 = _mm_shuffle_epi8(vecLane1, vecMask1); /* RGB RGB...G */ \
		/* Third = vecLane2 */ \
		vecLane2 = _mm_srli_si128(vecLane2, 5); \
		vecLane2 = _mm_alignr_epi8(vecLane2, vectmp1, 6); /* BR BR BR ...BBBBB */ \
		vecLane2 = _mm_shuffle_epi8(vecLane2, vecMask2); /* RGB RGB...B */ \
		/* Store */ \
		_mm_store_si128(reinterpret_cast<__m128i*>((ptr)), vecLane0); \
		_mm_store_si128(reinterpret_cast<__m128i*>((ptr)) + 1, vecLane1); \
		_mm_store_si128(reinterpret_cast<__m128i*>((ptr)) + 2, vecLane2); \
	}

#define COMPV_VST3_U8_SSSE3 COMPV_VST3_I8_SSSE3


// De-Interleave "ptr" into  "vecLane0", "vecLane1", "vecLane2" and "vecLane3"
// e.g. RGBARGBARGBA -> [RRRR], [GGGG], [BBBB], [AAAA]
//!\\ You should not need to use this function -> FASTER: convert to RGBX then process (more info: see RGB24 -> YUV)
#define COMPV_VLD4_I8_SSSE3(ptr, vecLane0, vecLane1, vecLane2, vecLane3, vectmp0, vectmp1) { \
		static const __m128i vecMask = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Deinterleave8uL4_i32)); \
		vecLane0 = _mm_load_si128(reinterpret_cast<const __m128i*>((ptr))); /* RGBA RGBA RGBA RGBA */ \
		vecLane1 = _mm_load_si128(reinterpret_cast<const __m128i*>((ptr)) + 1); /* RGBA RGBA RGBA RGBA */ \
		vecLane2 = _mm_load_si128(reinterpret_cast<const __m128i*>((ptr)) + 2); /* RGBA RGBA RGBA RGBA */ \
		vecLane3 = _mm_load_si128(reinterpret_cast<const __m128i*>((ptr)) + 3); /* RGBA RGBA RGBA RGBA */ \
		/* first round */ \
		vectmp0 = _mm_unpacklo_epi8(vecLane0, vecLane1); /* RR GG BB AA */ \
		vectmp1 = _mm_unpackhi_epi8(vecLane0, vecLane1); /* RR GG BB AA */ \
		vecLane0 = _mm_unpacklo_epi16(vectmp0, vectmp1); /* RRRR GGGG BBBB AAAA */ \
		vecLane1 = _mm_unpackhi_epi16(vectmp0, vectmp1); /* RRRR GGGG BBBB AAAA */ \
		vectmp0 = _mm_unpacklo_epi32(vecLane0, vecLane1); /* RRRRRRRR GGGGGGGG */ \
		vectmp1 = _mm_unpackhi_epi32(vecLane0, vecLane1); /* BBBBBBBB AAAAAAAA */ \
		/* second round */ \
		vecLane0 = _mm_unpacklo_epi8(vecLane2, vecLane3); /* RR GG BB AA */ \
		vecLane1 = _mm_unpackhi_epi8(vecLane2, vecLane3); /* RR GG BB AA */ \
		vecLane2 = _mm_unpacklo_epi16(vecLane0, vecLane1); /* RRRR GGGG BBBB AAAA */ \
		vecLane3 = _mm_unpackhi_epi16(vecLane0, vecLane1); /* RRRR GGGG BBBB AAAA */ \
		vecLane0 = _mm_unpacklo_epi32(vecLane2, vecLane3); /* RRRRRRRR GGGGGGGG */ \
		vecLane2 = _mm_unpackhi_epi32(vecLane2, vecLane3); /* BBBBBBBB AAAAAAAA */ \
		/* final round */ \
		vecLane1 = _mm_unpackhi_epi64(vecLane0, vectmp0); /* GGGGGGGG GGGGGGGG (not in order) */ \
		vecLane0 = _mm_unpacklo_epi64(vecLane0, vectmp0); /* RRRRRRRR RRRRRRRR (not in order) */ \
		vecLane3 = _mm_unpackhi_epi64(vecLane2, vectmp1); /* AAAAAAAA AAAAAAAA (not in order) */ \
		vecLane2 = _mm_unpacklo_epi64(vecLane2, vectmp1); /* BBBBBBBB BBBBBBBB (not in order) */ \
		/* re-order */ \
		vecLane0 = _mm_shuffle_epi8(vecLane0, vecMask); /* RRRRRRRR RRRRRRRR (in order) */ \
		vecLane1 = _mm_shuffle_epi8(vecLane1, vecMask); /* GGGGGGGG GGGGGGGG (in order) */ \
		vecLane2 = _mm_shuffle_epi8(vecLane2, vecMask); /* BBBBBBBB BBBBBBBB (in order) */ \
		vecLane3 = _mm_shuffle_epi8(vecLane3, vecMask); /* AAAAAAAA AAAAAAAA (in order) */ \
	}

#define COMPV_VLD4_U8_SSSE3 COMPV_VLD4_I8_SSSE3

// Interleave "vecLane0", "vecLane1", "vecLane2" and "vecLane3" then store into "ptr"
// !!! "vecLane0", "vecLane1", "vecLane2" and "vecLane3" NOT modified !!!
// e.g. [RRRR], [GGGG], [BBBB], [AAAA] -> RGBARGBARGBA
#define COMPV_VST4_I8_SSE2(ptr, vecLane0, vecLane1, vecLane2, vecLane3, vectmp0, vectmp1) { \
		vectmp0 = _mm_unpacklo_epi8(vecLane0, vecLane1); \
		vectmp1 = _mm_unpacklo_epi8(vecLane2, vecLane3); \
		_mm_store_si128(reinterpret_cast<__m128i*>((ptr)), _mm_unpacklo_epi16(vectmp0, vectmp1)); \
		_mm_store_si128(reinterpret_cast<__m128i*>((ptr)) + 1, _mm_unpackhi_epi16(vectmp0, vectmp1)); \
		vectmp0 = _mm_unpackhi_epi8(vecLane0, vecLane1); \
		vectmp1 = _mm_unpackhi_epi8(vecLane2, vecLane3); \
		_mm_store_si128(reinterpret_cast<__m128i*>((ptr)) + 2, _mm_unpacklo_epi16(vectmp0, vectmp1)); \
		_mm_store_si128(reinterpret_cast<__m128i*>((ptr)) + 3, _mm_unpackhi_epi16(vectmp0, vectmp1)); \
	}

#define COMPV_VST4_U8_SSE2 COMPV_VST4_I8_SSE2

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