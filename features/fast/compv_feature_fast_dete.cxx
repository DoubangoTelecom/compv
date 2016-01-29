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

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <iostream>
#include <string>

COMPV_NAMESPACE_BEGIN()

// Default threshold (pixel intensity: [0-255])
#define COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT		10
// Number of positive continuous pixel to have before declaring a candidate as an interest point
#define COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP			true
#define COMPV_FEATURE_DETE_FAST_MAX_FEATURTES			-1 // maximum number of features to retain (<0 means all)
#define COMPV_FEATURE_DETE_FAST_MIN_SAMPLES_PER_THREAD	250*250

static int32_t COMPV_INLINE __continuousCount(int32_t fasType) {
	switch (fasType)
	{
	case COMPV_FAST_TYPE_9: return 9;
	case COMPV_FAST_TYPE_12: return 12;
	default:COMPV_DEBUG_ERROR("Invalid fastType:%d", fasType); return 9;
	}
}

static bool COMPV_INLINE __compareStrengthDec(const CompVInterestPoint& i, const  CompVInterestPoint& j) {
	return (i.strength > j.strength);
}
static bool COMPV_INLINE __isNonMaximal(const CompVInterestPoint & point) {
	return point.x < 0;
}

// Flags generated using FastFalgs() in "tests/fast.cxx"
static const uint16_t Fast9Flags[16] = { 0x1ff, 0x3fe, 0x7fc, 0xff8, 0x1ff0, 0x3fe0, 0x7fc0, 0xff80, 0xff01, 0xfe03, 0xfc07, 0xf80f, 0xf01f, 0xe03f, 0xc07f, 0x80ff };
static const uint16_t Fast12Flags[16] = { 0xfff, 0x1ffe, 0x3ffc, 0x7ff8, 0xfff0, 0xffe1, 0xffc3, 0xff87, 0xff0f, 0xfe1f, 0xfc3f, 0xf87f, 0xf0ff, 0xe1ff, 0xc3ff, 0x87ff };

static COMPV_ERROR_CODE FastProcessRange_AsynExec(const struct compv_asynctoken_param_xs* pc_params);
static void FastProcessRange(const uint8_t* dataPtr, int32_t rowStart, int32_t rowEnd, int32_t rowCount, int32_t width, int32_t stride, int32_t threshold, int32_t N, const int32_t* pixels16, const uint16_t* flags16, std::vector<CompVInterestPoint >* interestPoints);
static void FastData_C(const uint8_t* cercle16, compv_scalar_t darker, compv_scalar_t brighter, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, compv_scalar_t* ddarkers16, compv_scalar_t* dbrighters16);
static void FastStrengths_C(const compv_scalar_t* dbrighters, const compv_scalar_t* ddarkers, compv_scalar_t fbrighters, compv_scalar_t fdarkers, int N, const uint16_t* FastXFlags, int* strength);

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

// override CompVSettable::set
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

