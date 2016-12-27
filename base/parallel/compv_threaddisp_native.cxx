/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_threaddisp_native.h"
#if COMPV_CPP11
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcherNative::CompVThreadDispatcherNative(int32_t numThreads)
	: CompVThreadDispatcher(numThreads)
	, m_pTasks(NULL)
	, m_bValid(false)
{
	COMPV_ASSERT(numThreads > 1); // never happen, we already checked it in newObj()
	m_pTasks = (CompVAsyncTask11PtrPtr)CompVMem::calloc(numThreads, sizeof(CompVAsyncTask11Ptr));
	if (!m_pTasks) {
		COMPV_DEBUG_ERROR("Failed to allocate the asynctasks");
		return;
	}
	compv_core_id_t coreId = CompVCpu::validCoreId(0);
	for (int32_t i = 0; i < threadsCount(); ++i) {
		if (COMPV_ERROR_CODE_IS_NOK(CompVAsyncTask11::newObj(&m_pTasks[i]))) {
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

CompVThreadDispatcherNative::~CompVThreadDispatcherNative()
{
	if (m_pTasks) {
		int32_t tasksCount = threadsCount();
		for (int32_t i = 0; i < tasksCount; ++i) {
			m_pTasks[i] = NULL;
		}
		CompVMem::free((void**)&m_pTasks);
	}
}

COMPV_ERROR_CODE CompVThreadDispatcherNative::invoke(std::function<COMPV_ERROR_CODE()> fFunc, CompVAsyncTask11Ids& taskIds) /*Overrides(CompVThreadDispatcher)*/
{
	uint64_t taskId_ = (uint64_t)nextTaskIdx();
	uint64_t tokenId_ = 0;
	CompVAsyncTask11Ptr  asyncTask = m_pTasks[taskId_];
	COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_CODE_RETURN(asyncTask->invoke(fFunc, &tokenId_));
	taskIds.push_back(CompVAsyncTask11Id(taskId_, tokenId_));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcherNative::wait(const CompVAsyncTask11Ids& taskIds, uint64_t u_timeout /*= 86400000 -> 1 day */) /*Overrides(CompVThreadDispatcher)*/
{
	for (size_t i = 0; i < taskIds.size(); ++i) {
		COMPV_CHECK_CODE_RETURN(waitOne(taskIds[i]));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcherNative::waitOne(const CompVAsyncTask11Id& taskId, uint64_t u_timeout /*= 86400000 -> 1 day */) /*Overrides(CompVThreadDispatcher)*/
{
	COMPV_CHECK_EXP_RETURN(taskId.uTaskId >= threadsCount(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(m_pTasks[taskId.uTaskId]->waitOne(taskId.uTokenId, u_timeout));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcherNative::waitAll(uint64_t u_timeout /*= 86400000 -> 1 day */)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Deadlock when mt functions are chained");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("We check all tasks and tokens");
	int32_t tasksCount = threadsCount();
	for (int32_t taskId = 0; taskId < tasksCount; ++taskId) {
		COMPV_CHECK_CODE_RETURN(m_pTasks[taskId]->waitAll(u_timeout));
	}
	return COMPV_ERROR_CODE_S_OK;
}

uint32_t CompVThreadDispatcherNative::threadIdxCurrent()
{
	compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
	int32_t tasksCount = threadsCount();
	for (int32_t i = 0; i < tasksCount; ++i) {
		if (m_pTasks[i]->getThread()->getId() == currentThreadId) {
			return (uint32_t)i;
		}
	}
	return 0;
}

bool CompVThreadDispatcherNative::isMotherOfTheCurrentThread() /*Overrides(CompVThreadDispatcher)*/
{
	compv_thread_id_t currentThreadId = CompVThread::getIdCurrent();
	int32_t tasksCount = threadsCount();
	for (int32_t i = 0; i < tasksCount; ++i) {
		if (m_pTasks[i]->getThread()->getId() == currentThreadId) {
			return true;
		}
	}
	return false;
}

long CompVThreadDispatcherNative::nextTaskIdx()
{
	static long taskIdx = 0;
	return compv_atomic_inc(&taskIdx) % threadsCount();
}

COMPV_ERROR_CODE CompVThreadDispatcherNative::newObj(CompVThreadDispatcherNativePtrPtr disp, int32_t numThreads)
{
	COMPV_CHECK_EXP_RETURN(!disp || numThreads <= 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVThreadDispatcherNativePtr disp_ = new CompVThreadDispatcherNative(numThreads);
	COMPV_CHECK_EXP_RETURN(!disp_ || !disp_->m_bValid, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*disp = disp_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_CPP11 */
