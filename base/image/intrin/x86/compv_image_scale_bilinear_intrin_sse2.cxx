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

// This function requires outHeight and outStride to be multiple of #2
void CompVImageScaleBilinear_Intrin_SSE2(
	const uint8_t* inPtr, compv_uscalar_t inWidth, compv_uscalar_t inHeight, compv_uscalar_t inStride,
	COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outHeight, compv_uscalar_t outYStart, COMPV_ALIGNED(SSE) compv_uscalar_t outStride,
	compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("This code is for testing only");

#if 1
	compv_uscalar_t i, j, y;
	const uint8_t* inPtr_;
	int sf_x_ = static_cast<int>(sf_x);
	COMPV_ALIGN_SSE() const int32_t SFX[4][4] = {
		{ sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 },
		{ sf_x_ * 4, sf_x_ * 5, sf_x_ * 6, sf_x_ * 7 },
		{ sf_x_ * 8, sf_x_ * 9, sf_x_ * 10, sf_x_ * 11 },
		{ sf_x_ * 12, sf_x_ * 13, sf_x_ * 14, sf_x_ * 15 }
	};
	__m128i vecX0, vecX1, vecX2, vecX3, vecY0, vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3, vec0, vec1, vec2, vec3, vecYand0xff;
	__m128i vecret0, vecret1, vecret2, vecret3;
	const __m128i vec0xff = _mm_set1_epi32(0xff);
	const __m128i vec0x1 = _mm_set1_epi32(0x1);
	const __m128i vecInStride = _mm_set1_epi32(static_cast<int>(inStride));
	const __m128i vecInStridePlusOne = _mm_add_epi32(vecInStride, vec0x1);
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ * 16);
	const __m128i vecSFX0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[0]));
	const __m128i vecSFX1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[1]));
	const __m128i vecSFX2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[2]));
	const __m128i vecSFX3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[3]));
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));

	int nearestX;

	// TODO(dmi): next code is used to avoid "uninitialized local variable 'vecNeighbx' used" error message
	// must not use in ASM
	vecNeighb0 = vecNeighb1 = vecNeighb2 = vecNeighb3 = _mm_setzero_si128();

	// TODO(dmi): SS41 have _mm_insert_epi8 
	// FIXME: you can use 'vecSFX0' only and add 'vecSFX'

	vecY0 = _mm_setzero_si128();
	for (j = 0, y = 0; j < outHeight; ++j, y += sf_y) { // FIXME: use for (y ... (sf_y*outHeight) and remove j
		inPtr_ = (inPtr + ((y >> 8) * inStride)); // FIXME: use SIMD (vecY0) and remove y += sf_y
		vecYand0xff = _mm_set1_epi8((y & 0xff)); // FIXME: use SIMD (vecY0)
		vecX0 = vecSFX0, vecX1 = vecSFX1, vecX2 = vecSFX2, vecX3 = vecSFX3;
		for (i = 0; i < outWidth; i += 16) {

			// FIXME: when we have #4 low or #4 high we dont need the rest, just add 1 -> do not convert to epi16
			// FIXME: indices are packed as epi16 which means inWidth must  be < 0xffff
			// FIXME: "_mm_extract_epi32" and "_mm_insert_epi32" are SSE41

			vec0 = _mm_srli_epi32(vecX0, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 0); // (vecNeighb0, vecNeighb1) -> 0,1,0,1,0,1,0,1,0,1,0,1
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 0); // (vecNeighb2, vecNeighb3) -> 2,3,2,3,2,3,2,3
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 1);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 2);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 3);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 3);

			vec0 = _mm_srli_epi32(vecX1, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 4); // (vecNeighb0, vecNeighb1)
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 4); // (vecNeighb2, vecNeighb3)
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 5);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 5);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 6);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 6);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX]), 7);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, *reinterpret_cast<const uint16_t*>(&inPtr_[nearestX + inStride]), 7);

			// FIXME: '_mm_set1_epi8(0xff)' -> const and declare once

			vec0 = _mm_unpacklo_epi16(vecNeighb0, vecNeighb2); // 0,1,2,3,0,1,2,3
			vec1 = _mm_unpackhi_epi16(vecNeighb0, vecNeighb2); // 0,1,2,3,0,1,2,3
			vec0 = _mm_and_si128(vecX1, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			


			/**** Part - #0 ****/
			vec0 = _mm_srli_epi32(vecX0, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX0, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret0 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #1 ****/
			// compute indices
			vec0 = _mm_srli_epi32(vecX1, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX1, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret1 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #2 ****/
			vec0 = _mm_srli_epi32(vecX2, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX2, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret2 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #3 ****/
			vec0 = _mm_srli_epi32(vecX3, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX3, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret3 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** move to next indices ****/
			vecX0 = _mm_add_epi32(vecX0, vecSfxTimes16);
			vecX1 = _mm_add_epi32(vecX1, vecSfxTimes16);
			vecX2 = _mm_add_epi32(vecX2, vecSfxTimes16);
			vecX3 = _mm_add_epi32(vecX3, vecSfxTimes16);

			/**** Packs result and write to outPtr ****/
			vecret0 = _mm_packus_epi32(vecret0, vecret1);
			vecret1 = _mm_packus_epi32(vecret2, vecret3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), _mm_packus_epi16(vecret0, vecret1));
		}
		outPtr += outStride;
		vecY0 = _mm_add_epi32(vecY0, vecSFY);
	}
#endif

#if 0
	compv_uscalar_t i, j, x, y, nearestX, nearestY;
	int sf_x_ = static_cast<int>(sf_x);
	const uint8_t* inPtr_;
	COMPV_ALIGN_SSE() const int32_t SFX[4][4] = {
		{ sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 },
		{ sf_x_ * 4, sf_x_ * 5, sf_x_ * 6, sf_x_ * 7 },
		{ sf_x_ * 8, sf_x_ * 9, sf_x_ * 10, sf_x_ * 11 },
		{ sf_x_ * 12, sf_x_ * 13, sf_x_ * 14, sf_x_ * 15 }
	};
	__m128i vecX0, vecX1, vecX2, vecX3, vecY0, vec0, vec1, vec2, vec3;
	__m128i vecret0, vecret1, vecret2, vecret3;
	__m128i vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3;
	const __m128i vec0xff = _mm_set1_epi32(0xff);
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ * 16);
	const __m128i vecSFX0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[0]));
	const __m128i vecSFX1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[1]));
	const __m128i vecSFX2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[2]));
	const __m128i vecSFX3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[3]));
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));

	// TODO(dmi): next code is used to avoid "uninitialized local variable 'vecNeighbx' used" error message
	// must not use in ASM
	vecNeighb0 = vecNeighb1 = vecNeighb2 = vecNeighb3 = _mm_setzero_si128();

	vecY0 = _mm_setzero_si128();
	for (j = 0, y = 0; j < outHeight; ++j, y += sf_y) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		vecX0 = vecSFX0, vecX1 = vecSFX1, vecX2 = vecSFX2, vecX3 = vecSFX3;
		for (i = 0, x = 0; i < outWidth; i += 16) {

			// FIXME: check if shufle indexes can be > 16
			nearestX = (x >> 8);
			vec0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&SFX[3]));


