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
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

// inPtr doesn't need to be aligned
// outPtr must be aligned
// outStride must be aligned
// image width and height must be <= SHRT_MAX
void scaleBilinearKernel11_Aligned_SSSE3(const uint8_t* inPtr, COMV_ALIGNED(SSE) uint8_t* outPtr, vcomp_scalar_t inHeight, vcomp_scalar_t inWidth, vcomp_scalar_t inStride, vcomp_scalar_t outHeight, vcomp_scalar_t outWidth, vcomp_scalar_t outStride, vcomp_scalar_t sf_x, vcomp_scalar_t sf_y)
{
#if 0
	vcomp_scalar_t x, y, x0, y0, x1, y1, nearestX, nearestY, weight0, weight1, weight2, weight3;
	uint8_t neighb0, neighb1, neighb2, neighb3;
	const uint8_t* inPtr_;
	uint8_t* outPtr_;

	__m128i xmmNeighb[2], xmmX, xmmY, xmmX0, xmmX1, xmmY0, xmmY1, xmmSFX, xmmSFY, xmmNearstX, xmmNearstY, xmmZeros, xmmOnes, xmm0123to15;

	_mm_store_si128(&xmmSFX, _mm_set1_epi16(sf_x));
	_mm_store_si128(&xmmSFY, _mm_set1_epi16(sf_y));
	_mm_store_si128(&xmmZeros, _mm_setzero_si128());
	_mm_store_si128(&xmmOnes, _mm_set1_epi16(1));
	_mm_store_si128(&xmm0123to15, _mm_set_epi16(15, 14, 13, 12, 11, 10, ));

	// There is no support for max32 ins SSE3 (only SSE4), so we're using max16 which means the image width must be <= SHRT_MAX (+3k)

	for (vcomp_scalar_t j = 0; j < outHeight; ++j) {
		y = j * sf_y;
		// nearestY = y >> 8
		_mm_store_si128(&xmmNearstY, _mm_set1_epi16(y >> 8)); // nearest y-point
		// nearestY = COMPV_MATH_CLIP2(nearestY, inHeight - 2);
		_mm_store_si128(&xmmNearstY, _mm_min_epi16(_mm_max_epi16(xmmZeros, xmmNearstY), _mm_set1_epi16(inHeight - 1)));

		inPtr_ = (inPtr + (nearestY * inStride));
		outPtr_ = (outPtr + (j * outStride));
		for (vcomp_scalar_t i = 0; i < outWidth; i += 16) {
			// x = [i...] * sf_x;
			_mm_store_si128(&xmmX, _mm_set1_epi16(i));
			_mm_add_epi16(xmmX, _mm_set1_)
			_mm_store_si128(&xmmX, _mm_mullo_epi16(xmmX, xmmSFX));
			// nearestX = (x >> 8)
			_mm_store_si128(&xmmNearstX, _mm_set1_epi16(x >> 8)); // nearest x-point
			_mm_store_si128(&xmmNearstX, _mm_add_epi16(xmmNearstX, xmmOnes));
			// nearestX = COMPV_MATH_CLIP2(nearestX, inWidth - 1);
			_mm_store_si128(&xmmNearstX, _mm_min_epi16(_mm_max_epi16(xmmZeros, xmmNearstX), _mm_set1_epi16(inWidth - 1)));

			_mm_storeu_si128(xmmNeighb, _mm_load_si128((__m128i*)&inPtr_[nearestX]));
			_mm_storeu_si128(xmmNeighb + 1, _mm_load_si128((__m128i*)&inPtr_[nearestX + inStride]));

			_mm_store_si128(&xmmX0, _mm_set1_epi32((nearestX << 8)));

			x0 = (x - (nearestX << 8));
			x1 = 256 - x0;
			y0 = (y - (nearestY << 8));
			y1 = 256 - y0;

			weight0 = x1 * y1;
			weight1 = x0 * y1;
			weight2 = x1 * y0;
			weight3 = x0 * y0;

			outPtr_[i] = (uint8_t)((neighb0 * weight0 + neighb1 * weight1 + neighb2 * weight2 + neighb3 * weight3) >> 16);
		}
	}
#endif
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
