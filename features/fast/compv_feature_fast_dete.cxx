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
#include "compv/compv_mathutils.h"
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
#if defined(COMPV_ARCH_X86) && (defined(COMPV_INTRINSIC) || defined(COMPV_ASM))
// Values generated using FastShufflesArc() in "tests/fast.cxx"
extern "C" COMPV_GEXTERN const COMPV_ALIGN_DEFAULT() uint8_t kFast9Arcs[16/*ArcStartIdx*/][16] = { // SHUFFLE_EPI8 values to select an arc
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
extern "C" COMPV_GEXTERN const COMPV_ALIGN_DEFAULT() uint8_t kFast12Arcs[16/*ArcStartIdx*/][16] = { // SHUFFLE_EPI8 values to select an arc
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
extern "C" COMPV_GEXTERN const COMPV_ALIGN_DEFAULT() uint16_t Fast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
extern "C" COMPV_GEXTERN const COMPV_ALIGN_DEFAULT() uint16_t Fast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };

// X86
#if defined(COMPV_ARCH_X86) && defined(COMPV_ASM)
extern "C" void FastData32Row_Asm_X86_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, COMPV_ALIGNED(AVX2) compv::compv_scalar_t(*pfdarkers16)[16], COMPV_ALIGNED(AVX2) compv::compv_scalar_t(*pfbrighters16)[16], COMPV_ALIGNED(AVX2) uint8_t* ddarkers16x32, COMPV_ALIGNED(AVX2) uint8_t* dbrighters16x32, compv::compv_scalar_t* rd, compv::compv_scalar_t* rb, compv::compv_scalar_t* me);

extern "C" void FastData16Row_Asm_X86_SSE2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv::compv_scalar_t(*pfdarkers16)[16], COMPV_ALIGNED(SSE) compv::compv_scalar_t(*pfbrighters16)[16], COMPV_ALIGNED(SSE) uint8_t* ddarkers16x16, COMPV_ALIGNED(SSE) uint8_t* dbrighters16x16, compv::compv_scalar_t* rd, compv::compv_scalar_t* rb, compv::compv_scalar_t* me);

extern "C" void Fast9Strengths16_Asm_CMOV_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
extern "C" void Fast9Strengths16_Asm_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
extern "C" void Fast12Strengths16_Asm_CMOV_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
extern "C" void Fast12Strengths16_Asm_X86_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
#endif
// X64
#if defined(COMPV_ARCH_X64) && defined(COMPV_ASM)
extern "C" void FastData32Row_Asm_X64_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, COMPV_ALIGNED(AVX2) compv::compv_scalar_t(*pfdarkers16)[16], COMPV_ALIGNED(AVX2) compv::compv_scalar_t(*pfbrighters16)[16], COMPV_ALIGNED(AVX2) uint8_t* ddarkers16x32, COMPV_ALIGNED(AVX2) uint8_t* dbrighters16x32, compv::compv_scalar_t* rd, compv::compv_scalar_t* rb, compv::compv_scalar_t* me);

extern "C" void FastData16Row_Asm_X64_SSE2(const uint8_t* IP, const uint8_t* IPprev, compv::compv_scalar_t width, const compv::compv_scalar_t(&pixels16)[16], compv::compv_scalar_t N, compv::compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv::compv_scalar_t(*pfdarkers16)[16], COMPV_ALIGNED(SSE) compv::compv_scalar_t(*pfbrighters16)[16], COMPV_ALIGNED(SSE) uint8_t* ddarkers16x16, COMPV_ALIGNED(SSE) uint8_t* dbrighters16x16, compv::compv_scalar_t* rd, compv::compv_scalar_t* rb, compv::compv_scalar_t* me);

extern "C" void Fast9Strengths16_Asm_CMOV_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
extern "C" void Fast9Strengths16_Asm_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
extern "C" void Fast12Strengths16_Asm_CMOV_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);
extern "C" void Fast12Strengths16_Asm_X64_SSE41(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N);

#endif

extern "C" COMPV_GEXTERN void FastStrengths16(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16x16, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16x16, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N)
{
	void(*FastStrengths)(compv::compv_scalar_t rbrighters, compv::compv_scalar_t rdarkers, COMPV_ALIGNED(DEFAULT) const uint8_t* dbrighters16xAlign, COMPV_ALIGNED(DEFAULT) const uint8_t* ddarkers16xAlign, const compv::compv_scalar_t(*fbrighters16)[16], const compv::compv_scalar_t(*fdarkers16)[16], uint8_t* strengths16, compv::compv_scalar_t N)
		= NULL; // FIXME: CPP version
	if (compv::CompVCpu::isSupported(compv::kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = compv::FastStrengths16_Intrin_SSE2);
	}
	if (compv::CompVCpu::isSupported(compv::kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = compv::FastStrengths16_Intrin_SSE41);
		COMPV_EXEC_IFDEF_ASM_X86(FastStrengths = (N == 9)
			? (compv::CompVCpu::isSupported(compv::kCpuFlagCMOV) ? Fast9Strengths16_Asm_CMOV_X86_SSE41 : Fast9Strengths16_Asm_X86_SSE41)
			: (compv::CompVCpu::isSupported(compv::kCpuFlagCMOV) ? Fast12Strengths16_Asm_CMOV_X86_SSE41 : Fast12Strengths16_Asm_X86_SSE41));
		COMPV_EXEC_IFDEF_ASM_X64(FastStrengths = (N == 9)
			? (compv::CompVCpu::isSupported(compv::kCpuFlagCMOV) ? Fast9Strengths16_Asm_CMOV_X64_SSE41 : Fast9Strengths16_Asm_X64_SSE41)
			: (compv::CompVCpu::isSupported(compv::kCpuFlagCMOV) ? Fast12Strengths16_Asm_CMOV_X64_SSE41 : Fast12Strengths16_Asm_X64_SSE41));
	}
	FastStrengths(rbrighters, rdarkers, dbrighters16x16, ddarkers16x16, fbrighters16, fdarkers16, strengths16, N);
}

COMPV_NAMESPACE_BEGIN()

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT		10
// Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP			true
#define COMPV_FEATURE_DETE_FAST_MAX_FEATURTES			-1 // maximum number of features to retain (<0 means all)
#define COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD		(250*250) // number of pixels
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
static compv_scalar_t FastData_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, int16_t(&ddarkers16)[16], int16_t(&dbrighters16)[16]);
static compv_scalar_t FastData16_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t(&pfdarkers16)[16], compv_scalar_t(&pfbrighters16)[16], int16_t(&ddarkers16x16)[16][16], int16_t(&dbrighters16x16)[16][16]);
static compv_scalar_t FastStrengths_C(const uint8_t(&dbrighters)[16], const uint8_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, const uint16_t(&FastXFlags)[16]);
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
COMPV_ERROR_CODE CompVFeatureDeteFAST::process(const CompVObjWrapper<CompVImage*>& image, CompVObjWrapper<CompVBoxInterestPoint* >& interestPoints)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    const uint16_t(&FastXFlags)[16] = m_iType == COMPV_FAST_TYPE_9 ? Fast9Flags : Fast12Flags;
    const uint8_t* dataPtr = (const uint8_t*)image->getDataPtr();
    int32_t width = image->getWidth();
    int32_t height = image->getHeight();
    int32_t stride = image->getStride();
    CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();
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
	uint8_t* strengths = m_pStrengthsMap + (3 * stride);
	uint8_t thresholdMinus1 = (uint8_t)m_iThreshold - 1;
	for (int32_t j = 3; j < height - 3; ++j) {
		for (int32_t i = 3; i < width - 3; ++i) {
			if (strengths[i]) {
				strengths[i] += thresholdMinus1;
				interestPoints->push(CompVInterestPoint(i, j, (float)strengths[i]));
			}
		}
		strengths += stride;
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

	// cleanup strengths map
	//CompVInterestPoint *begin, *end = interestPoints->end();
	//for (begin = interestPoints->begin(); begin < end; ++begin) {
	//	currentIdx = (begin->y * stride) + begin->x;
	//	m_pStrengthsMap[currentIdx] = 0;
	//}

    // Retain best "m_iMaxFeatures" features
    // TODO(dmi): use retainBest
    if (m_iMaxFeatures > 0 && (int32_t)interestPoints->size() > m_iMaxFeatures) {
		interestPoints->sort(__compareStrengthDec); // TODO(dmi): use sortStrengh() which is faster
        interestPoints->resize(m_iMaxFeatures);
    }

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDeteFAST::newObj(CompVObjWrapper<CompVFeatureDete* >* fast)
{
    COMPV_CHECK_EXP_RETURN(fast == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVFeatureDeteFAST* >_fast = new CompVFeatureDeteFAST();
    if (!_fast) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *fast = *_fast;
    return COMPV_ERROR_CODE_S_OK;
}

// FastXFlags = Fast9Flags or Fast16Flags
static compv_scalar_t FastStrengths_C(const uint8_t(&dbrighters)[16], const uint8_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, const uint16_t(&FastXFlags)[16])
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

    int16_t ndarker, nbrighter;
    unsigned i, j, k;
    int strength = 0;

    for (i = 0; i < 16; ++i) {
        ndarker = 255;
        nbrighter = 255;
        if ((fbrighters & FastXFlags[i]) == FastXFlags[i]) {
            // lowest diff
            k = unsigned(i + N);
            for (j = i; j < k; ++j) {
                if (dbrighters[j & 15] < nbrighter) {
                    nbrighter = dbrighters[j & 15];
                }
            }
        }
        if ((fdarkers & FastXFlags[i]) == FastXFlags[i]) {
            // lowest diff
            k = unsigned(i + N);
            for (j = i; j < k; ++j) {
                if (ddarkers[j & 15] < ndarker) {
                    ndarker = ddarkers[j & 15];
                }
            }
        }
        else if (nbrighter == 255) {
            continue;
        }

        strength = (std::max(strength, std::min((int)ndarker, (int)nbrighter)));
    }

    return compv_scalar_t(strength);
}

// FIXME: remove temp16 (SIMD doesn't need it)
static compv_scalar_t FastData_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, uint8_t(&ddarkers16)[16], uint8_t(&dbrighters16)[16])
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    int32_t sum;
    uint8_t temp16[16];

    uint8_t brighter = CompVMathUtils::clampPixel8(dataPtr[0] + (int16_t)threshold);
    uint8_t darker = CompVMathUtils::clampPixel8(dataPtr[0] - (int16_t)threshold);

    bool popcntHard = CompVCpu::isSupported(kCpuFlagPOPCNT);

    // compare I1 and I7
    temp16[0] = dataPtr[pixels16[0]];
    temp16[8] = dataPtr[pixels16[8]];
    ddarkers16[0] = CompVMathUtils::clampPixel8(darker - temp16[0]);
    ddarkers16[8] = CompVMathUtils::clampPixel8(darker - temp16[8]);
    dbrighters16[0] = CompVMathUtils::clampPixel8(temp16[0] - brighter);
    dbrighters16[8] = CompVMathUtils::clampPixel8(temp16[8] - brighter);

    sum = (dbrighters16[0] > 0 || ddarkers16[0] > 0) + (dbrighters16[8] > 0 || ddarkers16[8] > 0);

    // compare I5 and I13
    /*  Speed-Test-1 */
    if (N != 12 || sum > 0) {
        temp16[4] = dataPtr[pixels16[4]];
        temp16[12] = dataPtr[pixels16[12]];
        ddarkers16[4] = CompVMathUtils::clampPixel8(darker - temp16[4]); // I5-darkness
        ddarkers16[12] = CompVMathUtils::clampPixel8(darker - temp16[12]); // I13-darkness
        dbrighters16[4] = CompVMathUtils::clampPixel8(temp16[4] - brighter); // I5-brightness
        dbrighters16[12] = CompVMathUtils::clampPixel8(temp16[12] - brighter); // I13-brightness

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

            // 0, 8, 4 and 12 already filled by the calling function
            *pfdarkers = (ddarkers16[0] > 0 ? (1 << 0) : 0);
            *pfdarkers |= (ddarkers16[8] > 0 ? (1 << 8) : 0);
            *pfdarkers |= (ddarkers16[4] > 0 ? (1 << 4) : 0);
            *pfdarkers |= (ddarkers16[12] > 0 ? (1 << 12) : 0);

            ddarkers16[1] = CompVMathUtils::clampPixel8(darker - temp16[1]);
            *pfdarkers |= (ddarkers16[1] > 0 ? (1 << 1) : 0);
            ddarkers16[2] = CompVMathUtils::clampPixel8(darker - temp16[2]);
            *pfdarkers |= (ddarkers16[2] > 0 ? (1 << 2) : 0);
            ddarkers16[3] = CompVMathUtils::clampPixel8(darker - temp16[3]);
            *pfdarkers |= (ddarkers16[3] > 0 ? (1 << 3) : 0);
            ddarkers16[5] = CompVMathUtils::clampPixel8(darker - temp16[5]);
            *pfdarkers |= (ddarkers16[5] > 0 ? (1 << 5) : 0);
            ddarkers16[6] = CompVMathUtils::clampPixel8(darker - temp16[6]);
            *pfdarkers |= (ddarkers16[6] > 0 ? (1 << 6) : 0);
            ddarkers16[7] = CompVMathUtils::clampPixel8(darker - temp16[7]);
            *pfdarkers |= (ddarkers16[7] > 0 ? (1 << 7) : 0);
            ddarkers16[9] = CompVMathUtils::clampPixel8(darker - temp16[9]);
            *pfdarkers |= (ddarkers16[9] > 0 ? (1 << 9) : 0);
            ddarkers16[10] = CompVMathUtils::clampPixel8(darker - temp16[10]);
            *pfdarkers |= (ddarkers16[10] > 0 ? (1 << 10) : 0);
            ddarkers16[11] = CompVMathUtils::clampPixel8(darker - temp16[11]);
            *pfdarkers |= (ddarkers16[11] > 0 ? (1 << 11) : 0);
            ddarkers16[13] = CompVMathUtils::clampPixel8(darker - temp16[13]);
            *pfdarkers |= (ddarkers16[13] > 0 ? (1 << 13) : 0);
            ddarkers16[14] = CompVMathUtils::clampPixel8(darker - temp16[14]);
            *pfdarkers |= (ddarkers16[14] > 0 ? (1 << 14) : 0);
            ddarkers16[15] = CompVMathUtils::clampPixel8(darker - temp16[15]);
            *pfdarkers |= (ddarkers16[15] > 0 ? (1 << 15) : 0);

            // 0, 8, 4 and 12 already filled by the calling function
            *pfbrighters = (dbrighters16[0] > 0 ? (1 << 0) : 0);
            *pfbrighters |= (dbrighters16[8] > 0 ? (1 << 8) : 0);
            *pfbrighters |= (dbrighters16[4] > 0 ? (1 << 4) : 0);
            *pfbrighters |= (dbrighters16[12] > 0 ? (1 << 12) : 0);

            dbrighters16[1] = CompVMathUtils::clampPixel8(temp16[1] - brighter);
            *pfbrighters |= (dbrighters16[1] > 0 ? (1 << 1) : 0);
            dbrighters16[2] = CompVMathUtils::clampPixel8(temp16[2] - brighter);
            *pfbrighters |= (dbrighters16[2] > 0 ? (1 << 2) : 0);
            dbrighters16[3] = CompVMathUtils::clampPixel8(temp16[3] - brighter);
            *pfbrighters |= (dbrighters16[3] > 0 ? (1 << 3) : 0);
            dbrighters16[5] = CompVMathUtils::clampPixel8(temp16[5] - brighter);
            *pfbrighters |= (dbrighters16[5] > 0 ? (1 << 5) : 0);
            dbrighters16[6] = CompVMathUtils::clampPixel8(temp16[6] - brighter);
            *pfbrighters |= (dbrighters16[6] > 0 ? (1 << 6) : 0);
            dbrighters16[7] = CompVMathUtils::clampPixel8(temp16[7] - brighter);
            *pfbrighters |= (dbrighters16[7] > 0 ? (1 << 7) : 0);
            dbrighters16[9] = CompVMathUtils::clampPixel8(temp16[9] - brighter);
            *pfbrighters |= (dbrighters16[9] > 0 ? (1 << 9) : 0);
            dbrighters16[10] = CompVMathUtils::clampPixel8(temp16[10] - brighter);
            *pfbrighters |= (dbrighters16[10] > 0 ? (1 << 10) : 0);
            dbrighters16[11] = CompVMathUtils::clampPixel8(temp16[11] - brighter);
            *pfbrighters |= (dbrighters16[11] > 0 ? (1 << 11) : 0);
            dbrighters16[13] = CompVMathUtils::clampPixel8(temp16[13] - brighter);
            *pfbrighters |= (dbrighters16[13] > 0 ? (1 << 13) : 0);
            dbrighters16[14] = CompVMathUtils::clampPixel8(temp16[14] - brighter);
            *pfbrighters |= (dbrighters16[14] > 0 ? (1 << 14) : 0);
            dbrighters16[15] = CompVMathUtils::clampPixel8(temp16[15] - brighter);
            *pfbrighters |= (dbrighters16[15] > 0 ? (1 << 15) : 0);

            // FIXME: not correct
            return (compv_popcnt16(popcntHard, (unsigned short)*pfdarkers) >= N || compv_popcnt16(popcntHard, (unsigned short)*pfbrighters) >= N) ? 1 : 0;
        }
    }
    return compv_scalar_t(0);
}

static compv_scalar_t FastData16_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t(&pfdarkers16)[16], compv_scalar_t(&pfbrighters16)[16], uint8_t(&ddarkers16x16)[16][16], uint8_t(&dbrighters16x16)[16][16])
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

    int r = 0;
    for (int i = 0; i < 16; ++i) {
        if (FastData_C(&dataPtr[i], pixels16, N, threshold, &pfdarkers16[i], &pfbrighters16[i], ddarkers16x16[i], dbrighters16x16[i])) {
            r |= (1 << i);
        }
    }
    return compv_scalar_t(r);
}

static void FastProcessRange(RangeFAST* range)
{
    const uint8_t* IP, *IPprev;
	int32_t j, kalign, kextra, align = 1, alignTimes16, minj, maxj, rowstart, k;
    const uint16_t(&FastXFlags)[16] = range->N == 9 ? Fast9Flags : Fast12Flags; // FIXME: needed?
	uint8_t *strengths, *extra;
	int32_t stride = range->stride;
	void(*FastData16Row)(
		const uint8_t* IP,
		const uint8_t* IPprev,
		compv_scalar_t width,
		const compv_scalar_t(&pixels16)[16],
		compv_scalar_t N,
		compv_scalar_t threshold,
		uint8_t* strengths,
		compv_scalar_t* me) = NULL; // FIXME: C++ version

    // FIXME: remove all FastData16 (INTRIN, ASM, C++) and FastData -> Only FastData16Row
	// FIXME: C++ version doesn't work

    if (CompVCpu::isSupported(kCpuFlagSSE2)) {
        //COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = FastStrengths16_Intrin_SSE2);
		COMPV_EXEC_IFDEF_INTRIN_X86((FastData16Row = FastData16Row_Intrin_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
		//COMPV_EXEC_IFDEF_ASM_X86((FastData16Row = FastData16Row_Asm_X86_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
		//COMPV_EXEC_IFDEF_ASM_X64((FastData16Row = FastData16Row_Asm_X64_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
    }
   // if (CompVCpu::isSupported(kCpuFlagSSE41)) {
   //     COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = FastStrengths_SSE41);
   //     COMPV_EXEC_IFDEF_ASM_X86(FastStrengths = (range->N == 9)
   //                              ? (CompVCpu::isSupported(kCpuFlagCMOV) ? Fast9Strengths_Asm_CMOV_X86_SSE41 : Fast9Strengths_Asm_X86_SSE41)
    //                            : (CompVCpu::isSupported(kCpuFlagCMOV) ? Fast12Strengths_Asm_CMOV_X86_SSE41 : Fast12Strengths_Asm_X86_SSE41));
    //}
	//if (CompVCpu::isSupported(kCpuFlagAVX2)) {
	//	COMPV_EXEC_IFDEF_INTRIN_X86((FastData16Row = FastData32Row_Intrin_AVX2, align = COMPV_SIMD_ALIGNV_AVX2));
	//	COMPV_EXEC_IFDEF_ASM_X86((FastData16Row = FastData32Row_Asm_X86_AVX2, align = COMPV_SIMD_ALIGNV_AVX2)); // asm too much faster than intrin
	//	COMPV_EXEC_IFDEF_ASM_X64((FastData16Row = FastData32Row_Asm_X64_AVX2, align = COMPV_SIMD_ALIGNV_AVX2)); // TODO(dmi): asm not so much fatsre than intrin
	//}

    // Number of pixels to process (multiple of align)
    kalign = (int32_t)CompVMem::alignForward((-3 + range->width - 3), align);
    if (kalign > (range->stride - 3)) { // must never happen as the image always contains a border(default 7) aligned on 64B
        COMPV_DEBUG_ERROR("Unexpected code called. k16=%d, stride=%d", kalign, range->stride);
        COMPV_ASSERT(false);
        return;
    }
	alignTimes16 = align << 4;
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

	// FIXME
	static uint64_t kaka = 0;
    
    for (j = minj; j < maxj; ++j) {
		FastData16Row(IP, IPprev, kalign, (*range->pixels16), range->N, range->threshold, strengths, NULL);

		// remove extra samples
		extra = &strengths[kalign - 1];
		for (k = 0; k < kextra; ++k) {
			*extra-- = 0;
		}
        IP += range->stride;
		strengths += range->stride;
    } // for (j)
    
	//COMPV_DEBUG_INFO("kaka = %llu", kaka);
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
		currentIdx = (begin->y * stride) + begin->x;
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
	COMPV_DEBUG_INFO("FAST: alloc %d ranges", nRanges);

	COMPV_CHECK_EXP_RETURN(nRanges <= 0 || !ppRanges, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_CHECK_CODE_RETURN(FastRangesFree(nRanges, ppRanges));

	*ppRanges = (RangeFAST*)CompVMem::calloc(nRanges, sizeof(RangeFAST));
	COMPV_CHECK_EXP_RETURN(!*ppRanges, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	RangeFAST* pRanges = *ppRanges;

	for (int32_t i = 0; i < nRanges; ++i) {
#if 0
		pRanges[i].me = (compv_scalar_t*)CompVMem::malloc(stride * 1 * sizeof(compv_scalar_t));
		COMPV_CHECK_EXP_RETURN(!pRanges[i].me, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
#endif

	}
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE FastRangesFree(int32_t nRanges, RangeFAST** ppRanges)
{
	if (ppRanges && *ppRanges) {
		RangeFAST* pRanges = *ppRanges;
		for (int32_t i = 0; i < nRanges; ++i) {
#if 0
			CompVMem::free((void**)&pRanges[i].me);
#endif
		}
		CompVMem::free((void**)ppRanges);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
