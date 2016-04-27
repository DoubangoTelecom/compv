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
#include "compv/intrinsics/x86/features/orb/compv_feature_orb_desc_intrin_sse.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"
#include "compv/compv_math_utils.h"
#include "compv/compv_bits.h"
#include "compv/compv_cpu.h"

extern "C" const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AX[256];
extern "C" const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AY[256];
extern "C" const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BX[256];
extern "C" const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BY[256];

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add ASM version
void Brief256_31_Intrin_SSE2(const uint8_t* img_center, compv_scalar_t img_stride, float cosT, float sinT, COMPV_ALIGNED(SSE) void* out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // SSE41, ASM
	int i, u8_index;
	COMPV_ALIGN_SSE() int32_t xmmIndex[4];
	COMPV_ALIGN_SSE() uint8_t xmmA[16];
	COMPV_ALIGN_SSE() uint8_t xmmB[16];
	__m128i xmmX, xmmY, xmmStride, xmmR, xmm128;

	__m128 xmmCosT, xmmSinT, xmmXF, xmmYF;

	uint16_t* outPtr = (uint16_t*)out;

	const float* Brief256Pattern31AX = &kBrief256Pattern31AX[0];
	const float* Brief256Pattern31AY = &kBrief256Pattern31AY[0];
	const float* Brief256Pattern31BX = &kBrief256Pattern31BX[0];
	const float* Brief256Pattern31BY = &kBrief256Pattern31BY[0];
	
	_mm_store_si128(&xmm128, _mm_load_si128((__m128i*)k128_u8));
	_mm_store_si128(&xmmStride, _mm_set1_epi32((int)img_stride));
	_mm_store_ps((float*)&xmmCosT, _mm_set1_ps(cosT));
	_mm_store_ps((float*)&xmmSinT, _mm_set1_ps(sinT));

	u8_index = 0;

	for (i = 0; i < 256; i += 4) {
		// xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
		_mm_store_ps((float*)&xmmXF, _mm_sub_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31AX), xmmCosT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31AY), xmmSinT)));
		// yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
		_mm_store_ps((float*)&xmmYF, _mm_add_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31AX), xmmSinT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31AY), xmmCosT)));
		// x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		_mm_store_si128(&xmmX, _mm_cvtps_epi32(xmmXF));
		// y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		_mm_store_si128(&xmmY, _mm_cvtps_epi32(xmmYF));
		// a = img_center[(y * img_stride) + x];
		_mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY, xmmStride), xmmX)); // _mm_mullo_epi32 is SSE4.1
		xmmA[u8_index + 0] = img_center[xmmIndex[0]];
		xmmA[u8_index + 1] = img_center[xmmIndex[1]];
		xmmA[u8_index + 2] = img_center[xmmIndex[2]];
		xmmA[u8_index + 3] = img_center[xmmIndex[3]];

		// xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
		_mm_store_ps((float*)&xmmXF, _mm_sub_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31BX), xmmCosT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31BY), xmmSinT)));
		// yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
		_mm_store_ps((float*)&xmmYF, _mm_add_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31BX), xmmSinT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31BY), xmmCosT)));
		// x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		_mm_store_si128(&xmmX, _mm_cvtps_epi32(xmmXF));
		// y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		_mm_store_si128(&xmmY, _mm_cvtps_epi32(xmmYF));
		// b = img_center[(y * img_stride) + x];
		_mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32_SSE2(xmmY, xmmStride), xmmX)); // _mm_mullo_epi32 is SSE4.1
		xmmB[u8_index + 0] = img_center[xmmIndex[0]];
		xmmB[u8_index + 1] = img_center[xmmIndex[1]];
		xmmB[u8_index + 2] = img_center[xmmIndex[2]];
		xmmB[u8_index + 3] = img_center[xmmIndex[3]];

		if ((u8_index += 4) == 16) {
			// _out[0] |= (a < b) ? (u64_1 << j) : 0;
			_mm_store_si128(&xmmR, _mm_cmplt_epi8(_mm_sub_epi8(_mm_load_si128((__m128i*)xmmA), xmm128), _mm_sub_epi8(_mm_load_si128((__m128i*)xmmB), xmm128))); // _mm_cmplt_epu8 does exist
			*outPtr = _mm_movemask_epi8(xmmR);

			u8_index = 0;
			outPtr += 1;
		}

		Brief256Pattern31AX += 4;
		Brief256Pattern31AY += 4;
		Brief256Pattern31BX += 4;
		Brief256Pattern31BY += 4;
	}
}