#define INSERT_EPI32_SSE41(ii) \
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, static_cast<int32_t>(*(inPtr_ + nearestX)), ii); \
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, static_cast<int32_t>(*(inPtr_ + nearestX + 1)), ii); \
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, static_cast<int32_t>(*(inPtr_ + nearestX + inStride)), ii); \
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, static_cast<int32_t>(*(inPtr_ + nearestX + inStride + 1)), ii)		

			/* Part #0 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX0, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret0 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #1 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX1, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret1 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #2 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX2, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret2 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #3 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX3, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret3 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Packs result and write to outPtr ****/
			vecret0 = _mm_packus_epi32(vecret0, vecret1);
			vecret1 = _mm_packus_epi32(vecret2, vecret3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), _mm_packus_epi16(vecret0, vecret1));

			/**** move to next indices ****/
			vecX0 = _mm_add_epi32(vecX0, vecSfxTimes16);
			vecX1 = _mm_add_epi32(vecX1, vecSfxTimes16);
			vecX2 = _mm_add_epi32(vecX2, vecSfxTimes16);
			vecX3 = _mm_add_epi32(vecX3, vecSfxTimes16);
}
		vecY0 = _mm_add_epi32(vecY0, vecSFY);
		outPtr += outStride;
	}
#endif

	// FIXME: requiring height to be multiple of #2
