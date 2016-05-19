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

// Function not used
// inPtr doesn't need to be aligned
// outPtr doesn't need to be aligned
// outStride must be aligned
void ScaleBilinear_Intrin_SSE41(const uint8_t* inPtr, COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t inHeight, compv_uscalar_t inWidth, compv_uscalar_t inStride, COMPV_ALIGNED(SSE) compv_uscalar_t outHeight, compv_uscalar_t outWidth, compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // C++ code is faster
	compv_uscalar_t i, j, y, nearestY, maxI = ((outWidth + 3) & -4), outPad = (outStride - maxI);
	const uint8_t* inPtr_;
	COMPV_ALIGN_SSE() int32_t xmmNearestX[4];
	COMPV_ALIGN_SSE() uint8_t xmmNeighb[16];

	__m128i xmmTempNearestX, xmmX, xmmY, xmmX0, xmmX1, xmmY0, xmmY1, xmmSFX, xmmOnes, xmmZeros, xmmFF, xmmInStride, xmmXStart, xmmNeighb0, xmmNeighb1, xmmNeighb2, xmmNeighb3;
	
	xmmSFX = _mm_set1_epi32((int)(sf_x * 4));
	xmmOnes = _mm_set1_epi32(1);
	xmmZeros = _mm_setzero_si128();
	xmmInStride = _mm_set1_epi32((int)inStride);
	xmmFF = _mm_set1_epi32((int)0xFF);
	xmmXStart = _mm_set_epi32((int)sf_x * 3, (int)sf_x *2, (int)sf_x * 1, (int)sf_x * 0);

	for (j = 0, y = 0; j < outHeight; ++j) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));

		xmmY = _mm_set1_epi32((int)y);

		xmmX = xmmXStart;
		for (i = 0; i < outWidth; i+= 4) {
			// nearestX = (x >> 8); // nearest x-point
			_mm_store_si128((__m128i*)xmmNearestX, _mm_srli_epi32(xmmX, 8));

			xmmNeighb[0] = inPtr_[xmmNearestX[0]];
			xmmNeighb[1] = inPtr_[xmmNearestX[1]];
			xmmNeighb[2] = inPtr_[xmmNearestX[2]];
			xmmNeighb[3] = inPtr_[xmmNearestX[3]];
			xmmNeighb[4] = inPtr_[xmmNearestX[0] + 1];
			xmmNeighb[5] = inPtr_[xmmNearestX[1] + 1];
			xmmNeighb[6] = inPtr_[xmmNearestX[2] + 1];
			xmmNeighb[7] = inPtr_[xmmNearestX[3] + 1];
			xmmNeighb[8] = inPtr_[xmmNearestX[0] + inStride];
			xmmNeighb[9] = inPtr_[xmmNearestX[1] + inStride];
			xmmNeighb[10] = inPtr_[xmmNearestX[2] + inStride];
			xmmNeighb[11] = inPtr_[xmmNearestX[3] + inStride];
			xmmNeighb[12] = inPtr_[xmmNearestX[0] + inStride + 1];
			xmmNeighb[13] = inPtr_[xmmNearestX[1] + inStride + 1];
			xmmNeighb[14] = inPtr_[xmmNearestX[2] + inStride + 1];
			xmmNeighb[15] = inPtr_[xmmNearestX[3] + inStride + 1];
			xmmTempNearestX = _mm_load_si128((__m128i*)xmmNeighb);

			// x0 = x & 0xff;
			xmmX0 = _mm_and_si128(xmmX, xmmFF);
			// x1 = 255 - x0;
			xmmX1 = _mm_sub_epi32(xmmFF, xmmX0);
			// y0 = y & 0xff;
			xmmY0 = _mm_and_si128(xmmY, xmmFF);
			// y1 = 255 - y0;
			xmmY1 = _mm_sub_epi32(xmmFF, xmmY0);

			xmmNeighb1 = _mm_unpackhi_epi16(_mm_unpacklo_epi8(xmmTempNearestX, xmmZeros), xmmZeros);
			xmmNeighb3 = _mm_unpackhi_epi16(_mm_unpackhi_epi8(xmmTempNearestX, xmmZeros), xmmZeros);
			xmmNeighb2 = _mm_unpacklo_epi16(_mm_unpackhi_epi8(xmmTempNearestX, xmmZeros), xmmZeros);
			xmmNeighb0 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmTempNearestX, xmmZeros), xmmZeros);
			

			// S = y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1 ) + (neighb3 * x0))
			xmmNeighb1 = _mm_mullo_epi32(xmmNeighb1, xmmX0);
			xmmNeighb3 = _mm_mullo_epi32(xmmNeighb3, xmmX0);
			xmmNeighb0 = _mm_mullo_epi32(xmmNeighb0, xmmX1);
			xmmNeighb2 = _mm_mullo_epi32(xmmNeighb2, xmmX1);
			xmmNeighb0 = _mm_mullo_epi32(_mm_add_epi32(xmmNeighb0, xmmNeighb1), xmmY1);			
			xmmNeighb2 = _mm_mullo_epi32(_mm_add_epi32(xmmNeighb2, xmmNeighb3), xmmY0);
			xmmNeighb0 = _mm_srli_epi32(_mm_add_epi32(xmmNeighb0, xmmNeighb2), 16);
			xmmNeighb0 = _mm_packs_epi32(xmmNeighb0, xmmNeighb0);
			xmmNeighb0 = _mm_packus_epi16(xmmNeighb0, xmmNeighb0);
			*((uint32_t*)outPtr) = _mm_cvtsi128_si32(xmmNeighb0);

			// x += sf_x;
			xmmX = _mm_add_epi32(xmmX, xmmSFX);
			
			outPtr += 4;
		}

		y += sf_y;
		outPtr += outPad;
	}
}

