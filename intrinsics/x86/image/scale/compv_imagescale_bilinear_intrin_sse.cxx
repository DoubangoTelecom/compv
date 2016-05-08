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
#include "compv/intrinsics/x86/image/scale/compv_imagescale_bilinear_intrin_sse.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/compv_simd_globals.h"
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

// inPtr doesn't need to be aligned
// outPtr doesn't need to be aligned
void ScaleBilinear_Intrin_SSE2(const uint8_t* inPtr, uint8_t* outPtr, compv_scalar_t inHeight, compv_scalar_t inWidth, compv_scalar_t inStride, compv_scalar_t outHeight, compv_scalar_t outWidth, compv_scalar_t outPad, compv_scalar_t sf_x, compv_scalar_t sf_y)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
	compv_scalar_t i, j, y, nearestY;
	const uint8_t* inPtr_;

	__m128i xmmX, xmmY, xmmX0, xmmX1, xmmY0, xmmY1, xmmSFX, xmmOnes, xmmZeros, xmmFF, xmmInStride, xmmXStart, xmmNeighb0, xmmNeighb1, xmmNeighb2, xmmNeighb3;
	COMPV_ALIGN_DEFAULT() int32_t xmmNearestX[4];
	COMPV_ALIGN_DEFAULT() int32_t xmmTempNearestX[4];
	COMPV_ALIGN_DEFAULT() uint8_t xmmNeighb[16];
	
	xmmSFX = _mm_set1_epi32((int)(sf_x * 4));
	xmmOnes = _mm_set1_epi32(1);
	xmmZeros = _mm_setzero_si128();
	xmmInStride = _mm_set1_epi32((int)inStride);
	xmmFF = _mm_set1_epi32((int)0xFF);
	xmmXStart = _mm_set_epi32((int)sf_x * 3, (int)sf_x *2, (int)sf_x * 1, (int)sf_x * 0);
	
	outPad += (outWidth & 3);

	for (j = 0, y = 0; j < outHeight; ++j) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));

		xmmY = _mm_set1_epi32((int)y);

		xmmX = xmmXStart;
		i = outWidth;
		while (i > 3) {
			// nearestX = (x >> 8); // nearest x-point
			_mm_store_si128((__m128i*)xmmNearestX, _mm_srli_epi32(xmmX, 8));

			// neighb0 = inPtr_[nearestX];
			xmmNeighb[0] = inPtr_[xmmNearestX[0]];
			xmmNeighb[1] = inPtr_[xmmNearestX[1]];
			xmmNeighb[2] = inPtr_[xmmNearestX[2]];
			xmmNeighb[3] = inPtr_[xmmNearestX[3]];
			xmmNeighb0 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_load_si128((const __m128i*)xmmNeighb), xmmZeros), xmmZeros);

			// neighb1 = inPtr_[nearestX + 1];
			_mm_store_si128((__m128i*)xmmTempNearestX, _mm_add_epi32(_mm_load_si128((const __m128i*)xmmNearestX), xmmOnes));
			xmmNeighb[0] = inPtr_[xmmTempNearestX[0]];
			xmmNeighb[1] = inPtr_[xmmTempNearestX[1]];
			xmmNeighb[2] = inPtr_[xmmTempNearestX[2]];
			xmmNeighb[3] = inPtr_[xmmTempNearestX[3]];
			xmmNeighb1 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_load_si128((const __m128i*)xmmNeighb), xmmZeros), xmmZeros);

			// neighb2 = inPtr_[nearestX + inStride];
			_mm_store_si128((__m128i*)xmmTempNearestX, _mm_add_epi32(_mm_load_si128((const __m128i*)xmmNearestX), xmmInStride));
			xmmNeighb[0] = inPtr_[xmmTempNearestX[0]];
			xmmNeighb[1] = inPtr_[xmmTempNearestX[1]];
			xmmNeighb[2] = inPtr_[xmmTempNearestX[2]];
			xmmNeighb[3] = inPtr_[xmmTempNearestX[3]];
			xmmNeighb2 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_load_si128((const __m128i*)xmmNeighb), xmmZeros), xmmZeros);

			// neighb3 = inPtr_[nearestX + inStride + 1];
			_mm_store_si128((__m128i*)xmmTempNearestX, _mm_add_epi32(_mm_load_si128((const __m128i*)xmmTempNearestX), xmmOnes));
			xmmNeighb[0] = inPtr_[xmmTempNearestX[0]];
			xmmNeighb[1] = inPtr_[xmmTempNearestX[1]];
			xmmNeighb[2] = inPtr_[xmmTempNearestX[2]];
			xmmNeighb[3] = inPtr_[xmmTempNearestX[3]];
			xmmNeighb3 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_load_si128((const __m128i*)xmmNeighb), xmmZeros), xmmZeros);

			// x0 = x & 0xff;
			xmmX0 = _mm_and_si128(xmmX, xmmFF);
			// y0 = y & 0xff;
			xmmY0 = _mm_and_si128(xmmY, xmmFF);
			// x1 = 255 - x0;
			xmmX1 = _mm_sub_epi32(xmmFF, xmmX0);
			// y1 = 255 - y0;
			xmmY1 = _mm_sub_epi32(xmmFF, xmmY0);

			// S = y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1 ) + (neighb3 * x0))
			xmmNeighb0 = _mm_mullo_epi32_SSE2(xmmNeighb0, xmmX1);
			xmmNeighb1 = _mm_mullo_epi32_SSE2(xmmNeighb1, xmmX0);
			xmmNeighb0 = _mm_mullo_epi32_SSE2(_mm_add_epi32(xmmNeighb0, xmmNeighb1), xmmY1);
			xmmNeighb2 = _mm_mullo_epi32_SSE2(xmmNeighb2, xmmX1);
			xmmNeighb3 = _mm_mullo_epi32_SSE2(xmmNeighb3, xmmX0);
			xmmNeighb2 = _mm_mullo_epi32_SSE2(_mm_add_epi32(xmmNeighb2, xmmNeighb3), xmmY0);
			xmmNeighb0 = _mm_srli_epi32(_mm_add_epi32(xmmNeighb0, xmmNeighb2), 16);
			xmmNeighb0 = _mm_packs_epi32(xmmNeighb0, xmmNeighb0);
			xmmNeighb0 = _mm_packus_epi16(xmmNeighb0, xmmNeighb0);
			*((uint32_t*)outPtr) = _mm_cvtsi128_si32(xmmNeighb0);

			// x += sf_x;
			xmmX = _mm_add_epi32(xmmX, xmmSFX);

			i -= 4;
			outPtr += 4;
		}

		y += sf_y;
		outPtr += outPad;
	}
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
