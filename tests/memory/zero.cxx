#include <compv/compv_api.h>

using namespace compv;

#define loopCount	100000 //1000000
#define memSize		((32 << 10) * 2) + 16/*SSE-extra-no16x16*/ + 32/*AVX-extra-no16x16*/ + 7 // Core i7 cache1 size = 32KB
#define memAlign	64 // aligned on cache-line size

bool TestZeroNTA()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	uint64_t timeStart, timeEnd;
	uint8_t* memDstPtr = (uint8_t*)CompVMem::mallocAligned(memSize, memAlign);
	COMPV_CHECK_EXP_BAIL(!memDstPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	for (size_t i = 0; i < memSize; ++i) {
		memDstPtr[i] = rand() & 0xFF;
	}

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < loopCount; ++i) {
		CompVMem::zeroNTA(memDstPtr, memSize);
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("MemoryZeroNTA elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

	for (size_t i = 0; i < memSize; ++i) {
		if (memDstPtr[i] != 0) {
			COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E);
		}
	}

bail:
	CompVMem::free((void**)&memDstPtr);
	return COMPV_ERROR_CODE_IS_OK(err_);
}