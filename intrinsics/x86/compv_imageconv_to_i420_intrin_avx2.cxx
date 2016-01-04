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
#include "compv/intrinsics/x86/compv_imageconv_to_i420_intrin_avx2.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/image/compv_imageconv_common.h"
#include "compv/compv_simd_globals.h"

COMPV_NAMESPACE_BEGIN()

void rgbaToI420Kernel11_CompY_Intrin_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
{
	__m256i ymmRgba;
	__m256i ymmYCoeffs = _mm256_load_si256((__m256i*)kRGBAToYUV_YCoeffs8);
	__m256i y16 = _mm256_load_si256((__m256i*)k16_i16);
	size_t padRGBA = (stride - width) << 2;
	size_t padY = (stride - width);

	// Y = (((33 * R) + (65 * G) + (13 * B))) >> 7 + 16
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; i += 8) {
			_mm256_store_si256(&ymmRgba, _mm256_load_si256((__m256i*)rgbaPtr)); // 8 RGBA samples
			_mm256_store_si256(&ymmRgba, _mm256_maddubs_epi16(ymmRgba, ymmYCoeffs)); // 
			_mm256_store_si256(&ymmRgba, _mm256_hadd_epi16(ymmRgba, ymmRgba)); // aaaabbbbaaaabbbb
			_mm256_store_si256(&ymmRgba, _mm256_permute4x64_epi64(ymmRgba, COMPV_MM_SHUFFLE(0, 0, 2, 0))); // aaaaaaaa-------
			_mm256_store_si256(&ymmRgba, _mm256_srai_epi16(ymmRgba, 7)); // >> 7
			_mm256_store_si256(&ymmRgba, _mm256_add_epi16(ymmRgba, y16)); // + 16
			_mm256_store_si256(&ymmRgba, _mm256_packus_epi16(ymmRgba, ymmRgba)); // Saturate(I16 -> U8)
			*((uint64_t*)outYPtr) = _mm_cvtsi128_si64(_mm256_castsi256_si128(ymmRgba));

			outYPtr += 8;
			rgbaPtr += 32;
		}
		outYPtr += padY;
		rgbaPtr += padRGBA;
	}
}


COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */