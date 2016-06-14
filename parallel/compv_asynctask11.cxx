/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/parallel/compv_asynctask11.h"
#include "compv/time/compv_time.h"
#include "compv/compv_engine.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVAsyncTask11::CompVAsyncTask11()
: m_iCoreId(-1)
, m_bStarted(false)
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
	if (!m_SemRun) {
		COMPV_CHECK_CODE_RETURN(CompVSemaphore::newObj(&m_SemRun));
	}
	if (!m_SemExec) {
		COMPV_CHECK_CODE_RETURN(CompVSemaphore::newObj(&m_SemExec));
	}
	m_Thread = NULL; // join the thread
	m_bStarted = true; // must be here to make sure the run thread will have it equal to true
	err_ = CompVThread::newObj(&m_Thread, CompVAsyncTask11::run, this);
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

COMPV_ERROR_CODE CompVAsyncTask11::setAffinity(compv_core_id_t coreId)
{
	if (m_Thread) {
#if COMPV_THREAD_SET_AFFINITY
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
	
	CompVAsyncToken newToken(fFunc);
	if (tokenId) {
		*tokenId = newToken.uId;
	}
	m_Tokens.insert(std::pair<uint64_t, CompVAsyncToken>(newToken.uId, newToken));
	COMPV_ERROR_CODE err = m_SemRun->increment();
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		m_Tokens.erase(newToken.uId);
		COMPV_CHECK_CODE_RETURN(err); // must not exit the function: goto bail and call va_end(ap)
	}
#if 0
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
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask11::waitAll(uint64_t u_timeout /* = 86400000 -> 1 day */)
{
	COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
	std::map<uint64_t, CompVAsyncToken>::iterator it;
	uint64_t u_end = (CompVTime::getNowMills() + u_timeout);
	while ((it = m_Tokens.begin()) != m_Tokens.end() && u_end > CompVTime::getNowMills()) {
		m_SemExec->decrement();
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask11::waitOne(uint64_t tokenId, uint64_t u_timeout /* = 86400000 -> 1 day */)
{
	COMPV_DEBUG_INFO_CODE_NOT_TESTED();
	std::map<uint64_t, CompVAsyncToken>::iterator it;
	uint64_t u_end = (CompVTime::getNowMills() + u_timeout);
	while ((it = m_Tokens.find(tokenId)) != m_Tokens.end() && u_end > CompVTime::getNowMills()) {
		m_SemExec->decrement();
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
	COMPV_CHECK_CODE_RETURN(CompVEngine::init());
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
	compv_asynctoken_xt* pToken_;
	COMPV_ERROR_CODE err_;
	size_t size_;
	std::map<uint64_t, CompVAsyncToken>::iterator it;

	(pToken_);
	(size_);

	// Make sure the affinity is defined. This function is called in start() but after thread creation which means we could miss it if this function is called very fast
#if COMPV_THREAD_SET_AFFINITY
	if (Self_->m_iCoreId >= 0) {
		COMPV_CHECK_CODE_BAIL(err_ = Self_->m_Thread->setAffinity(Self_->m_iCoreId));
	}
	COMPV_DEBUG_INFO("CompVAsyncTask11::run(coreId:requested=%d,set=%d, threadId:%ld, kThreadSetAffinity:true) - ENTER", Self_->m_iCoreId, CompVThread::getCoreId(), (long)CompVThread::getIdCurrent());
#else
	COMPV_DEBUG_INFO("CompVAsyncTask11::run(coreId:requested=%d,set=useless, threadId:%ld, kThreadSetAffinity:false) - ENTER", Self_->m_iCoreId, (long)CompVThread::getIdCurrent());
#endif

	while (Self_->m_bStarted) {
		COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemRun->decrement());
		if (COMPV_ERROR_CODE_IS_NOK(err_) || !Self_->m_bStarted) {
			break;
		}
		while ((it = Self_->m_Tokens.begin()) != Self_->m_Tokens.end()) {
			COMPV_CHECK_CODE_BAIL(err_ = it->second.fFunc());
			Self_->m_Tokens.erase(it);
			COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemExec->increment());
		}
#if 0
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
#endif
	}

bail:
	COMPV_DEBUG_INFO("CompVAsyncTask11::run(threadId:%ld) - EXIT", (long)CompVThread::getIdCurrent());
	return NULL;
}

COMPV_NAMESPACE_END()
