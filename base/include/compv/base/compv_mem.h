/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
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
    static COMPV_ERROR_CODE copy(void* dstPtr, const void* srcPtr, size_t size);
    static COMPV_ERROR_CODE copyNTA(void* dstPtr, const void* srcPtr, size_t size);
	static COMPV_ERROR_CODE copy3(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, const compv_uint8x3_t* srcPtr, size_t width, size_t height, size_t stride);

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

	static bool isTbbMallocEnabled();
	static COMPV_ERROR_CODE setTbbMallocEnabled(bool enabled);
    static int bestAlignment();
    static bool isSpecial(void* ptr);
    static size_t specialTotalMemSize();
    static size_t specialsCount();
    static bool isEmpty();

private:
    static void specialsLock();
    static void specialsUnLock();

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static bool s_bInitialize;
	static bool s_bTbbMallocEnabled;
    static std::map<uintptr_t, compv_special_mem_t > s_Specials;
    static CompVPtr<CompVMutex* >s_SpecialsMutex;
    static void(*MemSetDword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
    static void(*MemSetQword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
    static void(*MemSetDQword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MEM_H_ */
