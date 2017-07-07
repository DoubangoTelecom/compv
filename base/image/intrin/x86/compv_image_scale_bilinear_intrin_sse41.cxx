/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bilinear_intrin_sse41.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

/****** x86_32 ******/
#define _mm_bilinear_insert_x86_sse41(vecDst, val32, index) vecDst = _mm_insert_epi32(vecDst, val32, index)
#define _mm_bilinear_insert_at_0_x86_sse41(vecDst, val32) vecDst = _mm_cvtsi32_si128(val32)
#define _mm_bilinear_insert_at_1_x86_sse41(vecDst, val32) _mm_bilinear_insert_x86_sse41(vecDst, val32, 1)
#define _mm_bilinear_insert_at_2_x86_sse41(vecDst, val32) _mm_bilinear_insert_x86_sse41(vecDst, val32, 2)
#define _mm_bilinear_insert_at_3_x86_sse41(vecDst, val32) _mm_bilinear_insert_x86_sse41(vecDst, val32, 3)
#define _mm_bilinear_extract_then_insert_x86_sse41(vecNeareastX, neareastIndex0, neareastIndex1, vecNeighbA, vecNeighbB, neighbIndex, tmp32) \
			/* Extract indices(neareastIndex0, neareastIndex1) */ \
			nearestX0 = _mm_extract_epi32(vecNeareastX, neareastIndex0); \
			nearestX1 = _mm_extract_epi32(vecNeareastX, neareastIndex1); \
			/* Insert in vecNeighbA(neighbIndex) */ \
			tmp32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); /* vecNeighbA  -> 0,1,0,1,0,1,0,1,0,1,0,1 */ \
			_mm_bilinear_insert_at_##neighbIndex##_x86_sse41(vecNeighbA, tmp32); \
			/* Insert in vecNeighbB(neighbIndex) */ \
			tmp32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); /* vecNeighbB -> 2,3,2,3,2,3,2,3 */ \
			_mm_bilinear_insert_at_##neighbIndex##_x86_sse41(vecNeighbB, tmp32)

#define _mm_bilinear_set_neighbs_x86_sse41(vecNeareastX, vecNeighbA, vecNeighbB, neighbIndex0, neighbIndex1, tmp32) \
			_mm_bilinear_extract_then_insert_x86_sse41(vecNeareastX, 0, 1, vecNeighbA, vecNeighbB, neighbIndex0, tmp32); \
			_mm_bilinear_extract_then_insert_x86_sse41(vecNeareastX, 2, 3, vecNeighbA, vecNeighbB, neighbIndex1, tmp32)

/****** x86_64 ******/
#define _mm_bilinear_insert_x64_sse41(vecDst, val64, index) vecDst = _mm_insert_epi64(vecDst, val64, index)
#define _mm_bilinear_insert_at_0_x64_sse41(vecDst, val64) vecDst = _mm_cvtsi64_si128(val64)
#define _mm_bilinear_insert_at_1_x64_sse41(vecDst, val64) _mm_bilinear_insert_x64_sse41(vecDst, val64, 1)
#define _mm_bilinear_set_neighbs_x64_sse41(vecNeareastX, vecNeighbA, vecNeighbB, index, tmp64_0, tmp64_1) \
			nearestX0 = _mm_extract_epi32(vecNeareastX, 0); \
			nearestX1 = _mm_extract_epi32(vecNeareastX, 1); \
			tmp64_0 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); /* vecNeighbA  -> 0,1,0,1,0,1,0,1,0,1,0,1 */ \
			tmp64_1 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); /* vecNeighbB -> 2,3,2,3,2,3,2,3 */ \
			nearestX0 = _mm_extract_epi32(vecNeareastX, 2); \
			nearestX1 = _mm_extract_epi32(vecNeareastX, 3); \
			tmp64_0 |= static_cast<int64_t>(*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16)) << 32; /* vecNeighbA -> 0,1,0,1,0,1,0,1,0,1,0,1 */ \
			tmp64_1 |= static_cast<int64_t>(*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16)) << 32; /* vecNeighbB -> 2,3,2,3,2,3,2,3 */ \
			_mm_bilinear_insert_at_##index##_x64_sse41(vecNeighbA, tmp64_0); \
			_mm_bilinear_insert_at_##index##_x64_sse41(vecNeighbB, tmp64_1)


