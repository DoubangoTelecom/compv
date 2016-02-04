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

#include "compv/features/fast/compv_feature_fast_dete.h"
#include "compv/features/fast/compv_feature_fast9_dete.h"
#include "compv/features/fast/compv_feature_fast12_dete.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_mathutils.h"
#include "compv/compv_cpu.h"
#include "compv/compv_bits.h"

#include "compv/intrinsics/x86/features/fast/compv_feature_fast_dete_intrin_sse.h"

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

COMPV_NAMESPACE_BEGIN()

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT		10
// Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP			true
#define COMPV_FEATURE_DETE_FAST_MAX_FEATURTES			-1 // maximum number of features to retain (<0 means all)
#define COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD	250*250

#if defined(COMPV_ARCH_X86) && defined(COMPV_ASM)
extern "C" compv_scalar_t Fast9Strengths_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16]);
extern "C" compv_scalar_t Fast9Strengths_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16]);
extern "C" compv_scalar_t Fast12Strengths_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16]);
extern "C" compv_scalar_t Fast12Strengths_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16]);

extern "C" compv_scalar_t Fast9Data16_Asm_POPCNT_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16]);
extern "C" compv_scalar_t Fast9Data16_Asm_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16]);
extern "C" compv_scalar_t Fast12Data16_Asm_POPCNT_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16]);
extern "C" compv_scalar_t Fast12Data16_Asm_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16]);

#endif

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

static bool COMPV_INLINE __compareStrengthDec(const CompVInterestPoint& i, const  CompVInterestPoint& j)
{
	return (i.strength > j.strength);
}
static bool COMPV_INLINE __isNonMaximal(const CompVInterestPoint & point)
{
	return point.x < 0;
}

// Flags generated using FastFlags() in "tests/fast.cxx"
static const COMPV_ALIGN_DEFAULT() uint16_t Fast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
static const COMPV_ALIGN_DEFAULT() uint16_t Fast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };

static COMPV_ERROR_CODE FastProcessRange_AsynExec(const struct compv_asynctoken_param_xs* pc_params);
static void FastProcessRange(const uint8_t* dataPtr, int32_t rowStart, int32_t rowEnd, int32_t rowCount, int32_t width, int32_t stride, int32_t threshold, int32_t N, const compv_scalar_t(&pixels16)[16], const uint16_t(&flags16)[16], std::vector<CompVInterestPoint >* interestPoints);
static compv_scalar_t FastData_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, int16_t(&ddarkers16)[16], int16_t(&dbrighters16)[16]);
static compv_scalar_t FastData16_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t(&pfdarkers16)[16], compv_scalar_t(&pfbrighters16)[16], int16_t(&ddarkers16x16)[16][16], int16_t(&dbrighters16x16)[16][16]);
static compv_scalar_t FastStrengths_C(const int16_t(&dbrighters)[16], const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, const uint16_t(&FastXFlags)[16]);

CompVFeatureDeteFAST::CompVFeatureDeteFAST()
	: CompVFeatureDete(COMPV_FAST_ID)
	, m_iThreshold(COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT)
	, m_iType(COMPV_FAST_TYPE_9)
	, m_iNumContinuous(__continuousCount(COMPV_FAST_TYPE_9))
	, m_bNonMaximaSupp(COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP)
	, m_iMaxFeatures(COMPV_FEATURE_DETE_FAST_MAX_FEATURTES)
{
}

