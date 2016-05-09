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
#include "compv/intrinsics/x86/compv_patch_intrin_sse.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void Moments0110_Intrin_SSE41(COMPV_ALIGNED(SSE) const uint8_t* top, COMPV_ALIGNED(SSE)const uint8_t* bottom, COMPV_ALIGNED(SSE)const int16_t* x, COMPV_ALIGNED(SSE) const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM, FMA3

	__m128i xmmTop, xmmBottom, xmmT, xmmB, xmm0, xmm1, xmm2, xmm3, xmm4, xmmX, xmmY, xmmZero;

	compv_scalar_t s01_ = *s01;
	compv_scalar_t s10_ = *s10;

	xmmZero = _mm_setzero_si128();

	// TODO(dmi): FMA3 for AVX

	// max(x|y) = 15 (patch radius)
	// max(top|bottom) = 255
	// -> (x|y) * (top|bottom +- (top|bottom)) is within [-32640, +32640] = [-0x7F80, +0x7F80]
	// -> we can use "_mm_mullo_epi16" without overflow

	// top, bottom, x, y are allocated with padding which means you can read up to align_fwd(count, 16)
	for (compv_scalar_t i = 0; i < count; i += 16) {
		xmmTop = _mm_load_si128((__m128i*)&top[i]);
		xmmBottom = _mm_load_si128((__m128i*)&bottom[i]);

		// s10_ += *x * (*top + *bottom) or (x * top) + (x * bottom)
		// s01_ += *y * (*top - *bottom) or (y * top) - (y * bottom)

		xmmT = _mm_unpacklo_epi8(xmmTop, xmmZero); // SSE4.1: _mm_cvtepi8_epi16
		xmmB = _mm_unpacklo_epi8(xmmBottom, xmmZero);
		xmmX = _mm_load_si128((__m128i*)&x[i]);
		xmmY = _mm_load_si128((__m128i*)&y[i]);
		
		xmm2 = _mm_mullo_epi16(xmmX, _mm_add_epi16(xmmT, xmmB));
		xmm3 = _mm_srai_epi32(_mm_unpacklo_epi16(xmmZero, xmm2), 16); // Convert from I16 to I32 while shifting in sign bits, ASM: use '_mm_cvtepi16_epi32' which is SSE4.1
		xmm4 = _mm_srai_epi32(_mm_unpackhi_epi16(xmmZero, xmm2), 16);
		xmm0 = _mm_hadd_epi32(xmm3, xmm4);

		xmm2 = _mm_mullo_epi16(xmmY, _mm_sub_epi16(xmmT, xmmB));
		xmm3 = _mm_srai_epi32(_mm_unpacklo_epi16(xmmZero, xmm2), 16);
		xmm4 = _mm_srai_epi32(_mm_unpackhi_epi16(xmmZero, xmm2), 16);
		xmm1 = _mm_hadd_epi32(xmm3, xmm4);

		xmm0 = _mm_hadd_epi32(xmm0, xmm1);

		xmmT = _mm_unpackhi_epi8(xmmTop, xmmZero);
		xmmB = _mm_unpackhi_epi8(xmmBottom, xmmZero);
		xmmX = _mm_load_si128((__m128i*)&x[i + 8]);
		xmmY = _mm_load_si128((__m128i*)&y[i + 8]);

		xmm2 = _mm_mullo_epi16(xmmX, _mm_add_epi16(xmmT, xmmB));
		xmm3 = _mm_srai_epi32(_mm_unpacklo_epi16(xmmZero, xmm2), 16);
		xmm4 = _mm_srai_epi32(_mm_unpackhi_epi16(xmmZero, xmm2), 16);
		xmm1 = _mm_hadd_epi32(xmm3, xmm4);

		xmm2 = _mm_mullo_epi16(xmmY, _mm_sub_epi16(xmmT, xmmB));
		xmm3 = _mm_srai_epi32(_mm_unpacklo_epi16(xmmZero, xmm2), 16);
		xmm4 = _mm_srai_epi32(_mm_unpackhi_epi16(xmmZero, xmm2), 16);
		xmm3 = _mm_hadd_epi32(xmm3, xmm4);

		xmm1 = _mm_hadd_epi32(xmm1, xmm3);

		xmm0 = _mm_hadd_epi32(xmm0, xmm1);
		s10_ += (int32_t)_mm_extract_epi32(xmm0, 0);
		s01_ += (int32_t)_mm_extract_epi32(xmm0, 1);
		s10_ += (int32_t)_mm_extract_epi32(xmm0, 2);
		s01_ += (int32_t)_mm_extract_epi32(xmm0, 3);
	}

	*s01 = s01_;
	*s10 = s10_;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
