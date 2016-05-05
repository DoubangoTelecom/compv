/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*  Copyright (C) 2016 Mamadou DIOP.
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

/* @description
This class implement FAST (Features from Accelerated Segment Test) algorithm.
Some literature about FAST:
- http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/AV1011/AV1FeaturefromAcceleratedSegmentTest.pdf
- https://en.wikipedia.org/wiki/Features_from_accelerated_segment_test
- http://www.edwardrosten.com/work/fast.html
- http://web.eecs.umich.edu/~silvio/teaching/EECS598_2010/slides/11_16_Hao.pdf
*/

// TODO(dmi):
// Allow setting max number of features to retain
// Add support for ragel fast9 and fast12.
// CPP version doesn't work

#include "compv/features/fast/compv_feature_fast_dete.h"
#include "compv/features/fast/compv_feature_fast9_dete.h"
#include "compv/features/fast/compv_feature_fast12_dete.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_math_utils.h"
#include "compv/compv_cpu.h"
#include "compv/compv_bits.h"

#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_sse.h"
#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_avx2.h"

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <iostream>
#include <string>

// Defines here outside the namespace to allow referencing in ASM code
#if COMPV_ARCH_X86 && (COMPV_INTRINSIC || COMPV_ASM)
// Values generated using FastShufflesArc() in "tests/fast.cxx"
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() uint8_t kFast9Arcs[16/*ArcStartIdx*/][16] = { // SHUFFLE_EPI8 values to select an arc
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, },
    { 0x1, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, },
    { 0x2, 0x2, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0x2, 0x2, 0x2, 0x2, 0x2, },
    { 0x3, 0x3, 0x3, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0x3, 0x3, 0x3, 0x3, },
    { 0x4, 0x4, 0x4, 0x4, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0x4, 0x4, 0x4, },
    { 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0x5, 0x5, },
    { 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0x6, },
    { 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, },
};
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() uint8_t kFast12Arcs[16/*ArcStartIdx*/][16] = { // SHUFFLE_EPI8 values to select an arc
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0x0, 0x0, 0x0, 0x0, },
    { 0x1, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0x1, 0x1, 0x1, },
    { 0x2, 0x2, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0x2, 0x2, },
    { 0x3, 0x3, 0x3, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0x3, },
    { 0x4, 0x4, 0x4, 0x4, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x5, 0x5, 0x5, 0x5, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x6, 0x6, 0x6, 0x6, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x8, 0x8, 0x8, 0x8, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x9, 0x9, 0x9, 0x9, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0xa, 0xa, 0xa, 0xa, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xb, 0xb, 0xb, 0xb, 0xb, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0xc, 0xc, 0xc, 0xc, 0xc, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0xd, 0xd, 0xd, 0xd, 0xd, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xe, 0xe, 0xe, 0xe, 0xe, 0xf, },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xf, 0xf, 0xf, 0xf, 0xf, },
};
#endif // (COMPV_ARCH_X86) && ((COMPV_INTRINSIC) || (COMPV_ASM))

// Flags generated using FastFlags() in "tests/fast.cxx"
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() uint16_t Fast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() uint16_t Fast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };

// X86
#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void FastData32Row_Asm_X86_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, uint8_t* strengths, compv::compv_scalar_t* me);

COMPV_EXTERNC void FastData16Row_Asm_X86_SSE2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, uint8_t* strengths, compv::compv_scalar_t* me);

COMPV_EXTERNC void Fast9Strengths16_Asm_CMOV_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x16, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast9Strengths16_Asm_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x16, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths16_Asm_CMOV_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x16, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths16_Asm_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x16, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);

COMPV_EXTERNC void Fast9Strengths32_Asm_CMOV_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast9Strengths32_Asm_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_CMOV_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);

COMPV_EXTERNC void Fast9Strengths32_Asm_CMOV_X86_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast9Strengths32_Asm_X86_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_CMOV_X86_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_X86_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);

#endif

// X64
#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void FastData32Row_Asm_X64_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, uint8_t* strengths, compv::compv_scalar_t* me);

COMPV_EXTERNC void FastData16Row_Asm_X64_SSE2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, uint8_t* strengths, compv::compv_scalar_t* me);

COMPV_EXTERNC void Fast9Strengths16_Asm_CMOV_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast9Strengths16_Asm_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths16_Asm_CMOV_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths16_Asm_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);

COMPV_EXTERNC void Fast9Strengths32_Asm_CMOV_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast9Strengths32_Asm_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_CMOV_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);

COMPV_EXTERNC void Fast9Strengths32_Asm_CMOV_X64_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast9Strengths32_Asm_X64_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_CMOV_X64_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);
COMPV_EXTERNC void Fast12Strengths32_Asm_X64_AVX2(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N);

