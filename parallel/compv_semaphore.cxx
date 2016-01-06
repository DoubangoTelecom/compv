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
#include "compv/parallel/compv_semaphore.h"
#include "compv/time/compv_time.h"
#include "compv/compv_cpu.h"
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_errno.h"
#include "compv/compv_debug.h"

/* Apple claims that they fully support POSIX semaphore but ...
*/
#if defined(COMPV_OS_APPLE) /* Mac OSX/Darwin/Iphone/Ipod Touch */
#	define COMPV_USE_NAMED_SEM	1
#else
#	define COMPV_USE_NAMED_SEM	0
#endif

#if !defined(NAME_MAX)
#   define	NAME_MAX		  255
#endif

#if COMPV_OS_WINDOWS /* Windows XP/Vista/7/CE */
#	include <windows.h>
#	define SEMAPHORE_S void
typedef HANDLE	SEMAPHORE_T;
#	if COMPV_OS_WINDOWS_RT
#		if !defined(CreateSemaphoreEx)
#			define CreateSemaphoreEx CreateSemaphoreExW
#		endif
#	endif
//#else if define(__APPLE__) /* Mac OSX/Darwin/Iphone/Ipod Touch */
//#	include <march/semaphore.h>
//#	include <march/task.h>
#else /* All *nix */

#	include <pthread.h>
#	include <semaphore.h>
#	if COMPV_USE_NAMED_SEM
#	include <fcntl.h> /* O_CREAT */
#	include <sys/stat.h> /* S_IRUSR, S_IWUSR*/

typedef struct named_sem_s
{
	sem_t* sem;
	char name[NAME_MAX + 1];
} named_sem_t;
#		define SEMAPHORE_S named_sem_t
#		define GET_SEM(PSEM) (((named_sem_t*)(PSEM))->sem)
#	else
#		define SEMAPHORE_S sem_t
#		define GET_SEM(PSEM) ((PSEM))
#	endif /* COMPV_USE_NAMED_SEM */
typedef sem_t* SEMAPHORE_T;

#endif

COMPV_NAMESPACE_BEGIN()

CompVSemaphore::CompVSemaphore(int initialVal /*= 0*/)
: m_pHandle(NULL)
{
#if COMPV_OS_WINDOWS
#	if TSK_UNDER_WINDOWS_RT
	m_pHandle = CreateSemaphoreEx(NULL, initialVal, 0x7FFFFFFF, NULL, 0x00000000, SEMAPHORE_ALL_ACCESS);
#	else
	m_pHandle = CreateSemaphore(NULL, initialVal, 0x7FFFFFFF, NULL);
#	endif
#else
	m_pHandle = CompVMem::calloc(1, sizeof(SEMAPHORE_S));
#if COMPV_USE_NAMED_SEM
	named_sem_t * nsem = (named_sem_t*)handle;
	snprintf(nsem->name, (sizeof(nsem->name) / sizeof(nsem->name[0])) - 1, "/sem/%llu/%d.", CompVTime::getEpochMillis(), rand() ^ rand());
	if ((nsem->sem = sem_open(nsem->name, O_CREAT /*| O_EXCL*/, S_IRUSR | S_IWUSR, initial_val)) == SEM_FAILED) {
#else
	if (sem_init((SEMAPHORE_T)m_pHandle, 0, initialVal)) {
#endif
		CompVMem::free(&m_pHandle);
		COMPV_DEBUG_ERROR("Failed to initialize the new semaphore (errno=%d).", errno);
	}
#endif
	if (!m_pHandle) {
		COMPV_DEBUG_ERROR("Failed to create new semaphore");
	}
}

CompVSemaphore::~CompVSemaphore()
{
	if (m_pHandle) {
#if COMPV_OS_WINDOWS
		CloseHandle((SEMAPHORE_T)m_pHandle);
		m_pHandle = NULL;
#else
#	if COMPV_USE_NAMED_SEM
		named_sem_t * nsem = ((named_sem_t*)m_pHandle);
		sem_close(nsem->sem);
#else
		sem_destroy((SEMAPHORE_T)GET_SEM(m_pHandle));
#endif /* TSK_USE_NAMED_SEM */
		CompVMem::free((void**)&m_pHandle);
#endif
	}
}

/**
* Increments a semaphore.
* @param handle The semaphore to increment.
* @retval Zero if succeed and otherwise the function returns -1 and sets errno to indicate the error.
* @sa @ref decrement.
*/
COMPV_ERROR_CODE CompVSemaphore::increment()
{
	int ret = EINVAL;
	if (!m_pHandle) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	}
#if COMPV_OS_WINDOWS
	if ((ret = ReleaseSemaphore((SEMAPHORE_T)m_pHandle, 1L, NULL) ? 0 : -1))
#else
	if ((ret = sem_post((SEMAPHORE_T)GET_SEM(m_pHandle))))
#endif
	{
		COMPV_DEBUG_ERROR("sem_post function failed: %d", ret);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM);
	}
	return COMPV_ERROR_CODE_S_OK;
}

/**
* Decrements a semaphore.
* @param handle The semaphore to decrement.
* @retval Zero if succeed and otherwise the function returns -1 and sets errno to indicate the error.
* @sa @ref increment.
*/
COMPV_ERROR_CODE CompVSemaphore::decrement()
{
	int ret = EINVAL;
	if (!m_pHandle) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	}
	
#if COMPV_OS_WINDOWS
#	if TSK_UNDER_WINDOWS_RT
	ret = (WaitForSingleObjectEx((SEMAPHORE_T)m_pHandle, INFINITE, TRUE) == WAIT_OBJECT_0) ? 0 : -1;
#	  else
	ret = (WaitForSingleObject((SEMAPHORE_T)m_pHandle, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
#endif
	if (ret)	{
		COMPV_DEBUG_ERROR("sem_wait function failed: %d", ret);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM);
	}
#else
	do {
		ret = sem_wait((SEMAPHORE_T)GET_SEM(m_pHandle));
	} while (errno == EINTR);
	if (ret) {
		COMPV_DEBUG_ERROR("sem_wait function failed: %d", errno);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM);
	}
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSemaphore::newObj(CompVObjWrapper<CompVSemaphore*>* sem, int initialVal /*= 0*/)
{
	COMPV_CHECK_CODE_RETURN(CompVEngine::init());
	COMPV_CHECK_EXP_RETURN(sem == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVObjWrapper<CompVSemaphore*> sem_ = new CompVSemaphore(initialVal);
	COMPV_CHECK_EXP_RETURN(*sem_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*sem = sem_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
