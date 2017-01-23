#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define TYPE_DOUBLE			0
#define TYPE_FLOAT			1

#define TYPE		TYPE_DOUBLE  // double or float
#define LOOP_COUNT	1

#if TYPE == TYPE_DOUBLE
#	define TYP		double
#	define TYP_SZ	"%e"
#else
#	define TYP		float
#	define TYP_SZ	"%f"
#	define ERR_MAX	0.f // not correct, update we float implemention is done
#endif

COMPV_ERROR_CODE TestVariance()
{
#	define POINTS_COUNT 2015
#	define VARIANCE		1524545458375.9475
#	define VARIANCE_SSE	1524545458375.9446
#	define VARIANCE_AVX	1524545458375.9414

    COMPV_ALIGN_DEFAULT() TYP data[POINTS_COUNT];
    TYP mean = 1234569.156325, var_computed, var_expected;
    uint64_t timeStart, timeEnd;

    for (signed i = 0; i < POINTS_COUNT; ++i) {
        data[i] = (TYP)((i & 1) ? i : (-i * 0.7)) + 0.5;
    }

    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_RETURN(CompVMathStats<TYP>::variance(&data[0], POINTS_COUNT, &mean, &var_computed));
    }
    timeEnd = CompVTime::getNowMills();

    var_expected = CompVCpu::isEnabled(compv::kCpuFlagAVX) ? VARIANCE_AVX : (CompVCpu::isEnabled(compv::kCpuFlagSSE2) ? VARIANCE_SSE : VARIANCE);
    TYP var_error = COMPV_MATH_ABS(var_computed - VARIANCE);

    COMPV_DEBUG_INFO_EX("TestVariance", "VAR_computed="TYP_SZ", VAR_expected="TYP_SZ", VAR_err="TYP_SZ, var_computed, var_expected, var_error);

    COMPV_DEBUG_INFO_EX("TestVariance", "Elapsed time (TestVariance) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    COMPV_CHECK_EXP_RETURN(var_computed != var_expected, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

    return COMPV_ERROR_CODE_S_OK;
}