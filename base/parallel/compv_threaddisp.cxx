/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_threaddisp.h"
#include "compv/base/parallel/compv_threaddisp_native.h"
#include "compv/base/parallel/compv_threaddisp_tbb.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_debug.h"

#define COMPV_THIS_CLASSNAME	"CompVThreadDispatcher"

COMPV_NAMESPACE_BEGIN()

CompVThreadDispatcher::CompVThreadDispatcher(int32_t numThreads)
	: m_nTasksCount(numThreads)

{
	COMPV_ASSERT(m_nTasksCount > 1); // never happen, we already checked it in newObj()
}

CompVThreadDispatcher::~CompVThreadDispatcher()
{
	
}

size_t CompVThreadDispatcher::guessNumThreadsDividingAcrossY(size_t xcount, size_t ycount, size_t maxThreads, size_t minSamplesPerThread)
{
	size_t divCount = 1;
	for (size_t div = 2; div <= maxThreads; ++div) {
		divCount = div;
		if ((xcount * (ycount / divCount)) <= minSamplesPerThread) { // we started with the smallest div, which mean largest number of pixs and break the loop when we're above the threshold
			break;
		}
	}
	return divCount;
}

COMPV_ERROR_CODE CompVThreadDispatcher::newObj(CompVThreadDispatcherPtrPtr disp, int32_t numThreads /*= -1*/)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init(), "Failed to initialize base module");
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
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Multi-threading requires at least #2 threads but you're requesting #%d", numThreads);
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
#endif /* COMPV_PARALLEL_THREAD_SET_AFFINITY */
	}
	if (numThreads > maxCores) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "You're requesting to use #%d threads but you only have #%d CPU cores, we recommend using %d instead", numThreads, numCores, maxCores);
	}

	CompVThreadDispatcherPtr _disp;

	// Create thread dispatcher using Intel TBB implementation
#if COMPV_HAVE_INTEL_TBB && 0
	if (!_disp && CompVParallel::isIntelTbbEnabled()) {
		CompVThreadDispatcherTbbPtr _dispTbb;
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcherTbb::newObj(&_dispTbb, numThreads), "Failed to create TBB thread dispatcher");
		_disp = *_dispTbb;
	}
#endif

	// Create thread dispacher using native implementation
	if (!_disp) {
#if COMPV_CPP11
		CompVThreadDispatcherNativePtr _dispNative;
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcherNative::newObj(&_dispNative, numThreads), "Failed to create native thread dispatcher");
		_disp = *_dispNative;
#else
		CompVThreadDispatcher10Ptr _disp10;
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher10::newObj(&_disp10, numThreads), "Failed to create default thread dispatcher");
		_disp = *_disp10;
#endif
	}

	COMPV_CHECK_EXP_RETURN(!_disp, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED,  "No thread dispatcher implementation found");

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Thread dispatcher created with #%d threads/#%d cores", numThreads, numCores);

	*disp = _disp;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()