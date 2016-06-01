/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/parallel/compv_asynctask.h"
#include "compv/time/compv_time.h"
#include "compv/compv_engine.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVAsyncTask::CompVAsyncTask()
    : m_iCoreId(-1)
    , m_iTokensCount(0)
    , m_bStarted(false)
{

}

CompVAsyncTask::~CompVAsyncTask()
{
    stop(); // stop(), join(), free() "thread"
}

COMPV_ERROR_CODE CompVAsyncTask::start()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    if (m_bStarted) {
        return COMPV_ERROR_CODE_S_OK;
    }
    if (!m_SemRun) {
        COMPV_CHECK_CODE_RETURN(CompVSemaphore::newObj(&m_SemRun));
    }
    if (!m_SemExec) {
        COMPV_CHECK_CODE_RETURN(CompVSemaphore::newObj(&m_SemExec));
    }
    m_Thread = NULL; // join the thread
    m_bStarted = true; // must be here to make sure the run thread will have it equal to true
    err_ = CompVThread::newObj(&m_Thread, CompVAsyncTask::run, this);
    if (COMPV_ERROR_CODE_IS_NOK(err_)) {
        m_bStarted = false;
        COMPV_CHECK_CODE_RETURN(err_);
    }
    if (m_iCoreId >= 0) {
#if COMPV_THREAD_SET_AFFINITY
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

    return err_;
}

