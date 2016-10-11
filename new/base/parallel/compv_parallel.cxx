/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"


COMPV_NAMESPACE_BEGIN()

bool CompVParallel::s_bInitialized = false;
bool CompVParallel::s_bInitializing = false;
#if COMPV_PARALLEL_THREADDISP11
CompVPtr<CompVThreadDispatcher11 *> CompVParallel::s_ThreadDisp11 = NULL;
#else
CompVPtr<CompVThreadDispatcher *> CompVParallel::s_ThreadDisp = NULL;
#endif

CompVParallel::CompVParallel()
{

}

CompVParallel:: ~CompVParallel()
{

}

COMPV_ERROR_CODE CompVParallel::init(int32_t numThreads /*= -1*/)
{
	COMPV_ERROR_CODE err_;

	if (s_bInitialized || s_bInitializing) {
		return COMPV_ERROR_CODE_S_OK;
	}

	s_bInitializing = true;

	COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_S_OK);

	COMPV_DEBUG_INFO("Initializing parallel module...");

bail:
	s_bInitialized = COMPV_ERROR_CODE_IS_OK(err_);
	s_bInitializing = false;
	// cleanup if initialization failed
	if (!s_bInitialized) {
#if COMPV_PARALLEL_THREADDISP11
		s_ThreadDisp11 = NULL;
#else
		s_ThreadDisp = NULL;
#endif
	}
	else {
		// The next functions are called here because they recursively call "CompVParallel::init()"
		// We call them now because "s_bInitialized" is already set to "true" and this is the way to avoid endless loops

		// ThreadDispatcher
		// maxThreads: <= 0 means choose the best one, ==1 means disable, > 1 means enable
		if (numThreads > 1 || (numThreads <= 0 && CompVCpu::getCoresCount() > 1)) {
#if COMPV_PARALLEL_THREADDISP11
			COMPV_CHECK_CODE_BAIL(err_ = CompVThreadDispatcher11::newObj(&s_ThreadDisp11, numThreads));
#else
			COMPV_CHECK_CODE_BAIL(err_ = CompVThreadDispatcher::newObj(&s_ThreadDisp, numThreads));
#endif
		}
	}
	COMPV_DEBUG_INFO("Parallel module initialized");
	return err_;
}

COMPV_ERROR_CODE CompVParallel::deInit()
{
	s_bInitialized = false;
#if COMPV_PARALLEL_THREADDISP11
	s_ThreadDisp11 = NULL;
#else
	s_ThreadDisp = NULL;
#endif	

	// TODO(dmi): deInit other modules (not an issue because there is no memory allocation)
	CompVMem::deInit();

	return COMPV_ERROR_CODE_S_OK;
}

#if COMPV_PARALLEL_THREADDISP11
COMPV_ERROR_CODE CompVParallel::multiThreadingEnable11(CompVPtr<CompVThreadDispatcher11* > dispatcher)
{
	COMPV_CHECK_EXP_RETURN(!dispatcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	s_ThreadDisp11 = dispatcher;
	return COMPV_ERROR_CODE_S_OK;
}
#else
COMPV_ERROR_CODE CompVParallel::multiThreadingEnable(CompVPtr<CompVThreadDispatcher* > dispatcher)
{
	COMPV_CHECK_EXP_RETURN(!dispatcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	s_ThreadDisp = dispatcher;
	return COMPV_ERROR_CODE_S_OK;
}
#endif /* COMPV_PARALLEL_THREADDISP11 */


COMPV_ERROR_CODE CompVParallel::multiThreadingDisable()
{
#if COMPV_PARALLEL_THREADDISP11
	s_ThreadDisp11 = NULL;
#else
	s_ThreadDisp = NULL;
#endif
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVParallel::multiThreadingSetMaxThreads(size_t maxThreads)
{
#if COMPV_PARALLEL_THREADDISP11
	CompVPtr<CompVThreadDispatcher11 *> newThreadDisp11;
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher11::newObj(&newThreadDisp11));
	s_ThreadDisp11 = newThreadDisp11;// TODO(dmi): function not optimal, we destroy all threads and create new ones
#else
	CompVPtr<CompVThreadDispatcher *> newThreadDisp;
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::newObj(&newThreadDisp));
	s_ThreadDisp = newThreadDisp;// TODO(dmi): function not optimal, we destroy all threads and create new ones
#endif	

	return COMPV_ERROR_CODE_S_OK;
}

bool CompVParallel::isMultiThreadingEnabled()
{
#if COMPV_PARALLEL_THREADDISP11
	return !!s_ThreadDisp11;
#else
	return !!s_ThreadDisp;
#endif
}

bool CompVParallel::isInitialized()
{
	return s_bInitialized;
}

bool CompVParallel::isInitializing()
{
	return s_bInitializing;
}

COMPV_NAMESPACE_END()
