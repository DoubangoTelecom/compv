/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/parallel/compv_threaddisp11.h"
#include "compv/compv_cpu.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcher11::CompVThreadDispatcher11(int32_t numThreads)
: m_pTasks(NULL)
, m_nTasksCount(numThreads)
, m_bValid(false)
{
	COMPV_ASSERT(m_nTasksCount > 1); // never happen, we already checked it in newObj()
	m_pTasks = (CompVPtr<CompVAsyncTask11 *>*)CompVMem::calloc(numThreads, sizeof(CompVPtr<CompVAsyncTask11 *>));
	if (!m_pTasks) {
		COMPV_DEBUG_ERROR("Failed to allocate the asynctasks");
		return;
	}
	compv_core_id_t coreId = CompVCpu::getValidCoreId(0);
	for (int32_t i = 0; i < m_nTasksCount; ++i) {
		if (COMPV_ERROR_CODE_IS_NOK(CompVAsyncTask11::newObj(&m_pTasks[i]))) {
			COMPV_DEBUG_ERROR("Failed to allocate the asynctask at index %d", i);
			return;
		}
		// Calling setAffinity is required to identify the thread even if affinity setting is disabled
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

CompVThreadDispatcher11::~CompVThreadDispatcher11()
{
	if (m_pTasks) {
		for (int32_t i = 0; i < m_nTasksCount; ++i) {
			m_pTasks[i] = NULL;
		}
		CompVMem::free((void**)&m_pTasks);
	}
}

COMPV_ERROR_CODE CompVThreadDispatcher11::invoke(std::function<COMPV_ERROR_CODE()> fFunc, CompVAsyncTaskIds& taskIds)
{
	uint64_t taskId_ = (uint64_t)nextTaskIdx();
	uint64_t tokenId_ = 0;
	CompVPtr<CompVAsyncTask11 *> asyncTask = m_pTasks[taskId_];
	COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_CODE_RETURN(asyncTask->invoke(fFunc, &tokenId_));
	taskIds.push_back(CompVAsyncTaskId(taskId_, tokenId_));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher11::wait(const CompVAsyncTaskIds& taskIds, uint64_t u_timeout /*= 86400000 -> 1 day */)
{
	for (size_t i = 0; i < taskIds.size(); ++i) {
		COMPV_CHECK_CODE_RETURN(waitOne(taskIds[i]));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher11::waitOne(const CompVAsyncTaskId& taskId, uint64_t u_timeout /*= 86400000 -> 1 day */)
{
	COMPV_CHECK_EXP_RETURN(taskId.uTaskId >= m_nTasksCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(m_pTasks[taskId.uTaskId]->waitOne(taskId.uTokenId, u_timeout));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher11::waitAll(uint64_t u_timeout /*= 86400000 -> 1 day */)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // Deadlock when mt functions are chained
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // We check all tasks and tokens
	for (int32_t taskId = 0; taskId < m_nTasksCount; ++taskId) {
		COMPV_CHECK_CODE_RETURN(m_pTasks[taskId]->waitAll(u_timeout));
	}
	return COMPV_ERROR_CODE_S_OK;
}

uint32_t CompVThreadDispatcher11::getThreadIdxCurrent()
{
	compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
	for (int32_t i = 0; i < m_nTasksCount; ++i) {
		if (m_pTasks[i]->getThread()->getId() == currentThreadId) {
			return (uint32_t)i;
		}
	}
	return 0;
}

bool CompVThreadDispatcher11::isMotherOfTheCurrentThread()
{
	compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
	for (int32_t i = 0; i < m_nTasksCount; ++i) {
		if (m_pTasks[i]->getThread()->getId() == currentThreadId) {
			return true;
		}
	}
	return false;
}

int32_t CompVThreadDispatcher11::guessNumThreadsDividingAcrossY(int32_t xcount, int32_t ycount, int32_t minSamplesPerThread)
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

long CompVThreadDispatcher11::nextTaskIdx()
{
	static long taskIdx = 0;
	return compv_atomic_inc(&taskIdx) % m_nTasksCount;
}

COMPV_ERROR_CODE CompVThreadDispatcher11::newObj(CompVPtr<CompVThreadDispatcher11*>* disp, int32_t numThreads /*= -1*/)
{
	COMPV_CHECK_CODE_RETURN(CompVEngine::init());
	COMPV_CHECK_EXP_RETURN(disp == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	int32_t numCores = CompVCpu::getCoresCount();
#if COMPV_THREAD_SET_AFFINITY
	int32_t maxCores = numCores > 0 ? (numCores - 1) : 0; // To avoid overusing all cores
#else
	int32_t maxCores = numCores; // Up to the system to dispatch the work and avoid overusing all cores
#endif /* COMPV_THREAD_SET_AFFINITY */

	if (numThreads <= 0) {
		numThreads = maxCores;
	}
	if (numThreads < 2) {
		COMPV_DEBUG_ERROR("Multi-threading requires at least #2 threads but you're requesting #%d", numThreads);
#if COMPV_THREAD_SET_AFFINITY
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
#endif /* COMPV_THREAD_SET_AFFINITY */
	}
	if (numThreads > maxCores) {
		COMPV_DEBUG_WARN("You're requesting to use #%d threads but you only have #%d CPU cores, we recommend using %d instead", numThreads, numCores, maxCores);
	}
	CompVPtr<CompVThreadDispatcher11*>_disp = new CompVThreadDispatcher11(numThreads);
	if (!_disp || !_disp->m_bValid) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	*disp = _disp;

	COMPV_DEBUG_INFO("Thread dispatcher created with #%d threads/#%d cores", numThreads, numCores);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()