#if 0
	compv_uscalar_t i, j, x, y, nearestX, nearestY;
	int sf_x_ = static_cast<int>(sf_x);
	const uint8_t* inPtr_;
	__m128i vecX, vecY, vec0, vec1, vec2, vec3;
	__m128i vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3;
	const __m128i vec0xff = _mm_set1_epi32(0xff);
	const __m128i vecSfxTimes4 = _mm_set1_epi32(sf_x_ * 4);
	const __m128i vecSFX = _mm_set_epi32(sf_x_ * 3, sf_x_ * 2, sf_x_ * 1, sf_x_ * 0);
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));
	const __m128i vecZero = _mm_setzero_si128();

	// TODO(dmi): next code is used to avoid "uninitialized local variable 'vecNeighbx' used" error message
	// must not use in ASM
	vecNeighb0 = vecNeighb1 = vecNeighb2 = vecNeighb3 = _mm_setzero_si128();

	vecY = _mm_setzero_si128();
	for (j = 0, y = 0; j < outHeight; ++j, y += sf_y) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		vecX = vecSFX;
		for (i = 0, x = 0; i < outWidth; i += 4) { // outStride is SSE aligned

		// FIXME: use _mm_insert_epi32 for SSE41

#define INSERT_EPI16_SSE2(ii) \
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, static_cast<int16_t>(*(inPtr_ + nearestX)), ii); \
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, static_cast<int16_t>(*(inPtr_ + nearestX + 1)), ii); \
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, static_cast<int16_t>(*(inPtr_ + nearestX + inStride)), ii); \
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, static_cast<int16_t>(*(inPtr_ + nearestX + inStride + 1)), ii)		

			
			nearestX = (x >> 8), INSERT_EPI16_SSE2(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI16_SSE2(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI16_SSE2(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI16_SSE2(3), x += sf_x;

			vecNeighb0 = _mm_unpacklo_epi16(vecNeighb0, vecZero);
			vecNeighb1 = _mm_unpacklo_epi16(vecNeighb1, vecZero);
			vecNeighb2 = _mm_unpacklo_epi16(vecNeighb2, vecZero);
			vecNeighb3 = _mm_unpacklo_epi16(vecNeighb3, vecZero);
			
			vec0 = _mm_and_si128(vecX, vec0xff);
			vec1 = _mm_and_si128(vecY, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vec0 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Packs result and write to outPtr ****/
			vec0 = _mm_packus_epi32(vec0, vec0);
			*reinterpret_cast<uint32_t*>(&outPtr[i]) = _mm_cvtsi128_si32(_mm_packus_epi16(vec0, vec0));

			/**** move to next indices ****/
			vecX = _mm_add_epi32(vecX, vecSfxTimes4);
		}
		vecY = _mm_add_epi32(vecY, vecSFY);
		outPtr += outStride;
	}
#endif

#if 0
	compv_uscalar_t i, j, x, y, nearestX0, nearestX1;
	unsigned neighb0, neighb1, neighb2, neighb3, x0, y0, x1, y1;
	compv_uscalar_t sf_x2 = sf_x << 1, sf_y2 = sf_y << 1, outStride2 = outStride << 1;
	const uint8_t *inPtr0, *inPtr1;
	__m128i vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3;

	outHeight &= -2; // height_mod = 0x1 (outHeight multiple of #2)

	// TODO(dmi): next code is used to avoid "uninitialized local variable 'vecNeighbx' used" error message
	// must not use in ASM
	vecNeighb0 = vecNeighb1 = vecNeighb2 = vecNeighb3 = _mm_setzero_si128();

	for (j = 0, y = 0; j < outHeight; j += 2, y += sf_y2) { // FIXME: -1 -> realy
		inPtr0 = (inPtr + ((y >> 8) * inStride));
		inPtr1 = (inPtr + (((y + sf_y) >> 8) * inStride));
		for (i = 0, x = 0; i < outWidth; i += 2, x += sf_x2) { // Do not check outofbound, caller checked 'outStride' is multiple of #2
			nearestX0 = (x >> 8);
			nearestX1 = (x >> 8);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr0[nearestX] | (inPtr1[nearestX] << 8), 0);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr0[nearestX + 1] | (inPtr1[nearestX + 1] << 8), 0);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr0[nearestX + inStride] | (inPtr1[nearestX + inStride] << 8), 0);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr0[nearestX + inStride + 1] | (inPtr1[nearestX + inStride + 1] << 8), 0);
			