CompVFeatureDeteFAST::~CompVFeatureDeteFAST()
{

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
COMPV_ERROR_CODE CompVFeatureDeteFAST::process(const CompVObjWrapper<CompVImage*>& image, std::vector<CompVInterestPoint >& interestPoints)
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
	int threadsCount = 1;
	// TODO(dmi): make the allocations once
	float* strengthsMap = NULL;

	if (m_bNonMaximaSupp) {
		//strengthsMap = (float*)CompVMem::calloc(stride * height, sizeof(float)); // Must use calloc to fill the strengths with null values
		//COMPV_CHECK_EXP_RETURN(!strengthsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	// clear old points
	interestPoints.clear();

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
		threadsCount = threadDip->guessNumThreadsDividingAcrossY(stride, height, COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD);
	}

	if (threadsCount > 1) {
		std::vector<std::vector<CompVInterestPoint>> points(threadsCount);
		int32_t rowStart = 0, threadHeight, totalHeight = 0;
		uint32_t threadIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
		for (int i = 0; i < threadsCount; ++i) {
			threadHeight = ((height - totalHeight) / (threadsCount - i)) & -2; // the & -2 is to make sure we'll deal with odd heights
			COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadIdx + i), COMPV_TOKENIDX_FEATURE_FAST_DETE, FastProcessRange_AsynExec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(
				dataPtr,
				rowStart,
				(rowStart + threadHeight),
				height,
				width,
				stride,
				m_iThreshold,
				m_iNumContinuous,
				pixels16,
				FastXFlags,
				&points[i]),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
			rowStart += threadHeight;
			totalHeight += threadHeight;
		}
		for (int32_t i = 0; i < threadsCount; ++i) {
			COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadIdx + i), COMPV_TOKENIDX_FEATURE_FAST_DETE));
		}
		// append the vectors
		for (int i = 0; i < threadsCount; ++i) {
			interestPoints.insert(interestPoints.end(), points[i].begin(), points[i].end());
		}
	}
	else {
		FastProcessRange(dataPtr, 0, height, height, width, stride, m_iThreshold, m_iNumContinuous, pixels16, FastXFlags, &interestPoints);
	}

	// FIXME: (x,y) not correct when multi-threding is enable: rowStart/rowEnd
	if (strengthsMap) {
		int32_t candIdx;
		const CompVInterestPoint* point;
		for (size_t i = 0; i < interestPoints.size(); ++i) {
			point = &interestPoints[i];
			candIdx = point->x + (stride * point->y);
			strengthsMap[candIdx] = (float)point->strength;
		}
	}

	// Non Maximal Suppression for removing adjacent corners
	if (strengthsMap) {
		int32_t currentIdx;
		for (size_t i = 0; i < interestPoints.size(); ++i) {
			CompVInterestPoint* point = &interestPoints[i];
			currentIdx = (point->y * stride) + point->x;
			// No need to chech index as the point always has coords in (+3, +3)
			if (strengthsMap[currentIdx - 1] >= point->strength) { // left
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + 1] >= point->strength) { // right
				point->x = -1;
			}
			else if (strengthsMap[currentIdx - stride - 1] >= point->strength) { // left-top
				point->x = -1;
			}
			else if (strengthsMap[currentIdx - stride] >= point->strength) { // top
				point->x = -1;
			}
			else if (strengthsMap[currentIdx - stride + 1] >= point->strength) { // right-top
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + stride - 1] >= point->strength) { // left-bottom
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + stride] >= point->strength) { // bottom
				point->x = -1;
			}
			else if (strengthsMap[currentIdx + stride + 1] >= point->strength) { // right-bottom
				point->x = -1;
			}
		}

		// Remove non maximal points
		interestPoints.erase(std::remove_if(interestPoints.begin(), interestPoints.end(), __isNonMaximal), interestPoints.end());
	}

	// Retain best "m_iMaxFeatures" features
	// TODO(dmi): use retainBest
	if (m_iMaxFeatures > 0 && (int32_t)interestPoints.size() > m_iMaxFeatures) {
		std::sort(interestPoints.begin(), interestPoints.end(), __compareStrengthDec);
		interestPoints.resize(m_iMaxFeatures);
	}


	CompVMem::free((void**)&strengthsMap);

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
static compv_scalar_t FastStrengths_C(const int16_t(&dbrighters)[16], const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, const uint16_t(&FastXFlags)[16])
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
static compv_scalar_t FastData_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, int16_t(&ddarkers16)[16], int16_t(&dbrighters16)[16])
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	int32_t sum;
	uint8_t temp16[16];

	int16_t brighter = (int16_t)(dataPtr[0] + threshold);
	int16_t darker = (int16_t)(dataPtr[0] - threshold);

	bool popcntHard = CompVCpu::isSupported(kCpuFlagPOPCNT);

	// compare I1 and I7
	temp16[0] = dataPtr[pixels16[0]];
	temp16[8] = dataPtr[pixels16[8]];
	ddarkers16[0] = (darker - temp16[0]);
	ddarkers16[8] = (darker - temp16[8]);
	dbrighters16[0] = (temp16[0] - brighter);
	dbrighters16[8] = (temp16[8] - brighter);

	sum = (dbrighters16[0] > 0 || ddarkers16[0] > 0) + (dbrighters16[8] > 0 || ddarkers16[8] > 0);

	// compare I5 and I13
	/*  Speed-Test-1 */
	if (N != 12 || sum > 0) {
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

			// 0, 8, 4 and 12 already filled by the calling function
			*pfdarkers = (ddarkers16[0] > 0 ? (1 << 0) : 0);
			*pfdarkers |= (ddarkers16[8] > 0 ? (1 << 8) : 0);
			*pfdarkers |= (ddarkers16[4] > 0 ? (1 << 4) : 0);
			*pfdarkers |= (ddarkers16[12] > 0 ? (1 << 12) : 0);

			ddarkers16[1] = (darker - temp16[1]);
			*pfdarkers |= (ddarkers16[1] > 0 ? (1 << 1) : 0);
			ddarkers16[2] = (darker - temp16[2]);
			*pfdarkers |= (ddarkers16[2] > 0 ? (1 << 2) : 0);
			ddarkers16[3] = (darker - temp16[3]);
			*pfdarkers |= (ddarkers16[3] > 0 ? (1 << 3) : 0);
			ddarkers16[5] = (darker - temp16[5]);
			*pfdarkers |= (ddarkers16[5] > 0 ? (1 << 5) : 0);
			ddarkers16[6] = (darker - temp16[6]);
			*pfdarkers |= (ddarkers16[6] > 0 ? (1 << 6) : 0);
			ddarkers16[7] = (darker - temp16[7]);
			*pfdarkers |= (ddarkers16[7] > 0 ? (1 << 7) : 0);
			ddarkers16[9] = (darker - temp16[9]);
			*pfdarkers |= (ddarkers16[9] > 0 ? (1 << 9) : 0);
			ddarkers16[10] = (darker - temp16[10]);
			*pfdarkers |= (ddarkers16[10] > 0 ? (1 << 10) : 0);
			ddarkers16[11] = (darker - temp16[11]);
			*pfdarkers |= (ddarkers16[11] > 0 ? (1 << 11) : 0);
			ddarkers16[13] = (darker - temp16[13]);
			*pfdarkers |= (ddarkers16[13] > 0 ? (1 << 13) : 0);
			ddarkers16[14] = (darker - temp16[14]);
			*pfdarkers |= (ddarkers16[14] > 0 ? (1 << 14) : 0);
			ddarkers16[15] = (darker - temp16[15]);
			*pfdarkers |= (ddarkers16[15] > 0 ? (1 << 15) : 0);

			// 0, 8, 4 and 12 already filled by the calling function
			*pfbrighters = (dbrighters16[0] > 0 ? (1 << 0) : 0);
			*pfbrighters |= (dbrighters16[8] > 0 ? (1 << 8) : 0);
			*pfbrighters |= (dbrighters16[4] > 0 ? (1 << 4) : 0);
			*pfbrighters |= (dbrighters16[12] > 0 ? (1 << 12) : 0);

			dbrighters16[1] = (temp16[1] - brighter);
			*pfbrighters |= (dbrighters16[1] > 0 ? (1 << 1) : 0);
			dbrighters16[2] = (temp16[2] - brighter);
			*pfbrighters |= (dbrighters16[2] > 0 ? (1 << 2) : 0);
			dbrighters16[3] = (temp16[3] - brighter);
			*pfbrighters |= (dbrighters16[3] > 0 ? (1 << 3) : 0);
			dbrighters16[5] = (temp16[5] - brighter);
			*pfbrighters |= (dbrighters16[5] > 0 ? (1 << 5) : 0);
			dbrighters16[6] = (temp16[6] - brighter);
			*pfbrighters |= (dbrighters16[6] > 0 ? (1 << 6) : 0);
			dbrighters16[7] = (temp16[7] - brighter);
			*pfbrighters |= (dbrighters16[7] > 0 ? (1 << 7) : 0);
			dbrighters16[9] = (temp16[9] - brighter);
			*pfbrighters |= (dbrighters16[9] > 0 ? (1 << 9) : 0);
			dbrighters16[10] = (temp16[10] - brighter);
			*pfbrighters |= (dbrighters16[10] > 0 ? (1 << 10) : 0);
			dbrighters16[11] = (temp16[11] - brighter);
			*pfbrighters |= (dbrighters16[11] > 0 ? (1 << 11) : 0);
			dbrighters16[13] = (temp16[13] - brighter);
			*pfbrighters |= (dbrighters16[13] > 0 ? (1 << 13) : 0);
			dbrighters16[14] = (temp16[14] - brighter);
			*pfbrighters |= (dbrighters16[14] > 0 ? (1 << 14) : 0);
			dbrighters16[15] = (temp16[15] - brighter);
			*pfbrighters |= (dbrighters16[15] > 0 ? (1 << 15) : 0);

			return (compv_popcnt16(popcntHard, (unsigned short)*pfdarkers) >= N || compv_popcnt16(popcntHard, (unsigned short)*pfbrighters) >= N) ? 1 : 0;
		}
	}
	return compv_scalar_t(0);
}

