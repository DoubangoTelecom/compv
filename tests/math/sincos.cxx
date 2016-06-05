#include <compv/compv_api.h>

using namespace compv;

#define LOOP_COUNT			1 // 1000000
#define ELMTS_COUNT			(360 + 500) // [0-360] degree then, xxx random values
#define P32_ERROR_MAX		0.0009f

static void sincos_System(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
{
    for (compv_scalar_t i = 0; i < count; ++i) {
        outSin[i] = ::sinf(inRad[i]);
        outCos[i] = ::cosf(inRad[i]);
    }
}

COMPV_ERROR_CODE TestSinCosP32(bool thetaWithinZeroPiTimes2 = false)
{
    uint64_t timeStart, timeEnd;
    float *inRad = NULL, *outSin = NULL, *outCos = NULL;
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    const compv_scalar_t eltmCount = thetaWithinZeroPiTimes2 ? 360 : ELMTS_COUNT;
    void(*sincos_compv)(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
        = thetaWithinZeroPiTimes2 ? CompVMathTrig::sincos_Zero_PiTime2_P32 : CompVMathTrig::sincos_P32;

    inRad = (float *)CompVMem::malloc(eltmCount * sizeof(float));
    if (!inRad) {
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    outSin = (float *)CompVMem::malloc(eltmCount * sizeof(float));
    if (!outSin) {
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    outCos = (float *)CompVMem::malloc(eltmCount * sizeof(float));
    if (!outCos) {
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }

    // Compute angles
    for (int i = 0; i < eltmCount; ++i) {
        if (i < 360) {
            inRad[i] = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT((float)i);
        }
        else {
            float flt = (rand() / (float)(rand() + 1)) * 360.f;
            if (rand() & 1) {
                flt = -flt;
            }
            inRad[i] = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(flt);
        }
    }

    // Compute sincos()
    timeStart = CompVTime::getNowMills();
    for (compv_scalar_t i = 0; i < LOOP_COUNT; ++i) {
        //sincos_System(inRad, outSin, outCos, eltmCount);
        sincos_compv(inRad, outSin, outCos, eltmCount);
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time (TestSinCos) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    float error;
    for (int i = 0; i < eltmCount; ++i) {
        error = abs(outCos[i] - ::cosf(inRad[i]));
        if (error > P32_ERROR_MAX) {
            COMPV_DEBUG_ERROR("Cosine error = %f", error);
            COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
        }
        error = abs(outSin[i] - ::sinf(inRad[i]));
        if (error > P32_ERROR_MAX) {
            COMPV_DEBUG_ERROR("Sine error = %f", error);
            COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
        }
    }

bail:
    CompVMem::free((void**)&inRad);
    CompVMem::free((void**)&outSin);
    CompVMem::free((void**)&outCos);

    return err_;
}
