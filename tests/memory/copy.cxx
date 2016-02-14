#include <compv/compv_api.h>

#include <vector>

using namespace compv;

#define loopCount	10000000
//#define memSize	((1024 * 1024) + 32 /*AVX unmatched*/ + 16 /*SSE unmatched*/ + 7/*unaligned*/) // should also check on small sizes
#define memSize	(4096)
#define memAlign	64 // aligned on cache-line size. FIXME: Test with data not aligned on the cache

bool TestCopy()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	uint64_t timeStart, timeEnd;
	uint8_t* memSrcPtr = (uint8_t*)CompVMem::mallocAligned(memSize, memAlign);
	COMPV_CHECK_EXP_BAIL(!memSrcPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	uint8_t* memDstPtr = (uint8_t*)CompVMem::mallocAligned(memSize, memAlign);
	COMPV_CHECK_EXP_BAIL(!memDstPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	for (size_t i = 0; i < memSize; ++i) {
		memSrcPtr[i] = i & 0xFF;
	}

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < loopCount; ++i) {
		CompVMem::copy(memDstPtr, memSrcPtr, memSize);
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("List Push elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

	for (size_t i = 0; i < memSize; ++i) {
		if (memSrcPtr[i] != memDstPtr[i]) {
			COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E);
		}
	}
	

bail:
	CompVMem::free((void**)&memSrcPtr);
	CompVMem::free((void**)&memDstPtr);
	return COMPV_ERROR_CODE_IS_OK(err_);
}