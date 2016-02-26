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
#include "compv/parallel/compv_threaddisp.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcher::CompVThreadDispatcher(int32_t numThreads)
    : m_pTasks(NULL)
    , m_nTasksCount(numThreads)
    , m_bValid(false)
{
    COMPV_ASSERT(m_nTasksCount > 1); // never happen, we already checked it in newObj()
    m_pTasks = (CompVObjWrapper<CompVAsyncTask *>*)CompVMem::calloc(numThreads, sizeof(CompVObjWrapper<CompVAsyncTask *>));
    if (!m_pTasks) {
        COMPV_DEBUG_ERROR("Failed to allocate the asynctasks");
        return;
    }
    compv_core_id_t coreId = CompVCpu::getValidCoreId(0);
    for (int32_t i = 0; i < m_nTasksCount; ++i) {
        if (COMPV_ERROR_CODE_IS_NOK(CompVAsyncTask::newObj(&m_pTasks[i]))) {
            COMPV_DEBUG_ERROR("Failed to allocate the asynctask at index %d", i);
            return;
        }
		if (COMPV_ERROR_CODE_IS_NOK(m_pTasks[i]->setAffinity(CompVCpu::getValidCoreId(coreId++)))) {
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
    CompVObjWrapper<CompVAsyncTask *> asyncTask = m_pTasks[threadIdx % m_nTasksCount];
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
    CompVObjWrapper<CompVAsyncTask *> asyncTask = m_pTasks[threadIdx % m_nTasksCount];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_CODE_RETURN(asyncTask->wait(tokenId, u_timeout));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher::getIdleTime(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t* timeIdle)
{
    CompVObjWrapper<CompVAsyncTask *> asyncTask = m_pTasks[threadIdx % m_nTasksCount];
    COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    return asyncTask->tokenGetIdleTime(tokenId, timeIdle);
}

uint32_t CompVThreadDispatcher::getThreadIdxByCoreId(compv_core_id_t coreId)
{
    for (int32_t i = 0; i < m_nTasksCount; ++i) {
        if (m_pTasks[i]->getCoreId() == coreId) {
            return (uint32_t)i;
        }
    }
    return 0;
}

uint32_t CompVThreadDispatcher::getThreadIdxForCurrentCore()
{
    return getThreadIdxByCoreId(CompVThread::getCoreId());
}

uint32_t CompVThreadDispatcher::getThreadIdxForNextToCurrentCore()
{
    return getThreadIdxByCoreId(CompVThread::getCoreId() + 1);
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

COMPV_ERROR_CODE CompVThreadDispatcher::newObj(CompVObjWrapper<CompVThreadDispatcher*>* disp, int32_t numThreads /*= -1*/)
{
    COMPV_CHECK_CODE_RETURN(CompVEngine::init());
    COMPV_CHECK_EXP_RETURN(disp == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    int32_t numCores = CompVCpu::getCoresCount();
    if (numThreads <= 0) {
        numThreads = CompVCpu::getCoresCount() - 1;
    }
    if (numThreads < 2) {
        COMPV_DEBUG_ERROR("Multi-threading requires at least #2 threads but you're requesting #%d", numThreads);
        return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
    }
    if (numThreads >= numCores) {
        COMPV_DEBUG_WARN("You're requesting to use #%d threads but you only have #%d CPU cores, we recommend using %d instead", numThreads, numCores, (numCores - 1));
    }
    CompVObjWrapper<CompVThreadDispatcher*>_disp = new CompVThreadDispatcher(numThreads);
    if (!_disp || !_disp->m_bValid) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *disp = _disp;

    COMPV_DEBUG_INFO("Thread dispatcher created with #%d threads/#%d cores", numThreads, numCores);

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