COMPV_NAMESPACE_BEGIN()

void CompVImageScaleBilinear_Intrin_SSE41(
	const uint8_t* inPtr, compv_uscalar_t inStride,
	COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outYStart, compv_uscalar_t outYEnd, COMPV_ALIGNED(SSE) compv_uscalar_t outStride,
	compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CHECK_SSE41();

	compv_uscalar_t i, nearestY;
	const uint8_t* inPtr_;
	int sf_x_ = static_cast<int>(sf_x);
	__m128i vecX0, vecX1, vecX2, vecX3, vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3, vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vecy0, vecy1;
	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vec0xff_epi32 = _mm_srli_epi32(_mm_cmpeq_epi32(vecZero, vecZero), 24); // 0x000000ff (faster than set1_epi32(0xff))
	static const __m128i vec0xff_epi16 = _mm_srli_epi16(_mm_cmpeq_epi16(vecZero, vecZero), 8); // 0x00ff (faster than set1_epi16(0xff))
	static const __m128i vecDeinterleave = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_DeinterleaveL2_i32));
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ << 4);
	const __m128i vecSfxTimes4 = _mm_set1_epi32(sf_x_ << 2);
	const __m128i vecSFX0 = _mm_set_epi32(sf_x_ * 3, sf_x_ * 2, sf_x_ * 1, sf_x_ * 0);
	const __m128i vecSFX1 = _mm_add_epi32(vecSFX0, vecSfxTimes4);
	const __m128i vecSFX2 = _mm_add_epi32(vecSFX1, vecSfxTimes4);
	const __m128i vecSFX3 = _mm_add_epi32(vecSFX2, vecSfxTimes4);
	int nearestX0, nearestX1;
#if COMPV_ARCH_X64 && 0
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("32 bits code is faster");
	int64_t i64_0, i64_1;
#else
	int32_t i32;
