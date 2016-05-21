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

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
// This function requires sizeof(float) = 4byte = 32bits
void Convlt1_verthz_float32_minpack16_Intrin_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const float* vhkern_ptr, compv_scalar_t kern_size)
{
    _mm256_zeroupper();
    // Use ASM which support FMA3
    // AVX/SSE mix penalities
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t i, j, k;
    __m256 ymmCoeff, ymmF0, ymmSF0, ymmSF1, ymmSF2, ymmSF3;
    __m256i ymmI0, ymmI1, ymmI2, ymmI3, ymmZero, ymmMaskToExtractFirst128Bits;

    ymmZero = _mm256_setzero_si256();
    ymmMaskToExtractFirst128Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_u64);

    pad += (width & 15); // 15 = (minpack - 1) = (16 - 1)

    for (j = 0; j < height; ++j) {
        i = width;
        // Loop-32
        while (i > 31) {
            ymmSF0 = _mm256_setzero_ps();
            ymmSF1 = _mm256_setzero_ps();
            ymmSF2 = _mm256_setzero_ps();
            ymmSF3 = _mm256_setzero_ps();
            for (k = 0; k < kern_size; ++k) {
                ymmI0 = _mm256_loadu_si256((__m256i*)&in_ptr[k * stride]);
                ymmCoeff = _mm256_set1_ps(vhkern_ptr[k]); // 0000

                ymmI1 = _mm256_unpacklo_epi8(ymmI0, ymmZero); // Low(U8) -> Low(I16)

                ymmF0 = _mm256_cvtepi32_ps(_mm256_unpacklo_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
                ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // a0b0c0d0
                ymmSF0 = _mm256_add_ps(ymmSF0, ymmF0);

                ymmF0 = _mm256_cvtepi32_ps(_mm256_unpackhi_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
                ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // e0f0g0h0
                ymmSF1 = _mm256_add_ps(ymmSF1, ymmF0);

                ymmI1 = _mm256_unpackhi_epi8(ymmI0, ymmZero); // High(U8) -> High(I16)

                ymmF0 = _mm256_cvtepi32_ps(_mm256_unpacklo_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
                ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // i0j0k0l0
                ymmSF2 = _mm256_add_ps(ymmSF2, ymmF0);

                ymmF0 = _mm256_cvtepi32_ps(_mm256_unpackhi_epi16(ymmI1, ymmZero)); // I16 -> I32 -> F32
                ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff); // m0n000p0
                ymmSF3 = _mm256_add_ps(ymmSF3, ymmF0);
            }

            ymmI0 = _mm256_cvtps_epi32(ymmSF0);
            ymmI1 = _mm256_cvtps_epi32(ymmSF1);
            ymmI2 = _mm256_cvtps_epi32(ymmSF2);
            ymmI3 = _mm256_cvtps_epi32(ymmSF3);
            ymmI0 = _mm256_packs_epi32(ymmI0, ymmI1);
            ymmI2 = _mm256_packs_epi32(ymmI2, ymmI3);
            ymmI0 = _mm256_packus_epi16(ymmI0, ymmI2);

            _mm256_storeu_si256((__m256i*)out_ptr, ymmI0);

            i -= 32;
            in_ptr += 32;
            out_ptr += 32;
        } // while (i > 31)

        // Loop-16 is executed at most #1 time

        /* Loop-16 */
        while (i > 15) {
            // When width is mof 32 this code isn't executed, make sure to disable previous "while" if you change something
            ymmSF0 = _mm256_setzero_ps();
            ymmSF1 = _mm256_setzero_ps();
            for (k = 0; k < kern_size; ++k) {
                ymmI0 = _mm256_maskload_epi64((const int64_t*)&in_ptr[k * stride], ymmMaskToExtractFirst128Bits); // ASM code: vmovdqa xmm0, [mem]
                ymmCoeff = _mm256_set1_ps(vhkern_ptr[k]);

                ymmI1 = _mm256_unpacklo_epi8(_mm256_permute4x64_epi64(ymmI0, COMPV_MM_SHUFFLE(3, 1, 2, 0)), ymmZero);

                ymmF0 = _mm256_cvtepi32_ps(_mm256_unpacklo_epi16(ymmI1, ymmZero));
                ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff);
                ymmSF0 = _mm256_add_ps(ymmSF0, ymmF0);

                ymmF0 = _mm256_cvtepi32_ps(_mm256_unpackhi_epi16(ymmI1, ymmZero));
                ymmF0 = _mm256_mul_ps(ymmF0, ymmCoeff);
                ymmSF1 = _mm256_add_ps(ymmSF1, ymmF0);
            }

            ymmI0 = _mm256_cvtps_epi32(ymmSF0);
            ymmI1 = _mm256_cvtps_epi32(ymmSF1);
            ymmI0 = _mm256_packs_epi32(ymmI0, ymmI1);
            ymmI0 = _mm256_packus_epi16(ymmI0, ymmI0);
            ymmI0 = _mm256_permute4x64_epi64(ymmI0, COMPV_MM_SHUFFLE(3, 1, 2, 0));

            _mm256_maskstore_epi64((int64_t*)out_ptr, ymmMaskToExtractFirst128Bits, ymmI0); // ASM code: vmovdqa [mem], xmm0

            i -= 16;
            in_ptr += 16;
            out_ptr += 16;
        }

        // Loop-8 is executed at most #1 time, doesn't worth it

        in_ptr += pad;
        out_ptr += pad;
    } // for (j...

    _mm256_zeroupper();
}

