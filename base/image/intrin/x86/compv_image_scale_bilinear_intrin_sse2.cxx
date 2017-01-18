/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bilinear_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVImageScaleBilinear_Intrin_SSE2(
	const uint8_t* inPtr, compv_uscalar_t inWidth, compv_uscalar_t inHeight, compv_uscalar_t inStride,
	COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outYStart, compv_uscalar_t outYEnd, COMPV_ALIGNED(SSE) compv_uscalar_t outStride,
	compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CHECK_SSE2(); // FIXME: may be SSE41

	compv_uscalar_t i, nearestY;
	const uint8_t* inPtr_;
	int sf_x_ = static_cast<int>(sf_x);
	COMPV_ALIGN_SSE() const int32_t SFX[4][4] = {
		{ sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 },
		{ sf_x_ * 4, sf_x_ * 5, sf_x_ * 6, sf_x_ * 7 },
		{ sf_x_ * 8, sf_x_ * 9, sf_x_ * 10, sf_x_ * 11 },
		{ sf_x_ * 12, sf_x_ * 13, sf_x_ * 14, sf_x_ * 15 }
	};
	// FIXME: move to simd_globals
	// Deinterleaves bytes [a,b,a,b,b,a,b,a,b] to [a,a,a,a,a,b,b,b,b,b]
	COMPV_ALIGN_DEFAULT() int32_t kShuffleEpi8_Deinterleave_i32[] = {  // To be used with _mm_shuffle_epi8
		COMPV_MM_SHUFFLE_EPI8(6, 4, 2, 0), COMPV_MM_SHUFFLE_EPI8(14, 12, 10, 8), COMPV_MM_SHUFFLE_EPI8(7, 5, 3, 1), COMPV_MM_SHUFFLE_EPI8(15, 13, 11, 9), // 128bits SSE register
		COMPV_MM_SHUFFLE_EPI8(6, 4, 2, 0), COMPV_MM_SHUFFLE_EPI8(14, 12, 10, 8), COMPV_MM_SHUFFLE_EPI8(7, 5, 3, 1), COMPV_MM_SHUFFLE_EPI8(15, 13, 11, 9), // 256bits AVX register
	};
	__m128i vecYStart, vecX0, vecX1, vecX2, vecX3, vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3, vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vecy0, vecy1;
	const __m128i vec0xff_epi32 = _mm_set1_epi32(0xff);
	const __m128i vec0xff_epi16 = _mm_set1_epi16(0xff);
	const __m128i vec0x1 = _mm_set1_epi32(0x1); // FIXME: not used
	const __m128i vecInStride = _mm_set1_epi32(static_cast<int>(inStride)); // FIXME: not used
	const __m128i vecInStridePlusOne = _mm_add_epi32(vecInStride, vec0x1); // FIXME: not used
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ * 16);
	const __m128i vecSFX0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[0]));
	const __m128i vecSFX1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[1]));
	const __m128i vecSFX2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[2]));
	const __m128i vecSFX3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[3]));
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));
	const __m128i vecDeinterleave = _mm_load_si128(reinterpret_cast<const __m128i*>(kShuffleEpi8_Deinterleave_i32));	
	const __m128i vecZero = _mm_setzero_si128();
	int i32, nearestX0, nearestX1;

	// TODO(dmi): SS41 have _mm_insert_epi8 
	// TODO(dmi): _mm_shuffle_epi8 is SSSE3
	// FIXME: you can use 'vecSFX0' only and add 'vecSFX'

	vecYStart = _mm_set1_epi32(static_cast<int>(outYStart));
	while (outYStart < outYEnd) {
		nearestY = (outYStart >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride)); // FIXME: use SIMD (vecY0) and remove y += sf_y
		vecy0 = _mm_and_si128(vecYStart, vec0xff_epi32);
		vecy1 = _mm_sub_epi32(vec0xff_epi32, vecy0);
		vecy0 = _mm_packs_epi32(vecy0, vecy0); // epi32 -> epi16
		vecy1 = _mm_packs_epi32(vecy1, vecy1); // epi32 -> epi16
		vecX0 = vecSFX0;
		vecX1 = vecSFX1;
		vecX2 = vecSFX2;
		vecX3 = vecSFX3;
		for (i = 0; i < outWidth; i += 16) {


			// FIXME: "_mm_extract_epi32" and "_mm_insert_epi32" are SSE41
			// FIXME: add support for insert_epi64 for x64

			// Part #0
			vec0 = _mm_srli_epi32(vecX0, 8);
			
			nearestX0 = _mm_extract_epi32(vec0, 0);
			nearestX1 = _mm_extract_epi32(vec0, 1);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb0 = _mm_cvtsi32_si128(i32);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb2 = _mm_cvtsi32_si128(i32);

			nearestX0 = _mm_extract_epi32(vec0, 2);
			nearestX1 = _mm_extract_epi32(vec0, 3);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, i32, 1);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, i32, 1);

			// Part #1
			vec0 = _mm_srli_epi32(vecX1, 8);
			
			nearestX0 = _mm_extract_epi32(vec0, 0);
			nearestX1 = _mm_extract_epi32(vec0, 1);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, i32, 2);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, i32, 2);
			
			nearestX0 = _mm_extract_epi32(vec0, 2);
			nearestX1 = _mm_extract_epi32(vec0, 3);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, i32, 3);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, i32, 3);



			// Part #2
			vec0 = _mm_srli_epi32(vecX2, 8);

			nearestX0 = _mm_extract_epi32(vec0, 0);
			nearestX1 = _mm_extract_epi32(vec0, 1);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb1 = _mm_cvtsi32_si128(i32);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb3 = _mm_cvtsi32_si128(i32);

			nearestX0 = _mm_extract_epi32(vec0, 2);
			nearestX1 = _mm_extract_epi32(vec0, 3);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, i32, 1);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, i32, 1);

			// Part #3
			vec0 = _mm_srli_epi32(vecX3, 8);

			nearestX0 = _mm_extract_epi32(vec0, 0);
			nearestX1 = _mm_extract_epi32(vec0, 1);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, i32, 2);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, i32, 2);

			nearestX0 = _mm_extract_epi32(vec0, 2);
			nearestX1 = _mm_extract_epi32(vec0, 3);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, i32, 3);
			i32 = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, i32, 3);

			// FIXME: _mm_unpack(lo/hi)_epi16(vecNeighbx, vecZero) computed several times

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
			vec1 = _mm_sub_epi16(vec0xff_epi16, vec0);			
			// compute vec4 = (neighb0 * x1) + (neighb1 * x0) - #8 epi16
			vec4 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb0, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb1, vecZero), vec0));
			// compute vec5 = (neighb2 * x1) + (neighb3 * x0) - #8 epi16
			vec5 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb2, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpacklo_epi8(vecNeighb3, vecZero), vec0));

			// compute x0 and x1 (second #8) and convert from epi32 and epi16
			vec0 = _mm_packus_epi32(_mm_and_si128(vecX2, vec0xff_epi32), _mm_and_si128(vecX3, vec0xff_epi32)); // epi16
			vec1 = _mm_sub_epi16(vec0xff_epi16, vec0);
			// compute vec6 = (neighb0 * x1) + (neighb1 * x0) - #8 epi16
			vec6 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb0, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb1, vecZero), vec0));
			// compute vec7 = (neighb2 * x1) + (neighb3 * x0) - #8 epi16
			vec7 = _mm_adds_epu16(_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb2, vecZero), vec1),
				_mm_mullo_epi16(_mm_unpackhi_epi8(vecNeighb3, vecZero), vec0));

			// Let's say:
			//		A = ((neighb0 * x1) + (neighb1 * x0))
			//		B = ((neighb2 * x1) + (neighb3 * x0))
			// Then:
			//		A = vec4, vec6
			//		B = vec5, vec7

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
		vecYStart = _mm_add_epi32(vecYStart, vecSFY);
		outYStart = _mm_extract_epi32(vecYStart, 0);
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
