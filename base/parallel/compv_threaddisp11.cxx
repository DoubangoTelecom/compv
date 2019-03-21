/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_threaddisp11.h"
#if COMPV_CPP11
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

#define COMPV_THIS_CLASSNAME "CompVThreadDispatcher11"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcher11::CompVThreadDispatcher11(int32_t numThreads)
	: CompVThreadDispatcher(numThreads)
	, m_ppTasks(NULL)
	, m_bValid(false)
{
	COMPV_ASSERT(numThreads > 1); // never happen, we already checked it in newObj()
	m_ppTasks = reinterpret_cast<CompVAsyncTask11PtrPtr>(CompVMem::calloc(numThreads, sizeof(CompVAsyncTask11Ptr)));
	if (!m_ppTasks) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to allocate the asynctasks");
		return;
	}
	compv_core_id_t coreId = CompVCpu::validCoreId(0);
	for (int32_t i = 0; i < threadsCount(); ++i) {
		if (COMPV_ERROR_CODE_IS_NOK(CompVAsyncTask11::newObj(&m_ppTasks[i]))) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to allocate the asynctask at index %d", i);
			return;
		}
		// Calling setAffinity is required to identify the thread even if affinity setting is disabled
		if (COMPV_ERROR_CODE_IS_NOK(m_ppTasks[i]->setAffinity(CompVCpu::validCoreId(coreId++)))) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to set affinity %d", i);
		}
		if (COMPV_ERROR_CODE_IS_NOK(m_ppTasks[i]->start())) {
			COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Failed to start the asynctask at index %d", i);
			return;
		}
	}
	m_bValid = true;
}

CompVThreadDispatcher11::~CompVThreadDispatcher11()
{
	if (m_ppTasks) {
		int32_t tasksCount = threadsCount();
		for (int32_t i = 0; i < tasksCount; ++i) {
			m_ppTasks[i] = nullptr;
		}
		CompVMem::free(reinterpret_cast<void**>(&m_ppTasks));
	}
}

COMPV_ERROR_CODE CompVThreadDispatcher11::invoke(std::function<void()> fFunc, CompVAsyncTaskIds& taskIds) /*Overrides(CompVThreadDispatcher)*/
{
	long taskId_ = nextTaskIdx();
	long childId_ = 0;
	CompVAsyncTask11Ptr  asyncTask = m_ppTasks[taskId_];
	COMPV_CHECK_EXP_RETURN(!asyncTask, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(isMotherOfTheCurrentThread(), COMPV_ERROR_CODE_E_THREAD_FORKING); // Do not allow forking from the same dispatcher
	COMPV_CHECK_CODE_RETURN(asyncTask->invoke(fFunc, &childId_));
	taskIds.push_back(std::make_pair(taskId_, childId_));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher11::wait(const CompVAsyncTaskIds& taskIds, uint64_t u_timeout /*= 86400000 -> 1 day */) /*Overrides(CompVThreadDispatcher)*/
{
	for (size_t i = 0; i < taskIds.size(); ++i) {
		COMPV_CHECK_CODE_RETURN(waitOne(taskIds[i]));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher11::waitOne(const CompVAsyncTask11Id& taskId, uint64_t u_timeout /*= 86400000 -> 1 day */) /*Overrides(CompVThreadDispatcher)*/
{
	COMPV_CHECK_EXP_RETURN(taskId.first >= threadsCount(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(m_ppTasks[taskId.first]->waitOne(taskId.second, u_timeout));
	return COMPV_ERROR_CODE_S_OK;
}

// Zero-based index, Zero if not the mother of the current one
uint32_t CompVThreadDispatcher11::threadIdxCurrent() const
{
	const compv_thread_id_t currentThreadId = CompVThread::idCurrent();
	const uint32_t tasksCount = static_cast<uint32_t>(threadsCount());
	for (uint32_t i = 0; i < tasksCount; ++i) {
		if (m_ppTasks[i]->thread()->id() == currentThreadId) {
			return i;
		}
	}
	return 0;
}

bool CompVThreadDispatcher11::isMotherOfTheCurrentThread() const
{
	const compv_thread_id_t currentThreadId = CompVThread::idCurrent();
	const int32_t tasksCount = threadsCount();
	for (int32_t i = 0; i < tasksCount; ++i) {
		if (m_ppTasks[i]->thread()->id() == currentThreadId) {
			return true;
		}
	}
	return false;
}

long CompVThreadDispatcher11::nextTaskIdx()
{
	static long taskIdx = 0;
	return compv_atomic_add(&taskIdx, 1) % threadsCount();
}

COMPV_ERROR_CODE CompVThreadDispatcher11::newObj(CompVThreadDispatcher11PtrPtr disp, int32_t numThreads)
{
	COMPV_CHECK_EXP_RETURN(!disp || numThreads <= 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVThreadDispatcher11Ptr disp_ = new CompVThreadDispatcher11(numThreads);
	COMPV_CHECK_EXP_RETURN(!disp_ || !disp_->m_bValid, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*disp = disp_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_CPP11 */
