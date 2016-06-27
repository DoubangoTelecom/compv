#include <compv/compv_api.h>

#include "../common.h"

#define CANNY_LOOP_COUNT	1

using namespace compv;

COMPV_ERROR_CODE TestCanny()
{
	// Detect edges
	uint64_t timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < CANNY_LOOP_COUNT; ++i) {
		
	}
	uint64_t timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time (TestCanny) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}