#if 0
			// x = 0, y = 0
			nearestX = (x >> 8);
			neighb0 = *(inPtr0 + nearestX);
			neighb1 = *(inPtr0 + nearestX + 1);
			neighb2 = *(inPtr0 + nearestX + inStride);
			neighb3 = *(inPtr0 + nearestX + inStride + 1);
			x0 = x & 0xff;
			y0 = y & 0xff;
			x1 = 0xff - x0;
			y1 = 0xff - y0;
			outPtr[i] = (uint8_t)((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16);

			// x = 0, y = 1
			neighb0 = *(inPtr1 + nearestX);
			neighb1 = *(inPtr1 + nearestX + 1);
			neighb2 = *(inPtr1 + nearestX + inStride);
			neighb3 = *(inPtr1 + nearestX + inStride + 1);
			x0 = x & 0xff;
			y0 = (y + sf_y) & 0xff;
			x1 = 0xff - x0;
			y1 = 0xff - y0;
			outPtr[i + outStride] = (uint8_t)((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16);

			// x = 1, y = 0
			nearestX = ((x + sf_x) >> 8);
			neighb0 = *(inPtr0 + nearestX);
			neighb1 = *(inPtr0 + nearestX + 1);
			neighb2 = *(inPtr0 + nearestX + inStride);
			neighb3 = *(inPtr0 + nearestX + inStride + 1);
			x0 = (x + sf_x) & 0xff;
			y0 = y & 0xff;
			x1 = 0xff - x0;
			y1 = 0xff - y0;
			outPtr[i + 1] = (uint8_t)((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16);

			// x = 1, y = 1
			neighb0 = *(inPtr1 + nearestX);
			neighb1 = *(inPtr1 + nearestX + 1);
			neighb2 = *(inPtr1 + nearestX + inStride);
			neighb3 = *(inPtr1 + nearestX + inStride + 1);
			x0 = (x + sf_x) & 0xff;
			y0 = (y + sf_y ) & 0xff;
			x1 = 0xff - x0;
			y1 = 0xff - y0;
			outPtr[i + outStride + 1] = (uint8_t)((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16);
#endif
		}		
		outPtr += outStride2;
	}
#endif

#if 0
	compv_uscalar_t i, j, x, y, nearestX, nearestY;
	int sf_x_ = static_cast<int>(sf_x);
	const uint8_t* inPtr_;
	COMPV_ALIGN_SSE() const int32_t SFX[4][4] = {
		{ sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 },
		{ sf_x_ * 4, sf_x_ * 5, sf_x_ * 6, sf_x_ * 7 },
		{ sf_x_ * 8, sf_x_ * 9, sf_x_ * 10, sf_x_ * 11 },
		{ sf_x_ * 12, sf_x_ * 13, sf_x_ * 14, sf_x_ * 15 }
	};
	__m128i vecX0, vecX1, vecX2, vecX3, vecY0, vec0, vec1, vec2, vec3;
	__m128i vecret0, vecret1, vecret2, vecret3;
	__m128i vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3;
	const __m128i vec0xff = _mm_set1_epi32(0xff);
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ * 16);
	const __m128i vecSFX0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[0]));
	const __m128i vecSFX1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[1]));
	const __m128i vecSFX2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[2]));
	const __m128i vecSFX3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[3]));
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));

	// TODO(dmi): next code is used to avoid "uninitialized local variable 'vecNeighbx' used" error message
	// must not use in ASM
	vecNeighb0 = vecNeighb1 = vecNeighb2 = vecNeighb3 = _mm_setzero_si128();

	vecY0 = _mm_setzero_si128();
	for (j = 0, y = 0; j < outHeight; ++j, y += sf_y) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		vecX0 = vecSFX0, vecX1 = vecSFX1, vecX2 = vecSFX2, vecX3 = vecSFX3;
		for (i = 0, x = 0; i < outWidth; i += 16) {

#define INSERT_EPI32_SSE41(ii) \
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, static_cast<int32_t>(*(inPtr_ + nearestX)), ii); \
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, static_cast<int32_t>(*(inPtr_ + nearestX + 1)), ii); \
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, static_cast<int32_t>(*(inPtr_ + nearestX + inStride)), ii); \
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, static_cast<int32_t>(*(inPtr_ + nearestX + inStride + 1)), ii)		

			/* Part #0 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX0, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret0 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #1 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX1, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret1 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #2 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX2, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret2 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #3 */
			nearestX = (x >> 8), INSERT_EPI32_SSE41(0), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(1), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(2), x += sf_x;
			nearestX = (x >> 8), INSERT_EPI32_SSE41(3), x += sf_x;
			vec0 = _mm_and_si128(vecX3, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret3 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Packs result and write to outPtr ****/
			vecret0 = _mm_packus_epi32(vecret0, vecret1);
			vecret1 = _mm_packus_epi32(vecret2, vecret3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), _mm_packus_epi16(vecret0, vecret1));

			/**** move to next indices ****/
			vecX0 = _mm_add_epi32(vecX0, vecSfxTimes16);
			vecX1 = _mm_add_epi32(vecX1, vecSfxTimes16);
			vecX2 = _mm_add_epi32(vecX2, vecSfxTimes16);
			vecX3 = _mm_add_epi32(vecX3, vecSfxTimes16);			
		}
		vecY0 = _mm_add_epi32(vecY0, vecSFY);
		outPtr += outStride;
	}
