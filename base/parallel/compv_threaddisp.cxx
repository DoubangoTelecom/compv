/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_threaddisp.h"
#if !COMPV_PARALLEL_THREADDISP11
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcher::CompVThreadDispatcher(int32_t numThreads)
    : m_pTasks(NULL)
    , m_nTasksCount(numThreads)
    , m_bValid(false)
{
    COMPV_ASSERT(m_nTasksCount > 1); // never happen, we already checked it in newObj()
    m_pTasks = (CompVPtr<CompVAsyncTask *>*)CompVMem::calloc(numThreads, sizeof(CompVPtr<CompVAsyncTask *>));
    if (!m_pTasks) {
        COMPV_DEBUG_ERROR("Failed to allocate the asynctasks");
        return;
    }
    compv_core_id_t coreId = CompVCpu::validCoreId(0);
    for (int32_t i = 0; i < m_nTasksCount; ++i) {
        if (COMPV_ERROR_CODE_IS_NOK(CompVAsyncTask::newObj(&m_pTasks[i]))) {
            COMPV_DEBUG_ERROR("Failed to allocate the asynctask at index %d", i);
            return;
        }
        // Calling setAffinity is required to identify the thread even if affinity setting is disabled
        if (COMPV_ERROR_CODE_IS_NOK(m_pTasks[i]->setAffinity(CompVCpu::validCoreId(coreId++)))) {
            COMPV_DEBUG_ERROR("Failed to set affinity %d", i);
        }
        if (COMPV_ERROR_CODE_IS_NOK(m_pTasks[i]->start())) {
            COMPV_DEBUG_ERROR("Failed to start the asynctask at index %d", i);
            return;
        }
    }
    m_bValid = true;
}

CompVThreadDispatcher::~CompVThreadDispatcher()
{
    if (m_pTasks) {
        for (int32_t i = 0; i < m_nTasksCount; ++i) {
            m_pTasks[i] = NULL;
        }
        CompVMem::free((void**)&m_pTasks);
    }
}

COMPV_ERROR_CODE CompVThreadDispatcher::execute(uint32_t threadIdx, compv_asynctoken_id_t tokenId, compv_asynctoken_f f_func, ...)
{
    CompVPtr<CompVAsyncTask *> asyncTask = m_pTasks[threadIdx % m_nTasksCount];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
    va_list ap;
    va_start(ap, f_func);
    COMPV_CHECK_CODE_BAIL(err = asyncTask->execute2(tokenId, f_func, &ap)); // must not exit the function: goto bail and call va_end(ap)
bail:
    va_end(ap);
    return err;
}

COMPV_ERROR_CODE CompVThreadDispatcher::wait(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t u_timeout /*= 86400000*//* 1 day */)
{
    CompVPtr<CompVAsyncTask *> asyncTask = m_pTasks[threadIdx % m_nTasksCount];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_RETURN(asyncTask->wait(tokenId, u_timeout));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher::getIdleTime(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t* timeIdle)
{
    CompVPtr<CompVAsyncTask *> asyncTask = m_pTasks[threadIdx % m_nTasksCount];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    return asyncTask->tokenGetIdleTime(tokenId, timeIdle);
}

uint32_t CompVThreadDispatcher::getThreadIdxForNextToCurrentCore()
{
    return (getThreadIdxCurrent() + 1) % m_nTasksCount;
}

uint32_t CompVThreadDispatcher::getThreadIdxCurrent()
{
    compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
    for (int32_t i = 0; i < m_nTasksCount; ++i) {
        if (m_pTasks[i]->getThread()->getId() == currentThreadId) {
            return (uint32_t)i;
        }
    }
    return 0;
}

bool CompVThreadDispatcher::isMotherOfTheCurrentThread()
{
    compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
    for (int32_t i = 0; i < m_nTasksCount; ++i) {
        if (m_pTasks[i]->getThread()->getId() == currentThreadId) {
            return true;
        }
    }
    return false;
}

int32_t CompVThreadDispatcher::guessNumThreadsDividingAcrossY(int32_t xcount, int32_t ycount, int32_t minSamplesPerThread)
{
    int32_t divCount = 1;
    int32_t maxThreads = getThreadsCount();
    for (int32_t div = 2; div <= maxThreads; ++div) {
        divCount = div;
        if ((xcount * (ycount / divCount)) <= minSamplesPerThread) { // we started with the smallest div, which mean largest number of pixs and break the loop when we're below the threshold
            break;
        }
    }
    return divCount;
}

COMPV_ERROR_CODE CompVThreadDispatcher::newObj(CompVPtr<CompVThreadDispatcher*>* disp, int32_t numThreads /*= -1*/)
{
    COMPV_CHECK_CODE_RETURN(CompVBase::init());
    COMPV_CHECK_EXP_RETURN(disp == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    int32_t numCores = CompVCpu::coresCount();
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
    int32_t maxCores = numCores > 0 ? (numCores - 1) : 0; // To avoid overusing all cores
#else
    int32_t maxCores = numCores; // Up to the system to dispatch the work and avoid overusing all cores
#endif /* COMPV_PARALLEL_THREAD_SET_AFFINITY */

    if (numThreads <= 0) {
        numThreads = maxCores;
    }
    if (numThreads < 2) {
        COMPV_DEBUG_ERROR("Multi-threading requires at least #2 threads but you're requesting #%d", numThreads);
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
#endif /* COMPV_PARALLEL_THREAD_SET_AFFINITY */
    }
    if (numThreads > maxCores) {
        COMPV_DEBUG_WARN("You're requesting to use #%d threads but you only have #%d CPU cores, we recommend using %d instead", numThreads, numCores, maxCores);
    }
    CompVPtr<CompVThreadDispatcher*>_disp = new CompVThreadDispatcher(numThreads);
    if (!_disp || !_disp->m_bValid) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *disp = _disp;

    COMPV_DEBUG_INFO("Thread dispatcher created with #%d threads/#%d cores", numThreads, numCores);

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_PARALLEL_THREADDISP11 */