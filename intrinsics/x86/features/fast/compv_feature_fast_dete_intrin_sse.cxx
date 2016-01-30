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
#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_sse.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/intrinsics/x86/compv_intrin_sse.h"
#include "compv/compv_simd_globals.h"
#include "compv/compv_mathutils.h"

COMPV_NAMESPACE_BEGIN()

// FIXME: SSE2?
int FastData_Intrin_SSSE3(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, COMPV_ALIGNED(SSE) uint8_t(&temp16)[16], int16_t darker, int16_t brighter, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, COMPV_ALIGNED(SSE) int16_t(&ddarkers16)[16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16)[16])
{
	int32_t sum;

	// compare I1 and I7
	temp16[0] = dataPtr[pixels16[0]];
	temp16[8] = dataPtr[pixels16[8]];
	ddarkers16[0] = (darker - temp16[0]);
	ddarkers16[8] = (darker - temp16[8]);
	dbrighters16[0] = (temp16[0] - brighter);
	dbrighters16[8] = (temp16[8] - brighter);

	sum = (dbrighters16[0] > 0 || ddarkers16[0] > 0) + (dbrighters16[8] > 0 || ddarkers16[8] > 0);
	
	/*  Speed-Test-1 */
	if (N != 12 || sum > 0) {
		// compare I5 and I13
		temp16[4] = dataPtr[pixels16[4]];
		temp16[12] = dataPtr[pixels16[12]];
		ddarkers16[4] = (darker - temp16[4]); // I5-darkness
		ddarkers16[12] = (darker - temp16[12]); // I13-darkness
		dbrighters16[4] = (temp16[4] - brighter); // I5-brightness
		dbrighters16[12] = (temp16[12] - brighter); // I13-brightness

		sum += (dbrighters16[4] > 0 || ddarkers16[4] > 0) + (dbrighters16[12] > 0 || ddarkers16[12] > 0);
		/*  Speed-Test-2 */
		if ((sum >= 2 && (N != 12 || sum >= 3))) {
			temp16[1] = dataPtr[pixels16[1]];
			temp16[2] = dataPtr[pixels16[2]];
			temp16[3] = dataPtr[pixels16[3]];
			temp16[5] = dataPtr[pixels16[5]];
			temp16[6] = dataPtr[pixels16[6]];
			temp16[7] = dataPtr[pixels16[7]];
			temp16[9] = dataPtr[pixels16[9]];
			temp16[10] = dataPtr[pixels16[10]];
			temp16[11] = dataPtr[pixels16[11]];
			temp16[13] = dataPtr[pixels16[13]];
			temp16[14] = dataPtr[pixels16[14]];
			temp16[15] = dataPtr[pixels16[15]];

			__m128i xmmTemp16, xmmDdarkers16, xmmDbrighters16, xmmDarker, xmmBrighter, xmmZeros;

			_mm_store_si128(&xmmZeros, _mm_setzero_si128());

			_mm_store_si128(&xmmTemp16, _mm_load_si128((__m128i*)temp16));
			_mm_store_si128(&xmmDarker, _mm_set1_epi8((int8_t)darker));
			_mm_store_si128(&xmmBrighter, _mm_set1_epi8((int8_t)brighter));

			_mm_store_si128(&xmmDdarkers16, _mm_subs_epu8(xmmDarker, xmmTemp16));
			_mm_store_si128(&xmmDbrighters16, _mm_subs_epu8(xmmTemp16, xmmBrighter));
			// _mm_cmpgt_epi8 uses signed integers while we're using unsigned values and there is no _mm_cmpneq_epi8.
			*pfdarkers = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDdarkers16, xmmZeros));
			*pfbrighters = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDbrighters16, xmmZeros));
			int popcnt0 = __popcnt16((unsigned short)*pfdarkers); // FIXME: not portable and requires cpuid checking
			int popcnt1 = __popcnt16((unsigned short)*pfbrighters); // FIXME: not portable and requires cpuid checking
			if (popcnt0 >= N || popcnt1 >= N) {
				// Convert ddarkers16 and dbrighters16 from epu8 to epi16
				_mm_store_si128((__m128i*)&ddarkers16[0], _mm_unpacklo_epi8(xmmDdarkers16, xmmZeros));
				_mm_store_si128((__m128i*)&ddarkers16[8], _mm_unpackhi_epi8(xmmDdarkers16, xmmZeros));
				_mm_store_si128((__m128i*)&dbrighters16[0], _mm_unpacklo_epi8(xmmDbrighters16, xmmZeros));
				_mm_store_si128((__m128i*)&dbrighters16[8], _mm_unpackhi_epi8(xmmDbrighters16, xmmZeros));
				return 1;
			}
		}
	}
	return 0;
}

