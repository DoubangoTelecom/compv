#include <compv/compv_api.h>

#include <vector>

using namespace compv;

#define loopCount	2 // 3500
#define STD_VECTOR	0
#define PRINT_LIST	1

bool TestPush()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	
	uint64_t timeStart, timeEnd;
	CompVObjWrapper<CompVListInterestPoint* >interestPointsList;
	std::vector<CompVInterestPoint> interestPointsVect;

	COMPV_CHECK_CODE_BAIL(err_ = CompVListInterestPoint::newObj(&interestPointsList));

	timeStart = CompVTime::getNowMills();
	for (int32_t y = 0; y < loopCount; ++y) {
		for (int32_t x = 0; x < loopCount; ++x) {
#if STD_VECTOR
			interestPointsVect.push_back(CompVInterestPoint((y*loopCount)+x, 0));
#else
			interestPointsList->push_back(CompVInterestPoint((y*loopCount)+x, 0));
#endif
		}
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