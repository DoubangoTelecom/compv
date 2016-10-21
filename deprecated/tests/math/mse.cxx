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

COMPV_ERROR_CODE TestMSE2D_homogeneous()
{
#define POINTS_COUNT 4 // #2015 (odd number) or #4 (is very common -Homography-)
#if POINTS_COUNT == 2015
#	define MD5		"20b0ae121b9af0bb7bca94529a01fbf3"
#	define MSE_SUM	3726586387.7721214
#elif POINTS_COUNT == 4
#	define MD5		"e5e02dbfae8d85f17441554eed8f9af5"
#	define MSE_SUM	332.33756704321195
#endif
	COMPV_ALIGN_DEFAULT() TYP aX_h[POINTS_COUNT];
	COMPV_ALIGN_DEFAULT() TYP aY_h[POINTS_COUNT];
	COMPV_ALIGN_DEFAULT() TYP aZ_h[POINTS_COUNT];
	COMPV_ALIGN_DEFAULT() TYP bX[POINTS_COUNT];
	COMPV_ALIGN_DEFAULT() TYP bY[POINTS_COUNT];
	CompVPtrArray(TYP) mse;
	uint64_t timeStart, timeEnd;

	for (signed i = 0; i < POINTS_COUNT; ++i) {
		aX_h[i] = (TYP)((i & 1) ? i : (-i * 0.7)) + 0.5;
		bX[i] = (TYP)((i & 1) ? i : (-i * 0.3)) + 5.;
		aY_h[i] = ((TYP)(i * 0.2)) + i + 0.7;
		bY[i] = (TYP)((i & 1) ? i : (-i * 0.8)) + 7.8;
		aZ_h[i] = ((TYP)(i * 0.3)) + i + 0.4;
	}

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathStats<TYP>::mse2D_homogeneous(&aX_h[0], &aY_h[0], &aZ_h[0], &bX[0], &bY[0], mse, POINTS_COUNT));
	}
	timeEnd = CompVTime::getNowMills();

	TYP mse_sum = 0;
	const TYP* msePtr = mse->ptr();
	for (size_t i = 0; i < mse->cols(); ++i) {
		mse_sum += msePtr[i];
	}
	COMPV_DEBUG_INFO_EX("TestMSE2D_homogeneous", "MSE_computed="TYP_SZ", MSE_expected="TYP_SZ, mse_sum, MSE_SUM);

	COMPV_DEBUG_INFO_EX("TestMSE2D_homogeneous", "Elapsed time (TestMSE2D_homogeneous) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_EXP_RETURN(mse_sum != MSE_SUM, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	// Not required to have same MD5 (AVX, SSE, NEON...). Check mse_sum error only
	const std::string md5 = arrayMD5(mse);
	COMPV_CHECK_EXP_RETURN(md5 != MD5, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	
	return COMPV_ERROR_CODE_S_OK;
}