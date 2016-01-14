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
#if !defined(_COMPV_INTRIN_AVX_H_)
#define _COMPV_INTRIN_AVX_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

/*
AVX uses #2 128bits SSE registers to emulate a 256bits register.
Let's say we have #2 SSE registers XMM-low=m1m2 and XMM-high=m3m4, then YMM=m1m2m3m4.
avx_shuffle_lo(YMM, YMM)=shuffle(m1,m3)||shuffle(m1,m3) not shuffle(m1,m2)||shuffle(m1,m2). AVX is performing #2 sse_shuffle(XMM-low, XMM-low).
The next macros change the behavior to consider the AVX registers as a single 256bits
/!\ These macros are time-consuming and should be used carrefully.
*/
#define compv_avx2_unpacklo_epi8(ymm0_, ymm1_)		_mm256_unpacklo_epi8(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define compv_avx2_unpackhi_epi8(ymm0_, ymm1_)		_mm256_unpackhi_epi8(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define compv_avx2_hadd_epi16(ymm0_, ymm1_)			_mm256_permute4x64_epi64(_mm256_hadd_epi16(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_packus_epi16(ymm0_, ymm1_)		_mm256_permute4x64_epi64(_mm256_packus_epi16(ymm0_, ymm1_), COMPV_MM_SHUFFLE(3, 1, 2, 0))
#define compv_avx2_unpacklo_epi16(ymm0_, ymm1_)		_mm256_unpacklo_epi16(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))
#define compv_avx2_unpackhi_epi16(ymm0_, ymm1_)		_mm256_unpackhi_epi16(_mm256_permute4x64_epi64(ymm0_, COMPV_MM_SHUFFLE(3, 1, 2, 0)), _mm256_permute4x64_epi64(ymm1_, COMPV_MM_SHUFFLE(3, 1, 2, 0)))

/*
Index for the 64bit packed values
*/
#define COMPV_AVX_A64 0
#define COMPV_AVX_B64 1
#define COMPV_AVX_C64 2
#define COMPV_AVX_D64 3
#define COMPV_AVX_E64 4
#define COMPV_AVX_F64 5
#define COMPV_AVX_G64 6
#define COMPV_AVX_H64 7

COMPV_NAMESPACE_END()

#endif /* _COMPV_INTRIN_AVX_H_ */
