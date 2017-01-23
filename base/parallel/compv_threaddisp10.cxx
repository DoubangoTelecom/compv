/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_threaddisp10.h"
#if !COMPV_CPP11
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcher10::CompVThreadDispatcher10(int32_t numThreads)
    : CompVThreadDispatcher(numThreads)
	, m_ppTasks(NULL)
    , m_bValid(false)
{
    COMPV_ASSERT(numThreads > 1); // never happen, we already checked it in newObj()
    m_ppTasks = (CompVAsyncTask10PtrPtr)CompVMem::calloc(numThreads, sizeof(CompVAsyncTask10Ptr));
    if (!m_ppTasks) {
        COMPV_DEBUG_ERROR("Failed to allocate the asynctasks");
        return;
    }
    compv_core_id_t coreId = CompVCpu::validCoreId(0);
    for (int32_t i = 0; i < tasksCount(); ++i) {
        if (COMPV_ERROR_CODE_IS_NOK(CompVAsyncTask10::newObj(&m_ppTasks[i]))) {
            COMPV_DEBUG_ERROR("Failed to allocate the asynctask at index %d", i);
            return;
        }
        // Calling setAffinity is required to identify the thread even if affinity setting is disabled
        if (COMPV_ERROR_CODE_IS_NOK(m_ppTasks[i]->setAffinity(CompVCpu::validCoreId(coreId++)))) {
            COMPV_DEBUG_ERROR("Failed to set affinity %d", i);
        }
        if (COMPV_ERROR_CODE_IS_NOK(m_ppTasks[i]->start())) {
            COMPV_DEBUG_ERROR("Failed to start the asynctask at index %d", i);
            return;
        }
    }
    m_bValid = true;
}

CompVThreadDispatcher10::~CompVThreadDispatcher10()
{
    if (m_ppTasks) {
		int32_t tasks = tasksCount();
        for (int32_t i = 0; i < tasks; ++i) {
            m_ppTasks[i] = NULL;
        }
        CompVMem::free((void**)&m_ppTasks);
    }
}

COMPV_ERROR_CODE CompVThreadDispatcher10::execute(uint32_t threadIdx, compv_asynctoken_id_t tokenId, compv_asynctoken_f f_func, ...)  /*Overrides(CompVThreadDispatcher)*/
{
    CompVAsyncTask10Ptr  asyncTask = m_ppTasks[threadIdx % tasksCount()];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    va_list ap;
    va_start(ap, f_func);
    COMPV_CHECK_CODE_BAIL(err = asyncTask->execute2(tokenId, f_func, &ap)); // must not exit the function: goto bail and call va_end(ap)
bail:
    va_end(ap);
    return err;
}

COMPV_ERROR_CODE CompVThreadDispatcher10::wait(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t u_timeout /*= 86400000*//* 1 day */)  /*Overrides(CompVThreadDispatcher)*/
{
    CompVAsyncTask10Ptr asyncTask = m_ppTasks[threadIdx % tasksCount()];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_RETURN(asyncTask->wait(tokenId, u_timeout));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher10::getIdleTime(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t* timeIdle)
{
    CompVAsyncTask10Ptr asyncTask = m_ppTasks[threadIdx % tasksCount()];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    return asyncTask->tokenGetIdleTime(tokenId, timeIdle);
}

uint32_t CompVThreadDispatcher10::getThreadIdxForNextToCurrentCore()
{
    return (getThreadIdxCurrent() + 1) % tasksCount();
}

uint32_t CompVThreadDispatcher10::getThreadIdxCurrent()
{
    compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
	int32_t tasks = tasksCount();
    for (int32_t i = 0; i < tasks; ++i) {
        if (m_ppTasks[i]->getThread()->getId() == currentThreadId) {
            return (uint32_t)i;
        }
    }
    return 0;
}

bool CompVThreadDispatcher10::isMotherOfTheCurrentThread()  /*Overrides(CompVThreadDispatcher)*/
{
    compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
	int32_t tasks = tasksCount();
    for (int32_t i = 0; i < tasks; ++i) {
        if (m_ppTasks[i]->getThread()->getId() == currentThreadId) {
            return true;
        }
    }
    return false;
}

COMPV_ERROR_CODE CompVThreadDispatcher10::newObj(CompVThreadDispatcher10PtrPtr disp, int32_t numThreads)
{
	COMPV_CHECK_EXP_RETURN(!disp || numThreads <= 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    
    CompVThreadDispatcher10Ptr disp_ = new CompVThreadDispatcher10(numThreads);
	COMPV_CHECK_EXP_RETURN(!disp_ || !disp_->m_bValid, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *disp = disp_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* !COMPV_CPP11 */