// TODO(dmi): add ASM version
void Brief256_31_Intrin_SSE41(const uint8_t* img_center, compv_scalar_t img_stride, float cosT, float sinT, COMPV_ALIGNED(SSE) void* out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
	int i, u8_index;
	COMPV_ALIGN_SSE() int32_t xmmIndex[4];
	COMPV_ALIGN_SSE() uint8_t xmmA[16];
	COMPV_ALIGN_SSE() uint8_t xmmB[16];
	__m128i xmmX, xmmY, xmmStride, xmmR, xmm128;

	__m128 xmmCosT, xmmSinT, xmmXF, xmmYF;

	uint16_t* outPtr = (uint16_t*)out;

	const float* Brief256Pattern31AX = &kBrief256Pattern31AX[0];
	const float* Brief256Pattern31AY = &kBrief256Pattern31AY[0];
	const float* Brief256Pattern31BX = &kBrief256Pattern31BX[0];
	const float* Brief256Pattern31BY = &kBrief256Pattern31BY[0];

	_mm_store_si128(&xmm128, _mm_load_si128((__m128i*)k128_u8));
	_mm_store_si128(&xmmStride, _mm_set1_epi32((int)img_stride));
	_mm_store_ps((float*)&xmmCosT, _mm_set1_ps(cosT));
	_mm_store_ps((float*)&xmmSinT, _mm_set1_ps(sinT));

	u8_index = 0;

	for (i = 0; i < 256; i += 4) {
		// xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
		_mm_store_ps((float*)&xmmXF, _mm_sub_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31AX), xmmCosT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31AY), xmmSinT)));
		// yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
		_mm_store_ps((float*)&xmmYF, _mm_add_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31AX), xmmSinT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31AY), xmmCosT)));
		// x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		_mm_store_si128(&xmmX, _mm_cvtps_epi32(xmmXF));
		// y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		_mm_store_si128(&xmmY, _mm_cvtps_epi32(xmmYF));
		// a = img_center[(y * img_stride) + x];
		_mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32(xmmY, xmmStride), xmmX)); // _mm_mullo_epi32 is SSE4.1
		xmmA[u8_index + 0] = img_center[xmmIndex[0]];
		xmmA[u8_index + 1] = img_center[xmmIndex[1]];
		xmmA[u8_index + 2] = img_center[xmmIndex[2]];
		xmmA[u8_index + 3] = img_center[xmmIndex[3]];

		// xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
		_mm_store_ps((float*)&xmmXF, _mm_sub_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31BX), xmmCosT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31BY), xmmSinT)));
		// yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
		_mm_store_ps((float*)&xmmYF, _mm_add_ps(_mm_mul_ps(_mm_load_ps(Brief256Pattern31BX), xmmSinT), _mm_mul_ps(_mm_load_ps(Brief256Pattern31BY), xmmCosT)));
		// x = COMPV_MATH_ROUNDF_2_INT(xf, int);
		_mm_store_si128(&xmmX, _mm_cvtps_epi32(xmmXF));
		// y = COMPV_MATH_ROUNDF_2_INT(yf, int);
		_mm_store_si128(&xmmY, _mm_cvtps_epi32(xmmYF));
		// b = img_center[(y * img_stride) + x];
		_mm_store_si128((__m128i*)xmmIndex, _mm_add_epi32(_mm_mullo_epi32(xmmY, xmmStride), xmmX)); // _mm_mullo_epi32 is SSE4.1
		xmmB[u8_index + 0] = img_center[xmmIndex[0]];
		xmmB[u8_index + 1] = img_center[xmmIndex[1]];
		xmmB[u8_index + 2] = img_center[xmmIndex[2]];
		xmmB[u8_index + 3] = img_center[xmmIndex[3]];

		if ((u8_index += 4) == 16) {
			// _out[0] |= (a < b) ? (u64_1 << j) : 0;
			_mm_store_si128(&xmmR, _mm_cmplt_epi8(_mm_sub_epi8(_mm_load_si128((__m128i*)xmmA), xmm128), _mm_sub_epi8(_mm_load_si128((__m128i*)xmmB), xmm128))); // _mm_cmplt_epu8 does exist
			*outPtr = _mm_movemask_epi8(xmmR);

			u8_index = 0;
			outPtr += 1;
		}

		Brief256Pattern31AX += 4;
		Brief256Pattern31AY += 4;
		Brief256Pattern31BX += 4;
		Brief256Pattern31BY += 4;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
