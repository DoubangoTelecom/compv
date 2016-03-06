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
#include "compv/parallel/compv_mutex.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_errno.h"
#include "compv/compv_debug.h"

#if COMPV_OS_WINDOWS
#	include <windows.h>
typedef HANDLE	MUTEX_T;
#	define MUTEX_S void
#	define COMPV_ERROR_NOT_OWNER ERROR_NOT_OWNER
#else
#	include <pthread.h>
#	define MUTEX_S pthread_mutex_t
typedef MUTEX_S* MUTEX_T;
#	define COMPV_ERROR_NOT_OWNER EPERM
#   if !defined(COMPV_RECURSIVE_MUTEXATTR)
#       if defined(PTHREAD_MUTEX_RECURSIVE)
#           define COMPV_RECURSIVE_MUTEXATTR PTHREAD_MUTEX_RECURSIVE
#       else
#           define COMPV_RECURSIVE_MUTEXATTR PTHREAD_MUTEX_RECURSIVE_NP
#       endif
#   endif /* COMPV_RECURSIVE_MUTEXATTR */
#endif


COMPV_NAMESPACE_BEGIN()

CompVMutex::CompVMutex(bool recursive /*= true*/)
    : m_pHandle(NULL)
{
#if COMPV_OS_WINDOWS
#	if COMPV_OS_WINDOWS_RT
    m_pHandle = CreateMutexEx(NULL, NULL, 0x00000000, MUTEX_ALL_ACCESS);
#	else
    m_pHandle = CreateMutex(NULL, FALSE, NULL);
#	endif
#else
    int ret;
    pthread_mutexattr_t   mta;

    if ((ret = pthread_mutexattr_init(&mta))) {
        COMPV_DEBUG_ERROR("pthread_mutexattr_init failed with error code %d", ret);
    }
    if (recursive && (ret = pthread_mutexattr_settype(&mta, COMPV_RECURSIVE_MUTEXATTR))) {
        COMPV_DEBUG_ERROR("pthread_mutexattr_settype failed with error code %d", ret);
        pthread_mutexattr_destroy(&mta);
    }

    /* if we are here: all is ok */
    m_pHandle = CompVMem::calloc(1, sizeof(MUTEX_S));
    if (pthread_mutex_init((MUTEX_T)m_pHandle, &mta)) {
        CompVMem::free((void**)m_pHandle);
    }
    pthread_mutexattr_destroy(&mta);
#endif

    if (!m_pHandle) {
        COMPV_DEBUG_ERROR("Failed to create new mutex.");
        COMPV_ASSERT(false);
    }
}

CompVMutex::~CompVMutex()
{
    if (m_pHandle) {
#if COMPV_OS_WINDOWS
        CloseHandle((MUTEX_T)m_pHandle);
        m_pHandle = NULL;
#else
        pthread_mutex_destroy((MUTEX_T)m_pHandle);
        CompVMem::free((void**)m_pHandle);
#endif
    }
}

/**
* Lock a mutex. You must use @ref unlock() to unlock the mutex.
* @param handle The handle of the mutex to lock.
* @retval Zero if succeed and non-zero error code otherwise.
* @sa @ref unlock().
*/
COMPV_ERROR_CODE CompVMutex::lock()
{
    int ret = EINVAL;
    COMPV_CHECK_EXP_RETURN(m_pHandle == NULL, COMPV_ERROR_CODE_E_INVALID_STATE);

#if COMPV_OS_WINDOWS
#	if COMPV_OS_WINDOWS_RT
    if ((ret = WaitForSingleObjectEx((MUTEX_T)m_pHandle, INFINITE, TRUE)) == WAIT_FAILED)
#	else
    if ((ret = WaitForSingleObject((MUTEX_T)m_pHandle, INFINITE)) == WAIT_FAILED)
#endif
#else
    if ((ret = pthread_mutex_lock((MUTEX_T)m_pHandle)))
#endif
    {
        COMPV_DEBUG_ERROR("Failed to lock the mutex: %d", ret);
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM);
    }
    return COMPV_ERROR_CODE_S_OK;
}


/**
* Unlock a mutex previously locked using @ref lock().
* @param handle The handle of the mutex to unlock.
* @retval Zero if succeed and non-zero otherwise.
* @sa @ref lock().
*/
COMPV_ERROR_CODE CompVMutex::unlock()
{
    int ret = EINVAL;
    COMPV_CHECK_EXP_RETURN(m_pHandle == NULL, COMPV_ERROR_CODE_E_INVALID_STATE);

#if COMPV_OS_WINDOWS
    if ((ret = ReleaseMutex((MUTEX_T)m_pHandle) ? 0 : -1)) {
        ret = GetLastError();
#else
    if ((ret = pthread_mutex_unlock((MUTEX_T)m_pHandle))) {
#endif
        if (ret == COMPV_ERROR_NOT_OWNER) {
            COMPV_DEBUG_WARN("The calling thread does not own the mutex: %d", ret);
        }
        else {
            COMPV_DEBUG_ERROR("Failed to unlock the mutex: %d", ret);
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM);
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMutex::newObj(CompVObjWrapper<CompVMutex*>* mutex, bool recursive /*= true*/)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    COMPV_CHECK_EXP_RETURN(mutex == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVMutex*> mutex_ = new CompVMutex(recursive);
    COMPV_CHECK_EXP_RETURN(*mutex_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *mutex = mutex_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()