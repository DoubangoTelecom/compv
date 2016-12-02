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
#	define ERR_MAX	2.8421709430404007e-014
#else
#	define TYP		float
#	define TYP_SZ	"%f"
#	define ERR_MAX	0.f // not correct, update we float implemention is done
#endif

COMPV_ERROR_CODE TestNormalize2DHartley()
{
#define POINTS_COUNT 2015 // #2015 (odd number) or #4 (is very common -Homography-)
    COMPV_ALIGN_DEFAULT() TYP x[POINTS_COUNT];
    COMPV_ALIGN_DEFAULT() TYP y[POINTS_COUNT];
    TYP tx, ty, scale;
    uint64_t timeStart, timeEnd;

#if POINTS_COUNT == 2015
#	define TX		151.12521091811413 // c++
#	define TY		1209.0999999999999 // c++
#	define SCALE	0.0012386495368348494 // c++
#	if COMPV_ARCH_X64
#		define TX_SSE2		151.12521091811411
#		define TY_SSE2		1209.0999999999999
#		define SCALE_SSE2	0.0012386495368348507
#		define TX_AVX		151.12521091811416
#		define TY_AVX		1209.0999999999999
#		define SCALE_AVX	0.0012386495368348523
#	else
    // No guarantee, this could change from one run to another -> be careful
#		define TX_SSE2		151.12521091811413
#		define TY_SSE2		1209.0999999999999
#		define SCALE_SSE2	0.0012386495368348494
#	endif
#elif POINTS_COUNT == 4
#	define TX		1.1499999999999999 // c++
#	define TY		2.5000000000000000 // c++
#	define SCALE	0.73422338733516390 // c++
#endif


    for (signed i = 0; i < POINTS_COUNT; ++i) {
        x[i] = (TYP)((i & 1) ? i : (-i * 0.7)) + 0.5;
        y[i] = ((TYP)(i * 0.2)) + i + 0.7;
    }

    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_RETURN(CompVMathStats<TYP>::normalize2D_hartley(&x[0], &y[0], POINTS_COUNT, &tx, &ty, &scale));
    }
    timeEnd = CompVTime::getNowMills();

    TYP err_tx = COMPV_MATH_ABS(TX - tx);
    TYP err_ty = COMPV_MATH_ABS(TY - ty);
    TYP err_scale = COMPV_MATH_ABS(SCALE - scale);
    COMPV_DEBUG_INFO_EX("TestNormalize2DHartley", "tx="TYP_SZ", ty="TYP_SZ", scale="TYP_SZ", err_tx="TYP_SZ", err_ty="TYP_SZ", err_scale="TYP_SZ, tx, ty, scale, err_tx, err_ty, err_scale);

    COMPV_DEBUG_INFO_EX("TestNormalize2DHartley", "Elapsed time (TestNormalize2DHartley) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    // Normale test (*must*)
    COMPV_CHECK_EXP_RETURN(err_tx > ERR_MAX || err_ty > ERR_MAX || err_scale > ERR_MAX, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

    // To enforce testing (*optionale*)
#if COMPV_ARCH_X64 && POINTS_COUNT == 2015
    if (CompVCpu::isEnabled(compv::kCpuFlagAVX)) {
        COMPV_CHECK_EXP_RETURN(tx != TX_AVX || ty != TY_AVX || scale != SCALE_AVX, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
    }
    else if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
        COMPV_CHECK_EXP_RETURN(tx != TX_SSE2 || ty != TY_SSE2 || scale != SCALE_SSE2, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
    }
#elif POINTS_COUNT == 4
    COMPV_CHECK_EXP_RETURN(err_tx != 0 || err_ty != 0 || err_scale != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
#endif

    return COMPV_ERROR_CODE_S_OK;
#undef TX
#undef TY
#undef SCALE
#undef TX_SSE2
#undef TY_SSE2
#undef SCALE_SSE2
}