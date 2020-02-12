/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_threaddisptbb.h"
#if COMPV_CPP11 && COMPV_HAVE_INTEL_TBB
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcherTbb::CompVThreadDispatcherTbb(int32_t numThreads)
	: CompVThreadDispatcher(numThreads)
{
	COMPV_ASSERT(numThreads > 1); // never happen, we already checked it in newObj()
}

COMPV_ERROR_CODE CompVThreadDispatcherTbb::invoke(std::function<void()> fFunc, CompVAsyncTaskIds& taskIds) /*Overrides(CompVThreadDispatcher)*/
{
	m_Group.run([fFunc]() {
		fFunc();
	});
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcherTbb::wait(const CompVAsyncTaskIds& taskIds, uint64_t u_timeout /*= 86400000 -> 1 day */) /*Overrides(CompVThreadDispatcher)*/
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("We're waiting for all tasks");
	m_Group.wait();
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcherTbb::waitOne(const CompVAsyncTask11Id& taskId, uint64_t u_timeout /*= 86400000 -> 1 day */) /*Overrides(CompVThreadDispatcher)*/
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("We're waiting for all tasks");
	m_Group.wait();
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcherTbb::waitAll(uint64_t u_timeout /*= 86400000 -> 1 day */)
{
	m_Group.wait();
	return COMPV_ERROR_CODE_S_OK;
}

bool CompVThreadDispatcherTbb::isMotherOfTheCurrentThread() /*Overrides(CompVThreadDispatcher)*/
{
	return false;
}

COMPV_ERROR_CODE CompVThreadDispatcherTbb::newObj(CompVThreadDispatcherTbbPtrPtr disp, int32_t numThreads)
{
	COMPV_CHECK_EXP_RETURN(!disp || numThreads <= 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVThreadDispatcherTbbPtr disp_ = new CompVThreadDispatcherTbb(numThreads);
	COMPV_CHECK_EXP_RETURN(!disp_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*disp = disp_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* COMPV_CPP11 */
