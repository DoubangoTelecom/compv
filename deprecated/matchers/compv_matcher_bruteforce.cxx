/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/matchers/compv_matcher_bruteforce.h"
#include "compv/compv_hamming.h"
#include "compv/compv_box.h"
#include "compv/math/compv_math.h"
#include "compv/compv_engine.h"

#define COMPV_MATCHER_BRUTEFORCE_MIN_SAMPLES_PER_THREAD	1 // very intensive op -> use max threads

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
// queryDescriptions and trainDescriptions must be strideless
COMPV_ERROR_CODE CompVMatcherBruteForce::process(const CompVPtr<CompVArray<uint8_t>* >&queryDescriptions, const CompVPtr<CompVArray<uint8_t>* >&trainDescriptions, CompVPtr<CompVArray<CompVDMatch>* >* matches)
{
    COMPV_CHECK_EXP_RETURN(
        !matches
        || !queryDescriptions
        || queryDescriptions->isEmpty()
        || !trainDescriptions
        || trainDescriptions->isEmpty()
        || queryDescriptions->cols() != trainDescriptions->cols()
		|| queryDescriptions->strideInBytes() != queryDescriptions->rowInBytes()
		|| trainDescriptions->strideInBytes() != trainDescriptions->rowInBytes()
        , COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    size_t trainRows_ = trainDescriptions->rows();
    size_t queryRows_ = queryDescriptions->rows();
    size_t matchesRows = COMPV_MATH_CLIP3(1, (int)trainRows_, m_nKNN);
    size_t matchesCols = queryRows_;

    // realloc() matchers
    COMPV_CHECK_CODE_RETURN(err_ = CompVArray<CompVDMatch>::newObj(matches, matchesRows, matchesCols, 1));

    int32_t threadsCount = 1;
	CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
    // Compute number of threads
	if (threadDisp && threadDisp->getThreadsCount() > 1 && !threadDisp->isMotherOfTheCurrentThread()) {
		threadsCount = COMPV_MATH_CLIP3(1, threadDisp->getThreadsCount(), (int32_t)matchesCols / COMPV_MATCHER_BRUTEFORCE_MIN_SAMPLES_PER_THREAD);
    }

    // process starting at queryIdxStart
    if (threadsCount > 1) {
        size_t total = matchesCols;
        size_t count = (size_t)(total / threadsCount); // must be "size_t"
        size_t queryIdxStart = 0; // must be "size_t"
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		auto funcPtr = [&](size_t queryIdxStart, size_t count, const CompVArray<uint8_t>* queryDescriptions, const CompVArray<uint8_t>* trainDescriptions, CompVArray<CompVDMatch>* matches) -> COMPV_ERROR_CODE {
			return CompVMatcherBruteForce::processAt(queryIdxStart, count, queryDescriptions, trainDescriptions, matches);
		};
        for (int32_t i = 0; i < threadsCount; ++i) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, queryIdxStart, count, *queryDescriptions, *trainDescriptions, **matches), taskIds));
            queryIdxStart += count;
            total -= count;
            if (i == (threadsCount - 2)) {
                count = (total); // the remaining
            }
        }
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
    }
    else {
        COMPV_CHECK_CODE_RETURN(err_ = CompVMatcherBruteForce::processAt(0, matchesCols, *queryDescriptions, *trainDescriptions, **matches));
    }

    return err_;
}