#endif
	
	do {
		nearestY = (outYStart >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		vecy0 = _mm_and_si128(_mm_set1_epi32(static_cast<int>(outYStart)), vec0xff_epi32);
		vecy1 = _mm_sub_epi32(vec0xff_epi32, vecy0);
		vecy0 = _mm_packs_epi32(vecy0, vecy0); // epi32 -> epi16
		vecy1 = _mm_packs_epi32(vecy1, vecy1); // epi32 -> epi16
		vecX0 = vecSFX0;
		vecX1 = vecSFX1;
		vecX2 = vecSFX2;
		vecX3 = vecSFX3;
		for (i = 0; i < outWidth; i += 16) {
			// nearest x-point
			vec0 = _mm_srli_epi32(vecX0, 8);
			vec1 = _mm_srli_epi32(vecX1, 8);
			vec2 = _mm_srli_epi32(vecX2, 8);
			vec3 = _mm_srli_epi32(vecX3, 8);

			// set neighbs
#if COMPV_ARCH_X64 && 0
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("32 bits code is faster");
			_mm_bilinear_set_neighbs_x64_sse41(vec0, vecNeighb0, vecNeighb2, 0, i64_0, i64_1);
			_mm_bilinear_set_neighbs_x64_sse41(vec1, vecNeighb0, vecNeighb2, 1, i64_0, i64_1);
			_mm_bilinear_set_neighbs_x64_sse41(vec2, vecNeighb1, vecNeighb3, 0, i64_0, i64_1);
			_mm_bilinear_set_neighbs_x64_sse41(vec3, vecNeighb1, vecNeighb3, 1, i64_0, i64_1);
#else
			_mm_bilinear_set_neighbs_x86_sse41(vec0, vecNeighb0, vecNeighb2, 0, 1, i32);
			_mm_bilinear_set_neighbs_x86_sse41(vec1, vecNeighb0, vecNeighb2, 2, 3, i32);
			_mm_bilinear_set_neighbs_x86_sse41(vec2, vecNeighb1, vecNeighb3, 0, 1, i32);
			_mm_bilinear_set_neighbs_x86_sse41(vec3, vecNeighb1, vecNeighb3, 2, 3, i32);
#endif

			// Deinterleave neighbs
			vec0 = _mm_shuffle_epi8(vecNeighb0, vecDeinterleave); // 0,0,0,0,1,1,1,1
			vec1 = _mm_shuffle_epi8(vecNeighb1, vecDeinterleave); // 0,0,0,0,1,1,1,1
			vec2 = _mm_shuffle_epi8(vecNeighb2, vecDeinterleave); // 2,2,2,2,3,3,3,3
			vec3 = _mm_shuffle_epi8(vecNeighb3, vecDeinterleave); // 2,2,2,2,3,3,3,3
			vecNeighb0 = _mm_unpacklo_epi64(vec0, vec1); // 0,0,0,0,0,0
			vecNeighb1 = _mm_unpackhi_epi64(vec0, vec1); // 1,1,1,1,1,1
			vecNeighb2 = _mm_unpacklo_epi64(vec2, vec3); // 2,2,2,2,2,2
			vecNeighb3 = _mm_unpackhi_epi64(vec2, vec3); // 3,3,3,3,3,3


			// x0 = (x & 0xff) and x1 = (0xff - x0)
			// This means max((u8 * x1) + (u8 * x0)) = #65282 -> no overflow for Epi16 arith (saturation using (_mm_adds_epu16) not required)

			// compute x0 and x1 (first #8) and convert from epi32 and epi16
			vec0 = _mm_packus_epi32(_mm_and_si128(vecX0, vec0xff_epi32), _mm_and_si128(vecX1, vec0xff_epi32)); // epi16
			vec1 = _mm_andnot_si128(vec0, vec0xff_epi16);
			// compute vec4 = (neighb0 * x1) + (neighb1 * x0) -> #8 epi16
			vec4 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb0, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb1, vecZero), vec0));
			// compute vec5 = (neighb2 * x1) + (neighb3 * x0) -> #8 epi16
			vec5 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb2, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb3, vecZero), vec0));

			// compute x0 and x1 (second #8) and convert from epi32 and epi16
			vec0 = _mm_packus_epi32(_mm_and_si128(vecX2, vec0xff_epi32), _mm_and_si128(vecX3, vec0xff_epi32)); // epi16
			vec1 = _mm_andnot_si128(vec0, vec0xff_epi16);
			// compute vec6 = (neighb0 * x1) + (neighb1 * x0) -> #8 epi16
			vec6 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb0, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb1, vecZero), vec0));
			// compute vec7 = (neighb2 * x1) + (neighb3 * x0) -> #8 epi16
			vec7 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb2, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb3, vecZero), vec0));

			// FIXME: remove
			//for (int i = 0; i < 8; ++i) printf("%u, ", vec1.m128i_u16[i]);

			// Let's say:
			//		A = ((neighb0 * x1) + (neighb1 * x0))
			//		B = ((neighb2 * x1) + (neighb3 * x0))
			// Then:
			//		A = vec4, vec6
			//		B = vec5, vec7
			//
			// We cannot use _mm_madd_epi16 to compute C and D because it operates on epi16 while A and B contain epu16 values

			// compute C = (y1 * A) >> 16
			vec0 = _mm_mulhi_epu16(vecy1, vec4);
			vec1 = _mm_mulhi_epu16(vecy1, vec6);

			// compute D = (y0 * B) >> 16
			vec2 = _mm_mulhi_epu16(vecy0, vec5);
			vec3 = _mm_mulhi_epu16(vecy0, vec7);

			// Compute R = (C + D)
			vec0 = _mm_adds_epu16(vec0, vec2);
			vec1 = _mm_adds_epu16(vec1, vec3);

			// Store the result
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), _mm_packus_epi16(vec0, vec1));

			// move to next indices
			vecX0 = _mm_add_epi32(vecX0, vecSfxTimes16);
			vecX1 = _mm_add_epi32(vecX1, vecSfxTimes16);
			vecX2 = _mm_add_epi32(vecX2, vecSfxTimes16);
			vecX3 = _mm_add_epi32(vecX3, vecSfxTimes16);			
		}
		outPtr += outStride;
		outYStart += sf_y;
	} while (outYStart < outYEnd);
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
