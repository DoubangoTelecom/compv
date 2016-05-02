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

COMPV_NAMESPACE_BEGIN()

// This function requires sizeof(float)=4
void Convlt1_hz4_float_Intrin_SSE3(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr)
{
	int i, j;
	__m128 xmmKernel, xmmFloats[4];
	__m128i xmmIn[4], xmmZero;

	xmmZero = _mm_setzero_si128();
	xmmKernel = _mm_loadu_ps(hkern_ptr);

#if 1
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
			
			// FIXME
			out_ptr[0] = (uint8_t)((float(&)[4])xmmFloats[0])[0];
			out_ptr[1] = (uint8_t)((float(&)[1])xmmFloats[0])[1];
			out_ptr[2] = (uint8_t)((float(&)[2])xmmFloats[0])[2];
			out_ptr[3] = (uint8_t)((float(&)[3])xmmFloats[0])[3];
			// *((uint32_t*)out_ptr) = (uint32_t)_mm_cvtsi128_si32(_mm_cvtps_epi32(xmmFloats[0]));
			
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
			out_ptr[0] = (uint8_t)_mm_cvtss_f32(xmmFloats[0]);

			in_ptr += 1;
			out_ptr += 1;
		}

		in_ptr += pad;
		out_ptr += pad;
	}
#else
	int col;
	float sum;
	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; i += 1) { // ignore others
			sum = 0;
			for (col = 0; col < 4; ++col) {
				sum += in_ptr[col] * hkern_ptr[col];
			}
			*out_ptr = (uint8_t)sum;
			++in_ptr;
			++out_ptr;
		}
		in_ptr += pad;
		out_ptr += pad;
	}
#endif
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
