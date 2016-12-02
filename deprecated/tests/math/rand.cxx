#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT			1000
#define ARRAY_COUNT			2

COMPV_ERROR_CODE TestRand()
{
    uint64_t timeStart, timeEnd;
    uint32_t randoms[ARRAY_COUNT] = {0} ;

    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < LOOP_COUNT; ++i) {
        CompVMathUtils::rand(randoms, ARRAY_COUNT);
    }
    timeEnd = CompVTime::getNowMills();

    COMPV_DEBUG_INFO("Elapsed time (TestRand) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    return COMPV_ERROR_CODE_S_OK;
}
