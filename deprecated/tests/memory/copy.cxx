#include <compv/compv_api.h>

using namespace compv;

#define loopCount	100000 //1000000
// MemCpy function use ASM or INTRIN only when we're sure that data caching is useless.
// Data caching is useless when the copied data size is more than the cache1 size (K). In this case
// we're sure the first K bytes will be overriden by the next N bytes and the calling function will likely read the copied
// data starting at index 0 which means within [0-K] and won't find them.
#define memSize		((32 << 10) * 2) + 16/*SSE-extra-no16x16*/ + 32/*AVX-extra-no16x16*/ + 7 // Core i7 cache1 size = 32KB
#define memAlign	64 // aligned on cache-line size

bool TestCopyNTA()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    uint64_t timeStart, timeEnd;
    uint8_t* memSrcPtr = (uint8_t*)CompVMem::mallocAligned(memSize, memAlign);
    COMPV_CHECK_EXP_BAIL(!memSrcPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    uint8_t* memDstPtr = (uint8_t*)CompVMem::mallocAligned(memSize, memAlign);
    COMPV_CHECK_EXP_BAIL(!memDstPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    for (size_t i = 0; i < memSize; ++i) {
        memSrcPtr[i] = rand() & 0xFF;
    }

    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < loopCount; ++i) {
        CompVMem::copyNTA(memDstPtr, memSrcPtr, memSize);
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("MemoryCopyNTA elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

    if (memSrcPtr && memDstPtr) {
        for (size_t i = 0; i < memSize; ++i) {
            if (memSrcPtr[i] != memDstPtr[i]) {
                COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E);
            }
        }
    }

bail:
    CompVMem::free((void**)&memSrcPtr);
    CompVMem::free((void**)&memDstPtr);
    return COMPV_ERROR_CODE_IS_OK(err_);
}