COMPV_ERROR_CODE CompVAsyncTask::setAffinity(compv_core_id_t coreId)
{
    if (m_Thread) {
#if COMPV_THREAD_SET_AFFINITY
        COMPV_CHECK_CODE_RETURN(m_Thread->setAffinity(coreId));
#endif
    }
    m_iCoreId = coreId;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask::tokenTake(compv_asynctoken_id_t* piToken)
{
    COMPV_CHECK_EXP_RETURN(!piToken, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(m_iTokensCount >= COMPV_ASYNCTASK_MAX_TOKEN_COUNT, COMPV_ERROR_CODE_E_OUT_OF_BOUND);

    int i;
    for (i = 0; i < COMPV_ASYNCTASK_MAX_TOKEN_COUNT; ++i) {
        if (!tokens[i].bTaken) {
            tokens[i].bTaken = true;
            *piToken = i;
            ++m_iTokensCount;
            break;
        }
    }
    return COMPV_ERROR_CODE_S_OK;

}

COMPV_ERROR_CODE CompVAsyncTask::tokenRelease(compv_asynctoken_id_t* piToken)
{
    compv_asynctoken_xt* pToken;
    COMPV_CHECK_EXP_RETURN(!piToken, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    if (COMPV_ASYNCTOKEN_ID_IS_VALID(*piToken)) {
        return COMPV_ERROR_CODE_S_OK;
    }
    pToken = &tokens[*piToken];
    if (pToken->bTaken) {
        pToken->bTaken = false;
        m_iTokensCount--;
    }
    *piToken = COMPV_ASYNCTOKEN_ID_INVALID;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask::tokenSetParam(compv_asynctoken_id_t token_id, int param_index, uintptr_t param_ptr, size_t param_size)
{
    COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(token_id) || !COMPV_ASYNCTASK_PARAM_INDEX_IS_VALID(param_index), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    compv_asynctoken_xt* pToken = &tokens[token_id];
    pToken->params[param_index].pcParamPtr = param_ptr;
    pToken->params[param_index].uParamSize = param_size;

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask::tokenSetParams(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, ...)
{
    COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(token_id) || !f_func, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    va_list ap;
    va_start(ap, f_func);
    COMPV_CHECK_CODE_BAIL(err = tokenSetParams2(token_id, f_func, &ap)); // must not exit the function: goto bail and call va_end(ap)

bail:
    va_end(ap);
    return err;
}

COMPV_ERROR_CODE CompVAsyncTask::tokenSetParams2(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, va_list* ap)
{
    COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(token_id) || !f_func || !ap, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    compv_asynctoken_xt* pToken = &tokens[token_id];
    if (pToken->bExecuting) {
        COMPV_DEBUG_ERROR("Token wit id = %llu already executing", token_id);
        return COMPV_ERROR_CODE_E_INVALID_STATE;
    }
    pToken->fFunc = f_func;
    pToken->iParamsCount = 0;

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    uintptr_t pc_param_ptr;
    while ((pc_param_ptr = va_arg(*ap, uintptr_t)) != COMPV_ASYNCTASK_PARAM_PTR_INVALID) {
        if (pToken->iParamsCount >= COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT) {
            COMPV_DEBUG_ERROR("Too many params");
            COMPV_CHECK_CODE_RETURN(err = COMPV_ERROR_CODE_E_OUT_OF_BOUND);
        }
        pToken->params[pToken->iParamsCount++].pcParamPtr = pc_param_ptr;
    }
    return err;
}

COMPV_ERROR_CODE CompVAsyncTask::tokenGetIdleTime(compv_asynctoken_id_t token_id, uint64_t* timeIdle)
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(token_id) || !timeIdle, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    compv_asynctoken_xt* pToken = &tokens[token_id];
    *timeIdle = (pToken->uTimeSchedStop - pToken->uTimeSchedStart) - (pToken->uTimeFuncExecStop - pToken->uTimeFuncExecStart);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask::execute(compv_asynctoken_id_t i_token, compv_asynctoken_f f_func, ...)
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(i_token) || !f_func, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    va_list ap;
    va_start(ap, f_func);
    COMPV_CHECK_CODE_BAIL(err = execute2(i_token, f_func, &ap)); // must not exit the function: goto bail and call va_end(ap)
bail:
    va_end(ap);
    return err;
}

COMPV_ERROR_CODE CompVAsyncTask::execute2(compv_asynctoken_id_t i_token, compv_asynctoken_f f_func, va_list* ap)
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(i_token) || !f_func || !ap, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    compv_asynctoken_xt* pToken = &tokens[i_token];
    if (pToken->bExecuting || pToken->bExecute) {
        COMPV_DEBUG_ERROR("Token with id = %llu already executing or scheduled", i_token);
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    }
    if (!pToken->bTaken) {
        // token was not taken -> use it with warning
        pToken->bTaken = true;
        ++m_iTokensCount;
    }
    pToken->fFunc = f_func;
    pToken->iParamsCount = 0;
    pToken->uTimeSchedStart = CompVTime::getNowMills();

    uintptr_t pc_param_ptr;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    while ((pc_param_ptr = va_arg(*ap, uintptr_t)) != COMPV_ASYNCTASK_PARAM_PTR_INVALID) {
        if (pToken->iParamsCount >= COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT) {
            COMPV_DEBUG_ERROR("Too many params");
            COMPV_CHECK_CODE_RETURN(err = COMPV_ERROR_CODE_E_OUT_OF_BOUND); // must not exit the function: goto bail and call va_end(ap)
        }
        pToken->params[pToken->iParamsCount++].pcParamPtr = pc_param_ptr;
    }

    pToken->bExecute = true;
    err = m_SemRun->increment();
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        pToken->bExecute = false;
        pToken->uTimeSchedStop = pToken->uTimeSchedStart;
        COMPV_CHECK_CODE_RETURN(err); // must not exit the function: goto bail and call va_end(ap)
    }

    return err;
}

COMPV_ERROR_CODE CompVAsyncTask::wait(compv_asynctoken_id_t token_id, uint64_t u_timeout /*= 86400000*//* 1 day */)
{
    COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(token_id), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    uint64_t u_end;
    compv_asynctoken_xt* pToken = &tokens[token_id];
    if (pToken->bExecuting || pToken->bExecute) { // "b_execute" means not started yet
        u_end = (CompVTime::getNowMills() + u_timeout);
        while ((pToken->bExecuting || pToken->bExecute) && u_end > CompVTime::getNowMills()) {
#if 0
            __asm PAUSE;
            //m_Thread->sleep(0);
#else
            m_SemExec->decrement();
#endif
        }
        if ((pToken->bExecuting || pToken->bExecute)) {
            COMPV_DEBUG_WARN("Async token with id = %llu timedout", token_id);
            return COMPV_ERROR_CODE_E_TIMEDOUT;
        }
        // uTimeSchedExecStop was computed in Run() bu we update it here to have more accurate value.
        // Also note that if the execute function in Run() is too fast then, we'll not reach this code because
        // "bExecuting" or/and "bExecute" will be equal to false
        pToken->uTimeSchedStop = CompVTime::getNowMills();
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask::stop()
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

compv_asynctoken_id_t CompVAsyncTask::getUniqueTokenId()
{
    static long uniqueId = 0;
    return compv_atomic_inc(&uniqueId);
}

COMPV_ERROR_CODE CompVAsyncTask::newObj(CompVPtr<CompVAsyncTask*>* asyncTask)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    COMPV_CHECK_EXP_RETURN(asyncTask == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVAsyncTask*> asyncTask_ = new CompVAsyncTask();
    COMPV_CHECK_EXP_RETURN(*asyncTask_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *asyncTask = asyncTask_;
    return COMPV_ERROR_CODE_S_OK;
}

void* COMPV_STDCALL CompVAsyncTask::run(void *pcArg)
{
    // "Self_" must not be "CompVPtr<CompVAsyncTask *>" to avoid incrementing the refCount
    // It the refCount is incremented here this means it's value will be equal to #2 after newObj() followed by start()
    // Now let's imagine you call "obj = NULL;", this will decrease the refCount to 1 but won't destroy it. This means you have to call stop() first (followed by = NULL if you want to destroy it).
    // This is why we use "CompVAsyncTask*" instead of "CompVPtr<CompVAsyncTask *>". We're sure that the object cannot be destroyed while
    // we're running the below code because the destructor() calls stop() and wait the exit
    CompVAsyncTask* Self_ = (CompVAsyncTask*)pcArg;
    compv_asynctoken_xt* pToken_;
    COMPV_ERROR_CODE err_;
    size_t size_;

    // Make sure the affinity is defined. This function is called in start() but after thread creation which means we could miss it if this function is called very fast
#if COMPV_THREAD_SET_AFFINITY
    if (Self_->m_iCoreId >= 0) {
        COMPV_CHECK_CODE_BAIL(err_ = Self_->m_Thread->setAffinity(Self_->m_iCoreId));
    }
    COMPV_DEBUG_INFO("CompVAsyncTask::run(coreId:requested=%d,set=%d, threadId:%ld, kThreadSetAffinity:true) - ENTER", Self_->m_iCoreId, CompVThread::getCoreId(), (long)CompVThread::getIdCurrent());
#else
    COMPV_DEBUG_INFO("CompVAsyncTask::run(coreId:requested=%d,set=useless, threadId:%ld, kThreadSetAffinity:false) - ENTER", Self_->m_iCoreId, (long)CompVThread::getIdCurrent());
#endif




    while (Self_->m_bStarted) {
        COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemRun->decrement());
        if (COMPV_ERROR_CODE_IS_NOK(err_) || !Self_->m_bStarted) {
            break;
        }
        for (size_ = 0; size_ < COMPV_ASYNCTASK_MAX_TOKEN_COUNT; ++size_) {
            pToken_ = &Self_->tokens[size_];
            if (pToken_->bExecute) {
                pToken_->bExecuting = true; // must be set first because "wait()" uses both "b_execute" and "b_executing"
                pToken_->uTimeFuncExecStart = CompVTime::getNowMills();
                pToken_->fFunc(pToken_->params);
                pToken_->uTimeFuncExecStop = CompVTime::getNowMills();
                pToken_->bExecute = false;
                pToken_->bExecuting = false;
                COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemExec->increment());
                pToken_->uTimeSchedStop = CompVTime::getNowMills(); // updated in wait() which means we are sure to have the highest value
            }
        }
    }

bail:
    COMPV_DEBUG_INFO("CompVAsyncTask::run(threadId:%ld) - EXIT", (long)CompVThread::getIdCurrent());
    return NULL;
}

COMPV_NAMESPACE_END()