#endif

COMPV_EXTERNC COMPV_API void FastStrengths16(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x16, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N)
{
    void(*FastStrengths)(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N)
        = NULL; // This function is called from FastData16Row(Intrin/Asm) which means we don't need C++ version
    if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
        COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = compv::FastStrengths16_Intrin_SSE2);
    }
    if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
        COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = compv::FastStrengths16_Intrin_SSE41);
        COMPV_EXEC_IFDEF_ASM_X86(FastStrengths = (N == 9)
                                 ? (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast9Strengths16_Asm_CMOV_X86_SSE41 : Fast9Strengths16_Asm_X86_SSE41)
                                     : (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast12Strengths16_Asm_CMOV_X86_SSE41 : Fast12Strengths16_Asm_X86_SSE41));
        COMPV_EXEC_IFDEF_ASM_X64(FastStrengths = (N == 9)
                                 ? (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast9Strengths16_Asm_CMOV_X64_SSE41 : Fast9Strengths16_Asm_X64_SSE41)
                                     : (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast12Strengths16_Asm_CMOV_X64_SSE41 : Fast12Strengths16_Asm_X64_SSE41));
    }
    FastStrengths(rbrighters, rdarkers, dbrighters16x16, ddarkers16x16, fbrighters16, fdarkers16, strengths16, N);
}

COMPV_EXTERNC COMPV_API void FastStrengths32(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x32, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x32, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv::compv_scalar_t N)
{
    void(*FastStrengths)(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N)
        = NULL;  // This function is called from FastData32Row(Intrin/Asm) which means we don't need C++ version
    if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
        COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = compv::FastStrengths32_Intrin_SSE41);
        COMPV_EXEC_IFDEF_ASM_X86(FastStrengths = (N == 9)
                                 ? (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast9Strengths32_Asm_CMOV_X86_SSE41 : Fast9Strengths32_Asm_X86_SSE41)
                                     : (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast12Strengths32_Asm_CMOV_X86_SSE41 : Fast12Strengths32_Asm_X86_SSE41));
        COMPV_EXEC_IFDEF_ASM_X64(FastStrengths = (N == 9)
                                 ? (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast9Strengths32_Asm_CMOV_X64_SSE41 : Fast9Strengths32_Asm_X64_SSE41)
                                     : (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast12Strengths32_Asm_CMOV_X64_SSE41 : Fast12Strengths32_Asm_X64_SSE41));
    }
    if (compv::CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
        COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = compv::FastStrengths32_Intrin_AVX2);
        COMPV_EXEC_IFDEF_ASM_X86(FastStrengths = (N == 9)
                                 ? (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast9Strengths32_Asm_CMOV_X86_AVX2 : Fast9Strengths32_Asm_X86_AVX2)
                                     : (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast12Strengths32_Asm_CMOV_X86_AVX2 : Fast12Strengths32_Asm_X86_AVX2));
        COMPV_EXEC_IFDEF_ASM_X64(FastStrengths = (N == 9)
                                 ? (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast9Strengths32_Asm_CMOV_X64_AVX2 : Fast9Strengths32_Asm_X64_AVX2)
                                     : (compv::CompVCpu::isEnabled(compv::kCpuFlagCMOV) ? Fast12Strengths32_Asm_CMOV_X64_AVX2 : Fast12Strengths32_Asm_X64_AVX2));
    }
    FastStrengths(rbrighters, rdarkers, dbrighters16x32, ddarkers16x32, fbrighters16, fdarkers16, strengths32, N);
}

COMPV_NAMESPACE_BEGIN()

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT			10
// Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP				true
#define COMPV_FEATURE_DETE_FAST_MAX_FEATURTES				2000 // maximum number of features to retain (<0 means all)
#define COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD		(200*250) // number of pixels
#define COMPV_FEATURE_DETE_FAST_NMS_MIN_SAMPLES_PER_THREAD	(80*80) // number of interestPoints

static int32_t COMPV_INLINE __continuousCount(int32_t fasType)
{
    switch (fasType) {
    case COMPV_FAST_TYPE_9:
        return 9;
    case COMPV_FAST_TYPE_12:
        return 12;
    default:
        COMPV_DEBUG_ERROR("Invalid fastType:%d", fasType);
        return 9;
    }
}

static bool COMPV_INLINE __compareStrengthDec(const CompVInterestPoint* i, const  CompVInterestPoint* j)
{
    return (i->strength > j->strength);
}
static bool COMPV_INLINE __isNonMaximal(const CompVInterestPoint* point)
{
    return point->x < 0;
}

