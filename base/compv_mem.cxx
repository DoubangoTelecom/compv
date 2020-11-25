/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"
#include "compv/base/math/compv_math_utils.h"

#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/intrin/arm/compv_mem_intrin_neon.h"
#include "compv/base/intrin/x86/compv_mem_intrin_sse2.h"
#include "compv/base/intrin/x86/compv_mem_intrin_ssse3.h"
#include "compv/base/intrin/x86/compv_mem_intrin_avx.h"

#if COMPV_DLMALLOC
#include "compv/base/compv_dlmalloc.h"
#endif /* COMPV_DLMALLOC */

#if COMPV_TBBMALLOC
#include "compv/base/tbbmalloc/scalable_allocator.h"
#endif /* COMPV_TBBMALLOC */

#include <stdio.h>
#include <stdlib.h>

#define COMPV_THIS_CLASSNAME "CompVMem"

COMPV_NAMESPACE_BEGIN()

#if !defined(COMPV_MEMALIGN_ALWAYS)
#	define COMPV_MEMALIGN_ALWAYS 1
#endif

#if !defined(COMPV_MEMALIGN_MINSIZE)
#	define COMPV_MEMALIGN_MINSIZE 8
#endif

#if ((defined(_DEBUG) && _DEBUG != 0) || (defined(DEBUG) && DEBUG != 0)) && !defined(COMPV_MEM_CHECK)
#	define COMPV_MEM_CHECK 1
#endif

#if !defined(COMPV_USE_DLMALLOC) && !defined(COMPV_TBBMALLOC)
#	define COMPV_USE_DLMALLOC 0 // Crash on MT (e.g. Morph test, "USE_LOCKS" defined in header but doesn't fix the issue)
#endif

#if !defined(COMPV_OS_WINDOWS) && !defined(HAVE_POSIX_MEMALIGN)
#   define HAVE_POSIX_MEMALIGN 1
#endif

// COMPV_MEM_CPY_SIZE_MIN_SIMD must be > 32 (default alignment)
#define COMPV_MEM_CPY_SIZE_MIN_SIMD						(32 * 16) // no real gain on small sizes
#define COMPV_MEM_CPY_COUNT_MIN_SAMPLES_PER_THREAD		(4096 * 5) // Should be multiple of 4096 (cache friendly)
#define COMPV_MEM_UNPACK3_COUNT_MIN_SAMPLES_PER_THREAD	(50 * 50)
#define COMPV_MEM_UNPACK2_COUNT_MIN_SAMPLES_PER_THREAD	(50 * 50)
#define COMPV_MEM_PACK3_COUNT_MIN_SAMPLES_PER_THREAD	(50 * 50)
#define COMPV_MEM_PACK2_COUNT_MIN_SAMPLES_PER_THREAD	(50 * 50)

