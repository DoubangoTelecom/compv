/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_condvar.h"
#include "compv/base/time/compv_time.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_errno.h"
#include "compv/base/compv_debug.h"

#include <time.h>

#if COMPV_OS_WINDOWS
#	include <windows.h>
#	define CONDWAIT_S void
typedef HANDLE	CONDWAIT_T;
#	define TIMED_OUT	WAIT_TIMEOUT
#else
#	include <sys/time.h>
#	include <pthread.h>
#	define CONDWAIT_S pthread_cond_t
typedef CONDWAIT_S* CONDWAIT_T;
#	define TIMED_OUT	ETIMEDOUT
#endif

COMPV_NAMESPACE_BEGIN()

typedef struct condvar_s {
    CONDWAIT_T pcond; /**< Pthread handle pointing to the internal condwait object. */
#if !defined(COMPV_OS_WINDOWS)
    CompVPtr<CompVMutex*> mutex;  /**< Locker. */
#endif
}
condvar_t;

CompVCondvar::CompVCondvar()
    : m_pHandle(NULL)
{
    condvar_t *condvar = (condvar_t*)CompVMem::calloc(1, sizeof(condvar_t));

    if (condvar) {
#if COMPV_OS_WINDOWS
#	if COMV_OS_WINDOWS_RT
        condvar->pcond = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
#else
        condvar->pcond = CreateEvent(NULL, TRUE, FALSE, NULL);
#	endif
        if (!condvar->pcond) {
            COMPV_DEBUG_ERROR("CreateEvent failed");
            CompVMem::free((void**)&condvar);
        }
#else
        condvar->pcond = (CONDWAIT_T)CompVMem::calloc(1, sizeof(CONDWAIT_S));
        if (pthread_cond_init(condvar->pcond, 0)) {
            COMPV_DEBUG_ERROR("Failed to initialize the new conwait.");
        }
        COMPV_CHECK_CODE_ASSERT(CompVMutex::newObj(&condvar->mutex));

        if (!condvar->mutex) {
            pthread_cond_destroy(condvar->pcond);

            CompVMem::free((void**)&condvar);
            COMPV_DEBUG_ERROR("Failed to initialize the internal mutex.");
        }
#endif
    }

    if (!condvar)	{
        COMPV_DEBUG_ERROR("Failed to create new conwait.");
        COMPV_ASSERT(false);
    }
    m_pHandle = condvar;
}

CompVCondvar::~CompVCondvar()
{
    if (m_pHandle) {
        condvar_t *condwait = (condvar_t*)m_pHandle;
#if COMPV_OS_WINDOWS
        CloseHandle(condwait->pcond);
#else
        condwait->mutex = NULL;
        pthread_cond_destroy(condwait->pcond);
        CompVMem::free((void**)&condwait->pcond);
#endif
        CompVMem::free((void**)&condwait);
    }
}

/**
* Block the current thread until the condition is opened or until @a ms milliseconds have passed.
* @param millis The number of milliseconds to wait for a given condition. Default value is zero which means forever.
* @retval Zero if succeed and non-zero error code otherwise.
* @sa Wait: @ref tsk_condwait_wait.
*/
COMPV_ERROR_CODE CompVCondvar::wait(uint64_t millis /*= 0*/)
{
    return millis == 0 ? waitWithoutTimeout() : waitWithTimeout(millis);
}

/*
* Wakes up at least one thread that is currently waiting.
* @param handle CondWait handle created using @ref tsk_condwait_create.
* @retval Zero if succeed and non-zero otherwise.
* @sa @ref tsk_condwait_broadcast.
*/
COMPV_ERROR_CODE CompVCondvar::signal()
{
    int ret = EINVAL;
    condvar_t *condwait = (condvar_t*)m_pHandle;
    COMPV_CHECK_EXP_RETURN(m_pHandle == NULL, COMPV_ERROR_CODE_E_INVALID_STATE);

#if COMPV_OS_WINDOWS
    if (ret = ((SetEvent(condwait->pcond) && ResetEvent(condwait->pcond)) ? 0 : -1)) {
        ret = GetLastError();
        COMPV_DEBUG_ERROR("SetEvent/ResetEvent function failed: %d", ret);
    }
#else
    if (condwait && condwait->mutex) {
        condwait->mutex->lock();

        if ((ret = pthread_cond_signal(condwait->pcond))) {
            COMPV_DEBUG_ERROR("pthread_cond_signal function failed: %d", ret);
        }
        condwait->mutex->unlock();
    }
#endif
    COMPV_CHECK_EXP_RETURN(ret != 0, COMPV_ERROR_CODE_E_SYSTEM);
    return COMPV_ERROR_CODE_S_OK;
}