static COMPV_ERROR_CODE FastProcessRange_AsynExec(const struct compv_asynctoken_param_xs* pc_params);
static void FastProcessRange(RangeFAST* range);
static COMPV_ERROR_CODE FastNMS_AsynExec(const struct compv_asynctoken_param_xs* pc_params);
static void FastNMS(int32_t stride, const uint8_t* pcStrengthsMap, CompVInterestPoint* begin, CompVInterestPoint* end);
static void FastDataRow1_C(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, uint8_t* strengths1, compv_scalar_t* me);
static void FastStrengths1_C(COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x1, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x1, const compv::compv_scalar_t fbrighters1, const compv::compv_scalar_t fdarkers1, uint8_t* strengths1, compv::compv_scalar_t N);
static COMPV_ERROR_CODE FastRangesAlloc(int32_t nRanges, RangeFAST** ppRanges, int32_t stride);
static COMPV_ERROR_CODE FastRangesFree(int32_t nRanges, RangeFAST** ppRanges);

CompVFeatureDeteFAST::CompVFeatureDeteFAST()
    : CompVFeatureDete(COMPV_FAST_ID)
    , m_iThreshold(COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT)
    , m_iType(COMPV_FAST_TYPE_9)
    , m_iNumContinuous(__continuousCount(COMPV_FAST_TYPE_9))
    , m_bNonMaximaSupp(COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP)
    , m_iMaxFeatures(COMPV_FEATURE_DETE_FAST_MAX_FEATURTES)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nStride(0)
    , m_pRanges(NULL)
    , m_nRanges(0)

    , m_pStrengthsMap(NULL)
{

}

CompVFeatureDeteFAST::~CompVFeatureDeteFAST()
{
    FastRangesFree(m_nRanges, &m_pRanges);
    CompVMem::free((void**)&m_pStrengthsMap);
}

// overrides CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDeteFAST::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_FAST_SET_INT32_THRESHOLD: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        int32_t threshold = *((int32_t*)valuePtr);
        m_iThreshold = COMPV_MATH_CLIP3(0, 255, threshold);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_INT32_MAX_FEATURES: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_iMaxFeatures = *((int32_t*)valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_INT32_FAST_TYPE: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        int32_t iType = *((int32_t*)valuePtr);
        COMPV_CHECK_EXP_RETURN(iType != COMPV_FAST_TYPE_9 && iType != COMPV_FAST_TYPE_12, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_iType = iType;
        m_iNumContinuous = __continuousCount(iType);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_bNonMaximaSupp = *((bool*)valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}

// overrides CompVFeatureDete::process
COMPV_ERROR_CODE CompVFeatureDeteFAST::process(const CompVPtr<CompVImage*>& image, CompVPtr<CompVBoxInterestPoint* >& interestPoints)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    const uint8_t* dataPtr = (const uint8_t*)image->getDataPtr();
    int32_t width = image->getWidth();
    int32_t height = image->getHeight();
    int32_t stride = image->getStride();
    CompVPtr<CompVThreadDispatcher* >threadDip = CompVEngine::getThreadDispatcher();
    int32_t threadsCountRange = 1;

    COMPV_CHECK_EXP_RETURN(width < 4 || height < 4, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    // Free ranges memory if stride increase
    if (m_nStride < stride) {
        FastRangesFree(m_nRanges, &m_pRanges);
        m_nRanges = 0;
    }

    // Even if newStride < oldStride, free the strenghtMap to cleanup old values.
    // FastData() function will cleanup only new matching positions.
    if (m_nStride != stride) {
        CompVMem::free((void**)&m_pStrengthsMap);
    }

    // Always alloc
    if (!m_pStrengthsMap) {
        size_t nStrengthMapSize = CompVMem::alignForward((3 + stride + 3) * (3 + height + 3)); // +3 for the borders, alignForward() for the SIMD functions
        m_pStrengthsMap = (uint8_t*)CompVMem::calloc(nStrengthMapSize, sizeof(uint8_t)); // Must use calloc to fill the strengths with null values
        COMPV_CHECK_EXP_RETURN(!m_pStrengthsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }

    // Update width and height
    m_nWidth = width;
    m_nHeight = height;
    m_nStride = stride;

    // create or reset points
    if (!interestPoints) {
        COMPV_CHECK_CODE_RETURN(CompVBoxInterestPoint::newObj(&interestPoints));
    }
    else {
        interestPoints->reset();
    }

    COMPV_ALIGN_DEFAULT() const compv_scalar_t pixels16[16] = {
        -(stride * 3) + 0, // 1
        -(stride * 3) + 1, // 2
        -(stride * 2) + 2, // 3
        -(stride * 1) + 3, // 4
        +(stride * 0) + 3, // 5
        +(stride * 1) + 3, // 6
        +(stride * 2) + 2, // 7
        +(stride * 3) + 1, // 8
        +(stride * 3) + 0, // 9
        +(stride * 3) - 1, // 10
        +(stride * 2) - 2, // 11
        +(stride * 1) - 3, // 12
        +(stride * 0) - 3, // 13
        -(stride * 1) - 3, // 14
        -(stride * 2) - 2, // 15
        -(stride * 3) - 1, // 16
    };

    // Compute number of threads
    if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
        threadsCountRange = threadDip->guessNumThreadsDividingAcrossY(stride, height, COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD);
    }

    // Alloc ranges
    if (m_nRanges < threadsCountRange) {
        m_nRanges = 0;
        COMPV_CHECK_CODE_RETURN(FastRangesAlloc(threadsCountRange, &m_pRanges, m_nStride));
        m_nRanges = threadsCountRange;
    }

    if (threadsCountRange > 1) {
        int32_t rowStart = 0, threadHeight, totalHeight = 0;
        uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
        RangeFAST* pRange;
        for (int i = 0; i < threadsCountRange; ++i) {
            threadHeight = ((height - totalHeight) / (threadsCountRange - i)) & -2; // the & -2 is to make sure we'll deal with odd heights
            pRange = &m_pRanges[i];
            pRange->IP = dataPtr;
            pRange->IPprev = NULL;
            pRange->rowStart = rowStart;
            pRange->rowEnd = (rowStart + threadHeight);
            pRange->rowCount = height;
            pRange->width = width;
            pRange->stride = stride;
            pRange->threshold = m_iThreshold;
            pRange->N = m_iNumContinuous;
            pRange->pixels16 = &pixels16;
            pRange->strengths = m_pStrengthsMap;
            COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX0, FastProcessRange_AsynExec,
                                    COMPV_ASYNCTASK_SET_PARAM_ASISS(pRange),
                                    COMPV_ASYNCTASK_SET_PARAM_NULL()));
            rowStart += threadHeight;
            totalHeight += threadHeight;
        }
        for (int32_t i = 0; i < threadsCountRange; ++i) {
            COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX0));
        }
    }
    else {
        RangeFAST* pRange = &m_pRanges[0];
        pRange->IP = dataPtr;
        pRange->IPprev = NULL;
        pRange->rowStart = 0;
        pRange->rowEnd = height;
        pRange->rowCount = height;
        pRange->width = width;
        pRange->stride = stride;
        pRange->threshold = m_iThreshold;
        pRange->N = m_iNumContinuous;
        pRange->pixels16 = &pixels16;
        pRange->strengths = m_pStrengthsMap;
        FastProcessRange(pRange);
    }

    // Build interest points
