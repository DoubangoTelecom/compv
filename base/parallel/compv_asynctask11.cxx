/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_asynctask11.h"
#if COMPV_CPP11
#include "compv/base/time/compv_time.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

#define COMPV_THIS_CLASSNAME "CompVAsyncTask11"

COMPV_NAMESPACE_BEGIN()

CompVAsyncTask11::CompVAsyncTask11()
    : m_bStarted(false)
    , m_iCoreId(-1)
{

}

CompVAsyncTask11::~CompVAsyncTask11()
{
    stop(); // stop(), join(), free() "thread"
}

COMPV_ERROR_CODE CompVAsyncTask11::start()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    if (m_bStarted) {
        return COMPV_ERROR_CODE_S_OK;
    }
#if COMPV_PARALLEL_SEMA11
    if (!m_SemRun) {
        m_SemRun = std::make_shared<CompVSemaphore11>();
    }
    if (!m_SemExec) {
        m_SemExec = std::make_shared<CompVSemaphore11>();
    }
#else
    if (!m_SemRun) {
        COMPV_CHECK_CODE_RETURN(CompVSemaphore::newObj(&m_SemRun));
    }
    if (!m_SemExec) {
        COMPV_CHECK_CODE_RETURN(CompVSemaphore::newObj(&m_SemExec));
    }
#endif
    if (!m_MutexChilds) {
        COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&m_MutexChilds));
    }
    m_Thread = nullptr; // join the thread
    m_bStarted = true; // must be here to make sure the run thread will have it equal to true
    err_ = CompVThread::newObj(&m_Thread, CompVAsyncTask11::run, this);
    if (COMPV_ERROR_CODE_IS_NOK(err_)) {
        m_bStarted = false;
        COMPV_CHECK_CODE_RETURN(err_, "Failed to create async thread for the task");
    }
    if (m_iCoreId >= 0) {
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
        err_ = m_Thread->setAffinity(m_iCoreId);
        if (COMPV_ERROR_CODE_IS_NOK(err_)) {
            COMPV_DEBUG_ERROR("Failed to set thread affinity value to %d with error code = %d", m_iCoreId, err_);
            err_ = COMPV_ERROR_CODE_S_OK; // not fatal error, once the user is alerted continue
        }
#endif
    }

    err_ = m_Thread->setPriority(COMPV_THREAD_PRIORITY_TIME_CRITICAL);
    if (COMPV_ERROR_CODE_IS_NOK(err_)) {
        COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to set thread priority value to %d with error code = %d", COMPV_THREAD_PRIORITY_TIME_CRITICAL, err_);
        err_ = COMPV_ERROR_CODE_S_OK; // not fatal error, once the user is alerted continue
    }

    return err_;
}

