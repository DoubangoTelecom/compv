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
#include "compv/parallel/compv_asynctask.h"
#include "compv/time/compv_time.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

extern COMPV_ERROR_CODE CompVInit();

CompVAsyncTask::CompVAsyncTask()
: m_iCoreId(0)
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
	
	err_ = m_Thread->setPriority(COMPV_THREAD_PRIORITY_TIME_CRITICAL);
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		COMPV_DEBUG_ERROR("Failed to set thread priority value to %d with error code = %d", COMPV_THREAD_PRIORITY_TIME_CRITICAL, err_);
		err_ = COMPV_ERROR_CODE_S_OK; // not fatal error, once the user is alerted continue
	}
	err_ = m_Thread->setAffinity(m_iCoreId);
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		COMPV_DEBUG_ERROR("Failed to set thread affinity value to %d with error code = %d", m_iCoreId, err_);
		err_ = COMPV_ERROR_CODE_S_OK; // not fatal error, once the user is alerted continue
	}

	return err_;
}

COMPV_ERROR_CODE CompVAsyncTask::setAffinity(vcomp_core_id_t coreId)
{
	if (m_Thread) {
		COMPV_CHECK_CODE_RETURN(m_Thread->setAffinity(coreId));
	}
	m_iCoreId = coreId;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask::tokenTake(compv_asynctoken_id_t* piToken)
{
	COMPV_CHECK_EXP_RETURN(!piToken, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(m_iTokensCount >= COMPV_ASYNCTASK_MAX_TOKEN_COUNT, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	
	int32_t i;
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

COMPV_ERROR_CODE CompVAsyncTask::tokenSetParam(compv_asynctoken_id_t token_id, int32_t param_index, const void* param_ptr, size_t param_size)
{
	COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(token_id) || !COMPV_ASYNCTASK_PARAM_INDEX_IS_VALID(param_index), COMPV_ERROR_CODE_E_INVALID_PARAMETER)

	compv_asynctoken_xt* pToken = &tokens[token_id];
	pToken->params[param_index].pcParamPtr = param_ptr;
	pToken->params[param_index].uParamSize = param_size;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVAsyncTask::tokenSetParams(compv_asynctoken_id_t token_id, compv_asynctoken_f f_func, ...)
{
	COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(token_id) || !f_func, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	compv_asynctoken_xt* pToken = &tokens[token_id];
	if (pToken->bExecuting) {
		COMPV_DEBUG_ERROR("Token wit id = %d already executing", token_id);
		return COMPV_ERROR_CODE_E_INVALID_STATE;
	}
	pToken->fFunc = f_func;
	pToken->iParamsCount = 0;

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	va_list ap;
	const void* pc_param_ptr;
	va_start(ap, f_func);
	while ((pc_param_ptr = va_arg(ap, const void*)) != COMPV_ASYNCTASK_PARAM_PTR_INVALID) {
		if (pToken->iParamsCount >= COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT) {
			COMPV_DEBUG_ERROR("Too many params");
			COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_OUT_OF_BOUND); // must not exit the function: goto bail and call va_end(ap)
		}
		pToken->params[pToken->iParamsCount++].pcParamPtr = pc_param_ptr;
	}

bail:
	va_end(ap);
	return err;
}

COMPV_ERROR_CODE CompVAsyncTask::execute(compv_asynctoken_id_t i_token, compv_asynctoken_f f_func, ...)
{
	COMPV_CHECK_EXP_RETURN(!m_bStarted, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(!COMPV_ASYNCTOKEN_ID_IS_VALID(i_token) || !f_func, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	compv_asynctoken_xt* pToken = &tokens[i_token];
	if (pToken->bExecuting || pToken->bExecute) {
		COMPV_DEBUG_ERROR("Token with id = %d already executing or scheduled", i_token);
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	}
	if (!pToken->bTaken) {
		// token was not taken -> use it with warning
		pToken->bTaken = true;
		++m_iTokensCount;
	}
	pToken->fFunc = f_func;
	pToken->iParamsCount = 0;

	const void* pc_param_ptr;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	va_list ap;
	va_start(ap, f_func);
	while ((pc_param_ptr = va_arg(ap, const void*)) != COMPV_ASYNCTASK_PARAM_PTR_INVALID) {
		if (pToken->iParamsCount >= COMPV_ASYNCTASK_MAX_TOKEN_PARAMS_COUNT) {
			COMPV_DEBUG_ERROR("Too many params");
			COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_E_OUT_OF_BOUND); // must not exit the function: goto bail and call va_end(ap)
		}
		pToken->params[pToken->iParamsCount++].pcParamPtr = pc_param_ptr;
	}

	pToken->bExecute = true;
	err = m_SemRun->increment();
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		pToken->bExecute = false;
		COMPV_CHECK_CODE_BAIL(err); // must not exit the function: goto bail and call va_end(ap)
	}

bail:
	va_end(ap);
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
			// hl_thread_sleep(100000);
			// __asm PAUSE; // FIXME
			m_SemExec->decrement();
		}
		if ((pToken->bExecuting || pToken->bExecute)) {
			COMPV_DEBUG_WARN("Async token with id = %d timedout", token_id);
			return COMPV_ERROR_CODE_E_TIMEDOUT;
		}
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

COMPV_ERROR_CODE CompVAsyncTask::newObj(CompVObjWrapper<CompVAsyncTask*>* asyncTask)
{
	COMPV_CHECK_CODE_RETURN(CompVInit());
	COMPV_CHECK_EXP_RETURN(asyncTask == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVObjWrapper<CompVAsyncTask*> asyncTask_ = new CompVAsyncTask();
	COMPV_CHECK_EXP_RETURN(*asyncTask_ == NULL, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*asyncTask = asyncTask_;
	return COMPV_ERROR_CODE_S_OK;
}

void* COMPV_STDCALL CompVAsyncTask::run(void *pcArg)
{
	// "Self_" must not be "CompVObjWrapper<CompVAsyncTask *>" to avoid incrementing the refCount
	// It the refCount is incremented here this means it's value will be equal to #2 after newObj() followed by start()
	// Now let's imagine you call "obj = NULL;", this will decrease the refCount to 1 but won't destroy it. This means you have to call stop() first (followed by = NULL if you want to destroy it).
	// This is why we use "CompVAsyncTask*" instead of "CompVObjWrapper<CompVAsyncTask *>". We're sure that the object cannot be destroyed while
	// we're running the below code because the destructor() calls stop() and wait the exit
	CompVAsyncTask* Self_ = (CompVAsyncTask*)pcArg;
	compv_asynctoken_xt* pToken_;
	COMPV_ERROR_CODE err_;
	size_t size_;

	COMPV_DEBUG_INFO("CompVAsyncTask::run(coreId:requested=%d,set=%d, threadId:%d) - ENTER", Self_->m_iCoreId, CompVThread::getCoreId(), CompVThread::getIdCurrent());

	while (Self_->m_bStarted) {
		COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemRun->decrement());
		if (COMPV_ERROR_CODE_IS_NOK(err_) || !Self_->m_bStarted) {
			break;
		}
		for (size_ = 0; size_ < COMPV_ASYNCTASK_MAX_TOKEN_COUNT; ++size_) {
			pToken_ = &Self_->tokens[size_];
			if (pToken_->bExecute) {
				pToken_->bExecuting = true; // must be set first because "wait()" uses both "b_execute" and "b_executing"
				pToken_->fFunc(pToken_->params);
				pToken_->bExecute = false;
				pToken_->bExecuting = false;
				COMPV_CHECK_CODE_BAIL(err_ = Self_->m_SemExec->increment());
			}
		}
	}

bail:
	COMPV_DEBUG_INFO("CompVAsyncTask::run(threadId:%d) - EXIT", CompVThread::getIdCurrent());
	return NULL;
}

COMPV_NAMESPACE_END()