#define COMPV_PUSH1() if (*begin1) { *begin1 += thresholdMinus1; interestPoints->push(CompVInterestPoint((float)(begin1 - strengths), (float)j, (float)*begin1)); } ++begin1;
#define COMPV_PUSH4() COMPV_PUSH1() COMPV_PUSH1() COMPV_PUSH1() COMPV_PUSH1()
#define COMPV_PUSH8() COMPV_PUSH4() COMPV_PUSH4()
    uint8_t *strengths = m_pStrengthsMap + (3 * stride), *begin1;
    if (COMPV_IS_ALIGNED(stride, 64) && COMPV_IS_ALIGNED(CompVCpu::getCache1LineSize(), 64)) {
        uint64_t *begin8, *end8;
        int32_t width_div8 = width >> 3;
        const uint8_t thresholdMinus1 = (uint8_t)m_iThreshold - 1;
        for (int32_t j = 3; j < height - 3; ++j) {
            begin8 = (uint64_t*)(strengths + 0); // i can start at +3 but we prefer +0 because strengths[0] is cacheline-aligned
            end8 = (begin8 + width_div8);
            do {
                if (*begin8) {
                    begin1 = (uint8_t*)begin8;
                    COMPV_PUSH8();
                }
            }
            while (begin8++ < end8);
            strengths += stride;
        }
    }
    else {
        uint32_t *begin4, *end4;
        int32_t width_div4 = width >> 2;
        const uint8_t thresholdMinus1 = (uint8_t)m_iThreshold - 1;
        for (int32_t j = 3; j < height - 3; ++j) {
            begin4 = (uint32_t*)(strengths + 0); // i can start at +3 but we prefer +0 because strengths[0] is cacheline-aligned
            end4 = (begin4 + width_div4);
            do {
                if (*begin4) {
                    begin1 = (uint8_t*)begin4;
                    COMPV_PUSH4();
                }
            }
            while (begin4++ < end4);
            strengths += stride;
        }
    }


    // Non Maximal Suppression for removing adjacent corners
    if (m_bNonMaximaSupp && interestPoints->size() > 0) {
        int32_t threadsCountNMS = 1;
        // NMS
        if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
            threadsCountNMS = (int32_t)(interestPoints->size() / COMPV_FEATURE_DETE_FAST_NMS_MIN_SAMPLES_PER_THREAD);
            threadsCountNMS = COMPV_MATH_CLIP3(0, threadDip->getThreadsCount() - 1, threadsCountNMS);
        }
        if (threadsCountNMS > 1) {
            int32_t total = 0, count, size = (int32_t)interestPoints->size();
            CompVInterestPoint * begin = interestPoints->begin();
            uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
            for (int32_t i = 0; i < threadsCountNMS; ++i) {
                count = ((size - total) / (threadsCountNMS - i)) & -2;
                COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX0, FastNMS_AsynExec,
                                        COMPV_ASYNCTASK_SET_PARAM_ASISS(
                                            stride,
                                            m_pStrengthsMap,
                                            begin,
                                            begin + count),
                                        COMPV_ASYNCTASK_SET_PARAM_NULL()));
                begin += count;
                total += count;
            }
            for (int32_t i = 0; i < threadsCountNMS; ++i) {
                COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX0));
            }
        }
        else {
            FastNMS(stride, m_pStrengthsMap, interestPoints->begin(), interestPoints->end());
        }

        // Remove non maximal points
        interestPoints->erase(__isNonMaximal);
    }

    // Retain best "m_iMaxFeatures" features
    if (m_iMaxFeatures > 0 && (int32_t)interestPoints->size() > m_iMaxFeatures) {
		interestPoints->retainBest(m_iMaxFeatures);
    }

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDeteFAST::newObj(CompVPtr<CompVFeatureDete* >* fast)
{
    COMPV_CHECK_EXP_RETURN(fast == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVFeatureDeteFAST* >_fast = new CompVFeatureDeteFAST();
    if (!_fast) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *fast = *_fast;
    return COMPV_ERROR_CODE_S_OK;
}

// FastXFlags = Fast9Flags or Fast16Flags
static void FastStrengths1_C(COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x1, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x1, const compv::compv_scalar_t fbrighters1, const compv::compv_scalar_t fdarkers1, uint8_t* strengths1, compv::compv_scalar_t N)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

    uint8_t ndarker, nbrighter;
    unsigned i, j, k;
    int strength = 0;
    const uint16_t(&FastXFlags)[16] = N == 9 ? Fast9Flags : Fast12Flags;

    for (i = 0; i < 16; ++i) {
        ndarker = 255;
        nbrighter = 255;
        if ((fbrighters1 & FastXFlags[i]) == FastXFlags[i]) {
            // lowest diff
            k = unsigned(i + N);
            for (j = i; j < k; ++j) {
                if (dbrighters16x1[j & 15] < nbrighter) {
                    nbrighter = dbrighters16x1[j & 15];
                }
            }
        }
        if ((fdarkers1 & FastXFlags[i]) == FastXFlags[i]) {
            // lowest diff
            k = unsigned(i + N);
            for (j = i; j < k; ++j) {
                if (ddarkers16x1[j & 15] < ndarker) {
                    ndarker = ddarkers16x1[j & 15];
                }
            }
        }
        else if (nbrighter == 255) {
            // (nbrighter == 255 and ndarker == 255) -> nothing to do
            continue;
        }

        strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
    }

    *strengths1 = (uint8_t)strength;
}

