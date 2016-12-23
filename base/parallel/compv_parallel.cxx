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

#define COMPV_THIS_CLASSNAME	"CompVParallel"

COMPV_NAMESPACE_BEGIN()

bool CompVParallel::s_bInitialized = false;
bool CompVParallel::s_bInitializing = false;
CompVThreadDispatcherPtr CompVParallel::s_ThreadDisp = NULL;

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

    COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_S_OK, "To avoid 'bail unreference' warning");

    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing parallel module...");

bail:
    s_bInitialized = COMPV_ERROR_CODE_IS_OK(err_);
    s_bInitializing = false;
    // cleanup if initialization failed
    if (!s_bInitialized) {
        s_ThreadDisp = NULL;
    }
    else {
        // The next functions are called here because they recursively call "CompVParallel::init()"
        // We call them now because "s_bInitialized" is already set to "true" and this is the way to avoid endless loops

        // ThreadDispatcher
        // maxThreads: <= 0 means choose the best one, ==1 means disable, > 1 means enable
        if (numThreads > 1 || (numThreads <= 0 && CompVCpu::coresCount() > 1)) {
            COMPV_CHECK_CODE_BAIL(err_ = CompVThreadDispatcher::newObj(&s_ThreadDisp, numThreads), "Failed to create thread dispatcher");
        }
    }
    COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Parallel module initialized");
    return err_;
}

COMPV_ERROR_CODE CompVParallel::deInit()
{
    s_bInitialized = false;
    s_ThreadDisp = NULL;

    // TODO(dmi): deInit other modules (not an issue because there is no memory allocation)
    CompVMem::deInit();

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVParallel::multiThreadingEnable(CompVThreadDispatcherPtr dispatcher)
{
    COMPV_CHECK_EXP_RETURN(!dispatcher, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    s_ThreadDisp = dispatcher;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVParallel::multiThreadingDisable()
{
    s_ThreadDisp = NULL;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVParallel::multiThreadingSetMaxThreads(size_t maxThreads)
{
    CompVPtr<CompVThreadDispatcher *> newThreadDisp;
    COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::newObj(&newThreadDisp), "Failed to create thread dispatcher");
    s_ThreadDisp = newThreadDisp;// TODO(dmi): function not optimal, we destroy all threads and create new ones

    return COMPV_ERROR_CODE_S_OK;
}

bool CompVParallel::isMultiThreadingEnabled()
{
    return !!s_ThreadDisp;
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
