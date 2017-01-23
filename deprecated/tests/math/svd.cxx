#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define LOOP_COUNT		1
#define TYPE			double  // double or float
#define NUM_ROWS		200 + 9 // +9 to make it SIMD-unfriendly for testing
#define NUM_COLS		100 + 9 // +9 to make it SIMD-unfriendly for testing
#define MD5_U_DOUBLE	"e363c0aac86870f0f2b0bad310e2bcd5"
#define MD5_D_DOUBLE	"8011c4733fe209db9b91ff8a04940075"
#define MD5_V_DOUBLE	"244829e0c806ac11a56b8263e540eb5f"

COMPV_ERROR_CODE TestSVD()
{
    CompVPtrArray(TYPE) array, U, D, V;
    TYPE* row;
    uint64_t timeStart, timeEnd;

    COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::newObjAligned(&array, NUM_ROWS, NUM_COLS));

    // Build random data (must be deterministic to have same MD5)
    for (signed j = 0; j < NUM_ROWS; ++j) {
        row = const_cast<TYPE*>(array->ptr(j));
        for (signed i = 0; i < NUM_COLS; ++i) {
            row[i] = (TYPE)(((i & 1) ? i : -i) + (j * 0.5) + j + 0.7);
        }
    }

    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::svd(array, U, D, V));
    }
    timeEnd = CompVTime::getNowMills();

    const std::string md5U = arrayMD5<TYPE>(U);
    const std::string md5D = arrayMD5<TYPE>(D);
    const std::string md5V = arrayMD5<TYPE>(V);
    COMPV_ASSERT(md5U == MD5_U_DOUBLE);
    COMPV_ASSERT(md5D == MD5_D_DOUBLE);
    COMPV_ASSERT(md5V == MD5_V_DOUBLE);

    COMPV_DEBUG_INFO("Elapsed time (TestSVD) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    return COMPV_ERROR_CODE_S_OK;
}

