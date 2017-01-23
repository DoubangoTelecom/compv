/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_thread.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC int32_t compv_utils_thread_get_core_id_x86_asm();
#endif

COMPV_NAMESPACE_BEGIN()

#define kModuleNameThread "Thread"

#if COMPV_OS_WINDOWS
#	include <windows.h>
#endif
#if COMPV_OS_WINDOWS_RT
#	include "../winrt/ThreadEmulation.h"
using namespace ThreadEmulation;
#endif

#if defined(HAVE_SCHED_H) || COMPV_OS_ANDROID || COMPV_OS_LINUX
#	include <sched.h>
#	include <sys/syscall.h>
#endif

#if COMPV_OS_WINDOWS
#   define THREAD_HANDLE_TYPE HANDLE
#else
#   define THREAD_HANDLE_TYPE pthread_t*
#endif

#if COMPV_OS_ANDROID && __ANDROID_API__ < 20
// https://android.googlesource.com/platform/development/+/73a5a3b/ndk/platforms/android-20/include/sched.h
#define CPU_SETSIZE 1024
#define __NCPUBITS  (8 * sizeof (unsigned long))
typedef struct{
	unsigned long __bits[CPU_SETSIZE / __NCPUBITS];
} cpu_set_t;

#define CPU_SET(cpu, cpusetp) \
  ((cpusetp)->__bits[(cpu)/__NCPUBITS] |= (1UL << ((cpu) % __NCPUBITS)))
#define CPU_ZERO(cpusetp) \
  memset((cpusetp), 0, sizeof(cpu_set_t))
#endif /* COMPV_OS_ANDROID */

#if COMPV_OS_WINDOWS
static compv_thread_id_t CompVWindowsGetThreadId(HANDLE handle)
{
	// GetThreadId: https://msdn.microsoft.com/en-us/library/windows/desktop/ms683233(v=vs.85).aspx
#if _WIN32_WINNT >= 0x0502
	return GetThreadId(handle);
#else
	if (CompVBase::isWinVistaOrLater()) {
		static HMODULE hKernel32 = NULL;
		static bool bTriedToLoadKernel32 = false;
		static DWORD(WINAPI *GetThreadIdFunc)(_In_ HANDLE Thread) = NULL;
		if (!bTriedToLoadKernel32) {
			// TODO(dmi): If another part of the code loads 'Kernel32.dll' then, make sure to reuse the HMODULE.
			COMPV_DEBUG_INFO_EX(kModuleNameThread, "Loading Kernel32.dll to extract Kernel32.dll");
			bTriedToLoadKernel32 = true;
			hKernel32 = LoadLibrary(TEXT("Kernel32.dll"));
			if (hKernel32) {
				GetThreadIdFunc = reinterpret_cast<DWORD(WINAPI *)(_In_ HANDLE Thread)>(GetProcAddress(hKernel32, "GetThreadId"));
				if (!GetThreadIdFunc) {
					COMPV_DEBUG_ERROR_EX(kModuleNameThread, "Failed to find GetThreadId from Kernel32.dll");
				}
			}
			else {
				COMPV_DEBUG_ERROR_EX(kModuleNameThread, "Failed to load Kernel32.dll");
			}
		}
		if (GetThreadIdFunc) {
			return GetThreadIdFunc(handle);
		}
	}
	
	COMPV_DEBUG_INFO_EX(kModuleNameThread, "GetThreadId not found, using dummy implementation");
	return reinterpret_cast<compv_thread_id_t>(handle); // we just want a unique Id
	
#endif
}
#endif

CompVThread::CompVThread(void *(COMPV_STDCALL *start) (void *), void *arg_ /*= NULL*/)
    : m_pHandle(NULL)
    , m_Id(0)
{
    void* arg = arg_ ? arg_ : this;
#if COMPV_OS_WINDOWS
    m_pHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start, arg, 0, NULL);