#if defined __INTEL_COMPILER
#	pragma intel optimization_parameter target_arch=avx
#endif
void Convlt1_verthz_fxpq16_minpack16_Intrin_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const uint16_t* vhkern_ptr, compv_scalar_t kern_size)
{
    _mm256_zeroupper();
    // Use ASM
    // AVX/SSE mix penalities
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    compv_scalar_t i, j, k;
    __m256i ymmI0, ymmS0, ymmS1, ymmZero, ymmCoeff, ymmMaskToExtractFirst128Bits;

    ymmZero = _mm256_setzero_si256();
    ymmMaskToExtractFirst128Bits = _mm256_load_si256((__m256i*)kAVXMaskstore_0_1_u64);

    pad += (width & 15); // 15 = (minpack - 1) = (16 - 1)

    for (j = 0; j < height; ++j) {
        i = width;
        // Loop-32
        while (i > 31) {
            ymmS0 = _mm256_setzero_si256();
            ymmS1 = _mm256_setzero_si256();
            for (k = 0; k < kern_size; ++k) {
                ymmI0 = _mm256_loadu_si256((__m256i*)&in_ptr[k * stride]);
                ymmCoeff = _mm256_set1_epi16(vhkern_ptr[k]);

                ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mulhi_epu16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff));
                ymmS1 = _mm256_add_epi16(ymmS1, _mm256_mulhi_epu16(_mm256_unpackhi_epi8(ymmI0, ymmZero), ymmCoeff));
            }
            _mm256_storeu_si256((__m256i*)out_ptr, _mm256_packus_epi16(ymmS0, ymmS1));

            i -= 32;
            in_ptr += 32;
            out_ptr += 32;
        } // while (i > 31)

        // Loop-16 is executed at most #1 time

        /* Loop-16 */
        while (i > 15) {
            // When width is mof 32 this code isn't executed, make sure to disable previous "while" if you change something
            ymmS0 = _mm256_setzero_si256();
            for (k = 0; k < kern_size; ++k) {
                ymmI0 = _mm256_maskload_epi64((const int64_t*)&in_ptr[k * stride], ymmMaskToExtractFirst128Bits); // ASM code: vmovdqa xmm0, [mem]
                ymmCoeff = _mm256_set1_epi16(vhkern_ptr[k]);

                ymmI0 = _mm256_permute4x64_epi64(ymmI0, COMPV_MM_SHUFFLE(3, 1, 2, 0));
                ymmS0 = _mm256_add_epi16(ymmS0, _mm256_mulhi_epu16(_mm256_unpacklo_epi8(ymmI0, ymmZero), ymmCoeff));
            }

            ymmS0 = _mm256_packus_epi16(ymmS0, ymmS0);
            ymmS0 = _mm256_permute4x64_epi64(ymmS0, COMPV_MM_SHUFFLE(3, 1, 2, 0));
            _mm256_maskstore_epi64((int64_t*)out_ptr, ymmMaskToExtractFirst128Bits, ymmS0); // ASM code: vmovdqa [mem], xmm0

            i -= 16;
            in_ptr += 16;
            out_ptr += 16;
        }

        // Loop-8 is executed at most #1 time, doesn't worth it

        in_ptr += pad;
        out_ptr += pad;
    } // for (j...

    _mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
