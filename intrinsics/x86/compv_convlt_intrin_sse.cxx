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
#include "compv/intrinsics/x86/compv_convlt_intrin_sse.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// This function requires sizeof(float) = 4
// TODO(dmi): add ASM
void Convlt1_hz4_float_Intrin_SSE3(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	int i, j;
	__m128 xmmKernel, xmmFloats[4];
	__m128i xmmIn[4], xmmZero, xmm0;

	xmmZero = _mm_setzero_si128();
	xmmKernel = _mm_loadu_ps(hkern_ptr);

	for (j = 0; j < height; ++j) {
		/* 4 by 4 loop */
		for (i = 0; i < width - 4; i+=4) { // We can read rows up to (width + kernel_size) = (width + 4)
			// Load 4*4 U8
			xmmIn[0] = _mm_cvtsi32_si128(*((uint32_t*)in_ptr));
			xmmIn[1] = _mm_cvtsi32_si128(*((uint32_t*)(in_ptr + 1)));
			xmmIn[2] = _mm_cvtsi32_si128(*((uint32_t*)(in_ptr + 2)));
			xmmIn[3] = _mm_cvtsi32_si128(*((uint32_t*)(in_ptr + 3)));

			// Convert each 4U8 to 4F32
			xmmFloats[0] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmIn[0], xmmZero), xmmZero));
			xmmFloats[1] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmIn[1], xmmZero), xmmZero));
			xmmFloats[2] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmIn[2], xmmZero), xmmZero));
			xmmFloats[3] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmIn[3], xmmZero), xmmZero));

			// IN * KERNEL
			xmmFloats[0] = _mm_mul_ps(xmmFloats[0], xmmKernel);
			xmmFloats[1] = _mm_mul_ps(xmmFloats[1], xmmKernel);
			xmmFloats[2] = _mm_mul_ps(xmmFloats[2], xmmKernel);
			xmmFloats[3] = _mm_mul_ps(xmmFloats[3], xmmKernel);
			// SUM += H(IN * KERNEL)
			xmmFloats[0] = _mm_hadd_ps(xmmFloats[0], xmmFloats[1]);
			xmmFloats[2] = _mm_hadd_ps(xmmFloats[2], xmmFloats[3]);
			xmmFloats[0] = _mm_hadd_ps(xmmFloats[0], xmmFloats[2]);

			// Truncate to have same result as C++ implementation: out_ptr[x] = (uint8_t)floats[x];
			xmm0 = _mm_cvttps_epi32(xmmFloats[0]); // Convert to Int32 with truncation
			xmm0 = _mm_packs_epi32(xmm0, xmm0); // Convert to Int16
			xmm0 = _mm_packus_epi16(xmm0, xmm0); // Convert to Uint8 and saturate
			*((uint32_t*)out_ptr) = (uint32_t)_mm_cvtsi128_si32(xmm0);

			
			in_ptr += 4;
			out_ptr += 4;
		}

		/* 1 by 1 loop */
		for (; i < width; ++i) {
			xmmIn[0] = _mm_cvtsi32_si128(*((uint32_t*)in_ptr));
			xmmFloats[0] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmIn[0], xmmZero), xmmZero));
			xmmFloats[0] = _mm_mul_ps(xmmFloats[0], xmmKernel);
			xmmFloats[0] = _mm_hadd_ps(xmmFloats[0], xmmFloats[0]);
			xmmFloats[0] = _mm_hadd_ps(xmmFloats[0], xmmFloats[0]);
			out_ptr[0] = (uint8_t)_mm_cvtt_ss2si(xmmFloats[0]);

			in_ptr += 1;
			out_ptr += 1;
		}

		in_ptr += pad;
		out_ptr += pad;
	}
}

// This function requires sizeof(float) = 4
// TODO(dmi): add ASM
void Convlt1_hz8_float_Intrin_SSE3(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	int i, j;
	__m128 xmmKernelLow, xmmKernelHigh, xmmFloatsLow, xmmFloatsHigh;
	__m128i xmmInLow, xmmInHigh, xmmZero;

	xmmZero = _mm_setzero_si128();
	xmmKernelLow = _mm_loadu_ps(hkern_ptr);
	xmmKernelHigh = _mm_loadu_ps(hkern_ptr + 4);

	// C++ code produce slightly different result because of the precision
	for (j = 0; j < height; ++j) {
		/* 4 by 4 loop */
		for (i = 0; i < width; i += 1) { // We can read rows up to (width + kernel_size) = (width + 8)
			xmmInLow = _mm_cvtsi32_si128(*((uint32_t*)in_ptr));
			xmmInHigh = _mm_cvtsi32_si128(*((uint32_t*)(in_ptr + 4)));
			xmmFloatsLow = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmInLow, xmmZero), xmmZero));
			xmmFloatsHigh = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmInHigh, xmmZero), xmmZero));
			xmmFloatsLow = _mm_mul_ps(xmmFloatsLow, xmmKernelLow);
			xmmFloatsHigh = _mm_mul_ps(xmmFloatsHigh, xmmKernelHigh);
			xmmFloatsLow = _mm_hadd_ps(xmmFloatsLow, xmmFloatsHigh);
			xmmFloatsLow = _mm_hadd_ps(xmmFloatsLow, xmmFloatsLow);
			xmmFloatsLow = _mm_hadd_ps(xmmFloatsLow, xmmFloatsLow);
			out_ptr[0] = (uint8_t)_mm_cvtt_ss2si(xmmFloatsLow);

			in_ptr += 1;
			out_ptr += 1;
		}

		in_ptr += pad;
		out_ptr += pad;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