#else
    m_pHandle = CompVMem::calloc(1, sizeof(pthread_t));
    COMPV_ASSERT(pthread_create(reinterpret_cast<pthread_t*>(m_pHandle), 0, start, arg) == 0);
#endif
    COMPV_ASSERT(m_pHandle != NULL);

    if (m_pHandle) {
#if COMPV_OS_WINDOWS
        m_Id = CompVWindowsGetThreadId(reinterpret_cast<HANDLE>(m_pHandle));
#elif 0
		pthread_getunique_np(reinterpret_cast<pthread_t*>(m_pHandle), &m_Id);
#elif 1
        m_Id = *reinterpret_cast<pthread_t*>(m_pHandle);
#endif
    }
}

CompVThread::~CompVThread()
{
    join();
    COMPV_DEBUG_INFO_EX(kModuleNameThread, "***Thread with id=%p destroyed***", m_Id);
}

void CompVThread::sleep(uint64_t ms)
{
#if COMPV_OS_WINDOWS
    Sleep((DWORD)ms);
#else
    struct timespec interval;

    interval.tv_sec = (long)(ms / 1000);
    interval.tv_nsec = (long)(ms % 1000) * 1000000;
    nanosleep(&interval, 0);
#endif
}

COMPV_ERROR_CODE CompVThread::setPriority(int32_t priority)
{
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    if (!m_pHandle) {
        COMPV_DEBUG_ERROR_EX(kModuleNameThread, "Invalid parameter");
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_INVALID_STATE, "Wrapped handle is null");
    }
#if COMPV_OS_WINDOWS
    {
        if (!SetThreadPriority((HANDLE)m_pHandle, priority)) {
            err = COMPV_ERROR_CODE_E_SYSTEM;
        }
#if COMPV_OS_WINDOWS_RT
        // It's not possible to set priority on WP8 when thread is not in suspended state -> do nothing and don't bother us
        if (COMPV_ERROR_CODE_IS_NOK(err)) {
            COMPV_DEBUG_INFO_EX(kModuleNameThread, "SetThreadPriority() failed but nothing to worry about");
            err = COMPV_ERROR_CODE_S_OK;
        }
#endif
        COMPV_CHECK_CODE_BAIL(err);
    }
#else
    struct sched_param sp;
    int ret;
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority = priority;
    if ((ret = pthread_setschedparam(*((pthread_t*)m_pHandle), SCHED_OTHER, &sp))) {
        COMPV_DEBUG_ERROR_EX(kModuleNameThread, "Failed to change priority to %d with error code=%d", priority, ret);
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_PTHREAD, "pthread_setschedparam failed");
    }
#endif

bail:
    return err;
}

compv_thread_id_t CompVThread::getId() const
{
    return m_Id;
}

COMPV_ERROR_CODE CompVThread::setAffinity(compv_core_id_t coreId)
{
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    if (!m_pHandle || coreId < 0 || coreId > CompVCpu::coresCount()) {
        COMPV_DEBUG_ERROR_EX(kModuleNameThread, "Invalid parameter");
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_INVALID_PARAMETER, "CoreId parameter of of bound");
    }
#if COMPV_OS_WINDOWS
    {
        const DWORD_PTR mask = (((DWORD_PTR)1) << coreId);
        if (!SetThreadAffinityMask(reinterpret_cast<HANDLE>(m_pHandle), mask)) {
            COMPV_DEBUG_ERROR_EX(kModuleNameThread, "SetThreadAffinityMask() failed");
            COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM, "SetThreadAffinityMask failed");
        }
    }
#elif COMPV_OS_ANDROID
    {
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("__NR_sched_setaffinity accept a pid_t --retrieved using gettid() instead of getId()");
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(coreId, &cpuset);
		COMPV_CHECK_EXP_RETURN(
			syscall(__NR_sched_setaffinity, getId(), sizeof(cpuset), &cpuset) != 0,
			err = COMPV_ERROR_CODE_E_SYSTEM,
			"Failed to set thread affinity");
    }