static compv_scalar_t FastData16_C(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t(&pfdarkers16)[16], compv_scalar_t(&pfbrighters16)[16], int16_t(&ddarkers16x16)[16][16], int16_t(&dbrighters16x16)[16][16])
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

static void FastProcessRange(const uint8_t* dataPtr, int32_t rowStart, int32_t rowEnd, int32_t rowCount, int32_t width, int32_t stride, int32_t threshold, int32_t N, const compv_scalar_t(&pixels16)[16], const uint16_t(&flags16)[16], std::vector<CompVInterestPoint >* interestPoints)
{
	const uint8_t* IP;
	int32_t j, i, minj, maxj;
	int32_t pad = (stride - width), padPlus6 = 3 + pad + 3;
	int r, r0, r1;
	//TODO(dmi): These arrays must be allocated on the thread pool
	COMPV_ALIGN_DEFAULT() int16_t ddarkers[16][16];
	COMPV_ALIGN_DEFAULT() int16_t dbrighters[16][16];
	COMPV_ALIGN_DEFAULT() compv_scalar_t fdarkers[16], fbrighters[16];
	compv_scalar_t strength;
	compv_scalar_t(*FastData)(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, int16_t(&ddarkers16)[16], int16_t(&dbrighters16)[16]) = FastData_C;
	compv_scalar_t(*FastData16)(const uint8_t* dataPtr, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t(&pfdarkers16)[16], compv_scalar_t(&pfbrighters16)[16], int16_t(&ddarkers16x16)[16][16], int16_t(&dbrighters16x16)[16][16]) = FastData16_C;
	compv_scalar_t(*FastStrengths)(const int16_t(&dbrighters)[16], const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, const uint16_t(&FastXFlags)[16])
		= N == 9 ? Fast9Strengths_C : Fast12Strengths_C;
#if 1 // Do not use build-in fast functions
	FastStrengths = FastStrengths_C;
#endif

	if (CompVCpu::isSupported(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(FastData = FastData_Intrin_SSE2);
		COMPV_EXEC_IFDEF_INTRIN_X86(FastData16 = FastData16_Intrin_SSE2);
		COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = FastStrengths_SSE2);
		COMPV_EXEC_IFDEF_ASM_X86(FastData16 = (N == 9)
			? (CompVCpu::isSupported(kCpuFlagPOPCNT) ? Fast9Data16_Asm_POPCNT_X86_SSE2 : Fast9Data16_Asm_X86_SSE2)
			: (CompVCpu::isSupported(kCpuFlagPOPCNT) ? Fast12Data16_Asm_POPCNT_X86_SSE2 : Fast12Data16_Asm_X86_SSE2));
	}
	if (CompVCpu::isSupported(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(FastStrengths = FastStrengths_SSE41);
		COMPV_EXEC_IFDEF_ASM_X86(FastStrengths = (N == 9)
			? (CompVCpu::isSupported(kCpuFlagCMOV) ? Fast9Strengths_Asm_CMOV_X86_SSE41 : Fast9Strengths_Asm_X86_SSE41)
			: (CompVCpu::isSupported(kCpuFlagCMOV) ? Fast12Strengths_Asm_CMOV_X86_SSE41 : Fast12Strengths_Asm_X86_SSE41));
	}

	minj = (rowStart == 0 ? 3 : 0);
	maxj = (rowEnd - rowStart) - ((rowCount - rowEnd) <= 3 ? 3 - (rowCount - rowEnd) : 0);
	IP = dataPtr + ((rowStart + minj) * stride) + 3;

	for (j = minj; j < maxj; ++j) {
		for (i = 3; i < width - 3; ++i) {
			// No need to clamp "brighter" and "darker".
			// "brighter" is always > 0 and brightness equation is ((u8 - brighter) > 0) => (u8 > brighter) => no need to saturate brighter to 255 as u8 already saturated to 255
			// "darker" is always <255 and darkness equatio is ((darker - u8) > 0) => (u8 < darker) => no need to saturate darker to 0 as u8 already satured to 0

			// FIXME
			if (i == 1619 && j == 279) {
				int kaka = 0;
			}
			if (i == 1587 && j == 195) {
				int kaka = 0;
			}

			if ((i < width - 3 - 16)) {
				// For testing with image "voiture", the first (i,j) to produce an interesting point is (1619, 279)
				// r bits: [0-15] contains the non-zero values to check, [18-19]: the number of bits to skip, [16]: whether to check darkers, [17]: whether to check brighters
				r = (int)FastData16(IP, (compv_scalar_t(&)[16])pixels16, N, threshold, fdarkers, fbrighters, ddarkers, dbrighters);
				
				if (r >= (1 << 18)) { // This is a skip process
					r0 = (r >> 18) & 0xFF; // bits [18-19]
					IP += r0;
					i += r0 - 1; // -1 because the loop will also increment i
				}
				else {
					if (r > 0) {
						r0 = (r & (1 << 16)) ? 0xFFFF : 0x0000; // whether to check darkers
						r1 = (r & (1 << 17)) ? 0xFFFF : 0x0000; // whether to check brighters
						for (unsigned z = 0; z < 16; ++z) {
							if (r & (1 << z)) {
								// The flags could contains garbage on the highest bits -> always use &0xFFFF
								strength = FastStrengths(dbrighters[z], ddarkers[z], fbrighters[z] & r1, fdarkers[z] & r0, N, flags16);
								if (strength > 0) {
									// strength is defined as the maximum value of t that makes p a corner
									interestPoints->push_back(CompVInterestPoint(i + z, j, (float)(strength + threshold - 1)));
								}
							}
						}
					}
					IP += 16;
					i += 15; // 15 because the loop will also increment i
				}
			}
			else {
				if (FastData(IP, pixels16, N, threshold, &fdarkers[0], &fbrighters[0], ddarkers[0], dbrighters[0])) {
					// The flags could contains garbage on the highest bits -> always use &0xFFFF
					strength = FastStrengths(dbrighters[0], ddarkers[0], fbrighters[0] & 0xFFFF, fdarkers[0] & 0xFFFF, N, flags16);
					if (strength > 0) {
						// strength is defined as the maximum value of t that makes p a corner
						interestPoints->push_back(CompVInterestPoint(i, j, (float)(strength + threshold - 1)));
					}
				}
				IP += 1;
			}
		}
		IP += padPlus6;
	}
}

static COMPV_ERROR_CODE FastProcessRange_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
	const uint8_t* dataPtr = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, const uint8_t*);
	int32_t rowStart = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, int32_t);
	int32_t rowEnd = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, int32_t);
	int32_t rowCount = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, int32_t);
	int32_t width = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, int32_t);
	int32_t stride = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[5].pcParamPtr, int32_t);
	int32_t threshold = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[6].pcParamPtr, int32_t);
	int32_t N = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[7].pcParamPtr, int32_t);
	const compv_scalar_t(&pixels16)[16] = COMPV_ASYNCTASK_GET_PARAM_REFARRAY1(pc_params[8].pcParamPtr, const compv_scalar_t, 16);
	const uint16_t(&flags16)[16] = COMPV_ASYNCTASK_GET_PARAM_REFARRAY1(pc_params[9].pcParamPtr, const uint16_t, 16);
	std::vector<CompVInterestPoint >* interestPoints = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[10].pcParamPtr, std::vector<CompVInterestPoint >*);

	FastProcessRange(dataPtr, rowStart, rowEnd, rowCount, width, stride, threshold, N, pixels16, flags16, interestPoints);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