COMPV_ERROR_CODE CompVAsyncTask11::setAffinity(compv_core_id_t coreId)
{
    if (m_Thread) {
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
        COMPV_CHECK_CODE_RETURN(m_Thread->setAffinity(coreId));
#endif
    }
    m_iCoreId = coreId;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask11::invoke(std::function<void()> fFunc, long* tokenId COMPV_DEFAULT(nullptr))
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_EXP_RETURN(!fFunc, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	long newId = CompVAsyncTask11::uniqueId();

    COMPV_CHECK_CODE_RETURN(m_MutexChilds->lock());
	COMPV_ASSERT(m_Childs.find(newId) == m_Childs.end());
	m_Childs[newId] = fFunc;
    COMPV_CHECK_CODE_RETURN(m_MutexChilds->unlock());
	
	COMPV_CHECK_CODE_RETURN(m_SemRun->increment());

	if (tokenId) {
		*tokenId = newId;
	}

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask11::waitOne(long childId, uint64_t u_timeout /* = 86400000 -> 1 day */)
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    uint64_t u_end = (CompVTime::nowMillis() + u_timeout);
	bool running = true;
    do {
		COMPV_CHECK_CODE_RETURN(m_SemExec->decrement());
		//CompVThread::sleep(0);
		COMPV_CHECK_CODE_RETURN(m_MutexChilds->lock());
		running = (m_Childs.find(childId) != m_Childs.end());
		COMPV_CHECK_CODE_RETURN(m_MutexChilds->unlock());
	} 
	while (running && u_end > CompVTime::nowMillis());

    if (running) {
        COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Async token with id = %ld timedout", childId);
        return COMPV_ERROR_CODE_E_TIMEDOUT;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask11::stop()
{
    m_bStarted = false;

    if (m_SemExec) {
        m_SemExec->increment();
    }
    if (m_SemRun) {
        m_SemRun->increment();
    }
    m_Thread = nullptr; // join()
    return COMPV_ERROR_CODE_S_OK;
}

long CompVAsyncTask11::uniqueId()
{
    static long s_UniqueId = 0;
    return compv_atomic_inc(&s_UniqueId);
}

COMPV_ERROR_CODE CompVAsyncTask11::newObj(CompVPtr<CompVAsyncTask11*>* asyncTask)
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
    COMPV_CHECK_EXP_RETURN(asyncTask == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVAsyncTask11*> asyncTask_ = new CompVAsyncTask11();
    COMPV_CHECK_EXP_RETURN(*asyncTask_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *asyncTask = asyncTask_;
    return COMPV_ERROR_CODE_S_OK;
}

void* COMPV_STDCALL CompVAsyncTask11::run(void *pcArg)
{
    // "Self_" must not be "CompVPtr<CompVAsyncTask11 *>" to avoid incrementing the refCount
    // It the refCount is incremented here this means it's value will be equal to #2 after newObj() followed by start()
    // Now let's imagine you call "obj = NULL;", this will decrease the refCount to 1 but won't destroy it. This means you have to call stop() first (followed by = NULL if you want to destroy it).
    // This is why we use "CompVAsyncTask11*" instead of "CompVPtr<CompVAsyncTask11 *>". We're sure that the object cannot be destroyed while
    // we're running the below code because the destructor() calls stop() and wait the exit
    CompVAsyncTask11* Self_ = static_cast<CompVAsyncTask11*>(pcArg);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    compv_thread_id_t threadId = CompVThread::idCurrent();

    // Make sure the affinity is defined. This function is called in start() but after thread creation which means we could miss it if this function is called very fast
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
    if (Self_->m_iCoreId >= 0) {
        COMPV_CHECK_CODE_BAIL(err_ = Self_->m_Thread->setAffinity(Self_->m_iCoreId));
    }
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(coreId:requested=%d,set=%d, threadId:%p, kThreadSetAffinity:true) - ENTER", __FUNCTION__, Self_->m_iCoreId, CompVThread::getCoreId(), (void*)threadId);
#else
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(coreId:requested=%d,set=useless, threadId:%p, kThreadSetAffinity:false) - ENTER", __FUNCTION__, Self_->m_iCoreId, (void*)threadId);
#endif

    while (Self_->m_bStarted) {
        COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemRun->decrement(), "Semaphore decrement failed");
        if (!Self_->m_bStarted) {
            break;
        }
		
		COMPV_CHECK_CODE_BAIL(err_ = Self_->m_MutexChilds->lock());
		std::function<void()> ex_fun = nullptr;
		long ex_id = 0;
		std::map<long, std::function<void()> >::iterator begin = Self_->m_Childs.begin();
		if (begin != Self_->m_Childs.end()) {
			ex_fun = begin->second;
			ex_id = begin->first;
		}
		COMPV_CHECK_CODE_BAIL(err_ = Self_->m_MutexChilds->unlock());

		if (ex_fun) {
			ex_fun();
			COMPV_CHECK_CODE_BAIL(err_ = Self_->m_MutexChilds->lock());
			Self_->m_Childs.erase(ex_id);
			COMPV_CHECK_CODE_BAIL(err_ = Self_->m_MutexChilds->unlock());
			COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemExec->increment());
		}
    }

bail:
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(threadId:%p) - EXIT", __FUNCTION__, reinterpret_cast<void*>(threadId));
    return nullptr;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_PARALLEL_THREADDISP11 */
