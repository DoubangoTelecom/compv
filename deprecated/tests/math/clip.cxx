#include <compv/compv_api.h>

using namespace compv;

#define LOOP_COUNT	100000000

COMPV_ERROR_CODE TestMax()
{
    compv_scalar_t x, y, z;
    uint64_t timeStart, timeEnd;

    timeStart = CompVTime::getNowMills();
    for (compv_scalar_t i = 0; i < LOOP_COUNT; ++i) {
        x = rand() * ((rand() & 1) ? 1 : -1);
        y = rand() * ((rand() & 1) ? 1 : -1);
        z = CompVMathUtils::maxVal(x, y);
        COMPV_ASSERT((z >= x && z >= y));
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time (TestMax) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE TestMin()
{
    compv_scalar_t x, y, z;
    uint64_t timeStart, timeEnd;

    timeStart = CompVTime::getNowMills();
    for (compv_scalar_t i = 0; i < LOOP_COUNT; ++i) {
        x = rand() * ((rand() & 1) ? 1 : -1);
        y = rand() * ((rand() & 1) ? 1 : -1);
        z = CompVMathUtils::minVal(x, y);
        COMPV_ASSERT((z <= x && z <= y));
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time (TestMin) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE TestClip3()
{
    compv_scalar_t x, y, z;
    uint64_t timeStart, timeEnd;

    timeStart = CompVTime::getNowMills();
    for (compv_scalar_t i = 0; i < LOOP_COUNT; ++i) {
        x = rand() * ((rand() & 1) ? 1 : -1);
        y = rand() * ((rand() & 1) ? 1 : -1);
        if (x > y) {
            z = x;
            x = y;
            y = z;
        }
        z = rand() * ((rand() & 1) ? 1 : -1);
        z = CompVMathUtils::clip3(x, y, z);
        COMPV_ASSERT(z <= y && z >= x);
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time (TestClip3) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE TestClip2()
{
    compv_scalar_t x, y, z;
    uint64_t timeStart, timeEnd;

    timeStart = CompVTime::getNowMills();
    for (compv_scalar_t i = 0; i < LOOP_COUNT; ++i) {
        x = rand() * rand();
        y = rand() * ((rand() & 1) ? 1 : -1);
        z = CompVMathUtils::clip2(x, y);
        COMPV_ASSERT(z >= 0 && z <= x);
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time (TestClip2) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    return COMPV_ERROR_CODE_S_OK;
}