// override CompVFeatureDete::process
COMPV_ERROR_CODE CompVFeatureDeteFAST::process(const CompVObjWrapper<CompVImage*>& image, std::vector<CompVInterestPoint >& interestPoints)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	const uint16_t * FastXFlags = m_iType == COMPV_FAST_TYPE_9 ? Fast9Flags : Fast12Flags;
	const uint8_t* dataPtr = (const uint8_t*)image->getDataPtr();
	int32_t width = image->getWidth();
	int32_t height = image->getHeight();
	int32_t stride = image->getStride();
	CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();
	int threadsCount = 1;
	// TODO(dmi): make the allocations once
	float* strengthsMap = NULL;

	if (m_bNonMaximaSupp) {
		strengthsMap = (float*)CompVMem::calloc(stride * height, sizeof(float)); // Must use calloc to fill the strengths with null values
		COMPV_CHECK_EXP_RETURN(!strengthsMap, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	// clear old points
	interestPoints.clear();

	const int32_t pixels16[16] = {
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
			threadHeight = ((height - totalHeight) / (threadsCount - i)) & -2; // the & -2 is to make sure we'll deal with even heights
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
	
	if (strengthsMap) {
		// FIXME: compute strengths map
		// candIdx = (j * stride) + col;
		// strengthsMap[candIdx] = (float)strength;
	}

	// Non Maximal Suppression for removing adjacent corners
	if (m_bNonMaximaSupp) {
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
static void FastStrengths_C(const compv_scalar_t* dbrighters, const compv_scalar_t* ddarkers, compv_scalar_t fbrighters, compv_scalar_t fdarkers, int N, const uint16_t* FastXFlags, int* strength)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

	compv_scalar_t ndarker, nbrighter;
	unsigned i, j, k;
	*strength = 0;

	for (i = 0; i < 16; ++i) {
		ndarker = 255;
		nbrighter = 255;
		if ((fbrighters & FastXFlags[i]) == FastXFlags[i]) {
			// lowest diff
			k = i + N;
			for (j = i; j < k; ++j) {
				if (dbrighters[j & 15] < nbrighter) nbrighter = dbrighters[j & 15];
			}
		}
		if ((fdarkers & FastXFlags[i]) == FastXFlags[i]) {
			// lowest diff
			k = i + N;
			for (j = i; j < k; ++j) {
				if (ddarkers[j & 15] < ndarker) ndarker = ddarkers[j & 15];
			}
		}
		else if (nbrighter == 255) {
			continue;
		}

		*strength = (std::max(*strength, std::min((int)ndarker, (int)nbrighter)));
	}
}

static int FastData_C(const uint8_t* dataPtr, const int32_t* pixels16, compv_scalar_t N, uint8_t* temp16, compv_scalar_t darker, compv_scalar_t brighter, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, compv_scalar_t* ddarkers16, compv_scalar_t* dbrighters16)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	int32_t sum;

	// compare I1 and I7
	temp16[0] = dataPtr[pixels16[0]];
	temp16[8] = dataPtr[pixels16[8]];
	ddarkers16[0] = (darker - temp16[0]);
	ddarkers16[8] = (darker - temp16[8]);
	dbrighters16[0] = (temp16[0] - brighter);
	dbrighters16[8] = (temp16[8] - brighter);

	sum = (dbrighters16[0] > 0 || ddarkers16[0] > 0) + (dbrighters16[8] > 0 || ddarkers16[8] > 0);

	// compare I5 and I13
	if (N != 12 || sum > 0) {
		temp16[4] = dataPtr[pixels16[4]];
		temp16[12] = dataPtr[pixels16[12]];
		ddarkers16[4] = (darker - temp16[4]); // I5-darkness
		ddarkers16[12] = (darker - temp16[12]); // I13-darkness
		dbrighters16[4] = (temp16[4] - brighter); // I5-brightness
		dbrighters16[12] = (temp16[12] - brighter); // I13-brightness

		sum += (dbrighters16[4] > 0 || ddarkers16[4] > 0) + (dbrighters16[12] > 0 || ddarkers16[12] > 0);

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

			ddarkers16[1] = (darker - temp16[1]); *pfdarkers |= (ddarkers16[1] > 0 ? (1 << 1) : 0);
			ddarkers16[2] = (darker - temp16[2]); *pfdarkers |= (ddarkers16[2] > 0 ? (1 << 2) : 0);
			ddarkers16[3] = (darker - temp16[3]); *pfdarkers |= (ddarkers16[3] > 0 ? (1 << 3) : 0);
			ddarkers16[5] = (darker - temp16[5]); *pfdarkers |= (ddarkers16[5] > 0 ? (1 << 5) : 0);
			ddarkers16[6] = (darker - temp16[6]); *pfdarkers |= (ddarkers16[6] > 0 ? (1 << 6) : 0);
			ddarkers16[7] = (darker - temp16[7]); *pfdarkers |= (ddarkers16[7] > 0 ? (1 << 7) : 0);
			ddarkers16[9] = (darker - temp16[9]); *pfdarkers |= (ddarkers16[9] > 0 ? (1 << 9) : 0);
			ddarkers16[10] = (darker - temp16[10]); *pfdarkers |= (ddarkers16[10] > 0 ? (1 << 10) : 0);
			ddarkers16[11] = (darker - temp16[11]); *pfdarkers |= (ddarkers16[11] > 0 ? (1 << 11) : 0);
			ddarkers16[13] = (darker - temp16[13]); *pfdarkers |= (ddarkers16[13] > 0 ? (1 << 13) : 0);
			ddarkers16[14] = (darker - temp16[14]); *pfdarkers |= (ddarkers16[14] > 0 ? (1 << 14) : 0);
			ddarkers16[15] = (darker - temp16[15]); *pfdarkers |= (ddarkers16[15] > 0 ? (1 << 15) : 0);

			// 0, 8, 4 and 12 already filled by the calling function
			*pfbrighters = (dbrighters16[0] > 0 ? (1 << 0) : 0);
			*pfbrighters |= (dbrighters16[8] > 0 ? (1 << 8) : 0);
			*pfbrighters |= (dbrighters16[4] > 0 ? (1 << 4) : 0);
			*pfbrighters |= (dbrighters16[12] > 0 ? (1 << 12) : 0);

			dbrighters16[1] = (temp16[1] - brighter); *pfbrighters |= (dbrighters16[1] > 0 ? (1 << 1) : 0);
			dbrighters16[2] = (temp16[2] - brighter); *pfbrighters |= (dbrighters16[2] > 0 ? (1 << 2) : 0);
			dbrighters16[3] = (temp16[3] - brighter); *pfbrighters |= (dbrighters16[3] > 0 ? (1 << 3) : 0);
			dbrighters16[5] = (temp16[5] - brighter); *pfbrighters |= (dbrighters16[5] > 0 ? (1 << 5) : 0);
			dbrighters16[6] = (temp16[6] - brighter); *pfbrighters |= (dbrighters16[6] > 0 ? (1 << 6) : 0);
			dbrighters16[7] = (temp16[7] - brighter); *pfbrighters |= (dbrighters16[7] > 0 ? (1 << 7) : 0);
			dbrighters16[9] = (temp16[9] - brighter); *pfbrighters |= (dbrighters16[9] > 0 ? (1 << 9) : 0);
			dbrighters16[10] = (temp16[10] - brighter); *pfbrighters |= (dbrighters16[10] > 0 ? (1 << 10) : 0);
			dbrighters16[11] = (temp16[11] - brighter); *pfbrighters |= (dbrighters16[11] > 0 ? (1 << 11) : 0);
			dbrighters16[13] = (temp16[13] - brighter); *pfbrighters |= (dbrighters16[13] > 0 ? (1 << 13) : 0);
			dbrighters16[14] = (temp16[14] - brighter); *pfbrighters |= (dbrighters16[14] > 0 ? (1 << 14) : 0);
			dbrighters16[15] = (temp16[15] - brighter); *pfbrighters |= (dbrighters16[15] > 0 ? (1 << 15) : 0);

			// FIXME(dmi): __popcnt16 not portable and check cpuid
			return (__popcnt16((unsigned short)*pfdarkers) >= N || __popcnt16((unsigned short)*pfbrighters) >= N) ? 1 : 0;
		}
	}
	return 0;
}

static void FastProcessRange(const uint8_t* dataPtr, int32_t rowStart, int32_t rowEnd, int32_t rowCount, int32_t width, int32_t stride, int32_t threshold, int32_t N, const int32_t* pixels16, const uint16_t* flags16, std::vector<CompVInterestPoint >* interestPoints)
{
	const uint8_t* IP;
	compv_scalar_t brighter, darker;
	int32_t j, i, minj, maxj;
	int32_t pad = (stride - width), padPlus6 = 3 + pad + 3;
	COMV_ALIGN_DEFAULT() compv_scalar_t ddarkers[16];
	COMV_ALIGN_DEFAULT() compv_scalar_t dbrighters[16];
	COMV_ALIGN_DEFAULT() uint8_t cercle[16]; // tempdata
	compv_scalar_t fdarkers, fbrighters;
	int strength;
	int (*FastData)(const uint8_t* dataPtr, const int32_t* pixels16, compv_scalar_t N, uint8_t* temp16, compv_scalar_t darker, compv_scalar_t brighter, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, compv_scalar_t* ddarkers16, compv_scalar_t* dbrighters16) = FastData_C;
	void(*FastStrengths)(const compv_scalar_t* dbrighters, const compv_scalar_t* ddarkers, compv_scalar_t fbrighters, compv_scalar_t fdarkers, int N, const uint16_t* FastXFlags, int* strength)
		= N == 9 ? Fast9Strengths_C : Fast12Strengths_C;
#if 1 // Do not use build-in fast functions
	FastStrengths = FastStrengths_C;
#endif
	

	minj = (rowStart == 0 ? 3 : 0);
	maxj = (rowEnd - rowStart) - ((rowCount - rowEnd) <= 3 ? 3 - (rowCount - rowEnd) : 0);
	IP = dataPtr + ((rowStart + minj) * stride) + 3;

	for (j = minj; j < maxj; ++j) {
		for (i = 3; i < width - 3; ++i) {
			/*  Speed-Test */
			// compute brighter and darker
			// FIXME: why clip2() is slow
			brighter = CompVMathUtils::clampPixel8(IP[0] + threshold); // CompVMathUtils::clip2(255, (IP[0] + m_iThreshold));
			darker = CompVMathUtils::clampPixel8(IP[0] - threshold); // CompVMathUtils::clip2(255, (IP[0] - m_iThreshold));
						
			if (FastData(IP, pixels16, N, cercle, darker, brighter, &fdarkers, &fbrighters, ddarkers, dbrighters)) {
				FastStrengths(dbrighters, ddarkers, fbrighters, fdarkers, N, flags16, &strength);
				if (strength > 0) {
					// strength is defined as the maximum value of t that makes p a corner
					interestPoints->push_back(CompVInterestPoint(i, j, (float)(strength + threshold - 1)));
				}
			}
			IP += 1;
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
	const int32_t* pixels16 = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[8].pcParamPtr, const int32_t*);
	const uint16_t* flags16 = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[9].pcParamPtr, const uint16_t*);
	std::vector<CompVInterestPoint >* interestPoints = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[10].pcParamPtr, std::vector<CompVInterestPoint >*);

	FastProcessRange(dataPtr, rowStart, rowEnd, rowCount, width, stride, threshold, N, pixels16, flags16, interestPoints);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
