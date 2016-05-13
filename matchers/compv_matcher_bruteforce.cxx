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
#include "compv/matchers/compv_matcher_bruteforce.h"
#include "compv/compv_hamming.h"
#include "compv/compv_math.h"

#include <vector> // FIXME
#include <algorithm> // FIXME

COMPV_NAMESPACE_BEGIN()

CompVMatcherBruteForce::CompVMatcherBruteForce()
: m_bCrossCheck(false)
, m_nNormType(COMPV_BRUTEFORCE_NORM_HAMMING)
, m_nKNN(1)
{

}

CompVMatcherBruteForce::~CompVMatcherBruteForce()
{

}
// override CompVSettable::set
COMPV_ERROR_CODE CompVMatcherBruteForce::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_BRUTEFORCE_SET_INT32_KNN: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int32_t knn = *((int32_t*)valuePtr);
		COMPV_CHECK_EXP_RETURN(knn < 1 || knn > 255, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nKNN = knn;
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_BRUTEFORCE_SET_INT32_NORM: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int32_t normType = *((int32_t*)valuePtr);
		COMPV_CHECK_EXP_RETURN(normType != COMPV_BRUTEFORCE_NORM_HAMMING, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // For now only HAMMING is supported
		m_nNormType = normType;
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_BRUTEFORCE_SET_BOOL_CROSS_CHECK: {
		// For now crosscheck is ignored, we prefer ratio test method which produce better results
		// As a workaround, a user can emulate crosschecking like this:
		// process(query, train, match1);
		// process (train, query, match2);
		// foreach m1 check that distance(m1.trainIdx) == distance(m2.queryIdx)
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		bool crossCheck = *((bool*)valuePtr);
		COMPV_CHECK_EXP_RETURN(crossCheck && m_nKNN != 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // cross check requires KNN = 1
		m_bCrossCheck = crossCheck;
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

// override CompVSettable::get
COMPV_ERROR_CODE CompVMatcherBruteForce::get(int id, const void*& valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case -1:
	default:
		return CompVSettable::get(id, valuePtr, valueSize);
	}
}

// FIXME:
struct DistanceLessThan
{
	inline bool operator() (const CompVDMatch& m1, const CompVDMatch& m2)
	{
		return (m1.distance < m2.distance);
	}
};
	
// override CompVMatcher::process
COMPV_ERROR_CODE CompVMatcherBruteForce::process(const CompVPtr<CompVArray<uint8_t>* >&queryDescriptions, const CompVPtr<CompVArray<uint8_t>* >&trainDescriptions, CompVPtr<CompVArray<CompVDMatch>* >* matches)
{
	COMPV_CHECK_EXP_RETURN(
		!matches
		|| !queryDescriptions 
		|| queryDescriptions->isEmpty() 
		|| !trainDescriptions 
		|| trainDescriptions->isEmpty()
		|| queryDescriptions->cols() != trainDescriptions->cols()
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	// CompVDMatch* match;

	COMPV_DEBUG_INFO_CODE_FOR_TESTING();

	int knn = COMPV_MATH_CLIP3(1, (int)trainDescriptions->rows(), m_nKNN);

	// realloc() matchers
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<CompVDMatch>::newObj(matches, queryDescriptions->rows(), knn));

	// realloc() hamming distances
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<int32_t>::newObj(&m_hammingDistances, queryDescriptions->rows(), 1));
#if 0
	// Process hamming distances
	// Each row in the train is used as patch over the entire query descriptions
	size_t trainRows = trainDescriptions->rows();
	for (size_t trainIdx = 0; trainIdx < trainRows; ++trainIdx) {
		COMPV_CHECK_CODE_RETURN(err_ = CompVHamming::distance(queryDescriptions->ptr(), (int)queryDescriptions->cols(), (int)queryDescriptions->strideInBytes(), (int)queryDescriptions->rows(),
			trainDescriptions->ptr(trainIdx), (int32_t*)m_hammingDistances->ptr(0)));
		if (trainIdx == 0) { // FIXME: move outside
			for (size_t queryIdx = 0; queryIdx < (*matches)->cols(); ++queryIdx) {
				match = (CompVDMatch*)(*matches)->ptr(0, queryIdx);
				match->distance = *m_hammingDistances->ptr(0, queryIdx);
				match->queryIdx = queryIdx;
				match->trainIdx = trainIdx;
			}
		}
		else {
			for (size_t queryIdx = 0; queryIdx < (*matches)->cols(); ++queryIdx) {
				match = (CompVDMatch*)(*matches)->ptr(0, queryIdx);
				if (*m_hammingDistances->ptr(0, queryIdx) < match->distance) {
					match->distance = *m_hammingDistances->ptr(0, queryIdx);
					match->queryIdx = queryIdx;
					match->trainIdx = trainIdx;
				}
			}
		}
	}
#else
	std::vector<std::vector<CompVDMatch> > vmatches;
	vmatches.resize(queryDescriptions->rows());
	size_t trainRows = trainDescriptions->rows();
	for (size_t trainIdx = 0; trainIdx < trainRows; ++trainIdx) {
		COMPV_CHECK_CODE_RETURN(err_ = CompVHamming::distance(queryDescriptions->ptr(), (int)queryDescriptions->cols(), (int)queryDescriptions->strideInBytes(), (int)queryDescriptions->rows(),
			trainDescriptions->ptr(trainIdx), (int32_t*)m_hammingDistances->ptr(0)));
		for (size_t queryIdx = 0; queryIdx < queryDescriptions->rows(); ++queryIdx) {
			vmatches[queryIdx].push_back(CompVDMatch((int32_t)queryIdx, (int32_t)trainIdx, *m_hammingDistances->ptr(0, queryIdx)));
		}
	}
	for (size_t i = 0; i < vmatches.size(); ++i) {
		std::sort(vmatches[i].begin(), vmatches[i].end(), DistanceLessThan());
		vmatches[i].resize(knn);
	}
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<CompVDMatch>::newObj(matches, vmatches.size(), knn));
	for (int k = 0; k < knn; ++k) {
		for (size_t i = 0; i < vmatches.size(); ++i) {
			*const_cast<CompVDMatch*>((*matches)->ptr(k, i)) = vmatches[i][k];
		}
	}
#endif

	return err_;
}

COMPV_ERROR_CODE CompVMatcherBruteForce::newObj(CompVPtr<CompVMatcher* >* matcher)
{
	COMPV_CHECK_EXP_RETURN(!matcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVPtr<CompVMatcherBruteForce* > matcher_;

	matcher_ = new CompVMatcherBruteForce();
	COMPV_CHECK_EXP_RETURN(!matcher_, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	*matcher = *matcher_;

	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()