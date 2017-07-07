/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bilinear_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

#define _mm_bilinear_extract_then_insert_x86_avx2(vecNeareastX, neareastIndex0, neareastIndex1, memNeighbA, memNeighbB, neighbIndex) \
			/* Extract indices(neareastIndex0, neareastIndex1) */ \
			nearestX0 = _mm_extract_epi32(vecNeareastX, neareastIndex0); \
			nearestX1 = _mm_extract_epi32(vecNeareastX, neareastIndex1); \
			/* Insert in memNeighbA(neighbIndex) */ \
			memNeighbA[neighbIndex] = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1]) << 16); /* vecNeighbA  -> 0,1,0,1,0,1,0,1,0,1,0,1 */ \
			/* Insert in memNeighbB(neighbIndex) */ \
			memNeighbB[neighbIndex] = *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX0 + inStride]) | (*reinterpret_cast<const uint16_t*>(&inPtr_[nearestX1 + inStride]) << 16); /* vecNeighbB -> 2,3,2,3,2,3,2,3 */

#define _mm_bilinear_set_neighbs_x86_avx2(vecNeareastX, memNeighbA, memNeighbB, neighbIndex0, neighbIndex1) \
			_mm_bilinear_extract_then_insert_x86_avx2(vecNeareastX, 0, 1, memNeighbA, memNeighbB, neighbIndex0); \
			_mm_bilinear_extract_then_insert_x86_avx2(vecNeareastX, 2, 3, memNeighbA, memNeighbB, neighbIndex1)


COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void CompVImageScaleBilinear_Intrin_AVX2(
	const uint8_t* inPtr, compv_uscalar_t inStride,
	COMPV_ALIGNED(AVX) uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outYStart, compv_uscalar_t outYEnd, COMPV_ALIGNED(AVX) compv_uscalar_t outStride,
	compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
	_mm256_zeroupper();
#if !defined(__AVX2__)
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("AVX <-> SSE transition issues. This code use SSE ops which must be vex encoded. You must build this file with AVX enabled or use ASM version");
#endif

	compv_uscalar_t i, nearestY;
	const uint8_t* inPtr_;
	int sf_x_ = static_cast<int>(sf_x);
	__m256i vecX0, vecX1, vecX2, vecX3, vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3, vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vecy0, vecy1;
	__m128i xmm0; // AVX<->SSE transition issues if code not build with /AVX2 options. Should use ASM code to avoid the risk
	COMPV_ALIGN_AVX2() uint32_t memNeighb0[8];
	COMPV_ALIGN_AVX2() uint32_t memNeighb1[8];
	COMPV_ALIGN_AVX2() uint32_t memNeighb2[8];
	COMPV_ALIGN_AVX2() uint32_t memNeighb3[8];
	static const __m256i vecZero = _mm256_setzero_si256();
	static const __m256i vec0xff_epi32 = _mm256_srli_epi32(_mm256_cmpeq_epi32(vecZero, vecZero), 24); // 0x000000ff (faster than set1_epi32(0xff))
	static const __m256i vec0xff_epi16 = _mm256_srli_epi16(_mm256_cmpeq_epi16(vecZero, vecZero), 8); // 0x00ff (faster than set1_epi16(0xff))
	static const __m256i vecDeinterleave = _mm256_load_si256(reinterpret_cast<const __m256i*>(kShuffleEpi8_DeinterleaveL2_i32));
	const __m256i vecSfxTimes32 = _mm256_set1_epi32(sf_x_ << 5);
	const __m256i vecSfxTimes8 = _mm256_set1_epi32(sf_x_ << 3);
	const __m256i vecSFX0 = _mm256_set_epi32(sf_x_ * 7, sf_x_ * 6, sf_x_ * 5, sf_x_ * 4, sf_x_ * 3, sf_x_ * 2, sf_x_ * 1, sf_x_ * 0);
	const __m256i vecSFX1 = _mm256_add_epi32(vecSFX0, vecSfxTimes8);
	const __m256i vecSFX2 = _mm256_add_epi32(vecSFX1, vecSfxTimes8);
	const __m256i vecSFX3 = _mm256_add_epi32(vecSFX2, vecSfxTimes8);
	int nearestX0, nearestX1;

	do {
		nearestY = (outYStart >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		vecy0 = _mm256_and_si256(_mm256_set1_epi32(static_cast<int>(outYStart)), vec0xff_epi32);
		vecy1 = _mm256_sub_epi32(vec0xff_epi32, vecy0);
		vecy0 = _mm256_packs_epi32(vecy0, vecy0); // epi32 -> epi16
		vecy1 = _mm256_packs_epi32(vecy1, vecy1); // epi32 -> epi16
		vecX0 = vecSFX0;
		vecX1 = vecSFX1;
		vecX2 = vecSFX2;
		vecX3 = vecSFX3;
		for (i = 0; i < outWidth; i += 32) {
			// nearest x-point
			vec0 = _mm256_srli_epi32(vecX0, 8);
			vec1 = _mm256_srli_epi32(vecX1, 8);
			vec2 = _mm256_srli_epi32(vecX2, 8);
			vec3 = _mm256_srli_epi32(vecX3, 8);

			// write memNeighbs
			xmm0 = _mm256_extractf128_si256(vec0, 0);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb0, memNeighb2, 0, 1);
			xmm0 = _mm256_extractf128_si256(vec0, 1);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb0, memNeighb2, 2, 3);
			xmm0 = _mm256_extractf128_si256(vec1, 0);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb0, memNeighb2, 4, 5);
			xmm0 = _mm256_extractf128_si256(vec1, 1);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb0, memNeighb2, 6, 7);
			xmm0 = _mm256_extractf128_si256(vec2, 0);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb1, memNeighb3, 0, 1);
			xmm0 = _mm256_extractf128_si256(vec2, 1);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb1, memNeighb3, 2, 3);
			xmm0 = _mm256_extractf128_si256(vec3, 0);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb1, memNeighb3, 4, 5);
			xmm0 = _mm256_extractf128_si256(vec3, 1);
			_mm_bilinear_set_neighbs_x86_avx2(xmm0, memNeighb1, memNeighb3, 6, 7);

			// read memNeighbs
			vecNeighb0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(memNeighb0));
			vecNeighb1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(memNeighb1));
			vecNeighb2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(memNeighb2));
			vecNeighb3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(memNeighb3));
			
			// Deinterleave neighbs
			vec0 = _mm256_shuffle_epi8(vecNeighb0, vecDeinterleave); // 0,0,0,0,1,1,1,1
			vec1 = _mm256_shuffle_epi8(vecNeighb1, vecDeinterleave); // 0,0,0,0,1,1,1,1
			vec2 = _mm256_shuffle_epi8(vecNeighb2, vecDeinterleave); // 2,2,2,2,3,3,3,3
			vec3 = _mm256_shuffle_epi8(vecNeighb3, vecDeinterleave); // 2,2,2,2,3,3,3,3
			vecNeighb0 = _mm256_unpacklo_epi64(vec0, vec1); // 0,0,0,0,0,0
			vecNeighb1 = _mm256_unpackhi_epi64(vec0, vec1); // 1,1,1,1,1,1
			vecNeighb2 = _mm256_unpacklo_epi64(vec2, vec3); // 2,2,2,2,2,2
			vecNeighb3 = _mm256_unpackhi_epi64(vec2, vec3); // 3,3,3,3,3,3


			// compute x0 and x1 (first #8) and convert from epi32 and epi16
			vec0 = compv_avx2_packus_epi32(_mm256_and_si256(vecX0, vec0xff_epi32), _mm256_and_si256(vecX1, vec0xff_epi32)); // epi16
			vec1 = _mm256_andnot_si256(vec0, vec0xff_epi16);
			// compute vec4 = (neighb0 * x1) + (neighb1 * x0) -> #8 epi16
			vec4 = _mm256_adds_epu16(_mm256_mullo_epi16(_mm256_unpacklo_epi8(vecNeighb0, vecZero), vec1),
				_mm256_mullo_epi16(_mm256_unpacklo_epi8(vecNeighb1, vecZero), vec0));
			// compute vec5 = (neighb2 * x1) + (neighb3 * x0) -> #8 epi16
			vec5 = _mm256_adds_epu16(_mm256_mullo_epi16(_mm256_unpacklo_epi8(vecNeighb2, vecZero), vec1),
				_mm256_mullo_epi16(_mm256_unpacklo_epi8(vecNeighb3, vecZero), vec0));

			// compute x0 and x1 (second #8) and convert from epi32 and epi16
			vec0 = compv_avx2_packus_epi32(_mm256_and_si256(vecX2, vec0xff_epi32), _mm256_and_si256(vecX3, vec0xff_epi32)); // epi16
			vec1 = _mm256_andnot_si256(vec0, vec0xff_epi16);
			// compute vec6 = (neighb0 * x1) + (neighb1 * x0) -> #8 epi16
			vec6 = _mm256_adds_epu16(_mm256_mullo_epi16(_mm256_unpackhi_epi8(vecNeighb0, vecZero), vec1),
				_mm256_mullo_epi16(_mm256_unpackhi_epi8(vecNeighb1, vecZero), vec0));
			// compute vec7 = (neighb2 * x1) + (neighb3 * x0) -> #8 epi16
			vec7 = _mm256_adds_epu16(_mm256_mullo_epi16(_mm256_unpackhi_epi8(vecNeighb2, vecZero), vec1),
				_mm256_mullo_epi16(_mm256_unpackhi_epi8(vecNeighb3, vecZero), vec0));

			// Let's say:
			//		A = ((neighb0 * x1) + (neighb1 * x0))
			//		B = ((neighb2 * x1) + (neighb3 * x0))
			// Then:
			//		A = vec4, vec6
			//		B = vec5, vec7
			//
			// We cannot use _mm256_madd_epi16 to compute C and D because it operates on epi16 while A and B contain epu16 values

			// compute C = (y1 * A) >> 16
			vec0 = _mm256_mulhi_epu16(vecy1, vec4);
			vec1 = _mm256_mulhi_epu16(vecy1, vec6);

			// compute D = (y0 * B) >> 16
			vec2 = _mm256_mulhi_epu16(vecy0, vec5);
			vec3 = _mm256_mulhi_epu16(vecy0, vec7);

			// Compute R = (C + D)
			vec0 = _mm256_adds_epu16(vec0, vec2);
			vec1 = _mm256_adds_epu16(vec1, vec3);

			// Store the result
			_mm256_store_si256(reinterpret_cast<__m256i*>(&outPtr[i]), compv_avx2_packus_epi16(vec0, vec1));

			// move to next indices
			vecX0 = _mm256_add_epi32(vecX0, vecSfxTimes32);
			vecX1 = _mm256_add_epi32(vecX1, vecSfxTimes32);
			vecX2 = _mm256_add_epi32(vecX2, vecSfxTimes32);
			vecX3 = _mm256_add_epi32(vecX3, vecSfxTimes32);
		}
		outPtr += outStride;
		outYStart += sf_y;
	} while (outYStart < outYEnd);
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