// Private function, do not check imput parameters
COMPV_ERROR_CODE CompVMatcherBruteForce::processAt(size_t queryIdxStart, size_t count, const CompVArray<uint8_t>* queryDescriptions, const CompVArray<uint8_t>* trainDescriptions, CompVArray<CompVDMatch>* matches)
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVDMatch *match0_, *match1_;
    size_t trainRows_ = trainDescriptions->rows();
    size_t trainStrideBytes_ = trainDescriptions->strideInBytes();
    size_t queryCols_ = queryDescriptions->cols();
    size_t queryIdx_, trainIdx_, oldTrainIdx_, oldQueryIdx_, k_;
    int32_t oldDistance_;
    size_t queryIdxEnd_ = queryIdxStart + count;

    size_t matchesRows = matches->rows();
    size_t matchesCols = matches->cols();

    // alloc() hamming distances
    CompVPtr<CompVArray<int32_t>* > hammingDistancesArray;
    COMPV_CHECK_CODE_RETURN(err_ = CompVArray<int32_t>::newObj(&hammingDistancesArray, 1, count, 1));
    int32_t *hammingDistances_ = const_cast<int32_t*>(hammingDistancesArray->ptr());

    const uint8_t *queryDescriptions_ = queryDescriptions->ptr(queryIdxStart, 0), *trainDescriptions_ = trainDescriptions->ptr(0, 0);
    CompVDMatch* matches_ = const_cast<CompVDMatch*>(matches->ptr(0, queryIdxStart));

    // Set default values for the first knn-th rows to INT_MAX to make sure we will sort correctly the first inserted values
    // We'll have knn sorted values for the first round-trip. As the sorting is done for knn elements only then, when a candidate is
    // rejected there is no need to save it for the next round-trip as the first knn elements are already the minimums.


    if (matchesRows == 2) { // Means KNN = 2, frequently used for ratio test
        const int32_t* hD;
        // initialization
        for (queryIdx_ = queryIdxStart, match0_ = matches_, match1_ = (matches_ + matchesCols); queryIdx_ < queryIdxEnd_; ++queryIdx_, ++match0_, ++match1_) {
            match0_->distance = INT_MAX;
            match0_->imageIdx = 0;
            match1_->distance = INT_MAX;
            match1_->imageIdx = 0;
        }
        // round-trips
        for (trainIdx_ = 0; trainIdx_ < trainRows_; ++trainIdx_, trainDescriptions_ += trainStrideBytes_) {
            COMPV_CHECK_CODE_RETURN(err_ = CompVHamming::distance(queryDescriptions_, (int)queryCols_, (int)queryDescriptions->strideInBytes(), (int)count,
                                           trainDescriptions_, hammingDistances_));
            for (queryIdx_ = queryIdxStart, hD = hammingDistances_, match0_ = matches_, match1_ = (matches_ + matchesCols); queryIdx_ < queryIdxEnd_; ++queryIdx_, ++hD, ++match0_, ++match1_) {
                // "match0_ <= match1_" -> if (newDistance_ not< match1_) then, newDistance_ not < match0_
                if (*hD < match1_->distance) {
                    if (*hD < match0_->distance) {
                        oldDistance_ = match0_->distance, oldTrainIdx_ = match0_->trainIdx, oldQueryIdx_ = match0_->queryIdx;
                        match0_->distance = *hD, match0_->trainIdx = trainIdx_, match0_->queryIdx = queryIdx_;
                        if (oldDistance_ < match1_->distance) {
                            match1_->distance = oldDistance_, match1_->trainIdx = oldTrainIdx_, match1_->queryIdx = oldQueryIdx_;
                        }
                    }
                    else {
                        match1_->distance = *hD, match1_->trainIdx = trainIdx_, match1_->queryIdx = queryIdx_;
                    }
                }
            }
        }
    }
    else {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
        size_t newTrainIdx_, newQueryIdx_, hammingIdx_;
        int32_t newDistance_;
        // initialization
        for (queryIdx_ = queryIdxStart, match0_ = matches_; queryIdx_ < queryIdxEnd_; ++queryIdx_, ++match0_) {
            match1_ = match0_;
            for (k_ = 0; k_ < matchesRows; ++k_) {
                match1_->distance = INT_MAX;
                match1_->imageIdx = 0;
                match1_ += matchesCols;
            }
        }
        // round-trips
        for (trainIdx_ = 0; trainIdx_ < trainRows_; ++trainIdx_, trainDescriptions_ += trainStrideBytes_) {
            COMPV_CHECK_CODE_RETURN(err_ = CompVHamming::distance(queryDescriptions_, (int)queryCols_, (int)queryDescriptions->strideInBytes(), (int)count,
                                           trainDescriptions_, hammingDistances_));
            for (queryIdx_ = queryIdxStart, hammingIdx_ = 0, match0_ = matches_; queryIdx_ < queryIdxEnd_; ++queryIdx_, ++hammingIdx_, ++match0_) {
                newDistance_ = hammingDistances_[hammingIdx_], newTrainIdx_ = trainIdx_, newQueryIdx_ = queryIdx_;
                match1_ = match0_;
                for (k_ = 0; k_ < matchesRows; ++k_) {
                    if (newDistance_ < match1_->distance) {
                        oldDistance_ = match1_->distance, oldTrainIdx_ = match1_->trainIdx, oldQueryIdx_ = match1_->queryIdx;
                        match1_->distance = newDistance_, match1_->trainIdx = newTrainIdx_, match1_->queryIdx = newQueryIdx_;
                        newDistance_ = oldDistance_, newTrainIdx_ = oldTrainIdx_, newQueryIdx_ = oldQueryIdx_;
                    }
                    match1_ += matchesCols;
                }
            }
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