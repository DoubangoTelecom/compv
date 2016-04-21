#include <compv/compv_api.h>

#include <vector>
#include <algorithm>

using namespace compv;

#define loopOutCount	100 // 10
#define loopInCount		350 // 3500
#define STD_VECTOR		1
#define PRINT_LIST		0
#define ERASE			0
#define SORT			1

static COMPV_INLINE bool __isXMoreThan10Box(const CompVInterestPoint* point)
{
    return point->xf() > 10;
}

static COMPV_INLINE bool __isXMoreThan10Vect(const CompVInterestPoint& point)
{
    return point.xf() > 10;
}

static bool COMPV_INLINE __compareStrengthDecVect(const CompVInterestPoint& i, const  CompVInterestPoint& j)
{
    return (i.strength > j.strength);
}

static bool COMPV_INLINE __compareStrengthDecBox(const CompVInterestPoint* i, const  CompVInterestPoint* j)
{
    return (i->strength > j->strength);
}

bool TestPush()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    uint64_t timeStart, timeEnd;
    CompVObjWrapper<CompVBoxInterestPoint* >box; // "CompVBoxInterestPoint" has multi-threded sort which is not the case for "CompVBox<CompVInterestPoint>"
    std::vector<CompVInterestPoint> interestPointsVect;

    COMPV_CHECK_CODE_BAIL(err_ = CompVBoxInterestPoint::newObj(&box));

    timeStart = CompVTime::getNowMills();

    // Test push
    for (unsigned k = 0; k < loopOutCount; ++k) {
        for (unsigned y = 0; y < loopInCount; ++y) {
            for (unsigned x = 0; x < loopInCount; ++x) {
#if STD_VECTOR
                interestPointsVect.push_back(CompVInterestPoint((y*loopInCount) + x, 0, (float)rand()));
#else
                box->push(CompVInterestPoint((y*loopInCount) + x, 0, (float)rand()));
#endif
            }
        }
        // TEST sort
#if	SORT
#	if STD_VECTOR
        std::sort(interestPointsVect.begin(), interestPointsVect.end(), __compareStrengthDecVect);
#	else
        box->sort(__compareStrengthDecBox);
#	endif
#endif
        // Test erase
#if ERASE
#	if STD_VECTOR
        interestPointsVect.erase(std::remove_if(interestPointsVect.begin(), interestPointsVect.end(), __isXMoreThan10Vect), interestPointsVect.end());
#	else
        box->erase(__isXMoreThan10Box);
#	endif
#endif

        // CLEAR if not last loop
#if STD_VECTOR
        if (k + 1 != loopOutCount) { // clear item if not last loop
            interestPointsVect.clear();
        }
#else
        if (k + 1 != loopOutCount) { // clear item if not last loop
            box->reset();
        }
#endif
    }

#if !STD_VECTOR && !ERASE
    if (box->size() != loopInCount * loopInCount) {
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
    }
#endif

    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("List Push elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

#if SORT && !ERASE && !STD_VECTOR
    for (size_t i = 0; i < box->size(); ++i) {
        for (size_t j = i + 1; j < box->size(); ++j) {
            if (box->at(i)->xf() == box->at(j)->xf() && box->at(i)->yf() == box->at(j)->yf()) {
                COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
            }
            if (!__compareStrengthDecBox(box->at(i), box->at(j)) && box->at(i)->strength != box->at(j)->strength) {
                COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
            }
        }
    }
#endif

#if ERASE && !STD_VECTOR
    for (size_t i = 0; i < box->size(); ++i) {
        if (__isXMoreThan10Box(box->at(i))) {
            COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
        }
    }
#endif

#if PRINT_LIST && !STD_VECTOR
    for (size_t i = 0; i < box->size(); ++i) {
        COMPV_DEBUG_INFO("Point: (%f, %f)", box->at(i)->xf(), box->at(i)->yf());
    }
#endif

    COMPV_DEBUG_INFO("List Push test done!");

    getchar();

bail:
    return COMPV_ERROR_CODE_IS_OK(err_);
}