static void FastDataRow1_C(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, uint8_t* strengths1, compv_scalar_t* me)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    int32_t sum, s;
    int16_t temp0, temp1, ddarkers16x1[16], dbrighters16x1[16]; // using int16_t to avoid clipping
    uint8_t ddarkers16[16], dbrighters16[16];
    compv_scalar_t fbrighters1, fdarkers1;
    void(*FastStrengths1)(
        COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x1,
        COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x1,
        const compv::compv_scalar_t fbrighters1,
        const compv::compv_scalar_t fdarkers1,
        uint8_t* strengths1,
        compv::compv_scalar_t N) = FastStrengths1_C;
#if 0
    FastStrengths1 = (N == 9 ? Fast9Strengths1_C : Fast12Strengths1_C);
#endif

    for (compv_scalar_t i = 0; i < width; ++i, ++IP, ++strengths1) {
        uint8_t brighter = CompVMathUtils::clampPixel8(IP[0] + (int16_t)threshold);
        uint8_t darker = CompVMathUtils::clampPixel8(IP[0] - (int16_t)threshold);

        // reset strength to zero
        *strengths1 = 0;

        // compare I1 and I9 aka 0 and 8
        temp0 = IP[pixels16[0]];
        temp1 = IP[pixels16[8]];
        ddarkers16x1[0] = (darker - temp0);
        ddarkers16x1[8] = (darker - temp1);
        dbrighters16x1[0] = (temp0 - brighter);
        dbrighters16x1[8] = (temp1 - brighter);
        sum = ((dbrighters16x1[0] > 0 || ddarkers16x1[0] > 0) ? 1 : 0) + ((dbrighters16x1[8] > 0 || ddarkers16x1[8] > 0) ? 1 : 0);
        if (!sum) {
            continue;
        }

        // compare I5 and I13 aka 4 and 12
        temp0 = IP[pixels16[4]];
        temp1 = IP[pixels16[12]];
        ddarkers16x1[4] = (darker - temp0); // I5-darkness
        ddarkers16x1[12] = (darker - temp1); // I13-darkness
        dbrighters16x1[4] = (temp0 - brighter); // I5-brightness
        dbrighters16x1[12] = (temp1 - brighter); // I13-brightness
        s = ((dbrighters16x1[4] > 0 || ddarkers16x1[4] > 0) ? 1 : 0) + ((dbrighters16x1[12] > 0 || ddarkers16x1[12] > 0) ? 1 : 0);
        if (!s) {
            continue;
        }
        sum += s;
        if (N == 12 ? sum < 3 : sum < 2) {
            continue;
        }

        // I2 and I10 aka 1 and 9
        temp0 = IP[pixels16[1]];
        temp1 = IP[pixels16[9]];
        ddarkers16x1[1] = (darker - temp0);
        dbrighters16x1[1] = (temp0 - brighter);
        ddarkers16x1[9] = (darker - temp1);
        dbrighters16x1[9] = (temp1 - brighter);
        sum = ((dbrighters16x1[1] > 0 || ddarkers16x1[1] > 0) ? 1 : 0) + ((dbrighters16x1[9] > 0 || ddarkers16x1[9] > 0) ? 1 : 0);
        if (!sum) {
            continue;
        }

        // I3 and I11 aka 2 and 10
        temp0 = IP[pixels16[2]];
        temp1 = IP[pixels16[10]];
        ddarkers16x1[2] = (darker - temp0);
        dbrighters16x1[2] = (temp0 - brighter);
        ddarkers16x1[10] = (darker - temp1);
        dbrighters16x1[10] = (temp1 - brighter);
        s = ((dbrighters16x1[2] > 0 || ddarkers16x1[2] > 0) ? 1 : 0) + ((dbrighters16x1[10] > 0 || ddarkers16x1[10] > 0) ? 1 : 0);
        if (!s) {
            continue;
        }
        sum += s;
        if (N == 12 ? sum < 3 : sum < 2) {
            continue;
        }

        // I4 and I12 aka 3 and 11
        temp0 = IP[pixels16[3]];
        temp1 = IP[pixels16[11]];
        ddarkers16x1[3] = (darker - temp0);
        dbrighters16x1[3] = (temp0 - brighter);
        ddarkers16x1[11] = (darker - temp1);
        dbrighters16x1[11] = (temp1 - brighter);
        s = ((dbrighters16x1[3] > 0 || ddarkers16x1[3] > 0) ? 1 : 0) + ((dbrighters16x1[11] > 0 || ddarkers16x1[11] > 0) ? 1 : 0);
        if (!s) {
            continue;
        }
        sum += s;
        if (N == 12 ? sum < 3 : sum < 2) {
            continue;
        }

        // I6 and I14 aka 5 and 13
        temp0 = IP[pixels16[5]];
        temp1 = IP[pixels16[13]];
        ddarkers16x1[5] = (darker - temp0);
        dbrighters16x1[5] = (temp0 - brighter);
        ddarkers16x1[13] = (darker - temp1);
        dbrighters16x1[13] = (temp1 - brighter);
        s = ((dbrighters16x1[5] > 0 || ddarkers16x1[5] > 0) ? 1 : 0) + ((dbrighters16x1[13] > 0 || ddarkers16x1[13] > 0) ? 1 : 0);
        if (!s) {
            continue;
        }
        sum += s;
        if (N == 12 ? sum < 3 : sum < 2) {
            continue;
        }

        // I7 and I15 aka 6 and 14
        temp0 = IP[pixels16[6]];
        temp1 = IP[pixels16[14]];
        ddarkers16x1[6] = (darker - temp0);
        dbrighters16x1[6] = (temp0- brighter);
        ddarkers16x1[14] = (darker - temp1);
        dbrighters16x1[14] = (temp1- brighter);
        sum = ((dbrighters16x1[6] > 0 || ddarkers16x1[6] > 0) ? 1 : 0) + ((dbrighters16x1[14] > 0 || ddarkers16x1[14] > 0) ? 1 : 0);
        if (!sum) {
            continue;
        }

        // I8 and I16 aka 7 and 15
        temp0 = IP[pixels16[7]];
        temp1 = IP[pixels16[15]];
        ddarkers16x1[7] = (darker - temp0);
        dbrighters16x1[7] = (temp0 - brighter);
        ddarkers16x1[15] = (darker - temp1);
        dbrighters16x1[15] = (temp1 - brighter);
        s = ((dbrighters16x1[7] > 0 || ddarkers16x1[7] > 0) ? 1 : 0) + ((dbrighters16x1[15] > 0 || ddarkers16x1[15] > 0) ? 1 : 0);
        if (!s) {
            continue;
        }
        sum += s;
        if (N == 12 ? sum < 3 : sum < 2) {
            continue;
        }

#define DARKERS_SET_AND_CLIP(k) if (ddarkers16x1[k] > 0) fdarkers1 |= (1 << k), ddarkers16[k] = (uint8_t)ddarkers16x1[k]; else ddarkers16[k] = 0;
        fdarkers1 = 0;
        DARKERS_SET_AND_CLIP(0);
        DARKERS_SET_AND_CLIP(1);
        DARKERS_SET_AND_CLIP(2);
        DARKERS_SET_AND_CLIP(3);
        DARKERS_SET_AND_CLIP(4);
        DARKERS_SET_AND_CLIP(5);
        DARKERS_SET_AND_CLIP(6);
        DARKERS_SET_AND_CLIP(7);
        DARKERS_SET_AND_CLIP(8);
        DARKERS_SET_AND_CLIP(9);
        DARKERS_SET_AND_CLIP(10);
        DARKERS_SET_AND_CLIP(11);
        DARKERS_SET_AND_CLIP(12);
        DARKERS_SET_AND_CLIP(13);
        DARKERS_SET_AND_CLIP(14);
        DARKERS_SET_AND_CLIP(15);

#define BRIGHTERS_SET_AND_CLIP(k) if (dbrighters16x1[k] > 0) fbrighters1 |= (1 << k), dbrighters16[k] = (uint8_t)dbrighters16x1[k]; else dbrighters16[k] = 0;
        fbrighters1 = 0;
        BRIGHTERS_SET_AND_CLIP(0);
        BRIGHTERS_SET_AND_CLIP(1);
        BRIGHTERS_SET_AND_CLIP(2);
        BRIGHTERS_SET_AND_CLIP(3);
        BRIGHTERS_SET_AND_CLIP(4);
        BRIGHTERS_SET_AND_CLIP(5);
        BRIGHTERS_SET_AND_CLIP(6);
        BRIGHTERS_SET_AND_CLIP(7);
        BRIGHTERS_SET_AND_CLIP(8);
        BRIGHTERS_SET_AND_CLIP(9);
        BRIGHTERS_SET_AND_CLIP(10);
        BRIGHTERS_SET_AND_CLIP(11);
        BRIGHTERS_SET_AND_CLIP(12);
        BRIGHTERS_SET_AND_CLIP(13);
        BRIGHTERS_SET_AND_CLIP(14);
        BRIGHTERS_SET_AND_CLIP(15);

        FastStrengths1(dbrighters16, ddarkers16, fbrighters1, fdarkers1, strengths1, N);

    } // for (i ....width)
}

