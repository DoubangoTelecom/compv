/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_asynctask11.h"
#if COMPV_PARALLEL_THREADDISP11
#include "compv/base/time/compv_time.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

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
#if COMPV_ASYNCTASK11_CHAIN_ENABLED
    if (!m_MutexTokens) {
        COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&m_MutexTokens));
    }
#endif
    m_Thread = NULL; // join the thread
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
        COMPV_DEBUG_ERROR("Failed to set thread priority value to %d with error code = %d", COMPV_THREAD_PRIORITY_TIME_CRITICAL, err_);
        err_ = COMPV_ERROR_CODE_S_OK; // not fatal error, once the user is alerted continue
    }

    // Init tokens
    CompVAsyncToken* token = &m_Tokens[0];
    for (unsigned i = 0; i < COMPV_ASYNCTASK11_MAX_TOKEN_COUNT; ++i) {
        token->init();
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

COMPV_ERROR_CODE CompVAsyncTask11::invoke(std::function<COMPV_ERROR_CODE()> fFunc, uint64_t *tokenId /*= NULL*/)
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_EXP_RETURN(!fFunc, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    // Init tokens
    CompVAsyncToken* tokens = &m_Tokens[0];
    CompVAsyncToken* token = NULL;
#if COMPV_ASYNCTASK11_CHAIN_ENABLED
    // We need to lock the tokens for acquisitions for conccurent call when mt functions are chained
    // Locking isn't needed for token release (done in run())
    COMPV_CHECK_CODE_RETURN(m_MutexTokens->lock());
#endif
    for (unsigned i = 0; i < COMPV_ASYNCTASK11_MAX_TOKEN_COUNT; ++i) {
        if (!tokens[i].bExecute) {
            token = &tokens[i];
            token->init();
            token->bExecute = true;
            if (tokenId) {
                *tokenId = i;
            }
            break;
        }
    }
#if COMPV_ASYNCTASK11_CHAIN_ENABLED
    COMPV_CHECK_CODE_RETURN(m_MutexTokens->unlock());
#endif

    if (!token) {
        COMPV_DEBUG_ERROR("Running out of tokens. You should increment COMPV_ASYNCTASK11_MAX_TOKEN_COUNT(%d)", COMPV_ASYNCTASK11_MAX_TOKEN_COUNT);
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_CALL);
    }
    token->fFunc = fFunc;
    COMPV_ERROR_CODE err = m_SemRun->increment();
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        token->bExecute = false;
        COMPV_CHECK_CODE_RETURN(err);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask11::waitAll(uint64_t u_timeout /* = 86400000 -> 1 day */)
{
    COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // Deadlock when mt functions are chained
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // We check all tokens
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    uint64_t u_end = (CompVTime::getNowMills() + u_timeout);
    bool empty;
    do {
        empty = true;
        for (int i = 0; i < COMPV_ASYNCTASK11_MAX_TOKEN_COUNT; ++i) {
            const CompVAsyncToken* token = &m_Tokens[i];
            if (token->bExecute && u_end > CompVTime::getNowMills()) {
                m_SemExec->decrement();
                empty &= !token->bExecute;
            }
        }
    }
    while (!empty);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask11::waitOne(uint64_t tokenId, uint64_t u_timeout /* = 86400000 -> 1 day */)
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted || tokenId >= COMPV_ASYNCTASK11_MAX_TOKEN_COUNT, COMPV_ERROR_CODE_E_INVALID_STATE);
    CompVAsyncToken* token = &m_Tokens[tokenId];
    uint64_t u_end = (CompVTime::getNowMills() + u_timeout);
    while (token->bExecute && u_end > CompVTime::getNowMills()) {
        m_SemExec->decrement();
    }
    if (token->bExecute) {
        COMPV_DEBUG_WARN("Async token with id = %llu timedout", tokenId);
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
    m_Thread = NULL; // join()
    return COMPV_ERROR_CODE_S_OK;
}

uint64_t CompVAsyncTask11::getUniqueTokenId()
{
    static long uniqueId = 0;
    return compv_atomic_inc(&uniqueId);
}

COMPV_ERROR_CODE CompVAsyncTask11::newObj(CompVPtr<CompVAsyncTask11*>* asyncTask)
{
    COMPV_CHECK_CODE_RETURN(CompVBase::init());
    COMPV_CHECK_EXP_RETURN(asyncTask == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVAsyncTask11*> asyncTask_ = new CompVAsyncTask11();
    COMPV_CHECK_EXP_RETURN(*asyncTask_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *asyncTask = asyncTask_;
    return COMPV_ERROR_CODE_S_OK;
}

void* COMPV_STDCALL CompVAsyncTask11::run(void *pcArg)
{
    // "Self_" must not be "CompVPtr<CompVAsyncTask *>" to avoid incrementing the refCount
    // It the refCount is incremented here this means it's value will be equal to #2 after newObj() followed by start()
    // Now let's imagine you call "obj = NULL;", this will decrease the refCount to 1 but won't destroy it. This means you have to call stop() first (followed by = NULL if you want to destroy it).
    // This is why we use "CompVAsyncTask*" instead of "CompVPtr<CompVAsyncTask *>". We're sure that the object cannot be destroyed while
    // we're running the below code because the destructor() calls stop() and wait the exit
    CompVAsyncTask11* Self_ = static_cast<CompVAsyncTask11*>(pcArg);
    CompVAsyncToken* pToken_;
    COMPV_ERROR_CODE err_;
    size_t size_;

    // Make sure the affinity is defined. This function is called in start() but after thread creation which means we could miss it if this function is called very fast
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
    if (Self_->m_iCoreId >= 0) {
        COMPV_CHECK_CODE_BAIL(err_ = Self_->m_Thread->setAffinity(Self_->m_iCoreId));
    }
    COMPV_DEBUG_INFO("CompVAsyncTask11::run(coreId:requested=%d,set=%d, threadId:%llu, kThreadSetAffinity:true) - ENTER", Self_->m_iCoreId, CompVThread::getCoreId(), (unsigned long)CompVThread::getIdCurrent());
#else
    COMPV_DEBUG_INFO("CompVAsyncTask11::run(coreId:requested=%d,set=useless, threadId:%lu, kThreadSetAffinity:false) - ENTER", Self_->m_iCoreId, (unsigned long)CompVThread::getIdCurrent());
#endif

    while (Self_->m_bStarted) {
        COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemRun->decrement());
        if (COMPV_ERROR_CODE_IS_NOK(err_) || !Self_->m_bStarted) {
            break;
        }
        for (size_ = 0; size_ < COMPV_ASYNCTASK11_MAX_TOKEN_COUNT; ++size_) {
            pToken_ = &Self_->m_Tokens[size_];
            if (pToken_->bExecute) {
                pToken_->fFunc();
                pToken_->bExecute = false;
                COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemExec->increment());
            }
        }
    }

bail:
    COMPV_DEBUG_INFO("CompVAsyncTask11::run(threadId:%ld) - EXIT", (long)CompVThread::getIdCurrent());
    return NULL;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_PARALLEL_THREADDISP11 */