/**
* Wakes up all threads that are currently waiting for the condition.
* @retval Zero if succeed and non-zero otherwise.
* @sa Signal: @ref tsk_condwait_signal.
*/
COMPV_ERROR_CODE CompVCondvar::broadcast()
{
    int ret = EINVAL;
    condvar_t *condwait = (condvar_t*)m_pHandle;
    COMPV_CHECK_EXP_RETURN(m_pHandle == NULL, COMPV_ERROR_CODE_E_INVALID_STATE);

#if COMPV_OS_WINDOWS
    if (ret = ((SetEvent(condwait->pcond) && ResetEvent(condwait->pcond)) ? 0 : -1)) {
        ret = GetLastError();
        COMPV_DEBUG_ERROR("SetEvent function failed: %d", ret);
    }
#else
    if (condwait && condwait->mutex) {
        condwait->mutex->lock();
        if ((ret = pthread_cond_broadcast(condwait->pcond))) {
            COMPV_DEBUG_ERROR("pthread_cond_broadcast function failed: %d", ret);
        }
        condwait->mutex->unlock();
    }
#endif

    COMPV_CHECK_EXP_RETURN(ret != 0, COMPV_ERROR_CODE_E_SYSTEM);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCondvar::waitWithoutTimeout()
{
    int ret = EINVAL;
    condvar_t *condwait = (condvar_t*)m_pHandle;
    COMPV_CHECK_EXP_RETURN(m_pHandle == NULL, COMPV_ERROR_CODE_E_INVALID_STATE);

#if COMPV_OS_WINDOWS
#	if TSK_UNDER_WINDOWS_RT
    if ((ret = (WaitForSingleObjectEx(condwait->pcond, INFINITE, TRUE) == WAIT_FAILED) ? -1 : 0)) {
#	else
    if ((ret = (WaitForSingleObject(condwait->pcond, INFINITE) == WAIT_FAILED) ? -1 : 0)) {
#endif
        COMPV_DEBUG_ERROR("WaitForSingleObject function failed: %d", ret);
    }
#else
    if (condwait && condwait->mutex) {
        condwait->mutex->lock();
        if ((ret = pthread_cond_wait(condwait->pcond, (pthread_mutex_t*)condwait->mutex->handle()))) {
            COMPV_DEBUG_ERROR("pthread_cond_wait function failed: %d", ret);
        }
        condwait->mutex->unlock();
    }
#endif
    COMPV_CHECK_EXP_RETURN(ret != 0, COMPV_ERROR_CODE_E_SYSTEM);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCondvar::waitWithTimeout(uint64_t millis)
{
#if COMPV_OS_WINDOWS
    DWORD ret = WAIT_FAILED;
#else
    int ret = EINVAL;
#endif
    condvar_t *condwait = (condvar_t*)m_pHandle;
    COMPV_CHECK_EXP_RETURN(m_pHandle == NULL, COMPV_ERROR_CODE_E_INVALID_STATE);

#if COMPV_OS_WINDOWS
#	   if COMPV_OS_WINDOWS_RT
    if ((ret = WaitForSingleObjectEx(condwait->pcond, (DWORD)millis, TRUE)) != WAIT_OBJECT_0) {
#	   else
    if ((ret = WaitForSingleObject(condwait->pcond, (DWORD)millis)) != WAIT_OBJECT_0) {
#endif
        if (ret == TIMED_OUT) {
            /* COMPV_DEBUG_INFO("WaitForSingleObject function timedout: %d", ret); */
        }
        else {
            COMPV_DEBUG_ERROR("WaitForSingleObject function failed: %d", ret);
        }
        ret = ((ret == TIMED_OUT) ? 0 : ret);
    }
#else
    if (condwait && condwait->mutex) {
        struct timespec   ts = { 0, 0 };
        struct timeval    tv = { 0, 0 };
        /*int rc =*/  CompVTime::timeOfDay(&tv, 0);

        ts.tv_sec = (tv.tv_sec + ((long)millis / 1000));
        ts.tv_nsec += ((tv.tv_usec * 1000) + ((long)millis % 1000 * 1000000));
        if (ts.tv_nsec > 999999999) {
            ts.tv_sec += 1, ts.tv_nsec = ts.tv_nsec % 1000000000;
        }

        condwait->mutex->lock();
        if ((ret = pthread_cond_timedwait(condwait->pcond, (pthread_mutex_t*)condwait->mutex->handle(), &ts))) {
            if (ret == TIMED_OUT) {
                /* COMPV_DEBUG_INFO("pthread_cond_timedwait function timedout: %d", ret); */
            }
            else {
                COMPV_DEBUG_ERROR("pthread_cond_timedwait function failed: %d", ret);
            }
        }

        condwait->mutex->unlock();

        ret = ((ret == TIMED_OUT) ? 0 : ret);
    }
#endif

    COMPV_CHECK_EXP_RETURN(ret != 0, COMPV_ERROR_CODE_E_SYSTEM);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCondvar::newObj(CompVPtr<CompVCondvar*>* condvar)
{
    COMPV_CHECK_CODE_RETURN(CompVBase::init());
    COMPV_CHECK_EXP_RETURN(condvar == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVCondvar*> condvar_ = new CompVCondvar();
    COMPV_CHECK_EXP_RETURN(*condvar_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *condvar = condvar_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