// FIXME: SSE2?
// FIXME: return compv_scalar_t
int FastData16_Intrin_SSSE3(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) uint8_t(&temp16x16)[16][16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16])
{
	compv_scalar_t r = 0, sum;
#if 1
	__m128i xmm0, xmm1, xmmBrighter, xmmDarker, xmmThreshold, xmmZeros;
	compv_scalar_t r0, r1;

	// ddarkers16x16 and ddarkers16x16 are int16 arrays but we want to use there memory to store uint8[] temp variables until the end of the process then we convert them
	// These arrays are int16 to make sure the CPP code won't need to sature all operations
	

	// FIXME: remplace the "0xFFFF - " with ~ when all is ok
	// FIXME: do the same for Fast1, the above function

	_mm_store_si128(&xmmZeros, _mm_setzero_si128());

	_mm_store_si128(&xmmThreshold, _mm_set1_epi8((uint8_t)threshold));
	_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)dataPtr));
	_mm_store_si128(&xmmBrighter, _mm_adds_epu8(xmm0, xmmThreshold));
	_mm_store_si128(&xmmDarker, _mm_subs_epu8(xmm0, xmmThreshold));
	
	// compare I1 and I7 aka 0 and 8
	_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[0]]));
	_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[8]]));
	_mm_store_si128((__m128i*)ddarkers16x16[0], _mm_subs_epu8(xmmDarker, xmm0));
	_mm_store_si128((__m128i*)ddarkers16x16[8], _mm_subs_epu8(xmmDarker, xmm1));
	pfdarkers16[0] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[0]), xmmZeros));
	pfdarkers16[8] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[8]), xmmZeros));
	_mm_store_si128((__m128i*)dbrighters16x16[0], _mm_subs_epu8(xmm0, xmmBrighter));
	_mm_store_si128((__m128i*)dbrighters16x16[8], _mm_subs_epu8(xmm1, xmmBrighter));
	pfbrighters16[0] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[0]), xmmZeros));
	pfbrighters16[8] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[8]), xmmZeros));

	//FIXME: remove
	xmm0 = _mm_load_si128((__m128i*)dbrighters16x16[0]);

	/*  Speed-Test-1 */
	r0 = (pfbrighters16[0] | pfdarkers16[0]);
	r1 = (pfbrighters16[8] | pfdarkers16[8]);
	r0 = __popcnt16((unsigned short)r0); // FIXME: not portable and requires cpuid checking
	r1 = __popcnt16((unsigned short)r1); // FIXME: not portable and requires cpuid checking
	sum = r0 + r1;

	if (N != 12 || sum > 0) {
		// compare I5 and I13 aka 4 and 12
		_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[4]]));
		_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[12]]));
		_mm_store_si128((__m128i*)ddarkers16x16[4], _mm_subs_epu8(xmmDarker, xmm0));
		_mm_store_si128((__m128i*)ddarkers16x16[12], _mm_subs_epu8(xmmDarker, xmm1));
		pfdarkers16[4] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[4]), xmmZeros));
		pfdarkers16[12] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[12]), xmmZeros));
		_mm_store_si128((__m128i*)dbrighters16x16[4], _mm_subs_epu8(xmm0, xmmBrighter));
		_mm_store_si128((__m128i*)dbrighters16x16[12], _mm_subs_epu8(xmm1, xmmBrighter));		
		pfbrighters16[4] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[4]), xmmZeros));
		pfbrighters16[12] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[12]), xmmZeros));

		r0 = (pfbrighters16[4] | pfdarkers16[4]);
		r1 = (pfbrighters16[12] | pfdarkers16[12]);
		r0 = __popcnt16((unsigned short)r0); // FIXME: not portable and requires cpuid checking
		r1 = __popcnt16((unsigned short)r1); // FIXME: not portable and requires cpuid checking
		sum += r0 + r1;

		/*  Speed-Test-2 */
		if ((sum >= 2 && (N != 12 || sum >= 3))) {
			// 1 and 2
			_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[1]]));
			_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[2]]));
			_mm_store_si128((__m128i*)ddarkers16x16[1], _mm_subs_epu8(xmmDarker, xmm0));
			_mm_store_si128((__m128i*)ddarkers16x16[2], _mm_subs_epu8(xmmDarker, xmm1));
			pfdarkers16[1] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[1]), xmmZeros));
			pfdarkers16[2] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[2]), xmmZeros));
			_mm_store_si128((__m128i*)dbrighters16x16[1], _mm_subs_epu8(xmm0, xmmBrighter));
			_mm_store_si128((__m128i*)dbrighters16x16[2], _mm_subs_epu8(xmm1, xmmBrighter));
			pfbrighters16[1] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[1]), xmmZeros));
			pfbrighters16[2] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[2]), xmmZeros));
			// 3 and 5
			_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[3]]));
			_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[5]]));
			_mm_store_si128((__m128i*)ddarkers16x16[3], _mm_subs_epu8(xmmDarker, xmm0));
			_mm_store_si128((__m128i*)ddarkers16x16[5], _mm_subs_epu8(xmmDarker, xmm1));
			pfdarkers16[3] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[3]), xmmZeros));
			pfdarkers16[5] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[5]), xmmZeros));
			_mm_store_si128((__m128i*)dbrighters16x16[3], _mm_subs_epu8(xmm0, xmmBrighter));
			_mm_store_si128((__m128i*)dbrighters16x16[5], _mm_subs_epu8(xmm1, xmmBrighter));
			pfbrighters16[3] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[3]), xmmZeros));
			pfbrighters16[5] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[5]), xmmZeros));
			// 6 and 7
			_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[6]]));
			_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[7]]));
			_mm_store_si128((__m128i*)ddarkers16x16[6], _mm_subs_epu8(xmmDarker, xmm0));
			_mm_store_si128((__m128i*)ddarkers16x16[7], _mm_subs_epu8(xmmDarker, xmm1));
			pfdarkers16[6] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[6]), xmmZeros));
			pfdarkers16[7] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[7]), xmmZeros));
			_mm_store_si128((__m128i*)dbrighters16x16[6], _mm_subs_epu8(xmm0, xmmBrighter));
			_mm_store_si128((__m128i*)dbrighters16x16[7], _mm_subs_epu8(xmm1, xmmBrighter));
			pfbrighters16[6] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[6]), xmmZeros));
			pfbrighters16[7] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[7]), xmmZeros));
			// 9 and 10
			_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[9]]));
			_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[10]]));
			_mm_store_si128((__m128i*)ddarkers16x16[9], _mm_subs_epu8(xmmDarker, xmm0));
			_mm_store_si128((__m128i*)ddarkers16x16[10], _mm_subs_epu8(xmmDarker, xmm1));
			pfdarkers16[9] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[9]), xmmZeros));
			pfdarkers16[10] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[10]), xmmZeros));
			_mm_store_si128((__m128i*)dbrighters16x16[9], _mm_subs_epu8(xmm0, xmmBrighter));
			_mm_store_si128((__m128i*)dbrighters16x16[10], _mm_subs_epu8(xmm1, xmmBrighter));
			pfbrighters16[9] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[9]), xmmZeros));
			pfbrighters16[10] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[10]), xmmZeros));
			//  11 and 13
			_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[11]]));
			_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[13]]));
			_mm_store_si128((__m128i*)ddarkers16x16[11], _mm_subs_epu8(xmmDarker, xmm0));
			_mm_store_si128((__m128i*)ddarkers16x16[13], _mm_subs_epu8(xmmDarker, xmm1));
			pfdarkers16[11] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[11]), xmmZeros));
			pfdarkers16[13] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[13]), xmmZeros));
			_mm_store_si128((__m128i*)dbrighters16x16[11], _mm_subs_epu8(xmm0, xmmBrighter));
			_mm_store_si128((__m128i*)dbrighters16x16[13], _mm_subs_epu8(xmm1, xmmBrighter));
			pfbrighters16[11] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[11]), xmmZeros));
			pfbrighters16[13] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[13]), xmmZeros));
			// 14 and 15
			_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[14]]));
			_mm_store_si128(&xmm1, _mm_loadu_si128((__m128i*)&dataPtr[pixels16[15]]));
			_mm_store_si128((__m128i*)ddarkers16x16[14], _mm_subs_epu8(xmmDarker, xmm0));
			_mm_store_si128((__m128i*)ddarkers16x16[15], _mm_subs_epu8(xmmDarker, xmm1));
			pfdarkers16[14] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[14]), xmmZeros));
			pfdarkers16[15] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)ddarkers16x16[15]), xmmZeros));
			_mm_store_si128((__m128i*)dbrighters16x16[14], _mm_subs_epu8(xmm0, xmmBrighter));
			_mm_store_si128((__m128i*)dbrighters16x16[15], _mm_subs_epu8(xmm1, xmmBrighter));
			pfbrighters16[14] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[14]), xmmZeros));
			pfbrighters16[15] = 0xFFFF - _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_load_si128((__m128i*)dbrighters16x16[15]), xmmZeros));

			//FIXME: remove
			for (int i = 0; i < 16; ++i) {
				_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)ddarkers16x16[i]));
				_mm_store_si128((__m128i*)&ddarkers16x16[i][0], _mm_unpacklo_epi8(xmm0, xmmZeros));
				_mm_store_si128((__m128i*)&ddarkers16x16[i][8], _mm_unpackhi_epi8(xmm0, xmmZeros));
				// convert dbrighters16x16 from epi8 to epi16
				_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)dbrighters16x16[i]));
				
				//FIXME: remove
				xmm1 = _mm_load_si128((__m128i*)dbrighters16x16[i]);

				_mm_store_si128((__m128i*)&dbrighters16x16[i][0], _mm_unpacklo_epi8(xmm0, xmmZeros));
				_mm_store_si128((__m128i*)&dbrighters16x16[i][8], _mm_unpackhi_epi8(xmm0, xmmZeros));
			}
			/*for (int j = 0; j < 16; j++) {
				for (int i = 0; i < 16; ++i){
					int16_t a = ddarkers16x16[i][j];
					ddarkers16x16[i][j] = ddarkers16x16[j][i];
					ddarkers16x16[j][i] = a;

					if (j > i){
						a = dbrighters16x16[i][j];
						dbrighters16x16[i][j] = dbrighters16x16[j][i];
						dbrighters16x16[j][i] = a;
					}
				}
			}*/

			// FIXME: unroll loop
			for (int i = 0; i < 16; ++i) {
				r0 = __popcnt16((unsigned short)pfdarkers16[i]); // FIXME: not portable and requires cpuid checking
				r1 = __popcnt16((unsigned short)pfbrighters16[i]); // FIXME: not portable and requires cpuid checking
				if (r0 >= N || r1 >= N) {
					r |= ((compv_scalar_t)1 << i);
					// FIXME: restaure if your remove above loop
					/*if (r0 >= N) {
						// convert ddarkers16x16 from epi8 to epi16
						_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)ddarkers16x16[i]));
						_mm_store_si128((__m128i*)&ddarkers16x16[i][0], _mm_unpacklo_epi8(xmm0, xmmZeros));
						_mm_store_si128((__m128i*)&ddarkers16x16[i][8], _mm_unpackhi_epi8(xmm0, xmmZeros));
					}
					else {
						// convert dbrighters16x16 from epi8 to epi16
						_mm_store_si128(&xmm0, _mm_loadu_si128((__m128i*)dbrighters16x16[i]));
						_mm_store_si128((__m128i*)&dbrighters16x16[i][0], _mm_unpacklo_epi8(xmm0, xmmZeros));
						_mm_store_si128((__m128i*)&dbrighters16x16[i][8], _mm_unpackhi_epi8(xmm0, xmmZeros));
					}*/
				}
			}
		}
	}
