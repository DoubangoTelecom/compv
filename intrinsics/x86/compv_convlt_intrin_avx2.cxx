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
#include "compv/intrinsics/x86/compv_convlt_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/intrinsics/x86/compv_intrin_avx.h"
#include "compv/compv_simd_globals.h"
#include "compv/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// This function requires sizeof(float) = 4byte = 32bits
// FIXME: minpack should be 16 or 4
void Convlt1_hz_float32_minpack4_Intrin_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // Not fully tested yet
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM and FMA3, Not optimized at all
	compv_scalar_t i, j, col;
	__m256 ymmCoeff, ymmF0, ymmSF0, ymmSF1, ymmSF2, ymmSF3;
	__m256i ymmI0, ymmI1, ymmI2, ymmI3, ymmZero;

	ymmZero = _mm256_setzero_si256();

	// FIXME
	//pad += (width & 3) ; // 3 = (minpack - 1) = (4 - 1)
	pad += (width & 31); // 3 = (minpack - 1) = (32 - 1)

	// (abcd conv 0123) = a0+b1+c2+d3
	
	for (j = 0; j < height; ++j) {
		i = width;
		/* Loop-32 */
		while (i > 31) {
			ymmSF0 = _mm256_setzero_ps();
			ymmSF1 = _mm256_setzero_ps();
			ymmSF2 = _mm256_setzero_ps();
			ymmSF3 = _mm256_setzero_ps();
			for (col = 0; col < kern_size; ++col) {
				ymmI0 = _mm256_loadu_si256((__m256i*)&in_ptr[col]);
				ymmCoeff = _mm256_set1_ps(hkern_ptr[col]); // 0000

				// TODO(dmi): Very good candidate for FMA3 / AVX
				
				ymmI1 = compv_avx2_unpacklo_epi8(ymmI0, ymmZero); // Low(U8) -> Low(I16)

				ymmF0 = _mm256_cvtepi32_ps(compv_avx2_unpacklo_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
				ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // a0b0c0d0
				ymmSF0 = _mm256_add_ps(ymmSF0, ymmF0);

				ymmF0 = _mm256_cvtepi32_ps(compv_avx2_unpackhi_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
				ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // e0f0g0h0
				ymmSF1 = _mm256_add_ps(ymmSF1, ymmF0);

				ymmI1 = compv_avx2_unpackhi_epi8(ymmI0, ymmZero); // High(U8) -> High(I16)

				ymmF0 = _mm256_cvtepi32_ps(compv_avx2_unpacklo_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
				ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // i0j0k0l0
				ymmSF2 = _mm256_add_ps(ymmSF2, ymmF0);

				ymmF0 = _mm256_cvtepi32_ps(compv_avx2_unpackhi_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
				ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // m0n000p0
				ymmSF3 = _mm256_add_ps(ymmSF3, ymmF0);
			}

			ymmI0 = _mm256_cvtps_epi32(ymmSF0);
			ymmI1 = _mm256_cvtps_epi32(ymmSF1);
			ymmI2 = _mm256_cvtps_epi32(ymmSF2);
			ymmI3 = _mm256_cvtps_epi32(ymmSF3);
			ymmI0 = compv_avx2_packs_epi32(ymmI0, ymmI1);
			ymmI2 = compv_avx2_packs_epi32(ymmI2, ymmI3);
			ymmI0 = compv_avx2_packus_epi16(ymmI0, ymmI2);

			_mm256_storeu_si256((__m256i*)out_ptr, ymmI0);
			
			i -= 32;
			in_ptr += 32;
			out_ptr += 32;
		} // while (i > 31)

		/* Loop-16 */
		/*while (i > 15) {
			ymmSF0 = _mm256_setzero_ps();
			ymmSF1 = _mm256_setzero_ps();
			for (col = 0; col < kern_size; ++col) {
				ymmI0 = _mm256_cvtsi32_si256(*((uint32_t*)&in_ptr[col]));
				ymmCoeff = _mm256_set1_ps(hkern_ptr[col]);

				ymmF0 = _mm256_cvtepi32_ps(_mm256_unpacklo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmZero));
				ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff);
				ymmSF0 = _mm256_add_ps(ymmSF0, ymmF0);
			}
			ymmI0 = _mm256_cvtps_epi32(ymmSF0);
			ymmI0 = _mm256_packs_epi32(ymmI0, ymmI0);
			ymmI0 = _mm256_packus_epi16(ymmI0, ymmI0);

			*((uint32_t*)out_ptr) = (uint32_t)_mm256_cvtsi128_si32(ymmI0);

			i -= 16;
			in_ptr += 16;
			out_ptr += 16;
		}*/

		/*while (i > 3) {
			ymmSF0 = _mm256_setzero_ps();
			for (col = 0; col < kern_size; ++col) {
				ymmI0 = _mm256_cvtsi32_si256(*((uint32_t*)&in_ptr[col]));
				ymmCoeff = _mm256_set1_ps(hkern_ptr[col]);

				ymmF0 = _mm256_cvtepi32_ps(_mm256_unpacklo_epi16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmZero));
				ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff);
				ymmSF0 = _mm256_add_ps(ymmSF0, ymmF0);
			}
			ymmI0 = _mm256_cvtps_epi32(ymmSF0);
			ymmI0 = _mm256_packs_epi32(ymmI0, ymmI0);
			ymmI0 = _mm256_packus_epi16(ymmI0, ymmI0);

			*((uint32_t*)out_ptr) = (uint32_t)_mm256_cvtsi128_si32(ymmI0);

			i -= 4;
			in_ptr += 4;
			out_ptr += 4;
		} */

		in_ptr += pad;
		out_ptr += pad;
	} // for (j...
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
