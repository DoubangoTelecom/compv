/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEM_H_)
#define _COMPV_BASE_MEM_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"

#include "compv/base/parallel/compv_mutex.h"

#include <map>

COMPV_NAMESPACE_BEGIN()

typedef struct compv_special_mem_s {
    uintptr_t addr;
    size_t size;
    size_t alignment;
public:
    compv_special_mem_s() : addr(0), size(0), alignment(0) { }
    compv_special_mem_s(uintptr_t _addr, size_t _size, size_t _alignment) {
        addr = _addr;
        size = _size;
        alignment = _alignment;
    }
}
compv_special_mem_t;

class COMPV_BASE_API CompVMem
{
public:
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE deInit();
    static COMPV_ERROR_CODE copy(void* dstPtr, const void* srcPtr, size_t size, const bool enforceSingleThread = false);
    static COMPV_ERROR_CODE copyNTA(void* dstPtr, const void* srcPtr, size_t size, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE unpack4(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, uint8_t* dstPt3, const compv_uint8x4_t* srcPtr, size_t width, size_t height, size_t stride, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE unpack3(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, const compv_uint8x3_t* srcPtr, size_t width, size_t height, size_t stride, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE unpack2(uint8_t* dstPt0, uint8_t* dstPt1, const compv_uint8x2_t* srcPtr, size_t width, size_t height, size_t stride, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE pack4(compv_uint8x4_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, const uint8_t* srcPt3, size_t width, size_t height, size_t stride, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE pack3(compv_uint8x3_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, size_t width, size_t height, size_t stride, const bool enforceSingleThread = false);
	static COMPV_ERROR_CODE pack2(compv_uint8x2_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, size_t width, size_t height, size_t stride, const bool enforceSingleThread = false);

    static COMPV_ERROR_CODE set(void* dstPtr, compv_scalar_t val, compv_uscalar_t count, compv_uscalar_t sizeOfEltInBytes = 1);

    static COMPV_ERROR_CODE zero(void* dstPtr, size_t size);
    static COMPV_ERROR_CODE zeroNTA(void* dstPtr, size_t size);

    static void* malloc(size_t size);
    static void* realloc(void * ptr, size_t size);
    static void* calloc(size_t num, size_t size);
    static void free(void** ptr);

    static void* mallocAligned(size_t size, size_t alignment = CompVMem::bestAlignment());
    static void* reallocAligned(void * ptr, size_t size, size_t alignment = CompVMem::bestAlignment());
    static void* callocAligned(size_t num, size_t size, size_t alignment = CompVMem::bestAlignment());
    static void freeAligned(void** ptr);

    static uintptr_t alignBackward(uintptr_t ptr, size_t alignment = CompVMem::bestAlignment());
    static uintptr_t alignForward(uintptr_t ptr, size_t alignment = CompVMem::bestAlignment());
    static size_t alignSizeOnCacheLineAndSIMD(size_t size);

	static bool isGpuFriendly(const void* mem, size_t size);

	static COMPV_ERROR_CODE poolSetHeapLimit(const size_t sizeInBytes);
	static COMPV_ERROR_CODE poolCleanBuffersForAllThreads();
	static COMPV_ERROR_CODE poolCleanBuffersForCurrentThread();

	static bool isTbbMallocEnabled();
    static int bestAlignment();
    static bool isSpecial(void* ptr);
    static size_t specialTotalMemSize();
    static size_t specialsCount();
    static bool isEmpty();
	static bool isInitialized();

private:
    static void specialsLock();
    static void specialsUnLock();

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static bool s_bInitialized;
    static std::map<uintptr_t, compv_special_mem_t > s_Specials;
    static CompVPtr<CompVMutex* >s_SpecialsMutex;
    static void(*MemSetDword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
    static void(*MemSetQword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
    static void(*MemSetDQword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MEM_H_ */