static void FastProcessRange(RangeFAST* range)
{
    const uint8_t* IP, *IPprev;
    int32_t j, kalign, kextra, align = 1, minj, maxj, rowstart, k;
    uint8_t *strengths, *extra;
    void(*FastDataRow)(
        const uint8_t* IP,
        const uint8_t* IPprev,
        compv_scalar_t width,
        const compv_scalar_t(&pixels16)[16],
        compv_scalar_t N,
        compv_scalar_t threshold,
        uint8_t* strengths,
        compv_scalar_t* me) = FastDataRow1_C;

    if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
        COMPV_EXEC_IFDEF_INTRIN_X86((FastDataRow = FastData16Row_Intrin_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
        COMPV_EXEC_IFDEF_ASM_X86((FastDataRow = FastData16Row_Asm_X86_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
        COMPV_EXEC_IFDEF_ASM_X64((FastDataRow = FastData16Row_Asm_X64_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
    }
    if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
        COMPV_EXEC_IFDEF_INTRIN_X86((FastDataRow = FastData32Row_Intrin_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
        COMPV_EXEC_IFDEF_ASM_X86((FastDataRow = FastData32Row_Asm_X86_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
        COMPV_EXEC_IFDEF_ASM_X64((FastDataRow = FastData32Row_Asm_X64_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
    }

    // Number of pixels to process (multiple of align)
    kalign = (int32_t)CompVMem::alignForward((-3 + range->width - 3), align);
    if (kalign > (range->stride - 3)) { // must never happen as the image always contains a border(default 7) aligned on 64B
        COMPV_DEBUG_ERROR("Unexpected code called. k16=%d, stride=%d", kalign, range->stride);
        COMPV_ASSERT(false);
        return;
    }
    // Number of pixels to ignore
    kextra = kalign - (-3 + range->width - 3);

    rowstart = range->rowStart;
    minj = (rowstart == 0 ? 3 : 0);
    maxj = (range->rowEnd - rowstart) - ((range->rowCount - range->rowEnd) <= 3 ? 3 - (range->rowCount - range->rowEnd) : 0);
    IP = range->IP + ((rowstart + minj) * range->stride) + 3;
    strengths = range->strengths + ((rowstart + minj) * range->stride) + 3;
    IPprev = range->IPprev ? (range->IPprev + ((rowstart + minj) * range->stride) + 3) : NULL;


    // For testing with image "voiture", the first (i,j) to produce an interesting point is (1620, 279)
    // We should have 64586 non-zero results for SSE and 66958 for AVX2

    for (j = minj; j < maxj; ++j) {
        FastDataRow(IP, IPprev, kalign, (*range->pixels16), range->N, range->threshold, strengths, NULL);

        // remove extra samples
        extra = &strengths[kalign - 1];
        for (k = 0; k < kextra; ++k) {
            *extra-- = 0;
        }
        IP += range->stride;
        strengths += range->stride;
    } // for (j)
}

static COMPV_ERROR_CODE FastProcessRange_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
    RangeFAST* range = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, RangeFAST*);
    FastProcessRange(range);
    return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE FastNMS_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
    int32_t stride = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, int32_t);
    const uint8_t* pcStrengthsMap = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, const uint8_t*);
    CompVInterestPoint* begin = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, CompVInterestPoint*);
    CompVInterestPoint* end = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, CompVInterestPoint*);
    FastNMS(stride, pcStrengthsMap, begin, end);
    return COMPV_ERROR_CODE_S_OK;
}

static void FastNMS(int32_t stride, const uint8_t* pcStrengthsMap, CompVInterestPoint* begin, CompVInterestPoint* end)
{
    uint8_t strength;
    int32_t currentIdx;
    for (; begin < end; ++begin) {
        strength = (uint8_t)begin->strength;
        currentIdx = (int32_t)((begin->y * stride) + begin->x);
        // No need to chech index as the point always has coords in (x+3, y+3)
        // If-Else faster than a single if(|||||||)
        if (pcStrengthsMap[currentIdx - 1] >= strength) { // left
            begin->x = -1;
        }
        else if (pcStrengthsMap[currentIdx + 1] >= strength) { // right
            begin->x = -1;
        }
        else if (pcStrengthsMap[currentIdx - stride - 1] >= strength) { // left-top
            begin->x = -1;
        }
        else if (pcStrengthsMap[currentIdx - stride] >= strength) { // top
            begin->x = -1;
        }
        else if (pcStrengthsMap[currentIdx - stride + 1] >= strength) { // right-top
            begin->x = -1;
        }
        else if (pcStrengthsMap[currentIdx + stride - 1] >= strength) { // left-bottom
            begin->x = -1;
        }
        else if (pcStrengthsMap[currentIdx + stride] >= strength) { // bottom
            begin->x = -1;
        }
        else if (pcStrengthsMap[currentIdx + stride + 1] >= strength) { // right-bottom
            begin->x = -1;
        }
    }
}

static COMPV_ERROR_CODE FastRangesAlloc(int32_t nRanges, RangeFAST** ppRanges, int32_t stride)
{
    // COMPV_DEBUG_INFO("FAST: alloc %d ranges", nRanges);

    COMPV_CHECK_EXP_RETURN(nRanges <= 0 || !ppRanges, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_CHECK_CODE_RETURN(FastRangesFree(nRanges, ppRanges));

    *ppRanges = (RangeFAST*)CompVMem::calloc(nRanges, sizeof(RangeFAST));
    COMPV_CHECK_EXP_RETURN(!*ppRanges, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
#if 0
    RangeFAST* pRanges = *ppRanges;
    for (int32_t i = 0; i < nRanges; ++i) {

        pRanges[i].me = (compv_scalar_t*)CompVMem::malloc(stride * 1 * sizeof(compv_scalar_t));
        COMPV_CHECK_EXP_RETURN(!pRanges[i].me, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
#endif
    return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE FastRangesFree(int32_t nRanges, RangeFAST** ppRanges)
{
    if (ppRanges && *ppRanges) {
#if 0
        RangeFAST* pRanges = *ppRanges;
        for (int32_t i = 0; i < nRanges; ++i) {
            CompVMem::free((void**)&pRanges[i].me);
        }
#endif
        CompVMem::free((void**)ppRanges);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
