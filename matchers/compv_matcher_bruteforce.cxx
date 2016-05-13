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

COMPV_NAMESPACE_BEGIN()

CompVMatcherBruteForce::CompVMatcherBruteForce()
: m_bCrossCheck(false)
, m_nNormType(COMPV_BRUTEFORCE_NORM_HAMMING)
, m_nKNN(2) // Use 2 to allow Lowe's ratio test
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

	int knn_ = COMPV_MATH_CLIP3(1, (int)trainDescriptions->rows(), m_nKNN);

	// realloc() matchers
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<CompVDMatch>::newObj(matches, queryDescriptions->rows(), knn_));

	// realloc() hamming distances
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<int32_t>::newObj(&m_hammingDistances, queryDescriptions->rows(), 1));

	CompVDMatch *match0_, *match1_;
	size_t trainRows_ = trainDescriptions->rows();
	size_t trainStrideBytes_ = trainDescriptions->strideInBytes();
	size_t queryRows_ = queryDescriptions->rows();
	size_t queryCols_ = queryDescriptions->cols();
	size_t queryIdx_, trainIdx_, k_;
	COMPV_CHECK_CODE_RETURN(err_ = CompVArray<CompVDMatch>::newObj(matches, queryRows_, knn_));
	int32_t oldDistance_, newDistance_;
	int32_t *hammingDistances_ = const_cast<int32_t*>(m_hammingDistances->ptr(0));
	const uint8_t *queryDescriptions_ = queryDescriptions->ptr(), *trainDescriptions_ = trainDescriptions->ptr();
	
	// Fill the first knn rows with default values
	for (trainIdx_ = 0; trainIdx_ < trainRows_ && trainIdx_ < knn_; ++trainIdx_, trainDescriptions_ += trainStrideBytes_) {
		COMPV_CHECK_CODE_RETURN(err_ = CompVHamming::distance(queryDescriptions_, (int)queryCols_, (int)queryDescriptions->strideInBytes(), (int)queryRows_,
			trainDescriptions_, hammingDistances_));
		for (queryIdx_ = 0, match0_ = const_cast<CompVDMatch*>((*matches)->ptr(trainIdx_)); queryIdx_ < queryRows_; ++queryIdx_, ++match0_) {
			match0_->distance = hammingDistances_[queryIdx_];
			match0_->queryIdx = queryIdx_;
			match0_->trainIdx = trainIdx_;
			match0_->imageIdx = 0; // only for the first time
		}
	}
	// Fill again the knn rows with new min values
	for (; trainIdx_ < trainRows_; ++trainIdx_, trainDescriptions_ += trainStrideBytes_) {
		COMPV_CHECK_CODE_RETURN(err_ = CompVHamming::distance(queryDescriptions_, (int)queryCols_, (int)queryDescriptions->strideInBytes(), (int)queryRows_,
			trainDescriptions_, hammingDistances_));
		for (queryIdx_ = 0, k_ = 0, match0_ = const_cast<CompVDMatch*>((*matches)->ptr()); queryIdx_ < queryRows_; ++queryIdx_, k_ = 0, ++match0_) {
			newDistance_ = hammingDistances_[queryIdx_];
			match1_ = match0_;
			do {
				if (newDistance_ < match1_->distance) {
					oldDistance_ = match1_->distance;
					match1_->distance = newDistance_;
					match1_->trainIdx = trainIdx_;
					newDistance_ = oldDistance_;
					k_ = 0;
					match1_ = match0_;
				}
				else {
					++k_;
					match1_ += queryRows_;
				}
			} 
			while (k_ < knn_);
		}
	}

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