#elif COMPV_OS_APPLE
    {
        /* /usr/include/mach/thread_policy.h: thread_policy_set and mach_thread_self() */
        // TODO(dmi): not implemented
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "setAffinity not implemented on Apple's devices");
    }
#else
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(coreId, &cpuset);
        if (pthread_setaffinity_np((pthread_t*)m_pHandle, sizeof(cpu_set_t), &cpuset) != 0) {
            COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_PTHREAD, "pthread_setaffinity_np failed");
        }
    }
#endif

bail:
    return err;
}

COMPV_ERROR_CODE CompVThread::join()
{
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    if (!m_pHandle) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_DEBUG_INFO_EX(kModuleNameThread, "Thread with id=%p will join", m_Id); // debug message to track deadlocks

#if COMPV_OS_WINDOWS
#	if COMPV_OS_WINDOWS_RT
    if (WaitForSingleObjectEx((HANDLE)m_pHandle, INFINITE, TRUE) == WAIT_FAILED) {
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM, "WaitForSingleObjectEx failed");
    }
#	else
    if (WaitForSingleObject(reinterpret_cast<HANDLE>(m_pHandle), INFINITE) == WAIT_FAILED) {
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM, "WaitForSingleObject failed");
    }
#endif
    CloseHandle(reinterpret_cast<HANDLE>(m_pHandle));
    m_pHandle = NULL;
#else
    if ((pthread_join(*(reinterpret_cast<pthread_t*>(m_pHandle)), 0)) != 0) {
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_PTHREAD, "pthread_join failed");
    }
    CompVMem::free(reinterpret_cast<void**>(&m_pHandle));
#endif

bail:
    COMPV_DEBUG_INFO("Thread with id=%p will join", m_Id);
    return err;
}

COMPV_ERROR_CODE CompVThread::setPriorityCurrent(int32_t priority)
{
    COMPV_DEBUG_ERROR_EX(kModuleNameThread, "setPriorityCurrent not implemented");
    return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

compv_thread_id_t CompVThread::getIdCurrent()
{
#if COMPV_OS_WINDOWS
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

bool CompVThread::isEquals(compv_thread_id_t id1, compv_thread_id_t id2)
{
#if COMPV_OS_WINDOWS
    return (id1 == id2);
#else
    return (pthread_equal(id1, id2) != 0);
#endif
}

compv_core_id_t CompVThread::getCoreId()
{
#if _WIN32_WINNT >= 0x0600
    return (int32_t)GetCurrentProcessorNumber();
#elif _WIN32_WINNT >= 0x0601
    PROCESSOR_NUMBER ProcNumber;
    GetCurrentProcessorNumberEx(&ProcNumber);
    return  ProcNumber.Number;
#elif COMPV_ARCH_X86 && COMPV_ASM
    return compv_utils_thread_get_core_id_x86_asm();
#elif COMPV_OS_APPLE
    COMPV_DEBUG_INFO_CODE_ONCE("sched_getcpu not supported always returning zero");
    return 0;
#elif COMPV_OS_ANDROID
    COMPV_DEBUG_INFO_CODE_ONCE("sched_getcpu not supported always returning zero");
    return 0;
#else
    int cpuId = sched_getcpu();
    if (cpuId < 0) {
        COMPV_DEBUG_ERROR_EX(kModuleNameThread, "sched_getcpu returned %d", cpuId);
        return 0;
    }
    return (compv_core_id_t)cpuId;
#endif
}


COMPV_ERROR_CODE CompVThread::newObj(CompVThreadPtrPtr thread, void *(COMPV_STDCALL *start) (void *), void *arg /*= NULL*/)
{
    COMPV_CHECK_CODE_RETURN(CompVBase::init());
    COMPV_CHECK_EXP_RETURN(thread == NULL || start == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVThreadPtr thread_ = new CompVThread(start, arg);
    COMPV_CHECK_EXP_RETURN(*thread_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *thread = thread_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
