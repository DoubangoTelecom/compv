/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/parallel/compv_thread.h"
#include "compv/compv_cpu.h"
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

#if COMPV_ARCH_X86 && COMPV_ASM
extern "C" int32_t compv_utils_thread_get_core_id_x86_asm();
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

CompVThread::CompVThread(void *(COMPV_STDCALL *start) (void *), void *arg_ /*= NULL*/)
    : m_pHandle(NULL)
    , m_Id(0)
{
    void* arg = arg_ ? arg_ : this;
#if COMPV_OS_WINDOWS
    m_pHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start, arg, 0, NULL);
#else
    m_pHandle = CompVMem::calloc(1, sizeof(pthread_t));
    COMPV_ASSERT(pthread_create((pthread_t*)m_pHandle, 0, start, arg) == 0);
#endif
    COMPV_ASSERT(m_pHandle != NULL);

    if (m_pHandle) {
#if COMPV_OS_WINDOWS
        m_Id = GetThreadId((HANDLE)m_pHandle);
#else
        pthread_t* self = ((pthread_t*)m_pHandle);
        m_Id = *self;
#endif
    }
}

CompVThread::~CompVThread()
{
    join();
    COMPV_DEBUG_INFO("***Thread with id=%ld destroyed***", (long)m_Id);
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
        COMPV_DEBUG_ERROR("Invalid parameter");
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    }
#if COMPV_OS_WINDOWS
    {
        if (!SetThreadPriority((HANDLE)m_pHandle, priority)) {
            err = COMPV_ERROR_CODE_E_SYSTEM;
        }
#if COMPV_OS_WINDOWS_RT
        // It's not possible to set priority on WP8 when thread is not in suspended state -> do nothing and don't bother us
        if (COMPV_ERROR_CODE_IS_NOK(err)) {
            COMPV_DEBUG_INFO("SetThreadPriority() failed but nothing to worry about");
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
        COMPV_DEBUG_ERROR("Failed to change priority to %d with error code=%d", priority, ret);
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM);
    }
#endif

bail:
    return err;
}

compv_thread_id_t CompVThread::getId()
{
    return m_Id;
}

COMPV_ERROR_CODE CompVThread::setAffinity(compv_core_id_t coreId)
{
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    if (!m_pHandle || coreId < 0 || coreId > CompVCpu::getCoresCount()) {
        COMPV_DEBUG_ERROR("Invalid parameter");
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    }
#if COMPV_OS_WINDOWS
    {
        const DWORD_PTR mask = (((DWORD_PTR)1) << coreId);
        if (!SetThreadAffinityMask((HANDLE)m_pHandle, mask)) {
            COMPV_DEBUG_ERROR("SetThreadAffinityMask() failed");
            COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM);
        }
    }
#elif COMPV_OS_ANDROID
    {
        // syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
        // TODO(dmi): not implemented
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
    }
#elif COMPV_OS_APPLE
    {
        /* /usr/include/mach/thread_policy.h: thread_policy_set and mach_thread_self() */
        // TODO(dmi): not implemented
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
    }
#else
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(coreId, &cpuset);
        if (pthread_setaffinity_np((pthread_t*)m_pHandle, sizeof(cpu_set_t), &cpuset) != 0) {
            COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM);
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

    COMPV_DEBUG_INFO("Thread with id=%ld will join", (long)m_Id); // debug message to track deadlocks

#if COMPV_OS_WINDOWS
#	if COMPV_OS_WINDOWS_RT
    if (WaitForSingleObjectEx((HANDLE)m_pHandle, INFINITE, TRUE) == WAIT_FAILED) {
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM);
    }
#	else
    if (WaitForSingleObject((HANDLE)m_pHandle, INFINITE) == WAIT_FAILED) {
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM);
    }
#endif
    CloseHandle((HANDLE)m_pHandle);
    m_pHandle = NULL;
#else
    if ((pthread_join(*((pthread_t*)m_pHandle), 0)) != 0) {
        COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_SYSTEM);
    }
    CompVMem::free((void**)&m_pHandle);
#endif

bail:
    COMPV_DEBUG_INFO("Thread with id=%ld joined", (long)m_Id);
    return err;
}

COMPV_ERROR_CODE CompVThread::setPriorityCurrent(int32_t priority)
{
    COMPV_DEBUG_ERROR("Not implemented");
    COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
    return COMPV_ERROR_CODE_S_OK;
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
    COMPV_DEBUG_ERROR_EX(kModuleNameThread, "sched_getcpu not supported always returning zero");
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


COMPV_ERROR_CODE CompVThread::newObj(CompVObjWrapper<CompVThread*>* thread, void *(COMPV_STDCALL *start) (void *), void *arg /*= NULL*/)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    COMPV_CHECK_EXP_RETURN(thread == NULL || start == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVThread*> thread_ = new CompVThread(start, arg);
    COMPV_CHECK_EXP_RETURN(*thread_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *thread = thread_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