#else
	
	for (compv_scalar_t i = 0; i < 16; ++i) {
		compv_scalar_t brighter = (dataPtr[i] + threshold);
		compv_scalar_t darker = (dataPtr[i] - threshold);
		
		temp16x16[i][0] = dataPtr[pixels16[0] + i];
		temp16x16[i][8] = dataPtr[pixels16[8] + i];
		ddarkers16x16[i][0] = CompVMathUtils::clampPixel8((int16_t)(darker - temp16x16[i][0]));
		ddarkers16x16[i][8] = CompVMathUtils::clampPixel8((int16_t)(darker - temp16x16[i][8]));
		dbrighters16x16[i][0] = CompVMathUtils::clampPixel8((int16_t)(temp16x16[i][0] - brighter));
		dbrighters16x16[i][8] = CompVMathUtils::clampPixel8((int16_t)(temp16x16[i][8] - brighter));

		sum = (dbrighters16x16[i][0] > 0 || ddarkers16x16[i][0] > 0) + (dbrighters16x16[i][8] > 0 || ddarkers16x16[i][8] > 0);
		if (N != 12 || sum > 0) {
			temp16x16[i][4] = dataPtr[pixels16[4] + i];
			temp16x16[i][12] = dataPtr[pixels16[12] + i];
			ddarkers16x16[i][4] = CompVMathUtils::clampPixel8((int16_t)(darker - temp16x16[i][4]));
			ddarkers16x16[i][12] = CompVMathUtils::clampPixel8((int16_t)(darker - temp16x16[i][12]));
			dbrighters16x16[i][4] = CompVMathUtils::clampPixel8((int16_t)(temp16x16[i][4] - brighter));
			dbrighters16x16[i][12] = CompVMathUtils::clampPixel8((int16_t)(temp16x16[i][12] - brighter));

			sum += (dbrighters16x16[i][4] > 0 || ddarkers16x16[i][4] > 0) + (dbrighters16x16[i][12] > 0 || ddarkers16x16[i][12] > 0);
			/*  Speed-Test-2 */
			if ((sum >= 2 && (N != 12 || sum >= 3))) {
				temp16x16[i][1] = dataPtr[pixels16[1] + i];
				temp16x16[i][2] = dataPtr[pixels16[2] + i];
				temp16x16[i][3] = dataPtr[pixels16[3] + i];
				temp16x16[i][5] = dataPtr[pixels16[5] + i];
				temp16x16[i][6] = dataPtr[pixels16[6] + i];
				temp16x16[i][7] = dataPtr[pixels16[7] + i];
				temp16x16[i][9] = dataPtr[pixels16[9] + i];
				temp16x16[i][10] = dataPtr[pixels16[10] + i];
				temp16x16[i][11] = dataPtr[pixels16[11] + i];
				temp16x16[i][13] = dataPtr[pixels16[13] + i];
				temp16x16[i][14] = dataPtr[pixels16[14] + i];
				temp16x16[i][15] = dataPtr[pixels16[15] + i];

				__m128i xmmTemp16, xmmDdarkers16, xmmDbrighters16, xmmDarker, xmmBrighter, xmmZeros;

				_mm_store_si128(&xmmZeros, _mm_setzero_si128());

				_mm_store_si128(&xmmTemp16, _mm_load_si128((__m128i*)temp16x16[i]));
				_mm_store_si128(&xmmDarker, _mm_set1_epi8((int8_t)darker));
				_mm_store_si128(&xmmBrighter, _mm_set1_epi8((int8_t)brighter));

				_mm_store_si128(&xmmDdarkers16, _mm_subs_epu8(xmmDarker, xmmTemp16));
				_mm_store_si128(&xmmDbrighters16, _mm_subs_epu8(xmmTemp16, xmmBrighter));
				// _mm_cmpgt_epi8 uses signed integers while we're using unsigned values and there is no _mm_cmpneq_epi8.
				pfdarkers16[i] = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDdarkers16, xmmZeros));
				pfbrighters16[i] = ~_mm_movemask_epi8(_mm_cmpeq_epi8(xmmDbrighters16, xmmZeros));
				int popcnt0 = __popcnt16((unsigned short)pfdarkers16[i]); // FIXME: not portable and requires cpuid checking
				int popcnt1 = __popcnt16((unsigned short)pfbrighters16[i]); // FIXME: not portable and requires cpuid checking
				if (popcnt0 >= N || popcnt1 >= N) {
					// Convert ddarkers16 and dbrighters16 from epu8 to epi16
					_mm_store_si128((__m128i*)&ddarkers16x16[i][0], _mm_unpacklo_epi8(xmmDdarkers16, xmmZeros));
					_mm_store_si128((__m128i*)&ddarkers16x16[i][8], _mm_unpackhi_epi8(xmmDdarkers16, xmmZeros));
					_mm_store_si128((__m128i*)&dbrighters16x16[i][0], _mm_unpacklo_epi8(xmmDbrighters16, xmmZeros));
					_mm_store_si128((__m128i*)&dbrighters16x16[i][8], _mm_unpackhi_epi8(xmmDbrighters16, xmmZeros));
					r |= ((compv_scalar_t)1 << i);
				}
			}
		}
	}
#endif

	return (int)r;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */