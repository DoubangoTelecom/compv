/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/parallel/compv_threaddisp.h"
#include "compv/base/parallel/compv_threaddisp10.h"
#include "compv/base/parallel/compv_threaddisp11.h"
#include "compv/base/parallel/compv_threaddisptbb.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_mem.h"
#include "compv/base/math/compv_math.h"
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

size_t CompVThreadDispatcher::guessNumThreadsDividingAcrossY(const size_t xcount, const size_t ycount, const size_t maxThreads, const size_t minSamplesPerThread)
{
#if 0
	size_t divCount = 1;
	for (size_t div = 2; div <= maxThreads; ++div) {
		if ((xcount * (ycount / divCount)) <= minSamplesPerThread) { // we started with the smallest div, which mean largest number of pixs and break the loop when we're above the threshold
			break;
		}
		divCount = div;
	}
	return divCount;
#else
	const size_t threadsCount = (xcount * ycount) / minSamplesPerThread;
	return COMPV_MATH_MAX(
		1,
		COMPV_MATH_MIN_3(threadsCount, maxThreads, ycount)
	);
#endif
}

#if COMPV_CPP11
COMPV_ERROR_CODE CompVThreadDispatcher::dispatchDividingAcrossY(std::function<COMPV_ERROR_CODE(const size_t ystart, const size_t yend)> funcPtr, const size_t xcount, const size_t ycount, const size_t minSamplesPerThread, CompVThreadDispatcherPtr threadDisp COMPV_DEFAULT(nullptr))
{
	/* Get Number of threads */
	if (!threadDisp) {
		threadDisp = CompVParallel::threadDispatcher();
	}
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCount = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(xcount, ycount, maxThreads, minSamplesPerThread)
		: 1;
	/* Dispatch tasks */
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		ycount,
		threadsCount,
		threadDisp
	));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVThreadDispatcher::dispatchDividingAcrossY(std::function<COMPV_ERROR_CODE(const size_t ystart, const size_t yend)> funcPtr, const size_t ycount, const size_t threadsCount, CompVThreadDispatcherPtr threadDisp COMPV_DEFAULT(nullptr))
{
	/* Get max number of threads */
	if (!threadDisp) {
		threadDisp = CompVParallel::threadDispatcher();
	}
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	COMPV_CHECK_EXP_RETURN(threadsCount > maxThreads || threadsCount > ycount, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	COMPV_CHECK_EXP_RETURN(threadsCount > 1 && threadDisp && threadDisp->isMotherOfTheCurrentThread(), COMPV_ERROR_CODE_E_RECURSIVE_CALL);

	/* Dispatch tasks */
	if (threadsCount > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		const size_t heights = (ycount / threadsCount);
		size_t YStart = 0, YEnd;
		for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
			YEnd = (threadIdx == (threadsCount - 1)) ? ycount : (YStart + heights);
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, YStart, YEnd), taskIds), "Dispatching task failed");
			YStart += heights;
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
	else {
		COMPV_CHECK_CODE_RETURN(funcPtr(0, ycount));
	}
	return COMPV_ERROR_CODE_S_OK;
}
#endif

COMPV_ERROR_CODE CompVThreadDispatcher::newObj(CompVThreadDispatcherPtrPtr disp, int32_t numThreads /*= -1*/)
{
	COMPV_CHECK_EXP_RETURN(!CompVBase::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);
	COMPV_CHECK_EXP_RETURN(disp == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const int32_t numCores = static_cast<int32_t>(CompVCpu::coresCount());

#if COMPV_PARALLEL_THREAD_SET_AFFINITY
	const int32_t maxCores = numCores > 0 ? (numCores - 1) : 0; // To avoid overusing all cores
#else
	const int32_t maxCores = numCores; // Up to the system to dispatch the work and avoid overusing all cores
#endif /* COMPV_PARALLEL_THREAD_SET_AFFINITY */

#if COMPV_ARCH_ARM && 0
	// On ARM no hyperthreading and our tests showed that using #2 times the number of cores provides better performances.
	const int32_t numThreadsBest = maxCores << 1;
#else
	const int32_t numThreadsBest = maxCores;
#endif

	// numThreads: <= 0 means choose the best one, ==1 means disable, > 1 means enable
	numThreads = (numThreads <= 0) ? numThreadsBest : numThreads;

	// Check if we're using all available cores
	if (numThreads < numCores) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Not optimized -> Your system have #%d cores but you're only using #%d. Sad!!", maxCores, numThreads);
	}
	
	if (numThreads < 2) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Multi-threading requires at least #2 threads but you're requesting #%d", numThreads);
#if COMPV_PARALLEL_THREAD_SET_AFFINITY
		return COMPV_ERROR_CODE_E_INVALID_PARAMETER;
#endif /* COMPV_PARALLEL_THREAD_SET_AFFINITY */
	}
	if (numThreads > numThreadsBest) {
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "You're requesting to use #%d threads but you only have #%d CPU cores, we recommend using %d instead", numThreads, numCores, numThreadsBest);
	}

	CompVThreadDispatcherPtr _disp;

	// Create thread dispatcher using Intel TBB implementation
#if COMPV_HAVE_INTEL_TBB && 0 // our native implementation is faster and always available (no external dependencies)
	if (!_disp && CompVParallel::isIntelTbbEnabled()) {
		CompVThreadDispatcherTbbPtr _dispTbb;
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcherTbb::newObj(&_dispTbb, numThreads), "Failed to create TBB thread dispatcher");
		_disp = *_dispTbb;
	}
#endif

	// Create thread dispacher using native implementation
	if (!_disp) {
#if COMPV_CPP11
		CompVThreadDispatcher11Ptr _dispNative;
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher11::newObj(&_dispNative, numThreads), "Failed to create native thread dispatcher");
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