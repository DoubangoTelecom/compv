#include <compv/compv_api.h>

#include <vector>

using namespace compv;

#define loopOutCount	5
#define loopInCount		3500 // 3500
#define STD_VECTOR	0
#define PRINT_LIST	0

bool TestPush()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    uint64_t timeStart, timeEnd;
    CompVObjWrapper<CompVListInterestPoint* >interestPointsList;
    std::vector<CompVInterestPoint> interestPointsVect;

    COMPV_CHECK_CODE_BAIL(err_ = CompVListInterestPoint::newObj(&interestPointsList));

    timeStart = CompVTime::getNowMills();
    for (unsigned k = 0; k < loopOutCount; ++k) {
        for (unsigned y = 0; y < loopInCount; ++y) {
            for (unsigned x = 0; x < loopInCount; ++x) {
#if STD_VECTOR
                interestPointsVect.push_back(CompVInterestPoint((y*loopInCount)+x, 0));
#else
                interestPointsList->push_back(CompVInterestPoint((y*loopInCount) + x, 0));
#endif
            }
        }
#if STD_VECTOR
        interestPointsVect.clear();
#else
        interestPointsList->reset();
#endif
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("List Push elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

#if PRINT_LIST && !STD_VECTOR
    const CompVInterestPoint* point = interestPointsList->front();
    while (point) {
        COMPV_DEBUG_INFO("Point: (%f, %f)", point->xf(), point->yf());
        point = interestPointsList->next(point);
    }
#endif

bail:

    return COMPV_ERROR_CODE_IS_OK(err_);
}