// Function not used
// nearestX, nearestY, x0, x1, y0 and y1 are aligned temp memories allocated in C++ to cleanup ASM code. Up to the ASM code to fill them.
// These memories contain N int32_t values with N aligned on 32 bytes.
// outPtr doesn't need to be aligned
void ScaleBilinearGrayscale_Intrin_SSE41(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t inStride, compv_uscalar_t outHeight, compv_uscalar_t outWidth, COMPV_ALIGNED(SSE) compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y, COMPV_ALIGNED(SSE) int32_t *nearestX, COMPV_ALIGNED(SSE) int32_t *nearestY, COMPV_ALIGNED(SSE) int32_t *x0, COMPV_ALIGNED(SSE) int32_t *y0, COMPV_ALIGNED(SSE) int32_t *x1, COMPV_ALIGNED(SSE) int32_t *y1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // C++ code is faster
	compv_uscalar_t i, j;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmmInStride, xmmFF, xmmNeighb0, xmmNeighb1, xmmNeighb2, xmmNeighb3, xmmZeros;
	const uint8_t* inPtr_;
	COMPV_ALIGN_SSE() uint8_t xmmNeighb[16];

	xmmZeros = _mm_setzero_si128();
	xmmFF = _mm_set1_epi32((int)0xFF);
	xmmInStride = _mm_set1_epi32((int)inStride);

	// Compute Y0 and Y1
	xmm1 = _mm_set1_epi32((int)sf_y * 4);
	xmm2 = _mm_set_epi32((int)sf_y * 3, (int)sf_y * 2, (int)sf_y * 1, (int)sf_y * 0);
	for (j = 0; j < outHeight; j += 4) { // outHeight doesn't need to be aligned. nearestY, y0 and y1 contains N samples with N aligned.
		_mm_store_si128((__m128i*)(nearestY + j), _mm_mullo_epi32(_mm_srli_epi32(xmm2, 8), xmmInStride));
		_mm_store_si128(&xmm3, _mm_and_si128(xmm2, xmmFF));
		_mm_store_si128((__m128i*)(y0 + j), xmm3);
		_mm_store_si128((__m128i*)(y1 + j), _mm_sub_epi32(xmmFF, xmm3));
		_mm_store_si128(&xmm2, _mm_add_epi32(xmm2, xmm1));
	}

	// Compute X0 and X1
	xmm1 = _mm_set1_epi32((int)sf_x * 4);
	xmm2 = _mm_set_epi32((int)sf_x * 3, (int)sf_x * 2, (int)sf_x * 1, (int)sf_x * 0);
	for (i = 0; i < outWidth; i += 4) { // outWidth doesn't need to be aligned. nearestX, x0 and x1 contains N samples with N aligned.
		_mm_store_si128((__m128i*)(nearestX + i), _mm_srli_epi32(xmm2, 8));
		_mm_store_si128(&xmm3, _mm_and_si128(xmm2, xmmFF));
		_mm_store_si128((__m128i*)(x0 + i), xmm3);
		_mm_store_si128((__m128i*)(x1 + i), _mm_sub_epi32(xmmFF, xmm3));
		_mm_store_si128(&xmm2, _mm_add_epi32(xmm2, xmm1));
	}

	// Compute results
	for (j = 0; j < outHeight; ++j) {
		inPtr_ = (inPtr + nearestY[j]);
		xmm2 = _mm_set1_epi32(y0[j]);
		xmm3 = _mm_set1_epi32(y1[j]);
		for (i = 0; i < outWidth; i += 4) { // outStride is aligned
			xmm0 = _mm_load_si128((__m128i*)(x0 + i));
			xmm1 = _mm_load_si128((__m128i*)(x1 + i));
			
			//  TODO(dmi): nearestX[0] + 1 -> use SIMD and store in same dest
			
			xmmNeighb[0] = inPtr_[nearestX[i + 0]];
			xmmNeighb[1] = inPtr_[nearestX[i + 1]];
			xmmNeighb[2] = inPtr_[nearestX[i + 2]];
			xmmNeighb[3] = inPtr_[nearestX[i + 3]];
			xmmNeighb[4] = inPtr_[nearestX[i + 0] + 1];
			xmmNeighb[5] = inPtr_[nearestX[i + 1] + 1];
			xmmNeighb[6] = inPtr_[nearestX[i + 2] + 1];
			xmmNeighb[7] = inPtr_[nearestX[i + 3] + 1];
			xmmNeighb[8] = inPtr_[nearestX[i + 0] + inStride];
			xmmNeighb[9] = inPtr_[nearestX[i + 1] + inStride];
			xmmNeighb[10] = inPtr_[nearestX[i + 2] + inStride];
			xmmNeighb[11] = inPtr_[nearestX[i + 3] + inStride];
			xmmNeighb[12] = inPtr_[nearestX[i + 0] + inStride + 1];
			xmmNeighb[13] = inPtr_[nearestX[i + 1] + inStride + 1];
			xmmNeighb[14] = inPtr_[nearestX[i + 2] + inStride + 1];
			xmmNeighb[15] = inPtr_[nearestX[i + 3] + inStride + 1];
			
			xmm4 = _mm_load_si128((__m128i*)xmmNeighb);

			xmmNeighb0 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(xmm4, xmmZeros), xmmZeros);
			xmmNeighb1 = _mm_unpackhi_epi16(_mm_unpacklo_epi8(xmm4, xmmZeros), xmmZeros);
			xmmNeighb2 = _mm_unpacklo_epi16(_mm_unpackhi_epi8(xmm4, xmmZeros), xmmZeros);			
			xmmNeighb3 = _mm_unpackhi_epi16(_mm_unpackhi_epi8(xmm4, xmmZeros), xmmZeros);
			

			// S = y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1 ) + (neighb3 * x0))
			xmmNeighb1 = _mm_mullo_epi32(xmmNeighb1, xmm0);
			xmmNeighb3 = _mm_mullo_epi32(xmmNeighb3, xmm0);
			xmmNeighb0 = _mm_mullo_epi32(xmmNeighb0, xmm1);
			xmmNeighb2 = _mm_mullo_epi32(xmmNeighb2, xmm1);
			xmmNeighb0 = _mm_mullo_epi32(_mm_add_epi32(xmmNeighb0, xmmNeighb1), xmm3);
			xmmNeighb2 = _mm_mullo_epi32(_mm_add_epi32(xmmNeighb2, xmmNeighb3), xmm2);
			xmmNeighb0 = _mm_srli_epi32(_mm_add_epi32(xmmNeighb0, xmmNeighb2), 16);
			xmmNeighb0 = _mm_packs_epi32(xmmNeighb0, xmmNeighb0);
			xmmNeighb0 = _mm_packus_epi16(xmmNeighb0, xmmNeighb0);
			*((uint32_t*)&outPtr[i]) = _mm_cvtsi128_si32(xmmNeighb0);
		}
		outPtr += outStride;
	}
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
