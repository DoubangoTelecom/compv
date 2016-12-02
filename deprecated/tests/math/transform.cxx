#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define TYPE_DOUBLE			0
#define TYPE_FLOAT			1

#define LOOP_COUNT			1
#define NUM_POINTS			200 + 15 // +15 to make it SIMD-unfriendly for testing
#define TYPE				TYPE_DOUBLE  // double or float

#if TYPE == TYPE_DOUBLE
#	define TYP double
#else
#	define TYP float
#endif

COMPV_ERROR_CODE TestHomogeneousToCartesian2D()
{
#define MD5_215 "28aa351d8531f8f140e51059bf0c2428"
#define MD5_4	"3e7d29ad3635479a3763cc963d66c9a0"
    CompVPtrArray(TYP) src, dst;

    COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::newObjAligned(&src, 3, NUM_POINTS));

    TYP* x = const_cast<TYP*>(src->ptr(0));
    TYP* y = const_cast<TYP*>(src->ptr(1));
    TYP* z = const_cast<TYP*>(src->ptr(2));

    for (signed i = 0; i < NUM_POINTS; ++i) {
        x[i] = (TYP)((i & 1) ? i : (-i * 0.7)) + 0.5;
        y[i] = ((TYP)(i * 0.2)) + i + 0.7;
        z[i] = i*i*0.8+0.8;
    }

    uint64_t timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_RETURN(CompVTransform<TYP>::homogeneousToCartesian2D(src, dst));
    }
    uint64_t timeEnd = CompVTime::getNowMills();

    const std::string md5 = arrayMD5<TYP>(dst);

    COMPV_DEBUG_INFO("Elapsed time (TestHomogeneousToCartesian2D) = [[[ %llu millis ]]]", (timeEnd - timeStart));

#if NUM_POINTS == 215
    COMPV_CHECK_EXP_RETURN(md5 != MD5_215, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
#elif NUM_POINTS == 4
    COMPV_CHECK_EXP_RETURN(md5 != MD5_4, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
#endif

    return COMPV_ERROR_CODE_S_OK;
#undef MD5_215
#undef MD5_4
}