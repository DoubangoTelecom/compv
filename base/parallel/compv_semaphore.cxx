/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_semaphore.h"
#include "compv/base/time/compv_time.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_errno.h"
#include "compv/base/compv_debug.h"

#include "compv/base/parallel/compv_parallel_asm_armv7.h"

/* Apple claims that they fully support POSIX semaphore but ...
*/
#if COMPV_OS_APPLE /* Mac OSX/Darwin/Iphone/Ipod Touch */
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

typedef struct named_sem_s {
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

#define COMPV_THIS_CLASSNAME "CompVSemaphore"

COMPV_NAMESPACE_BEGIN()

CompVSemaphore::CompVSemaphore()
    : m_pHandle(nullptr)
{
	/* Must not initialize data here: ctor called by childs like "CompVSemaphoreAsmARMv7" which don't need init() to be called */
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
        CompVMem::free(reinterpret_cast<void**>(&m_pHandle));
#endif
    }
}

COMPV_ERROR_CODE CompVSemaphore::init(int initialVal COMPV_DEFAULT(0))
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
	static long nsemi = 0;
	named_sem_t * nsem = (named_sem_t*)m_pHandle;
	snprintf(nsem->name, (sizeof(nsem->name) / sizeof(nsem->name[0])) - 1, "/sem/%llu/%ld.", CompVTime::epochMillis(), compv_atomic_inc(&nsemi));
	if ((nsem->sem = sem_open(nsem->name, O_CREAT /*| O_EXCL*/, S_IRUSR | S_IWUSR, initialVal)) == SEM_FAILED) {
#else
	if (sem_init((SEMAPHORE_T)m_pHandle, 0, initialVal)) {
#endif
		CompVMem::free(&m_pHandle);
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to initialize the new semaphore (errno=%d).", errno);
	}
#endif
	if (!m_pHandle) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to create new semaphore");
		return COMPV_ERROR_CODE_E_OUT_OF_MEMORY;
	}
	return COMPV_ERROR_CODE_S_OK;
}

/**
* Increments a semaphore.
* @retval Zero if succeed and otherwise the function returns -1 and sets errno to indicate the error.
* @sa Decrement: @ref decrement.
*/
COMPV_ERROR_CODE CompVSemaphore::increment()
{
    int ret = EINVAL;
#if COMPV_OS_WINDOWS
    if ((ret = ReleaseSemaphore((SEMAPHORE_T)m_pHandle, 1L, NULL) ? 0 : -1))
#else
    if ((ret = sem_post((SEMAPHORE_T)GET_SEM(m_pHandle))))
#endif
    {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sem_post function failed: %d", ret);
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM);
    }
    return COMPV_ERROR_CODE_S_OK;
}

/**
* Decrements a semaphore.
* @retval Zero if succeed and otherwise the function returns -1 and sets errno to indicate the error.
* @sa Increment: @ref increment.
*/
COMPV_ERROR_CODE CompVSemaphore::decrement()
{
    int ret = EINVAL;
#if COMPV_OS_WINDOWS
#	if TSK_UNDER_WINDOWS_RT
    ret = (WaitForSingleObjectEx((SEMAPHORE_T)m_pHandle, INFINITE, TRUE) == WAIT_OBJECT_0) ? 0 : -1;
#	  else
    ret = (WaitForSingleObject((SEMAPHORE_T)m_pHandle, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
#endif
    if (ret)	{
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sem_wait function failed: %d", ret);
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM);
    }
#else
    do {
        ret = sem_wait((SEMAPHORE_T)GET_SEM(m_pHandle));
    }
    while (ret == -1 && errno == EINTR);
    if (ret) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "sem_wait function failed: %d", errno);
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_SYSTEM, "sem_wait failed");
    }
#endif
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSemaphore::newObj(CompVSemaphorePtrPtr sem, int initialVal /*= 0*/)
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    COMPV_CHECK_EXP_RETURN(!sem, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVSemaphorePtr sem_;
#if COMPV_ASM && COMPV_ARCH_ARM32 && 0 // Must not use: pthread is faster
	if (CompVCpu::isAsmEnabled() && CompVCpu::isEnabled(kCpuFlagARM_NEON)) { // NEON introduced in ARMv7 (https://developer.arm.com/technologies/neon)
		sem_ = new CompVSemaphoreAsmARMv7();
		// do nothing if it fails -> fallback to pthread-based semaphore
	}
#endif /* COMPV_ASM && COMPV_ARCH_ARM */
	if (!sem_) {
		sem_ = new CompVSemaphore(); // create pthread-based semaphore
	}
	COMPV_CHECK_CODE_RETURN(sem_->init(initialVal));
    COMPV_CHECK_EXP_RETURN(!sem_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *sem = sem_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
