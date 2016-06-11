/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_mem.h"
#include "compv/compv_cpu.h"
#include "compv/compv_debug.h"
#include "compv/math/compv_math_utils.h"

#include "compv/parallel/compv_mutex.h"

#include "compv/intrinsics/x86/compv_mem_intrin_sse2.h"
#include "compv/intrinsics/x86/compv_mem_intrin_avx.h"

COMPV_NAMESPACE_BEGIN()

#if !defined(COMPV_MEMALIGN_ALWAYS)
#	define COMPV_MEMALIGN_ALWAYS 1
#endif

#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0)) && !defined(COMPV_MEM_CHECK)
#	define COMPV_MEM_CHECK 1
#endif

#if !defined(COMPV_OS_WINDOWS) && !defined(HAVE_POSIX_MEMALIGN)
#   define HAVE_POSIX_MEMALIGN 1
#endif

// COMPV_MEM_SIZE_MIN_SIMD must be > 32 (default alignment)
#define COMPV_MEM_SIZE_MIN_SIMD 32*16 // no real gain on small sizes

// X86
#if defined(COMPV_ARCH_X86) && defined(COMPV_ASM)
COMPV_EXTERNC void MemCopyNTA_Asm_Aligned11_X86_SSE2(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void MemCopyNTA_Asm_Aligned11_X86_AVX(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
#endif
// X64
#if defined(COMPV_ARCH_X64) && defined(COMPV_ASM)
COMPV_EXTERNC void MemCopyNTA_Asm_Aligned11_X64_SSE2(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void MemCopyNTA_Asm_Aligned11_X64_AVX(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
#endif

std::map<uintptr_t, compv_special_mem_t > CompVMem::s_Specials;
CompVPtr<CompVMutex* > CompVMem::s_SpecialsMutex;
bool CompVMem::s_bInitialize = false;

typedef void(*CompVMemCopy)(void* dstPtr, const void*srcPtr, compv_uscalar_t size);

static void CompVMemCopy_C(void* dstPtr, const void*srcPtr, compv_uscalar_t size)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    memcpy(dstPtr, srcPtr, (size_t)size);
}

COMPV_ERROR_CODE CompVMem::init()
{
    if (!s_bInitialize) {
#if COMPV_MEM_CHECK
        COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&s_SpecialsMutex));
        COMPV_DEBUG_INFO("Memory check enabled for debugging, this may slowdown the code");
#endif
        s_bInitialize = true;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::deInit()
{
    s_SpecialsMutex = NULL;
    s_bInitialize = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::copy(void* dstPtr, const void*srcPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !srcPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVMemCopy cpy = CompVMemCopy_C;
    cpy(dstPtr, srcPtr, size);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::copyNTA(void* dstPtr, const void*srcPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !srcPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    size_t align = 1;
    CompVMemCopy cpy = CompVMemCopy_C;

    if (size > COMPV_MEM_SIZE_MIN_SIMD) {
        if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
            if (COMPV_IS_ALIGNED_SSE(dstPtr) && COMPV_IS_ALIGNED_SSE(srcPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86((cpy = MemCopyNTA_Intrin_Aligned_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
                COMPV_EXEC_IFDEF_ASM_X86((cpy = MemCopyNTA_Asm_Aligned11_X86_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
                COMPV_EXEC_IFDEF_ASM_X64((cpy = MemCopyNTA_Asm_Aligned11_X64_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
            }
        }
        if (CompVCpu::isEnabled(kCpuFlagAVX)) {
            if (COMPV_IS_ALIGNED_AVX(dstPtr) && COMPV_IS_ALIGNED_AVX(srcPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86((cpy = MemCopyNTA_Intrin_Aligned_AVX, align = COMPV_SIMD_ALIGNV_AVX));
                COMPV_EXEC_IFDEF_ASM_X86((cpy = MemCopyNTA_Asm_Aligned11_X86_AVX, align = COMPV_SIMD_ALIGNV_AVX));
                COMPV_EXEC_IFDEF_ASM_X64((cpy = MemCopyNTA_Asm_Aligned11_X64_AVX, align = COMPV_SIMD_ALIGNV_AVX));
            }
        }
    }

    cpy(dstPtr, srcPtr, size);
    size_t copied = (size / align) * align;
    if (copied < size) {
        uint8_t* dstPtr_ = ((uint8_t*)dstPtr) + copied;
        const uint8_t* srcPtr_ = ((const uint8_t*)srcPtr) + copied;
        for (size_t i = copied; i < size; ++i) {
            *dstPtr_++ = *srcPtr_++;
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

typedef void(*CompVMemZero)(void* dstPtr, compv_uscalar_t size);

static void CompVMemZero_C(void* dstPtr, compv_uscalar_t size)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
    memset(dstPtr, 0, (size_t)size);
}

COMPV_ERROR_CODE CompVMem::zero(void* dstPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVMemZero setz = CompVMemZero_C;
    setz(dstPtr, size);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::zeroNTA(void* dstPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVMemZero setz = CompVMemZero_C;
    size_t align = 1;

    if (size > COMPV_MEM_SIZE_MIN_SIMD) {
        if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
            if (COMPV_IS_ALIGNED_SSE(dstPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86((setz = MemZeroNTA_Intrin_Aligned_SSE2, align = COMPV_SIMD_ALIGNV_SSE));
            }
        }
        if (CompVCpu::isEnabled(kCpuFlagAVX)) {
            if (COMPV_IS_ALIGNED_AVX(dstPtr)) {
            }
        }
    }

    setz(dstPtr, size);
    size_t copied = (size / align) * align;
    if (copied < size) {
        uint8_t* dstPtr_ = ((uint8_t*)dstPtr) + copied;
        for (size_t i = copied; i < size; ++i) {
            *dstPtr_++ = 0;
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

/**
* Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
* The content of the newly allocated block of memory is not initialized, remaining with indeterminate values.
* @param size Size of the memory block, in bytes.
* @retval On success, a pointer to the memory block allocated by the function.
* It is up to you to free the returned pointer.
*/
void* CompVMem::malloc(size_t size)
{
#if COMPV_MEMALIGN_ALWAYS
    return mallocAligned(size);
#else
    void *pMem = ::malloc(size);
    if (!pMem) {
        COMPV_DEBUG_ERROR("Memory allocation failed");
    }
    return pMem;
#endif
}

/**
* Reallocate memory block.
* In case that ptr is NULL, the function behaves exactly as @a tsk_malloc, assigning a new block of size bytes and returning a pointer to the beginning of it.
* The function may move the memory block to a new location, in which case the new location is returned. The content of the memory block is preserved up to the lesser of the
* new and old sizes, even if the block is moved. If the new size is larger, the value of the newly allocated portion is indeterminate.
* In case that the size is 0, the memory previously allocated in ptr is deallocated as if a call to free was made, and a NULL pointer is returned.
* @param ptr Pointer to a memory block previously allocated with malloc, calloc or realloc to be reallocated.
* If this is NULL, a new block is allocated and a pointer to it is returned by the function.
* @param size New size for the memory block, in bytes.
* If it is 0 and ptr points to an existing block of memory, the memory block pointed by ptr is deallocated and a NULL pointer is returned.
* @retval A pointer to the reallocated memory block, which may be either the same as the ptr argument or a new location.
* The type of this pointer is void*, which can be cast to the desired type of data pointer in order to be dereferenceable.
* If the function failed to allocate the requested block of memory, a NULL pointer is returned.
* It is up to you to free the returned pointer.
*/
void* CompVMem::realloc(void* ptr, size_t size)
{
#if COMPV_MEM_CHECK
    if (isSpecial(ptr)) {
        return reallocAligned(ptr, size);
    }
#endif
#if COMPV_MEMALIGN_ALWAYS
    return reallocAligned(ptr, size);
#else
    void *pMem = NULL;
    if (size) {
        if (ptr) {
            if (!(pMem = ::realloc(ptr, size))) {
                COMPV_DEBUG_ERROR("Memory reallocation failed");
            }
        }
        else {
            if (!(pMem = ::calloc(size, 1))) {
                COMPV_DEBUG_ERROR("Memory allocation (%u) failed", (unsigned)size);
            }
        }
    }
    else if (ptr) {
        ::free(ptr);
    }

    return pMem;
#endif
}

/**
* Allocates a block of memory for an array of num elements, each of them size bytes long, and initializes all its bits to zero.
* The effective result is the allocation of an zero-initialized memory block of (num * size) bytes.
* @param num Number of elements to be allocated
* @param size Size of elements
* @retval A pointer to the memory block allocated by the function. The type of this pointer is always void*, which can be cast to the desired type of data pointer in order to be dereferenceable.
* If the function failed to allocate the requested block of memory, a NULL pointer is returned.
* It is up to you to free the returned pointer.
*/
void* CompVMem::calloc(size_t num, size_t size)
{
#if COMPV_MEMALIGN_ALWAYS
    return callocAligned(num, size);
#else
    void* pMem = NULL;
    if (num && size) {
        pMem = ::calloc(num, size);
        if (!pMem) {
            COMPV_DEBUG_ERROR("Memory allocation failed. num=%u and size=%u", (unsigned)num, (unsigned)size);
        }
    }
    return pMem;
#endif
}

/**
* Deallocate space in memory.
* @param ptr Pointer to a memory block previously allocated with @a tsk_malloc, @a tsk_calloc or @a tsk_realloc to be deallocated.
* If a null pointer is passed as argument, no action occurs.
*/
void CompVMem::free(void** ptr)
{
    if (ptr && *ptr) {
#	if COMPV_MEM_CHECK
        if (isSpecial(*ptr)) {
            freeAligned(ptr);
        }
        else
#	endif
        {
#if COMPV_MEMALIGN_ALWAYS
            freeAligned(ptr);
#else
            ::free(*ptr);
#endif
        }
        *ptr = NULL;
    }
}

void* CompVMem::mallocAligned(size_t size, int alignment/*= CompVMem::getBestAlignment()*/)
{
    void* pMem;
#if COMPV_OS_WINDOWS && !COMPV_UNDER_OS_CE && !COMPV_OS_WINDOWS_RT
    pMem = _aligned_malloc(size, alignment);
#elif HAVE_POSIX_MEMALIGN
    pMem = NULL;
    posix_memalign(&pMem, (size_t)alignment, size);
#else
    pMem = ::malloc(size + alignment);
    if (pMem) {
        long pad = ((~(long)pMem) % alignment) + 1;
        pMem = ((uint8_t*)pMem) + pad; // pad
        ((uint8_t*)pMem)[-1] = (uint8_t)pad; // store the pad for later use
    }
#endif
#	if COMPV_MEM_CHECK
    if (pMem) {
        CompVMem::specialsLock();
        CompVMem::s_Specials.insert(std::pair<uintptr_t, compv_special_mem_t>((uintptr_t)pMem, compv_special_mem_t((uintptr_t)pMem, size, alignment)));
        CompVMem::specialsUnLock();
    }
#	endif
    return pMem;
}

void* CompVMem::reallocAligned(void* ptr, size_t size, int alignment/*= CompVMem::getBestAlignment()*/)
{
#if COMPV_MEM_CHECK
    if (ptr && !isSpecial(ptr)) {
        COMPV_DEBUG_FATAL("Using reallocAligned on no-special address: %lx", (uintptr_t)ptr);
        return NULL;
    }
#endif
    void* pMem;
#if COMPV_OS_WINDOWS && !COMPV_OS_WINDOWS_CE && !COMPV_OS_WINDOWS_RT
    pMem = _aligned_realloc(ptr, size, alignment);
#	if COMPV_MEM_CHECK
    if (pMem != ptr) {
        CompVMem::specialsLock();
        CompVMem::s_Specials.erase((uintptr_t)ptr);
        if (pMem) {
            CompVMem::s_Specials.insert(std::pair<uintptr_t, compv_special_mem_t>((uintptr_t)pMem, compv_special_mem_t((uintptr_t)pMem, size, alignment)));
        }
        CompVMem::specialsUnLock();
    }
#	endif
#else
    pMem = CompVMem::mallocAligned(size);
    if (pMem && ptr) {
#	if COMPV_MEM_CHECK
        CompVMem::specialsLock();
        std::map<uintptr_t, compv_special_mem_t >::iterator it = CompVMem::s_Specials.find((uintptr_t)ptr);
        COMPV_ASSERT(it != CompVMem::s_Specials.end());
        memcpy(pMem, ptr, COMPV_MATH_MIN(it->second.size, size));
        CompVMem::specialsUnLock();
#	else
        COMPV_DEBUG_ERROR("Data lost");
#	endif
    }
    CompVMem::freeAligned(&ptr);
#endif
    return pMem;
}

void* CompVMem::callocAligned(size_t num, size_t size, int alignment/*= CompVMem::getBestAlignment()*/)
{
    void* pMem = CompVMem::mallocAligned((size * num), alignment);
    if (pMem) {
        CompVMem::zero(pMem, (size * num));
#	if COMPV_MEM_CHECK
        CompVMem::specialsLock();
        CompVMem::s_Specials.insert(std::pair<uintptr_t, compv_special_mem_t>((uintptr_t)pMem, compv_special_mem_t((uintptr_t)pMem, (size * num), alignment)));
        CompVMem::specialsUnLock();
#	endif
    }
    return pMem;
}

void CompVMem::freeAligned(void** ptr)
{
    if (ptr && *ptr) {
        void* ptr_ = *ptr;
#if COMPV_MEM_CHECK
        if (!isSpecial(ptr_)) {
            COMPV_DEBUG_FATAL("Using freeAligned on no-special address: %lx", (uintptr_t)ptr_);
        }
        else {
            CompVMem::specialsLock();
            CompVMem::s_Specials.erase((uintptr_t)ptr_);
            CompVMem::specialsUnLock();
        }
#endif
#if COMPV_OS_WINDOWS && !COMPV_OS_WINDOWS_CE && !COMPV_OS_WINDOWS_RT
        _aligned_free(ptr_);
#elif HAVE_POSIX_MEMALIGN
        ::free(ptr_);
#else
        ::free((((uint8_t*)ptr_) - ((uint8_t*)ptr_)[-1]));
#endif
        *ptr = NULL;
    }
}

// alignment must be power of two
uintptr_t CompVMem::alignBackward(uintptr_t ptr, int alignment /*= CompVMem::getBestAlignment()*/)
{
    COMPV_ASSERT(COMPV_IS_POW2(alignment));
    return (ptr & -alignment);
}

uintptr_t CompVMem::alignForward(uintptr_t ptr, int alignment /*= CompVMem::getBestAlignment()*/)
{
    COMPV_ASSERT(COMPV_IS_POW2(alignment));
    return (ptr + (alignment - 1)) & -alignment;
}

int CompVMem::getBestAlignment()
{
    static int _bestAlignment = 0;
    if (_bestAlignment == 0) {
        _bestAlignment = COMPV_SIMD_ALIGNV_DEFAULT;
        const int L1CacheLineSize = CompVCpu::getCache1LineSize(); // probably #64 or #128
        if (L1CacheLineSize > _bestAlignment && L1CacheLineSize <= 128 && (L1CacheLineSize & (_bestAlignment - 1)) == 0) {
            _bestAlignment = L1CacheLineSize;
        }
    }
    return _bestAlignment;
}

// Align the size on cache line to avoid false sharing: https://software.intel.com/en-us/articles/avoiding-and-identifying-false-sharing-among-threads
// This also make sure we'll have the right alignment required by the active SIMD implementation (e.g. AVX or NEON)
size_t CompVMem::alignSizeOnCacheLineAndSIMD(size_t size)
{
    return CompVMem::alignForward((uintptr_t)size, CompVMem::getBestAlignment());
}

// Allocated using mallocAligned, callocAligned or reallocAligned
bool CompVMem::isSpecial(void* ptr)
{
#	if COMPV_MEM_CHECK
    CompVMem::specialsLock();
    bool ret = CompVMem::s_Specials.find((uintptr_t)ptr) != CompVMem::s_Specials.end();
    CompVMem::specialsUnLock();
    return ret;
#else
    COMPV_DEBUG_INFO("Memory check disabled. Returning false for CompVMem::isSpecial() function");
    return false;
#endif
}

size_t CompVMem::getSpecialTotalMemSize()
{
#	if COMPV_MEM_CHECK
    size_t total = 0;
    CompVMem::specialsLock();
    std::map<uintptr_t, compv_special_mem_t >::iterator it = CompVMem::s_Specials.begin();
    for (; it != CompVMem::s_Specials.end(); ++it) {
        total += it->second.size;
    }
    CompVMem::specialsUnLock();
    return total;
#else
    COMPV_DEBUG_INFO("Memory check disabled. Returning 0 for CompVMem::getSpecialTotalMemSize() function");
    return 0;
#endif
}

size_t CompVMem::getSpecialsCount()
{
#	if COMPV_MEM_CHECK
    CompVMem::specialsLock();
    size_t ret = CompVMem::s_Specials.size();
    CompVMem::specialsUnLock();
    return ret;
#else
    COMPV_DEBUG_INFO("Memory check disabled. Returning 0 for CompVMem::getSpecialsCount() function");
    return 0;
#endif
}

bool CompVMem::isEmpty()
{
    return CompVMem::getSpecialsCount() == 0;
}

void CompVMem::specialsLock()
{
    if (CompVMem::s_SpecialsMutex) {
        CompVMem::s_SpecialsMutex->lock();
    }
}

void CompVMem::specialsUnLock()
{
    if (CompVMem::s_SpecialsMutex) {
        CompVMem::s_SpecialsMutex->unlock();
    }
}

COMPV_NAMESPACE_END()