#endif

#if 0
	compv_uscalar_t i, j, y;
	const uint8_t* inPtr_;
	int sf_x_ = static_cast<int>(sf_x);
	COMPV_ALIGN_SSE() const int32_t SFX[4][4] = {
		{ sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 },
		{ sf_x_ * 4, sf_x_ * 5, sf_x_ * 6, sf_x_ * 7 },
		{ sf_x_ * 8, sf_x_ * 9, sf_x_ * 10, sf_x_ * 11 },
		{ sf_x_ * 12, sf_x_ * 13, sf_x_ * 14, sf_x_ * 15 }
	};
	__m128i vecX0, vecX1, vecX2, vecX3, vecY0, vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3, vec0, vec1, vec2, vec3;
	__m128i vecret0, vecret1, vecret2, vecret3;
	const __m128i vec0xff = _mm_set1_epi32(0xff);
	const __m128i vec0x1 = _mm_set1_epi32(0x1);
	const __m128i vecInStride = _mm_set1_epi32(static_cast<int>(inStride));
	const __m128i vecInStridePlusOne = _mm_add_epi32(vecInStride, vec0x1);
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ * 16);
	const __m128i vecSFX0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[0]));
	const __m128i vecSFX1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[1]));
	const __m128i vecSFX2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[2]));
	const __m128i vecSFX3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[3]));
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));
	int nearestX;

	// TODO(dmi): next code is used to avoid "uninitialized local variable 'vecNeighbx' used" error message
	// must not use in ASM
	vecNeighb0 = vecNeighb1 = vecNeighb2 = vecNeighb3 = _mm_setzero_si128();

	// TODO(dmi): SS41 have _mm_insert_epi8 
	// FIXME: you can use 'vecSFX0' only and add 'vecSFX'

	vecY0 = _mm_setzero_si128();
	for (j = 0, y = 0; j < outHeight; ++j, y += sf_y) { // FIXME: use for (y ... (sf_y*outHeight) and remove j
		inPtr_ = (inPtr + ((y >> 8) * inStride)); // FIXME: use SIMD (vecY0) and remove y += sf_y
		vecX0 = vecSFX0, vecX1 = vecSFX1, vecX2 = vecSFX2, vecX3 = vecSFX3;
		for (i = 0; i < outWidth; i += 16) {

			// FIXME: when we have #4 low or #4 high we dont need the rest, just add 1 -> do not convert to epi16
			// FIXME: indices are packed as epi16 which means inWidth must  be < 0xffff
			// FIXME: "_mm_extract_epi32" and "_mm_insert_epi32" are SSE41

			/**** Part - #0 ****/
			vec0 = _mm_srli_epi32(vecX0, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX0, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret0 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #1 ****/
			// compute indices
			vec0 = _mm_srli_epi32(vecX1, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX1, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret1 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #2 ****/
			vec0 = _mm_srli_epi32(vecX2, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX2, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret2 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #3 ****/
			vec0 = _mm_srli_epi32(vecX3, 8);
			nearestX = _mm_extract_epi32(vec0, 0);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 0);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 0);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 0);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 0);
			nearestX = _mm_extract_epi32(vec0, 1);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 1);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 1);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 1);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 1);
			nearestX = _mm_extract_epi32(vec0, 2);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 2);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 2);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 2);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 2);
			nearestX = _mm_extract_epi32(vec0, 3);
			vecNeighb0 = _mm_insert_epi32(vecNeighb0, inPtr_[nearestX], 3);
			vecNeighb1 = _mm_insert_epi32(vecNeighb1, inPtr_[nearestX + 1], 3);
			vecNeighb2 = _mm_insert_epi32(vecNeighb2, inPtr_[nearestX + inStride], 3);
			vecNeighb3 = _mm_insert_epi32(vecNeighb3, inPtr_[nearestX + inStride + 1], 3);
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX3, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret3 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** move to next indices ****/
			vecX0 = _mm_add_epi32(vecX0, vecSfxTimes16);
			vecX1 = _mm_add_epi32(vecX1, vecSfxTimes16);
			vecX2 = _mm_add_epi32(vecX2, vecSfxTimes16);
			vecX3 = _mm_add_epi32(vecX3, vecSfxTimes16);

			/**** Packs result and write to outPtr ****/
			vecret0 = _mm_packus_epi32(vecret0, vecret1);
			vecret1 = _mm_packus_epi32(vecret2, vecret3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), _mm_packus_epi16(vecret0, vecret1));
		}
		outPtr += outStride;
		vecY0 = _mm_add_epi32(vecY0, vecSFY);
	}
#endif

#if 0
	compv_uscalar_t i, j, x, y, nearestX, nearestY;
	unsigned int neighb0, neighb1, neighb2, neighb3, x0, y0, x1, y1;
	const uint8_t* inPtr_;

	for (j = 0, y = 0; j < outHeight; ++j, y +=sf_y) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		for (i = 0, x = 0; i < outWidth; ++i, x += sf_x) {
			nearestX = (x >> 8); // nearest x-point

			neighb0 = inPtr_[nearestX];
			neighb1 = inPtr_[nearestX + 1];
			neighb2 = inPtr_[nearestX + inStride];
			neighb3 = inPtr_[nearestX + inStride + 1];

			x0 = x & 0xff;
			y0 = y & 0xff;
			x1 = 0xff - x0;
			y1 = 0xff - y0;

			outPtr[i] = static_cast<uint8_t>((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16); // no need for saturation after >> 16
		}
	}
#endif
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