// X86
#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void CompVMemCopyNTA_Asm_Aligned11_X86_SSE2(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemCopyNTA_Asm_Aligned11_X86_AVX(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemSetDword_Asm_X86(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
COMPV_EXTERNC void CompVMemSetDword_Asm_X86_SSE2(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
COMPV_EXTERNC void CompVMemSetQword_Asm_X86_SSE2(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
COMPV_EXTERNC void CompVMemSetDQword_Asm_X86_SSE2(void* dstPtr, compv_scalar_t val, compv_uscalar_t count);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

// X64
#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void CompVMemCopyNTA_Asm_Aligned11_X64_SSE2(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemCopyNTA_Asm_Aligned11_X64_AVX(COMPV_ALIGNED(SSE) void* dstPtr, COMPV_ALIGNED(SSE) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemUnpack4_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) uint8_t* dstPt2, COMPV_ALIGNED(SSE) uint8_t* dstPt3, COMPV_ALIGNED(SSE) const compv_uint8x4_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemUnpack3_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) uint8_t* dstPt2, COMPV_ALIGNED(SSE) const compv_uint8x3_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemUnpack2_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) const compv_uint8x2_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack4_Asm_X64_SSE2(COMPV_ALIGNED(SSE) compv_uint8x4_t* dstPtr, COMPV_ALIGNED(SSE) const uint8_t* srcPt0, COMPV_ALIGNED(SSE) const uint8_t* srcPt1, COMPV_ALIGNED(SSE) const uint8_t* srcPt2, COMPV_ALIGNED(SSE) const uint8_t* srcPt3, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack3_Asm_X64_SSSE3(COMPV_ALIGNED(SSE) compv_uint8x3_t* dstPtr, COMPV_ALIGNED(SSE) const uint8_t* srcPt0, COMPV_ALIGNED(SSE) const uint8_t* srcPt1, COMPV_ALIGNED(SSE) const uint8_t* srcPt2, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack2_Asm_X64_SSE2(COMPV_ALIGNED(SSE) compv_uint8x2_t* dstPtr, COMPV_ALIGNED(SSE) const uint8_t* srcPt0, COMPV_ALIGNED(SSE) const uint8_t* srcPt1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
#endif /* COMPV_ARCH_X64 && COMPV_ASM */

// ARM32
#if COMPV_ARCH_ARM32 && COMPV_ASM
COMPV_EXTERNC void CompVMemCopy_Asm_NEON32(COMPV_ALIGNED(NEON) void* dstPtr, COMPV_ALIGNED(NEON) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemZero_Asm_NEON32(COMPV_ALIGNED(NEON) void* dstPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemUnpack4_Asm_NEON32(COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) uint8_t* dstPt2, COMPV_ALIGNED(NEON) uint8_t* dstPt3, COMPV_ALIGNED(NEON) const compv_uint8x4_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemUnpack3_Asm_NEON32(COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) uint8_t* dstPt2, COMPV_ALIGNED(NEON) const compv_uint8x3_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemUnpack2_Asm_NEON32(COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) const compv_uint8x2_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack4_Asm_NEON32(COMPV_ALIGNED(NEON) compv_uint8x4_t* dstPtr, COMPV_ALIGNED(NEON) const uint8_t* srcPt0, COMPV_ALIGNED(NEON) const uint8_t* srcPt1, COMPV_ALIGNED(NEON) const uint8_t* srcPt2, COMPV_ALIGNED(NEON) const uint8_t* srcPt3, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack3_Asm_NEON32(COMPV_ALIGNED(NEON) compv_uint8x3_t* dstPtr, COMPV_ALIGNED(NEON) const uint8_t* srcPt0, COMPV_ALIGNED(NEON) const uint8_t* srcPt1, COMPV_ALIGNED(NEON) const uint8_t* srcPt2, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack2_Asm_NEON32(COMPV_ALIGNED(NEON) compv_uint8x2_t* dstPtr, COMPV_ALIGNED(NEON) const uint8_t* srcPt0, COMPV_ALIGNED(NEON) const uint8_t* srcPt1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#endif /* COMPV_ARCH_ARM32 && COMPV_ASM */

// ARM64
#if COMPV_ARCH_ARM64 && COMPV_ASM
COMPV_EXTERNC void CompVMemCopy_Asm_NEON64(COMPV_ALIGNED(NEON) void* dstPtr, COMPV_ALIGNED(NEON) const void*srcPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemZero_Asm_NEON64(COMPV_ALIGNED(NEON) void* dstPtr, compv_uscalar_t size);
COMPV_EXTERNC void CompVMemUnpack4_Asm_NEON64(COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) uint8_t* dstPt2, COMPV_ALIGNED(NEON) uint8_t* dstPt3, COMPV_ALIGNED(NEON) const compv_uint8x4_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemUnpack3_Asm_NEON64(COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) uint8_t* dstPt2, COMPV_ALIGNED(NEON) const compv_uint8x3_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemUnpack2_Asm_NEON64(COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) const compv_uint8x2_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack4_Asm_NEON64(COMPV_ALIGNED(NEON) compv_uint8x4_t* dstPtr, COMPV_ALIGNED(NEON) const uint8_t* srcPt0, COMPV_ALIGNED(NEON) const uint8_t* srcPt1, COMPV_ALIGNED(NEON) const uint8_t* srcPt2, COMPV_ALIGNED(NEON) const uint8_t* srcPt3, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack3_Asm_NEON64(COMPV_ALIGNED(NEON) compv_uint8x3_t* dstPtr, COMPV_ALIGNED(NEON) const uint8_t* srcPt0, COMPV_ALIGNED(NEON) const uint8_t* srcPt1, COMPV_ALIGNED(NEON) const uint8_t* srcPt2, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMemPack2_Asm_NEON64(COMPV_ALIGNED(NEON) compv_uint8x2_t* dstPtr, COMPV_ALIGNED(NEON) const uint8_t* srcPt0, COMPV_ALIGNED(NEON) const uint8_t* srcPt1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#endif /* COMPV_ARCH_ARM64 && COMPV_ASM */

std::map<uintptr_t, compv_special_mem_t > CompVMem::s_Specials;
CompVMutexPtr CompVMem::s_SpecialsMutex;
bool CompVMem::s_bInitialized = false;
void(*CompVMem::MemSetDword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count) = nullptr;
void(*CompVMem::MemSetQword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count) = nullptr;
void(*CompVMem::MemSetDQword)(void* dstPtr, compv_scalar_t val, compv_uscalar_t count) = nullptr;

typedef void(*CompVMemCopy)(void* dstPtr, const void*srcPtr, compv_uscalar_t size);

static void CompVMemCopy_C(void* dstPtr, const void*srcPtr, compv_uscalar_t size);
static void CompVMemUnpack4_C(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, uint8_t* dstPt3, const compv_uint8x4_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMemUnpack3_C(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, const compv_uint8x3_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMemUnpack2_C(uint8_t* dstPt0, uint8_t* dstPt1, const compv_uint8x2_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMemPack4_C(compv_uint8x4_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, const uint8_t* srcPt3, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMemPack3_C(compv_uint8x3_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMemPack2_C(compv_uint8x2_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

COMPV_ERROR_CODE CompVMem::init()
{
    if (!isInitialized()) {
#if COMPV_MEM_CHECK
        COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&s_SpecialsMutex));
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Memory check enabled for debugging, this may slowdown the code");
#endif
#if !COMPV_TBBMALLOC
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Intel tbbmalloc not enabled. You may have some perf issues on memory allocation and cache management. Sad!");
#endif
        COMPV_EXEC_IFDEF_ASM_X86(CompVMem::MemSetDword = CompVMemSetDword_Asm_X86);
        if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
            COMPV_EXEC_IFDEF_ASM_X86(CompVMem::MemSetDword = CompVMemSetDword_Asm_X86_SSE2);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMem::MemSetQword = CompVMemSetQword_Asm_X86_SSE2);
            COMPV_EXEC_IFDEF_ASM_X86(CompVMem::MemSetDQword = CompVMemSetDQword_Asm_X86_SSE2);
        }
        s_bInitialized = true;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::deInit()
{
    s_SpecialsMutex = NULL;
    s_bInitialized = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::copy(void* dstPtr, const void* srcPtr, size_t size, const bool enforceSingleThread COMPV_DEFAULT(false))
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !srcPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVMemCopy cpy = CompVMemCopy_C;
	
#if COMPV_ARCH_ARM
	if (size >= 64 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(dstPtr) && COMPV_IS_ALIGNED_NEON(srcPtr)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(cpy = CompVMemCopy_Intrin_NEON); // IMPORTANT: this intrin implementation uses "__compv_builtin_assume_aligned" to require memory alignment
		COMPV_EXEC_IFDEF_ASM_ARM32(cpy = CompVMemCopy_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(cpy = CompVMemCopy_Asm_NEON64);
	}
#endif /* COMPV_ARCH_ARM */

	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread() && !enforceSingleThread)
		? std::min((size / COMPV_MEM_CPY_COUNT_MIN_SAMPLES_PER_THREAD), static_cast<size_t>(threadDisp->threadsCount()))
		: 1;
	if (threadsCount > 1) {
		size_t threadIdx, index;
		const size_t countAny = (size / threadsCount) & -COMPV_ALIGNV_SIMD_DEFAULT; // make sure it's SIMD-aligned so that 'mt_dstPtr' and 'mt_srcPtr' remain aligned for each thread
		auto funcPtr = [&](uint8_t* mt_dstPtr, const uint8_t* mt_srcPtr, size_t mt_size) -> COMPV_ERROR_CODE {
			cpy(reinterpret_cast<void*>(mt_dstPtr), reinterpret_cast<const void*>(mt_srcPtr), mt_size);
			return COMPV_ERROR_CODE_S_OK;
		};
		uint8_t* mt_dstPtr = reinterpret_cast<uint8_t*>(dstPtr);
		const uint8_t* mt_srcPtr = reinterpret_cast<const uint8_t*>(srcPtr);
		CompVAsyncTaskIds taskIds;
		for (threadIdx = 0, index = 0; threadIdx < (threadsCount - 1); ++threadIdx, index += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, &mt_dstPtr[index], &mt_srcPtr[index], countAny), taskIds));
		}
		if (index < size) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, &mt_dstPtr[index], &mt_srcPtr[index], (size - index)), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
	}
	else {
		cpy(dstPtr, srcPtr, size);
	}
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::copyNTA(void* dstPtr, const void*srcPtr, size_t size, const bool enforceSingleThread COMPV_DEFAULT(false))
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !srcPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    size_t align = 1;
    CompVMemCopy cpy = CompVMemCopy_C;

    if (size > COMPV_MEM_CPY_SIZE_MIN_SIMD) {
        if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
            if (COMPV_IS_ALIGNED_SSE(dstPtr) && COMPV_IS_ALIGNED_SSE(srcPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86((cpy = CompVMemCopyNTA_Intrin_Aligned_SSE2, align = COMPV_ALIGNV_SIMD_SSE));
                COMPV_EXEC_IFDEF_ASM_X86((cpy = CompVMemCopyNTA_Asm_Aligned11_X86_SSE2, align = COMPV_ALIGNV_SIMD_SSE));
                COMPV_EXEC_IFDEF_ASM_X64((cpy = CompVMemCopyNTA_Asm_Aligned11_X64_SSE2, align = COMPV_ALIGNV_SIMD_SSE));
            }
        }
        if (CompVCpu::isEnabled(kCpuFlagAVX)) {
            if (COMPV_IS_ALIGNED_AVX(dstPtr) && COMPV_IS_ALIGNED_AVX(srcPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86((cpy = MemCopyNTA_Intrin_Aligned_AVX, align = COMPV_ALIGNV_SIMD_AVX));
                COMPV_EXEC_IFDEF_ASM_X86((cpy = CompVMemCopyNTA_Asm_Aligned11_X86_AVX, align = COMPV_ALIGNV_SIMD_AVX));
                COMPV_EXEC_IFDEF_ASM_X64((cpy = CompVMemCopyNTA_Asm_Aligned11_X64_AVX, align = COMPV_ALIGNV_SIMD_AVX));
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

// like arm neon vld3
COMPV_ERROR_CODE CompVMem::unpack4(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, uint8_t* dstPt3, const compv_uint8x4_t* srcPtr, size_t width, size_t height, size_t stride, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!dstPt0 || !dstPt1 || !dstPt2 || !dstPt3 || !srcPtr || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	void(*CompVMemUnpack4)(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, uint8_t* dstPt3, const compv_uint8x4_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVMemUnpack4_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSSE3) && COMPV_IS_ALIGNED_SSE(dstPt0) && COMPV_IS_ALIGNED_SSE(dstPt1) && COMPV_IS_ALIGNED_SSE(dstPt2) && COMPV_IS_ALIGNED_SSE(dstPt3) && COMPV_IS_ALIGNED_SSE(srcPtr) && COMPV_IS_ALIGNED_SSE(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMemUnpack4 = CompVMemUnpack4_Intrin_SSSE3);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMemUnpack4 = CompVMemUnpack4_Asm_X64_SSSE3);
		// No need for AVX2 implementation, tried and slower
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMemUnpack4 = CompVMemUnpack4_Intrin_NEON);
		if (COMPV_IS_ALIGNED_NEON(dstPt0) && COMPV_IS_ALIGNED_NEON(dstPt1) && COMPV_IS_ALIGNED_NEON(dstPt2) && COMPV_IS_ALIGNED_NEON(dstPt3) && COMPV_IS_ALIGNED_NEON(srcPtr)) { // ASM requires mem to be aligned
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMemUnpack4 = CompVMemUnpack4_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMemUnpack4 = CompVMemUnpack4_Asm_NEON64);
		}
	}
#endif
	// Processing
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		CompVMemUnpack4((dstPt0 + offset), (dstPt1 + offset), (dstPt2 + offset), (dstPt3 + offset), (srcPtr + offset),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		enforceSingleThread ? SIZE_MAX : COMPV_MEM_UNPACK3_COUNT_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

// like arm neon vld3
COMPV_ERROR_CODE CompVMem::unpack3(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, const compv_uint8x3_t* srcPtr, size_t width, size_t height, size_t stride, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!dstPt0 || !dstPt1 || !dstPt2 || !srcPtr || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	void (*CompVMemUnpack3)(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, const compv_uint8x3_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVMemUnpack3_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSSE3) && COMPV_IS_ALIGNED_SSE(dstPt0) && COMPV_IS_ALIGNED_SSE(dstPt1) && COMPV_IS_ALIGNED_SSE(dstPt2) && COMPV_IS_ALIGNED_SSE(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMemUnpack3 = CompVMemUnpack3_SrcPtrNotAligned_Intrin_SSSE3);
		if (COMPV_IS_ALIGNED_SSE(srcPtr)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMemUnpack3 = CompVMemUnpack3_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMemUnpack3 = CompVMemUnpack3_Asm_X64_SSSE3);
		}
		// No need for AVX2 implementation, tried and slower
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMemUnpack3 = CompVMemUnpack3_Intrin_NEON);
		if (COMPV_IS_ALIGNED_NEON(dstPt0) && COMPV_IS_ALIGNED_NEON(dstPt1) && COMPV_IS_ALIGNED_NEON(dstPt2) && COMPV_IS_ALIGNED_NEON(srcPtr)) { // ASM requires mem to be aligned
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMemUnpack3 = CompVMemUnpack3_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMemUnpack3 = CompVMemUnpack3_Asm_NEON64);
		}
	}
#endif
	// Processing
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		CompVMemUnpack3((dstPt0 + offset), (dstPt1 + offset), (dstPt2 + offset), (srcPtr + offset),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		enforceSingleThread ? SIZE_MAX : COMPV_MEM_UNPACK3_COUNT_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::unpack2(uint8_t* dstPt0, uint8_t* dstPt1, const compv_uint8x2_t* srcPtr, size_t width, size_t height, size_t stride, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!dstPt0 || !dstPt1 || !srcPtr || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	void(*CompVMemUnpack2)(uint8_t* dstPt0, uint8_t* dstPt1, const compv_uint8x2_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVMemUnpack2_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSSE3) && COMPV_IS_ALIGNED_SSE(dstPt0) && COMPV_IS_ALIGNED_SSE(dstPt1) && COMPV_IS_ALIGNED_SSE(srcPtr) && COMPV_IS_ALIGNED_SSE(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMemUnpack2 = CompVMemUnpack2_Intrin_SSSE3);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMemUnpack2 = CompVMemUnpack2_Asm_X64_SSSE3);
		// No need for AVX2 implementation, tried and slower
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMemUnpack2 = CompVMemUnpack2_Intrin_NEON);
		if (COMPV_IS_ALIGNED_NEON(dstPt0) && COMPV_IS_ALIGNED_NEON(dstPt1) && COMPV_IS_ALIGNED_NEON(srcPtr)) { // ASM requires mem to be aligned
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMemUnpack2 = CompVMemUnpack2_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMemUnpack2 = CompVMemUnpack2_Asm_NEON64);
		}
	}
#endif
	// Processing
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		CompVMemUnpack2((dstPt0 + offset), (dstPt1 + offset), (srcPtr + offset),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		enforceSingleThread ? SIZE_MAX : COMPV_MEM_UNPACK2_COUNT_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

// like arm neon vst4
COMPV_ERROR_CODE CompVMem::pack4(compv_uint8x4_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, const uint8_t* srcPt3, size_t width, size_t height, size_t stride, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!dstPtr || !srcPt0 || !srcPt1 || !srcPt2 || !srcPt3 || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	void(*CompVMemPack4)(compv_uint8x4_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, const uint8_t* srcPt3, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVMemPack4_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(dstPtr) && COMPV_IS_ALIGNED_SSE(srcPt0) && COMPV_IS_ALIGNED_SSE(srcPt1) && COMPV_IS_ALIGNED_SSE(srcPt2) && COMPV_IS_ALIGNED_SSE(srcPt3) && COMPV_IS_ALIGNED_SSE(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMemPack4 = CompVMemPack4_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMemPack4 = CompVMemPack4_Asm_X64_SSE2);
		// No need for AVX2 implementation, tried and slower
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMemPack4 = CompVMemPack4_Intrin_NEON);
		if (COMPV_IS_ALIGNED_NEON(dstPtr) && COMPV_IS_ALIGNED_NEON(srcPt0) && COMPV_IS_ALIGNED_NEON(srcPt1) && COMPV_IS_ALIGNED_NEON(srcPt2) && COMPV_IS_ALIGNED_NEON(srcPt2)) { // ASM requires mem to be aligned
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMemPack4 = CompVMemPack4_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMemPack4 = CompVMemPack4_Asm_NEON64);
		}
	}
#endif

	// Processing
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		CompVMemPack4((dstPtr + offset), (srcPt0 + offset), (srcPt1 + offset), (srcPt2 + offset), (srcPt3 + offset),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		enforceSingleThread ? SIZE_MAX : COMPV_MEM_PACK3_COUNT_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

// like arm neon vst3
COMPV_ERROR_CODE CompVMem::pack3(compv_uint8x3_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, size_t width, size_t height, size_t stride, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!dstPtr || !srcPt0 || !srcPt1 || !srcPt2 || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	void(*CompVMemPack3)(compv_uint8x3_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVMemPack3_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSSE3) && COMPV_IS_ALIGNED_SSE(dstPtr) && COMPV_IS_ALIGNED_SSE(srcPt0) && COMPV_IS_ALIGNED_SSE(srcPt1) && COMPV_IS_ALIGNED_SSE(srcPt2) && COMPV_IS_ALIGNED_SSE(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMemPack3 = CompVMemPack3_Intrin_SSSE3);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMemPack3 = CompVMemPack3_Asm_X64_SSSE3);
		// No need for AVX2 implementation, tried and slower
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMemPack3 = CompVMemPack3_Intrin_NEON);
		if (COMPV_IS_ALIGNED_NEON(dstPtr) && COMPV_IS_ALIGNED_NEON(srcPt0) && COMPV_IS_ALIGNED_NEON(srcPt1) && COMPV_IS_ALIGNED_NEON(srcPt2)) { // ASM requires mem to be aligned
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMemPack3 = CompVMemPack3_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMemPack3 = CompVMemPack3_Asm_NEON64);
		}
	}
#endif

	// Processing
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		CompVMemPack3((dstPtr + offset), (srcPt0 + offset), (srcPt1 + offset), (srcPt2 + offset),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		enforceSingleThread ? SIZE_MAX : COMPV_MEM_PACK3_COUNT_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

// like arm neon vst2
COMPV_ERROR_CODE CompVMem::pack2(compv_uint8x2_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, size_t width, size_t height, size_t stride, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!dstPtr || !srcPt0 || !srcPt1 || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	void(*CompVMemPack2)(compv_uint8x2_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVMemPack2_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(dstPtr) && COMPV_IS_ALIGNED_SSE(srcPt0) && COMPV_IS_ALIGNED_SSE(srcPt1) && COMPV_IS_ALIGNED_SSE(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMemPack2 = CompVMemPack2_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMemPack2 = CompVMemPack2_Asm_X64_SSE2);
		// No need for AVX2 implementation, tried and slower
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMemPack2 = CompVMemPack2_Intrin_NEON);
		if (COMPV_IS_ALIGNED_NEON(dstPtr) && COMPV_IS_ALIGNED_NEON(srcPt0) && COMPV_IS_ALIGNED_NEON(srcPt1)) { // ASM requires mem to be aligned
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMemPack2 = CompVMemPack2_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMemPack2 = CompVMemPack2_Asm_NEON64);
		}
	}
#endif
	
	// Processing
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		CompVMemPack2((dstPtr + offset), (srcPt0 + offset), (srcPt1 + offset),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		enforceSingleThread ? SIZE_MAX : COMPV_MEM_PACK2_COUNT_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::set(void* dstPtr, compv_scalar_t val, compv_uscalar_t count, compv_uscalar_t sizeOfEltInBytes /*= 1*/)
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !count || !sizeOfEltInBytes, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (sizeOfEltInBytes) {
    case 4:
        if (CompVMem::MemSetDword) {
            CompVMem::MemSetDword(dstPtr, val, count);
            return COMPV_ERROR_CODE_S_OK;
        }
        break;
    case 8:
        if (CompVMem::MemSetQword) {
            CompVMem::MemSetQword(dstPtr, val, count);
            return COMPV_ERROR_CODE_S_OK;
        }
        break;
    case 16:
        if (CompVMem::MemSetDQword && COMPV_IS_ALIGNED(dstPtr, 16)) {
            CompVMem::MemSetDQword(dstPtr, val, count);
            return COMPV_ERROR_CODE_S_OK;
        }
        break;
    }
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
    COMPV_CHECK_EXP_RETURN(val > 0xff, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // memset() supports "byte" only
    memset(dstPtr, static_cast<int>(val), count*sizeOfEltInBytes);

    return COMPV_ERROR_CODE_S_OK;
}

typedef void(*CompVMemZero)(void* dstPtr, compv_uscalar_t size);

static void CompVMemZero_C(void* dstPtr, compv_uscalar_t size)
{
	if (size >= 64) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found");
	}
    memset(dstPtr, 0, (size_t)size);
}

COMPV_ERROR_CODE CompVMem::zero(void* dstPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
#if 0
    uint8_t* dstPtr_ = static_cast<uint8_t*>(dstPtr);
    if (size >= 16) {
        COMPV_CHECK_CODE_RETURN(CompVMem::set(dstPtr_, 0, size >> 4, 16));
        dstPtr_ += (size - (size & 15));
        size &= 15;
    }
    else if (size >= 8) {
        COMPV_CHECK_CODE_RETURN(CompVMem::set(dstPtr_, 0, size >> 3, 8));
        dstPtr_ += (size - (size & 7));
        size &= 7;
    }
    if (size) {
        CompVMemZero setz = CompVMemZero_C;
        setz(dstPtr_, size);
    }
#else
    CompVMemZero setz = CompVMemZero_C;
	if (size >= 64 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(dstPtr)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(setz = CompVMemZero_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(setz = CompVMemZero_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(setz = CompVMemZero_Asm_NEON64);
	}
    setz(dstPtr, size);
#endif
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMem::zeroNTA(void* dstPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!dstPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVMemZero setz = CompVMemZero_C;
    size_t align = 1;

    if (size > COMPV_MEM_CPY_SIZE_MIN_SIMD) {
        if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
            if (COMPV_IS_ALIGNED_SSE(dstPtr)) {
                COMPV_EXEC_IFDEF_INTRIN_X86((setz = CompVMemZeroNTA_Intrin_Aligned_SSE2, align = COMPV_ALIGNV_SIMD_SSE));
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
#	if COMPV_TBBMALLOC
	void *pMem = scalable_malloc(size);
#	elif COMPV_USE_DLMALLOC
	void *pMem = dlmalloc(size);
#	else
    void *pMem = ::malloc(size);
#	endif
    if (!pMem) {
		COMPV_DEBUG_FATAL_EX(COMPV_THIS_CLASSNAME, "Memory allocation failed");
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
				COMPV_DEBUG_FATAL_EX(COMPV_THIS_CLASSNAME, "Memory reallocation failed");
            }
        }
        else {
            if (!(pMem = ::calloc(size, 1))) {
				COMPV_DEBUG_FATAL_EX(COMPV_THIS_CLASSNAME, "Memory allocation (%u) failed", (unsigned)size);
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
#	if COMPV_TBBMALLOC
		pMem = scalable_calloc(num, size);
#	else
        pMem = ::calloc(num, size);
#endif
        if (!pMem) {
			COMPV_DEBUG_FATAL_EX(COMPV_THIS_CLASSNAME, "Memory allocation failed. num=%zu and size=%zu", num, size);
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
#	if COMPV_TBBMALLOC
			scalable_free(*ptr);
#	elif COMPV_USE_DLMALLOC
			dlfree(*ptr);
#	else
            ::free(*ptr);
#	endif
#endif
        }
        *ptr = nullptr;
    }
}

void* CompVMem::mallocAligned(size_t size, size_t alignment_/*= CompVMem::bestAlignment()*/)
{
    void* pMem;
	const size_t alignment = COMPV_MATH_MAX(alignment_, COMPV_MEMALIGN_MINSIZE); // For example, posix_memalign(&pMem, 1, ...) return null on Android
#if COMPV_TBBMALLOC
	pMem = scalable_aligned_malloc(size, alignment);
#elif COMPV_USE_DLMALLOC
	pMem = dlmemalign(alignment, size);
#elif COMPV_OS_WINDOWS && !COMPV_UNDER_OS_CE && !COMPV_OS_WINDOWS_RT
    pMem = _aligned_malloc(size, alignment);
#elif HAVE_POSIX_MEMALIGN || _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
    pMem = NULL;
    posix_memalign(&pMem, alignment, size); // TODO(dmi): available starting 'android-18'
#elif _ISOC11_SOURCE
    pMem = aligned_alloc(alignment, size);
#else
    COMPV_DEBUG_INFO_CODE_NOT_TESTED();
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
        CompVMem::s_Specials.insert(std::pair<uintptr_t, compv_special_mem_t>(reinterpret_cast<uintptr_t>(pMem), compv_special_mem_t(reinterpret_cast<uintptr_t>(pMem), size, alignment)));
        CompVMem::specialsUnLock();
    }
#	endif
    return pMem;
}

void* CompVMem::reallocAligned(void* ptr, size_t size, size_t alignment/*= CompVMem::bestAlignment()*/)
{
#if COMPV_MEM_CHECK
    if (ptr && !isSpecial(ptr)) {
		COMPV_DEBUG_FATAL_EX(COMPV_THIS_CLASSNAME, "Using reallocAligned on no-special address: %p", ptr);
        return NULL;
    }
#endif
    void* pMem;
#if COMPV_TBBMALLOC
	pMem = scalable_aligned_realloc(ptr, size, alignment);
#elif COMPV_USE_DLMALLOC
	pMem = dlrealloc(ptr, size);
#elif COMPV_OS_WINDOWS && !COMPV_OS_WINDOWS_CE && !COMPV_OS_WINDOWS_RT
    pMem = _aligned_realloc(ptr, size, alignment);
#else
	COMPV_ASSERT(false); // Not implemented
#endif
#	if COMPV_MEM_CHECK
	if (pMem != ptr) {
		CompVMem::specialsLock();
		CompVMem::s_Specials.erase(reinterpret_cast<uintptr_t>(ptr));
		if (pMem) {
			CompVMem::s_Specials.insert(std::pair<uintptr_t, compv_special_mem_t>(reinterpret_cast<uintptr_t>(pMem), compv_special_mem_t(reinterpret_cast<uintptr_t>(pMem), size, alignment)));
		}
		CompVMem::specialsUnLock();
	}
#	endif
    return pMem;
}

void* CompVMem::callocAligned(size_t num, size_t size, size_t alignment/*= CompVMem::bestAlignment()*/)
{
	void* pMem = nullptr;
#if COMPV_TBBMALLOC
	if (alignment <= 8) { // no alignment needed -> calloc only, no need for malloc followed by memset(0)
		pMem = scalable_calloc(num, size);
		if (!pMem) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "scalable_calloc(%zu, %zu) failed", num, size);
			return pMem;
		}
	}
#endif
	if (!pMem) {
		pMem = CompVMem::mallocAligned((size * num), alignment);
		if (pMem) {
			CompVMem::zero(pMem, (size * num));
		}
	}
#	if COMPV_MEM_CHECK
	if (pMem) {
		CompVMem::specialsLock();
		CompVMem::s_Specials.insert(std::pair<uintptr_t, compv_special_mem_t>(reinterpret_cast<uintptr_t>(pMem), compv_special_mem_t(reinterpret_cast<uintptr_t>(pMem), (size * num), alignment)));
		CompVMem::specialsUnLock();
	}
#	endif
    return pMem;
}

void CompVMem::freeAligned(void** ptr)
{
    if (ptr && *ptr) {
        void* ptr_ = *ptr;
#if COMPV_MEM_CHECK
        if (!isSpecial(ptr_)) {
			COMPV_DEBUG_FATAL_EX(COMPV_THIS_CLASSNAME, "Using freeAligned on no-special address: %p", ptr_);
        }
        else {
            CompVMem::specialsLock();
            CompVMem::s_Specials.erase(reinterpret_cast<uintptr_t>(ptr_));
            CompVMem::specialsUnLock();
        }
#endif
#if COMPV_TBBMALLOC
		scalable_aligned_free(ptr_);
#elif COMPV_USE_DLMALLOC
		dlfree(ptr_);
#elif COMPV_OS_WINDOWS && !COMPV_OS_WINDOWS_CE && !COMPV_OS_WINDOWS_RT
        _aligned_free(ptr_);
#elif HAVE_POSIX_MEMALIGN || _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 || _ISOC11_SOURCE
        ::free(ptr_);
#else
        COMPV_DEBUG_INFO_CODE_NOT_TESTED();
        ::free((((uint8_t*)ptr_) - ((uint8_t*)ptr_)[-1]));
#endif
        *ptr = nullptr;
    }
}

// alignment must be power of two
uintptr_t CompVMem::alignBackward(uintptr_t ptr, size_t alignment /*= CompVMem::bestAlignment()*/)
{
    COMPV_ASSERT(COMPV_IS_POW2(alignment));
    return (ptr & -static_cast<int>(alignment));
}

uintptr_t CompVMem::alignForward(uintptr_t ptr, size_t alignment /*= CompVMem::bestAlignment()*/)
{
    COMPV_ASSERT(COMPV_IS_POW2(alignment));
    return (ptr + (alignment - 1)) & -static_cast<int>(alignment);
}

bool CompVMem::isTbbMallocEnabled()
{
#if COMPV_TBBMALLOC
	return true;
#else
	return false;
#endif
}

int CompVMem::bestAlignment()
{
    static int _bestAlignment = 0;
    if (_bestAlignment == 0) {
        _bestAlignment = COMPV_ALIGNV_SIMD_DEFAULT;
        const int L1CacheLineSize = static_cast<int>(CompVCpu::cache1LineSize()); // probably #64 or #128
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
    return CompVMem::alignForward(static_cast<uintptr_t>(size), CompVMem::bestAlignment());
}

// https://software.intel.com/en-us/articles/getting-the-most-from-opencl-12-how-to-increase-performance-by-minimizing-buffer-copies-on-intel-processor-graphics
bool CompVMem::isGpuFriendly(const void* mem, size_t size)
{
	return COMPV_IS_ALIGNED(reinterpret_cast<uintptr_t>(mem), COMPV_ALIGNV_GPU_PAGE)
		&& COMPV_IS_ALIGNED(size, COMPV_ALIGNV_GPU_LINE);
}

// https://software.intel.com/en-us/articles/controlling-memory-consumption-with-intel-threading-building-blocks-intel-tbb-scalable
COMPV_ERROR_CODE CompVMem::setHeapLimit(const size_t sizeInBytes)
{
#if COMPV_TBBMALLOC
	const size_t physMemSize = CompVCpu::physMemSize();
	COMPV_CHECK_EXP_RETURN(!sizeInBytes || (physMemSize && sizeInBytes > physMemSize), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Value must be within ]0, RAM] range");
	COMPV_CHECK_EXP_RETURN((scalable_allocation_mode(TBBMALLOC_SET_SOFT_HEAP_LIMIT, sizeInBytes) != TBBMALLOC_OK),
		COMPV_ERROR_CODE_E_INTEL_TBB, "scalable_allocation_mode failed");
#endif
	return COMPV_ERROR_CODE_S_OK;
}

// Allocated using mallocAligned, callocAligned or reallocAligned
bool CompVMem::isSpecial(void* ptr)
{
#if COMPV_MEM_CHECK
    CompVMem::specialsLock();
    bool ret = CompVMem::s_Specials.find(reinterpret_cast<uintptr_t>(ptr)) != CompVMem::s_Specials.end();
    CompVMem::specialsUnLock();
    return ret;
#else
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Memory check disabled. Returning false for CompVMem::isSpecial() function");
    return false;
#endif
}

size_t CompVMem::specialTotalMemSize()
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
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Memory check disabled. Returning 0 for CompVMem::specialTotalMemSize() function");
    return 0;
#endif
}

size_t CompVMem::specialsCount()
{
#	if COMPV_MEM_CHECK
    CompVMem::specialsLock();
    size_t ret = CompVMem::s_Specials.size();
    CompVMem::specialsUnLock();
    return ret;
#else
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Memory check disabled. Returning 0 for CompVMem::specialsCount() function");
    return 0;
#endif
}

bool CompVMem::isEmpty()
{
    return CompVMem::specialsCount() == 0;
}

bool CompVMem::isInitialized()
{
	return s_bInitialized;
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

static void CompVMemCopy_C(void* dstPtr, const void*srcPtr, compv_uscalar_t size)
{
	if (size >= 64) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation found. On ARM consider http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13544.html");
	}
	memcpy(dstPtr, srcPtr, static_cast<size_t>(size));
}

static void CompVMemUnpack4_C(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, uint8_t* dstPt3, const compv_uint8x4_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const compv_uint8x4_t& src = srcPtr[i];
			dstPt0[i] = src[0];
			dstPt1[i] = src[1];
			dstPt2[i] = src[2];
			dstPt3[i] = src[3];
		}
		dstPt0 += stride;
		dstPt1 += stride;
		dstPt2 += stride;
		dstPt3 += stride;
		srcPtr += stride;
	}
}

static void CompVMemUnpack3_C(uint8_t* dstPt0, uint8_t* dstPt1, uint8_t* dstPt2, const compv_uint8x3_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const compv_uint8x3_t& src = srcPtr[i];
			dstPt0[i] = src[0];
			dstPt1[i] = src[1];
			dstPt2[i] = src[2];
		}
		dstPt0 += stride;
		dstPt1 += stride;
		dstPt2 += stride;
		srcPtr += stride;
	}
}

static void CompVMemUnpack2_C(uint8_t* dstPt0, uint8_t* dstPt1, const compv_uint8x2_t* srcPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const compv_uint8x2_t& src = srcPtr[i];
			dstPt0[i] = src[0];
			dstPt1[i] = src[1];
		}
		dstPt0 += stride;
		dstPt1 += stride;
		srcPtr += stride;
	}
}

static void CompVMemPack4_C(compv_uint8x4_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, const uint8_t* srcPt3, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			compv_uint8x4_t& dst = dstPtr[i];
			dst[0] = srcPt0[i];
			dst[1] = srcPt1[i];
			dst[2] = srcPt2[i];
			dst[3] = srcPt3[i];
		}
		dstPtr += stride;
		srcPt0 += stride;
		srcPt1 += stride;
		srcPt2 += stride;
		srcPt3 += stride;
	}
}

static void CompVMemPack3_C(compv_uint8x3_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, const uint8_t* srcPt2, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			compv_uint8x3_t& dst = dstPtr[i];
			dst[0] = srcPt0[i];
			dst[1] = srcPt1[i];
			dst[2] = srcPt2[i];
		}
		dstPtr += stride;
		srcPt0 += stride;
		srcPt1 += stride;
		srcPt2 += stride;
	}
}

static void CompVMemPack2_C(compv_uint8x2_t* dstPtr, const uint8_t* srcPt0, const uint8_t* srcPt1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			compv_uint8x2_t& dst = dstPtr[i];
			dst[0] = srcPt0[i];
			dst[1] = srcPt1[i];
		}
		dstPtr += stride;
		srcPt0 += stride;
		srcPt1 += stride;
	}
}

COMPV_NAMESPACE_END()
